/* Copyright 2018-2020 Google LLC
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

#include <inttypes.h>

#include "iotc_event_dispatcher_api.h"
#include "iotc_helpers.h"
#include "iotc_list.h"

static inline int8_t iotc_evtd_cmp_fd(
    const union iotc_vector_selector_u* e0,
    const union iotc_vector_selector_u* value) {
  iotc_evtd_fd_tuple_t* tuple = (iotc_evtd_fd_tuple_t*)e0->ptr_value;

  assert(sizeof(tuple->fd) == sizeof(value->iptr_value));

  if (tuple->fd == value->iptr_value) {
    return 0;
  }

  return -1;
}

static int8_t iotc_evtd_register_fd(iotc_evtd_instance_t* instance,
                                    iotc_vector_t* container,
                                    iotc_event_type_t event_type,
                                    iotc_evtd_fd_type_t fd_type, iotc_fd_t fd,
                                    iotc_event_handle_t read_handle,
                                    iotc_event_handle_t current_handle) {
  /* PRECONDITIONS */
  assert(NULL != container);
  assert(NULL != instance);
  assert(iotc_vector_find(container,
                          IOTC_VEC_CONST_VALUE_PARAM(IOTC_VEC_VALUE_IPTR(fd)),
                          &iotc_evtd_cmp_fd) == -1);

  IOTC_UNUSED(instance);

  iotc_state_t state = IOTC_STATE_OK;

  iotc_lock_critical_section(instance->cs);

  /* add an entry with the proper event for file descriptor */
  IOTC_ALLOC(iotc_evtd_fd_tuple_t, tuple, state);

  tuple->fd = fd;
  tuple->event_type = event_type;
  tuple->read_handle = read_handle;
  tuple->handle = current_handle;
  tuple->fd_type = fd_type;

  /* register within the handles */
  {
    const iotc_vector_elem_t* e = iotc_vector_push(
        container, IOTC_VEC_CONST_VALUE_PARAM(IOTC_VEC_VALUE_PTR(tuple)));
    if (NULL == e) {
      goto err_handling;
    }
  }

  iotc_unlock_critical_section(instance->cs);

  return 1;

err_handling:
  IOTC_SAFE_FREE(tuple);
  iotc_unlock_critical_section(instance->cs);

  return 0;
}

int8_t iotc_evtd_register_file_fd(iotc_evtd_instance_t* instance,
                                  iotc_event_type_t event_type, iotc_fd_t fd,
                                  iotc_event_handle_t handle) {
  return iotc_evtd_register_fd(instance, instance->handles_and_file_fd,
                               event_type, IOTC_EVTD_FD_TYPE_FILE, fd, handle,
                               handle);
}

int8_t iotc_evtd_register_socket_fd(iotc_evtd_instance_t* instance,
                                    iotc_fd_t fd,
                                    iotc_event_handle_t read_handle) {
  return iotc_evtd_register_fd(instance, instance->handles_and_socket_fd,
                               IOTC_EVENT_WANT_READ, IOTC_EVTD_FD_TYPE_SOCKET,
                               fd, read_handle, read_handle);
}

static int8_t iotc_evtd_unregister_fd(iotc_evtd_instance_t* instance,
                                      iotc_vector_t* container, iotc_fd_t fd) {
  /* PRE-CONDITIONS */
  assert(NULL != instance);
  assert(NULL != container);

  IOTC_UNUSED(instance);

  iotc_lock_critical_section(instance->cs);

  iotc_vector_index_type_t id = iotc_vector_find(
      container, IOTC_VEC_CONST_VALUE_PARAM(IOTC_VEC_VALUE_IPTR(fd)),
      &iotc_evtd_cmp_fd);

  /* remove from the vector */
  if (-1 != id) {
    assert(NULL != container->array[id].selector_t.ptr_value);
    IOTC_SAFE_FREE(container->array[id].selector_t.ptr_value);
    iotc_vector_del(container, id);

    iotc_unlock_critical_section(instance->cs);
    return 1;
  }

  iotc_unlock_critical_section(instance->cs);

  return -1;
}

int8_t iotc_evtd_unregister_file_fd(iotc_evtd_instance_t* instance,
                                    iotc_fd_t fd) {
  return iotc_evtd_unregister_fd(instance, instance->handles_and_file_fd, fd);
}

