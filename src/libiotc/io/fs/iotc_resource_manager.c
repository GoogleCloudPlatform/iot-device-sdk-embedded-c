/* Copyright 2018-2019 Google LLC
 *
 * This is part of the Google Cloud IoT Device SDK for Embedded C.
 * It is licensed under the BSD 3-Clause license; you may not use this file
 * except in compliance with the License.
 *
 * You may obtain a copy of the License at:
 *  https://opensource.org/licenses/BSD-3-Clause
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "iotc_resource_manager.h"
#include "iotc_allocator.h"
#include "iotc_coroutine.h"
#include "iotc_event_dispatcher_api.h"
#include "iotc_globals.h"
#include "iotc_internals.h"

/* counter for unique filedescriptors required before calling open on resource
 */
static volatile iotc_fd_t iotc_resource_manager_counter = -1;

static iotc_state_t iotc_resource_manager_get_callback_state(
    iotc_event_handle_t* event_handle) {
  switch (event_handle->handle_type) {
    case IOTC_EVENT_HANDLE_ARGC3:
      return event_handle->handlers.h3.a3;
    case IOTC_EVENT_HANDLE_ARGC4:
      return event_handle->handlers.h4.a3;
    default:
      return IOTC_INVALID_PARAMETER;
  }
}

static iotc_state_t iotc_resource_manager_set_callback_state(
    iotc_event_handle_t* event_handle, iotc_state_t state) {
  /* get already set state */
  const iotc_state_t callback_state =
      iotc_resource_manager_get_callback_state(event_handle);

  /* calculate the real ret_state */
  const iotc_state_t ret_state =
      (IOTC_STATE_OK == callback_state) ? state : callback_state;

  switch (event_handle->handle_type) {
    case IOTC_EVENT_HANDLE_ARGC3:
      event_handle->handlers.h3.a3 = ret_state;
      break;
    case IOTC_EVENT_HANDLE_ARGC4:
      event_handle->handlers.h4.a3 = ret_state;
      break;
    default:
      return IOTC_INVALID_PARAMETER;
  }

  return IOTC_STATE_OK;
}

/* to remove code repetition */
#define IOTC_RESOURCE_MANAGER_YIELD_FROM(cs, evtd_instance, resource_handle, \
                                         event_type, until_state, callback,  \
                                         state, post_invocation, f, ...)     \
  {                                                                          \
    int8_t res = iotc_evtd_register_file_fd(evtd_instance, event_type,       \
                                            resource_handle, callback);      \
                                                                             \
    IOTC_CHECK_CND(0 == res, IOTC_OUT_OF_MEMORY, state);                     \
  }                                                                          \
                                                                             \
  for (;;) {                                                                 \
    assert(NULL != f);                                                       \
    state = f(__VA_ARGS__);                                                  \
                                                                             \
    post_invocation;                                                         \
                                                                             \
    IOTC_CR_YIELD_UNTIL(cs, state == until_state, until_state);              \
                                                                             \
    iotc_evtd_unregister_file_fd(evtd_instance, resource_handle);            \
                                                                             \
    break;                                                                   \
  }                                                                          \
                                                                             \
  IOTC_CHECK_STATE(state)

/* to remove code repetition */
#define IOTC_RESOURCE_MANGER_INVOKE_CALLBACK()                              \
  IOTC_CHECK_STATE(                                                         \
      iotc_resource_manager_set_callback_state(&ctx->callback, ret_state)); \
                                                                            \
  /* register callback invocation */                                        \
  IOTC_CHECK_MEMORY(                                                        \
      iotc_evtd_execute(iotc_globals.evtd_instance, ctx->callback),         \
      ret_state);                                                           \
                                                                            \
  /* dispose the callback handle */                                         \
  iotc_dispose_handle(&ctx->callback)

