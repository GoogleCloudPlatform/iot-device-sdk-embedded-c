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

#include "iotc_layer_api.h"
#include "iotc_config.h"
#include "iotc_event_thread_dispatcher.h"
#include "iotc_globals.h"

/**
 * @brief get_next_layer_state Function that checks what should be the next
 * layer state according to some very simple rules related to which function on
 * which layer is being called.
 *
 * @param func next function.
 * @param from_context layer's context from which we are moving.
 * @param context target context of function invocation.
 * @param state state in which the transition is happening.
 * @return next layer state.
 */
static iotc_layer_state_t get_next_layer_state(
    iotc_layer_func_t* func, iotc_layer_connectivity_t* from_context,
    iotc_layer_connectivity_t* context, iotc_state_t state) {
  if (func == IOTC_THIS_LAYER(context)->layer_funcs->close) {
    if (IOTC_THIS_LAYER_STATE(from_context) == IOTC_LAYER_STATE_CONNECTED) {
      return IOTC_LAYER_STATE_CLOSING;
    } else {
      return IOTC_THIS_LAYER_STATE(from_context);
    }
  } else if (func == IOTC_THIS_LAYER(context)->layer_funcs->close_externally) {
    return IOTC_LAYER_STATE_CLOSED;
  } else if (func == IOTC_THIS_LAYER(context)->layer_funcs->connect) {
    return state == IOTC_STATE_OK ? IOTC_LAYER_STATE_CONNECTED
                                  : IOTC_LAYER_STATE_CLOSED;
  } else if (func == IOTC_THIS_LAYER(context)->layer_funcs->init) {
    return IOTC_LAYER_STATE_CONNECTING;
  } else if (func == IOTC_THIS_LAYER(context)->layer_funcs->pull) {
    return IOTC_THIS_LAYER_STATE(from_context);
  } else if (func == IOTC_THIS_LAYER(context)->layer_funcs->push) {
    return IOTC_THIS_LAYER_STATE(from_context);
  } else if (func == IOTC_THIS_LAYER(context)->layer_funcs->post_connect) {
    return IOTC_THIS_LAYER_STATE(from_context);
  }

  return IOTC_LAYER_STATE_NONE;
}

#if IOTC_DEBUG_EXTRA_INFO

iotc_state_t iotc_layer_continue_with_impl(
    iotc_layer_func_t* func, iotc_layer_connectivity_t* from_context,
    iotc_layer_connectivity_t* context, void* data, iotc_state_t state,
    const char* file_name, const int line_no) {
  iotc_state_t local_state = IOTC_STATE_OK;

  if (NULL != context) {
    context->self->debug_info.debug_file_last_call = file_name;
    context->self->debug_info.debug_line_last_call = line_no;
  }

  if (func != NULL) {
    iotc_event_handle_queue_t* e_ptr =
        iotc_evttd_execute(IOTC_CONTEXT_DATA(context)->evtd_instance,
                           iotc_make_handle(func, context, data, state));
    IOTC_CHECK_MEMORY(e_ptr, local_state);

    iotc_layer_state_t next_state =
        get_next_layer_state(func, from_context, context, state);

    IOTC_THIS_LAYER_STATE_UPDATE(from_context, next_state);
  }

err_handling:
  return local_state;
}

#else /* IOTC_DEBUG_EXTRA_INFO */

iotc_state_t iotc_layer_continue_with_impl(
    iotc_layer_func_t* func, iotc_layer_connectivity_t* from_context,
    iotc_layer_connectivity_t* context, void* data, iotc_state_t state) {
  iotc_state_t local_state = IOTC_STATE_OK;

  if (func != NULL) {
    iotc_event_handle_queue_t* e_ptr =
        iotc_evttd_execute(IOTC_CONTEXT_DATA(context)->evtd_instance,
                           iotc_make_handle(func, context, data, state));
    IOTC_CHECK_MEMORY(e_ptr, local_state);

    iotc_layer_state_t next_state =
        get_next_layer_state(func, from_context, context, state);

    IOTC_THIS_LAYER_STATE_UPDATE(from_context, next_state);
  }

err_handling:
  return local_state;
}

#endif