int8_t iotc_evtd_unregister_socket_fd(iotc_evtd_instance_t* instance,
                                      iotc_fd_t fd) {
  return iotc_evtd_unregister_fd(instance, instance->handles_and_socket_fd, fd);
}

int8_t iotc_evtd_continue_when_evt_on_socket(iotc_evtd_instance_t* instance,
                                             iotc_event_type_t event_type,
                                             iotc_event_handle_t handle,
                                             iotc_fd_t fd) {
  /* PRECONDITIONS */
  assert(instance != 0);

  iotc_lock_critical_section(instance->cs);

  iotc_vector_index_type_t id = iotc_vector_find(
      instance->handles_and_socket_fd,
      IOTC_VEC_CONST_VALUE_PARAM(IOTC_VEC_VALUE_IPTR(fd)), &iotc_evtd_cmp_fd);

  /* set up the values of the tuple */
  if (-1 != id) {
    iotc_evtd_fd_tuple_t* tuple =
        (iotc_evtd_fd_tuple_t*)instance->handles_and_socket_fd->array[id]
            .selector_t.ptr_value;

    assert(IOTC_EVTD_FD_TYPE_SOCKET == tuple->fd_type);

    tuple->event_type = event_type;
    tuple->handle = handle;

    iotc_unlock_critical_section(instance->cs);

    return 1;
  }

  iotc_unlock_critical_section(instance->cs);

  return -1;
}

void iotc_evtd_continue_when_empty(iotc_evtd_instance_t* instance,
                                   iotc_event_handle_t handle) {
  instance->on_empty = handle;
}

iotc_event_handle_queue_t* iotc_evtd_execute(iotc_evtd_instance_t* instance,
                                             iotc_event_handle_t handle) {
  iotc_state_t state = IOTC_STATE_OK;
  IOTC_ALLOC_SYSTEM(iotc_event_handle_queue_t, queue_elem, state);

  queue_elem->handle = handle;

  iotc_lock_critical_section(instance->cs);

  IOTC_LIST_PUSH_BACK(iotc_event_handle_queue_t, instance->call_queue,
                      queue_elem);

  iotc_unlock_critical_section(instance->cs);

  return queue_elem;

err_handling:
  return NULL;
}

iotc_state_t iotc_evtd_execute_in(
    iotc_evtd_instance_t* instance, iotc_event_handle_t handle,
    iotc_time_t time_diff, iotc_time_event_handle_t* ret_time_event_handle) {
  iotc_state_t ret_state = IOTC_STATE_OK;

  IOTC_ALLOC(iotc_time_event_t, time_event, ret_state);

  time_event->event_handle = handle;
  time_event->time_of_execution = instance->current_step + time_diff;

  iotc_lock_critical_section(instance->cs);

  ret_state = iotc_time_event_add(instance->time_events_container, time_event,
                                  ret_time_event_handle);

  iotc_unlock_critical_section(instance->cs);

  return ret_state;

err_handling:
  return ret_state;
}

iotc_state_t iotc_evtd_cancel(iotc_evtd_instance_t* instance,
                              iotc_time_event_handle_t* time_event_handle) {
  iotc_time_event_t* time_event = NULL;
  iotc_state_t ret_state = IOTC_STATE_OK;

  iotc_lock_critical_section(instance->cs);

  ret_state = iotc_time_event_cancel(instance->time_events_container,
                                     time_event_handle, &time_event);

  iotc_unlock_critical_section(instance->cs);

  IOTC_SAFE_FREE(time_event);

  return ret_state;
}

iotc_state_t iotc_evtd_restart(iotc_evtd_instance_t* instance,
                               iotc_time_event_handle_t* time_event_handle,
                               iotc_time_t new_time) {
  iotc_state_t ret_state = IOTC_STATE_OK;

  iotc_lock_critical_section(instance->cs);

  ret_state = iotc_time_event_restart(instance->time_events_container,
                                      time_event_handle,
                                      instance->current_step + new_time);

  iotc_unlock_critical_section(instance->cs);

  return ret_state;
}