/* helper function that decrease the counter and keeps it < 0 */
static iotc_fd_t iotc_resource_manager_get_invalid_unique_fd(void) {
  iotc_fd_t ret = iotc_resource_manager_counter;

  iotc_resource_manager_counter = iotc_resource_manager_counter - 1;

  if (iotc_resource_manager_counter >= 0) {
    iotc_resource_manager_counter = -1;
  }

  return ret;
}

iotc_state_t iotc_resource_manager_make_context(
    iotc_data_desc_t* data_buffer,
    iotc_resource_manager_context_t** const context) {
  if (NULL == context) {
    return IOTC_INVALID_PARAMETER;
  }

  if (NULL != *context) {
    return IOTC_INVALID_PARAMETER;
  }

  iotc_state_t state = IOTC_STATE_OK;

  IOTC_ALLOC_AT(iotc_resource_manager_context_t, *context, state);

  if (NULL == data_buffer) {
    (*context)->data_buffer = NULL; /* read suppose to allocate memory cause it
                                       does the stat operation */
    (*context)->memory_type = IOTC_MEMORY_TYPE_MANAGED;
  } else {
    (*context)->data_buffer = data_buffer;
    (*context)->memory_type = IOTC_MEMORY_TYPE_UNMANAGED;
  }

  (*context)->resource_handle = iotc_resource_manager_get_invalid_unique_fd();

  return state;

err_handling:
  IOTC_SAFE_FREE(*context);
  return state;
}

iotc_state_t iotc_resource_manager_free_context(
    iotc_resource_manager_context_t** const context) {
  if (NULL != context && NULL != *context) {
    /* PRE-CONDITION */
    assert(IOTC_MEMORY_TYPE_UNKNOWN != (*context)->memory_type);

    if (IOTC_MEMORY_TYPE_MANAGED == (*context)->memory_type) {
      iotc_free_desc(&(*context)->data_buffer);
    }

    IOTC_SAFE_FREE((*context));
  }

  return IOTC_STATE_OK;
}

static iotc_state_t iotc_resource_manager_open_coroutine(void* context,
                                                         void* res_type,
                                                         iotc_state_t state,
                                                         void* res_name) {
  /* PRE-CONDITIONS */
  assert(NULL != context);

  if (NULL == res_name) {
    return IOTC_INVALID_PARAMETER;
  }

  /* local variables */
  iotc_state_t ret_state = IOTC_STATE_OK;
  iotc_fs_resource_handle_t resource_handle = iotc_fs_init_resource_handle();
  iotc_event_handle_t local_handle =
      iotc_make_handle(&iotc_resource_manager_open_coroutine, context, res_type,
                       state, res_name);

  /* local variables <- parameters */
  iotc_resource_manager_context_t* const ctx = context;
  const iotc_fs_resource_type_t resource_type =
      (iotc_fs_resource_type_t)(intptr_t)res_type;
  const char* const resource_name = (char*)res_name;

  IOTC_CR_START(ctx->cs);

  /* yield from stat_resource */
  IOTC_RESOURCE_MANAGER_YIELD_FROM(
      ctx->cs, iotc_globals.evtd_instance, ctx->resource_handle,
      IOTC_EVENT_WANT_READ, IOTC_STATE_WANT_READ, local_handle, ret_state,
      {/* empty block of code */}, iotc_internals.fs_functions.stat_resource,
      NULL, resource_type, resource_name, &ctx->resource_stat);

  /* yield from open_resource */
  IOTC_RESOURCE_MANAGER_YIELD_FROM(
      ctx->cs, iotc_globals.evtd_instance, ctx->resource_handle,
      IOTC_EVENT_WANT_READ, IOTC_STATE_WANT_READ, local_handle, ret_state,
      {/* empty block of code */}, iotc_internals.fs_functions.open_resource,
      NULL, resource_type, resource_name, ctx->open_flags, &resource_handle);

  /* remember the valid resource handle */
  ctx->resource_handle = resource_handle;

  /* invoke the callback */
  IOTC_RESOURCE_MANGER_INVOKE_CALLBACK();

  /* end of coroutine */
  IOTC_CR_EXIT(ctx->cs, ret_state);
  IOTC_CR_END();

err_handling:;
  IOTC_CR_RESET(ctx->cs);
  iotc_event_handle_t event_handle = ctx->callback;
  iotc_dispose_handle(&ctx->callback);
  iotc_resource_manager_set_callback_state(&event_handle, ret_state);
  iotc_evtd_execute_handle(&event_handle);

  return ret_state;
}