iotc_evtd_instance_t* iotc_evtd_create_instance(void) {
  iotc_state_t state = IOTC_STATE_OK;
  IOTC_ALLOC(iotc_evtd_instance_t, evtd_instance, state);

  evtd_instance->time_events_container = iotc_vector_create();
  IOTC_CHECK_MEMORY(evtd_instance->time_events_container, state);

  evtd_instance->handles_and_socket_fd = iotc_vector_create();
  IOTC_CHECK_MEMORY(evtd_instance->handles_and_socket_fd, state);

  evtd_instance->handles_and_file_fd = iotc_vector_create();
  IOTC_CHECK_MEMORY(evtd_instance->handles_and_file_fd, state);

  IOTC_CHECK_STATE(iotc_init_critical_section(&evtd_instance->cs));

  return evtd_instance;

err_handling:
  IOTC_SAFE_FREE(evtd_instance);
  return 0;
}

void iotc_evtd_destroy_instance(iotc_evtd_instance_t* instance) {
  if (instance == NULL) return;

  struct iotc_critical_section_s* cs = instance->cs;

  IOTC_UNUSED(cs); /* yeah, I know */

  iotc_lock_critical_section(cs);

  iotc_vector_destroy(instance->handles_and_file_fd);
  iotc_vector_destroy(instance->handles_and_socket_fd);
  iotc_time_event_destroy(instance->time_events_container);
  iotc_vector_destroy(instance->time_events_container);

  IOTC_SAFE_FREE(instance);

  iotc_unlock_critical_section(cs);

  iotc_destroy_critical_section(&cs);
}

iotc_event_handle_return_t iotc_evtd_execute_handle(
    iotc_event_handle_t* handle) {
  switch (handle->handle_type) {
    case IOTC_EVENT_HANDLE_ARGC0:
      return (*handle->handlers.h0.fn_argc0)();
    case IOTC_EVENT_HANDLE_ARGC1:
      return (*handle->handlers.h1.fn_argc1)(handle->handlers.h1.a1);
    case IOTC_EVENT_HANDLE_ARGC2:
      return (*handle->handlers.h2.fn_argc2)(handle->handlers.h2.a1,
                                             handle->handlers.h2.a2);
    case IOTC_EVENT_HANDLE_ARGC3:
      return (*handle->handlers.h3.fn_argc3)(handle->handlers.h3.a1,
                                             handle->handlers.h3.a2,
                                             handle->handlers.h3.a3);
    case IOTC_EVENT_HANDLE_ARGC4:
      return (*handle->handlers.h4.fn_argc4)(
          handle->handlers.h4.a1, handle->handlers.h4.a2,
          handle->handlers.h4.a3, handle->handlers.h4.a4);
    case IOTC_EVENT_HANDLE_ARGC5:
      return (*handle->handlers.h5.fn_argc5)(
          handle->handlers.h5.a1, handle->handlers.h5.a2,
          handle->handlers.h5.a3, handle->handlers.h5.a4,
          handle->handlers.h5.a5);
    case IOTC_EVENT_HANDLE_ARGC6:
      return (*handle->handlers.h6.fn_argc6)(
          handle->handlers.h6.a1, handle->handlers.h6.a2,
          handle->handlers.h6.a3, handle->handlers.h6.a4,
          handle->handlers.h6.a5, handle->handlers.h6.a6);
    case IOTC_EVENT_HANDLE_UNSET:
      iotc_debug_logger("you are trying to call an unset handler!");
#if IOTC_DEBUG_EXTRA_INFO
      iotc_debug_format("handler created in %s:%d",
                        handle->debug_info.debug_file_init,
                        handle->debug_info.debug_line_init);
#endif
      return (iotc_event_handle_return_t)IOTC_UNSET_HANDLER_ERROR;
  }

  return (iotc_event_handle_return_t)IOTC_STATE_OK;
}

static uint8_t iotc_state_is_fatal(iotc_state_t e) {
  switch (e) {
    case IOTC_OUT_OF_MEMORY:
    case IOTC_INTERNAL_ERROR:
    case IOTC_MQTT_UNKNOWN_MESSAGE_ID:
      return 1;
    default:
      return 0;
  }
}

extern uint8_t iotc_evtd_single_step(iotc_evtd_instance_t* evtd_instance,
                                     iotc_time_t new_step) {
  if (evtd_instance == NULL) return 0;

  evtd_instance->current_step = new_step;

  iotc_event_handle_queue_t* queue_elem = NULL;

  iotc_lock_critical_section(evtd_instance->cs);
  if (!IOTC_LIST_EMPTY(iotc_event_handle_queue_t, evtd_instance->call_queue)) {
    IOTC_LIST_POP(iotc_event_handle_queue_t, evtd_instance->call_queue,
                  queue_elem);
  }
  iotc_unlock_critical_section(evtd_instance->cs);

  if (queue_elem == NULL) return 0;

  const iotc_state_t result = iotc_evtd_execute_handle(&queue_elem->handle);

  if (iotc_state_is_fatal(result) == 1) {
    iotc_debug_logger("error while processing normal events");
  }

  IOTC_SAFE_FREE(queue_elem);

  return 1;
}

void iotc_evtd_step(iotc_evtd_instance_t* evtd_instance, iotc_time_t new_step) {
  if (evtd_instance == NULL) return;

  evtd_instance->current_step = new_step;
  iotc_time_event_t* tmp = NULL;

#ifdef IOTC_DEUBG_OUTPUT_EVENT_SYSTEM
  iotc_debug_format("[size of time event queue: %d]",
                    evtd_instance->call_heap->first_free);
  iotc_debug_logger("[calling time events]");
#endif

  iotc_lock_critical_section(evtd_instance->cs);

  /* zero - not NULL elem_no it's a number not a pointer */
  while (0 != evtd_instance->time_events_container->elem_no) {
    tmp = iotc_time_event_peek_top(evtd_instance->time_events_container);
    if (tmp->time_of_execution <= evtd_instance->current_step) {
      tmp = iotc_time_event_get_top(evtd_instance->time_events_container);
      iotc_event_handle_t* handle = (iotc_event_handle_t*)&tmp->event_handle;

      iotc_unlock_critical_section(evtd_instance->cs);

      iotc_state_t result = iotc_evtd_execute_handle(handle);

      IOTC_SAFE_FREE(tmp);

      if (iotc_state_is_fatal(result) == 1) {
        iotc_debug_logger("error while processing timed events");
        iotc_evtd_stop(evtd_instance);
      }

      iotc_lock_critical_section(evtd_instance->cs);
    } else {
#ifdef IOTC_DEUBG_OUTPUT_EVENT_SYSTEM
      iotc_debug_format("[next key execution time: %" SCNuPTR
                        "\tcurrent time: %" SCNuPTR "]",
                        tmp->key, evtd_instance->current_step);
#endif
      break;
    }
  }

  iotc_unlock_critical_section(evtd_instance->cs);

#ifdef IOTC_DEUBG_OUTPUT_EVENT_SYSTEM
  iotc_debug_logger("[enqueued events]");
#endif

  /* execute all handlers in call_queue */
  while (iotc_evtd_single_step(evtd_instance, new_step))
    ;

  iotc_lock_critical_section(evtd_instance->cs);
  /* here we can call the on_empty handler
   * watch out, handler is called only once and
   * it is disposed after that */
  if ((0 == evtd_instance->time_events_container->elem_no) &&
      (evtd_instance->on_empty.handle_type != IOTC_EVENT_HANDLE_UNSET)) {
    iotc_debug_logger("calling on_empty_handler");

    iotc_unlock_critical_section(evtd_instance->cs);
    iotc_evtd_execute_handle(&evtd_instance->on_empty);
    iotc_lock_critical_section(evtd_instance->cs);

    iotc_dispose_handle(&evtd_instance->on_empty);
  }

  iotc_unlock_critical_section(evtd_instance->cs);
}

uint8_t iotc_evtd_dispatcher_continue(iotc_evtd_instance_t* instance) {
  return instance != NULL && instance->stop != 1;
}

uint8_t iotc_evtd_all_continue(iotc_evtd_instance_t** event_dispatchers,
                               uint8_t num_evtds) {
  uint8_t all_continue = 1;

  uint8_t it_evtd = 0;
  for (; it_evtd < num_evtds && all_continue; ++it_evtd) {
    all_continue = iotc_evtd_dispatcher_continue(event_dispatchers[it_evtd]);
  }

  return all_continue;
}