iotc_state_t iotc_resource_manager_read_coroutine(void* context) {
  /* PRE-CONDITIONS */
  assert(NULL != context);

  /* local variables */
  iotc_state_t ret_state = IOTC_STATE_OK;
  iotc_event_handle_t local_handle =
      iotc_make_handle(&iotc_resource_manager_read_coroutine, context);
  const uint8_t* buffer = NULL;
  size_t buffer_size = 0;

  /* local variables <- parameters */
  iotc_resource_manager_context_t* const ctx =
      (iotc_resource_manager_context_t*)context;

  IOTC_CR_START(ctx->cs);

  /* always reset the offset before further processing */
  ctx->data_offset = 0;

  IOTC_RESOURCE_MANAGER_YIELD_FROM(
      ctx->cs, iotc_globals.evtd_instance, ctx->resource_handle,
      IOTC_EVENT_WANT_READ, IOTC_STATE_WANT_READ, local_handle, ret_state,
      { /* this section will be called after each read_resource function
           invocation */
        ctx->data_offset += buffer_size;

        /* if not the whole file has been read */
        if (IOTC_STATE_OK == ret_state &&
            ctx->data_offset != ctx->resource_stat.resource_size) {
          ret_state = IOTC_STATE_WANT_READ;
        }

        if (NULL == ctx->data_buffer) /* if buffer was not allocated */
        {
          if (IOTC_STATE_OK == ret_state) /* if all read at once */
          {
            ctx->data_buffer =
                iotc_make_desc_from_buffer_share((uint8_t*)buffer, buffer_size);

            IOTC_CHECK_MEMORY(ctx->data_buffer, ret_state);
          } else if (IOTC_STATE_WANT_READ == ret_state) {
            ctx->data_buffer =
                iotc_make_empty_desc_alloc(ctx->resource_stat.resource_size);

            IOTC_CHECK_MEMORY(ctx->data_buffer, ret_state);

            /* append first chunk */
            IOTC_CHECK_STATE(iotc_data_desc_append_bytes(ctx->data_buffer,
                                                         buffer, buffer_size));
          }
        } else if (IOTC_STATE_WANT_READ == ret_state ||
                   IOTC_STATE_OK == ret_state) /* if continuation */
        {
          /* accumulate */
          IOTC_CHECK_STATE(iotc_data_desc_append_data_resize(
              ctx->data_buffer, (char*)buffer, buffer_size));
        }

        /* if it's not the whole file than let's keep reading */
        if (ctx->data_offset < ctx->resource_stat.resource_size) {
          ret_state = IOTC_STATE_WANT_READ;
        }
      },
      iotc_internals.fs_functions.read_resource, NULL, ctx->resource_handle,
      ctx->data_offset, &buffer, &buffer_size);

  /* POST-CONDITION */
  assert(NULL != ctx->data_buffer);

  IOTC_RESOURCE_MANGER_INVOKE_CALLBACK();

  /* end of coroutine */
  IOTC_CR_EXIT(ctx->cs, ret_state);
  IOTC_CR_END();

err_handling:;
  iotc_event_handle_t event_handle = ctx->callback;
  iotc_dispose_handle(&ctx->callback);
  iotc_resource_manager_set_callback_state(&event_handle, ret_state);
  iotc_evtd_execute_handle(&event_handle);

  return ret_state;
}

iotc_state_t iotc_resource_manager_close_coroutine(void* context) {
  /* PRE-CONDITIONS */
  assert(NULL != context);

  /* local variables */
  iotc_state_t ret_state = IOTC_STATE_OK;
  iotc_event_handle_t local_handle =
      iotc_make_handle(&iotc_resource_manager_close_coroutine, context);

  /* local variables <- parameters */
  iotc_resource_manager_context_t* const ctx =
      (iotc_resource_manager_context_t*)context;

  IOTC_CR_START(ctx->cs);

  IOTC_RESOURCE_MANAGER_YIELD_FROM(
      ctx->cs, iotc_globals.evtd_instance, ctx->resource_handle,
      IOTC_EVENT_WANT_READ, IOTC_STATE_WANT_READ, local_handle, ret_state,
      {/* empty block of code */}, iotc_internals.fs_functions.close_resource,
      NULL, ctx->resource_handle);

  /* POST-CONDITION */
  assert(NULL != ctx->data_buffer);

  ctx->resource_handle = iotc_fs_init_resource_handle();

  IOTC_RESOURCE_MANGER_INVOKE_CALLBACK();

  /* end of coroutine */
  IOTC_CR_EXIT(ctx->cs, ret_state);
  IOTC_CR_END();

err_handling:;
  iotc_event_handle_t event_handle = ctx->callback;
  iotc_dispose_handle(&ctx->callback);
  iotc_resource_manager_set_callback_state(&event_handle, ret_state);
  iotc_evtd_execute_handle(&event_handle);

  return ret_state;
}

iotc_state_t iotc_resource_manager_open(
    iotc_resource_manager_context_t* const context,
    iotc_event_handle_t callback, const iotc_fs_resource_type_t resource_type,
    const char* const resource_name, const iotc_fs_open_flags_t open_flags,
    void* fs_context) {
  IOTC_UNUSED(fs_context);

  if (NULL == resource_name || NULL == context) {
    return IOTC_INVALID_PARAMETER;
  }

  if (1 != iotc_handle_disposed(&context->callback) ||
      context->resource_handle >= 0) {
    return IOTC_INVALID_PARAMETER;
  }

  iotc_state_t state = IOTC_STATE_OK;

  context->open_flags = open_flags;
  context->callback = callback;

  IOTC_CHECK_MEMORY(
      iotc_evtd_execute(
          iotc_globals.evtd_instance,
          iotc_make_handle(&iotc_resource_manager_open_coroutine,
                           (void*)context, (void*)(intptr_t)resource_type,
                           IOTC_STATE_OK, (void*)resource_name)),
      state);
  return state;

err_handling:
  return state;
}

iotc_state_t iotc_resource_manager_read(
    iotc_resource_manager_context_t* const context,
    iotc_event_handle_t callback, void* fs_context) {
  IOTC_UNUSED(fs_context);

  if (NULL == context) {
    return IOTC_INVALID_PARAMETER;
  }

  if (1 != iotc_handle_disposed(&context->callback) ||
      context->resource_handle < 0) {
    return IOTC_INVALID_PARAMETER;
  }

  iotc_state_t state = IOTC_STATE_OK;

  context->callback = callback;

  IOTC_CHECK_MEMORY(
      iotc_evtd_execute(iotc_globals.evtd_instance,
                        iotc_make_handle(&iotc_resource_manager_read_coroutine,
                                         (void*)context)),
      state);

  return state;

err_handling:
  return state;
}

iotc_state_t iotc_resource_manager_close(
    iotc_resource_manager_context_t* const context,
    iotc_event_handle_t callback, void* fs_context) {
  IOTC_UNUSED(fs_context);

  if (NULL == context) {
    return IOTC_INVALID_PARAMETER;
  }

  if (1 != iotc_handle_disposed(&context->callback) ||
      context->resource_handle < 0) {
    return IOTC_INVALID_PARAMETER;
  }

  iotc_state_t state = IOTC_STATE_OK;

  context->callback = callback;

  IOTC_CHECK_MEMORY(
      iotc_evtd_execute(iotc_globals.evtd_instance,
                        iotc_make_handle(&iotc_resource_manager_close_coroutine,
                                         (void*)context)),
      state);

  return state;

err_handling:
  return state;
}