iotc_state_t iotc_evtd_update_event_on_fd(iotc_evtd_instance_t* instance,
                                          iotc_vector_t* container,
                                          iotc_fd_t fd) {
  assert(instance != 0);
  iotc_lock_critical_section(instance->cs);

  iotc_vector_index_type_t id = iotc_vector_find(
      container, IOTC_VEC_CONST_VALUE_PARAM(IOTC_VEC_VALUE_IPTR(fd)),
      &iotc_evtd_cmp_fd);

  if (id != -1) {
    iotc_evtd_fd_tuple_t* tuple =
        (iotc_evtd_fd_tuple_t*)container->array[id].selector_t.ptr_value;

    /* save the handle to execute */
    iotc_event_handle_t to_exec = tuple->handle;

    /* set the default one if fd type socket */
    if (IOTC_EVTD_FD_TYPE_SOCKET == tuple->fd_type) {
      tuple->event_type = IOTC_EVENT_WANT_READ;  // default
      tuple->handle = tuple->read_handle;
    }

    /* execute previously saved handle
     * we save the handle because the tuple->handle
     * may be overrided within the handle execution
     * so we don't won't to override that again */
    iotc_unlock_critical_section(instance->cs);

    iotc_evtd_execute_handle(&to_exec);

    iotc_lock_critical_section(instance->cs);
  } else {
    iotc_evtd_stop(instance);

    iotc_unlock_critical_section(instance->cs);

    return IOTC_FD_HANDLER_NOT_FOUND;
  }

  iotc_unlock_critical_section(instance->cs);
  return IOTC_STATE_OK;
}

iotc_state_t iotc_evtd_update_event_on_socket(iotc_evtd_instance_t* instance,
                                              iotc_fd_t fd) {
  return iotc_evtd_update_event_on_fd(instance, instance->handles_and_socket_fd,
                                      fd);
}

iotc_state_t iotc_evtd_update_event_on_file(iotc_evtd_instance_t* instance,
                                            iotc_fd_t fd) {
  return iotc_evtd_update_event_on_fd(instance, instance->handles_and_file_fd,
                                      fd);
}

void iotc_evtd_stop(iotc_evtd_instance_t* instance) {
  assert(instance != 0);

  instance->stop = 1;
}

uint8_t iotc_evtd_update_file_fd_events(
    iotc_evtd_instance_t* const event_dispatcher) {
  uint8_t was_there_an_event = 0;
  iotc_vector_index_type_t i = 0;

  for (i = 0; i < event_dispatcher->handles_and_file_fd->elem_no; ++i) {
    iotc_evtd_fd_tuple_t* tuple =
        (iotc_evtd_fd_tuple_t*)event_dispatcher->handles_and_file_fd->array[i]
            .selector_t.ptr_value;

    assert(IOTC_EVTD_FD_TYPE_FILE == tuple->fd_type);

    if ((tuple->event_type & IOTC_EVENT_WANT_READ) > 0) {
      iotc_evtd_update_event_on_file(event_dispatcher, tuple->fd);
      was_there_an_event |= 1;
    } else if ((tuple->event_type & IOTC_EVENT_WANT_WRITE) > 0) {
      iotc_evtd_update_event_on_file(event_dispatcher, tuple->fd);
      was_there_an_event |= 1;
    } else if ((tuple->event_type & IOTC_EVENT_ERROR) > 0) {
      iotc_evtd_update_event_on_file(event_dispatcher, tuple->fd);
      was_there_an_event |= 1;
    }
  }

  return was_there_an_event;
}

iotc_state_t iotc_evtd_get_time_of_earliest_event(
    iotc_evtd_instance_t* instance, iotc_time_t* out_timeout) {
  assert(NULL != instance);
  assert(NULL != out_timeout);
  assert(NULL != instance->time_events_container);

  iotc_state_t ret_state = IOTC_ELEMENT_NOT_FOUND;

  iotc_lock_critical_section(instance->cs);

  if (0 != instance->time_events_container->elem_no) {
    iotc_time_event_t* elem =
        iotc_time_event_peek_top(instance->time_events_container);
    *out_timeout = elem->time_of_execution;
    ret_state = IOTC_STATE_OK;
  }

  iotc_unlock_critical_section(instance->cs);

  return ret_state;
}
