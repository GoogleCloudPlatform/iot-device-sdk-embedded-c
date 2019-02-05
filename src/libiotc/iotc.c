/* Copyright 2018 Google LLC
 *
 * This is part of the Google Cloud IoT Edge Embedded C Client,
 * it is licensed under the BSD 3-Clause license; you may not use this file
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "iotc.h"
#include "iotc_allocator.h"
#include "iotc_backoff_lut_config.h"
#include "iotc_backoff_status_api.h"
#include "iotc_common.h"
#include "iotc_connection_data_internal.h"
#include "iotc_debug.h"
#include "iotc_err.h"
#include "iotc_event_loop.h"
#include "iotc_globals.h"
#include "iotc_handle.h"
#include "iotc_helpers.h"
#include "iotc_internals.h"
#include "iotc_jwt.h"
#include "iotc_layer.h"
#include "iotc_layer_api.h"
#include "iotc_layer_chain.h"
#include "iotc_layer_default_allocators.h"
#include "iotc_layer_factory.h"
#include "iotc_layer_macros.h"
#include "iotc_list.h"
#include "iotc_macros.h"
#include "iotc_timed_task.h"
#include "iotc_version.h"

#include "iotc_layer_stack.h"

#include "iotc_thread_threadpool.h"

#include "iotc_user_sub_call_wrapper.h"

#include <iotc_bsp_rng.h>
#include <iotc_bsp_time.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * CONSTANTS
 */

#ifndef IOTC_MAX_NUM_CONTEXTS
#define IOTC_MAX_NUM_CONTEXTS 2
#endif

const uint16_t iotc_major = IOTC_MAJOR;
const uint16_t iotc_minor = IOTC_MINOR;
const uint16_t iotc_revision = IOTC_REVISION;

#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)

const char iotc_cilent_version_str[] = "IoTC Embedded C Client Version: " STR(
    IOTC_MAJOR) "." STR(IOTC_MINOR) "." STR(IOTC_REVISION);

/*
 * HELPER FUNCTIONS
 */
void iotc_set_network_timeout(uint32_t timeout) {
  iotc_globals.network_timeout = timeout;
}

uint32_t iotc_get_network_timeout(void) { return iotc_globals.network_timeout; }

/*
 * MAIN LIBRARY FUNCTIONS
 */
iotc_state_t iotc_initialize() {
  iotc_bsp_time_init();
  iotc_bsp_rng_init();

  return IOTC_STATE_OK;
}

iotc_state_t iotc_shutdown() {
  iotc_bsp_rng_shutdown();

  return IOTC_STATE_OK;
}

iotc_state_t iotc_create_context_with_custom_layers_and_evtd(
    iotc_context_t** context, iotc_layer_type_t layer_config[],
    iotc_layer_type_id_t layer_chain[], size_t layer_chain_size,
    iotc_evtd_instance_t* event_dispatcher) /* PLEASE NOTE: Event dispatcher's
                                             * ownership is not taken here! */
{
  iotc_state_t state = IOTC_STATE_OK;

  if (NULL == context) {
    return IOTC_INVALID_PARAMETER;
  }

  *context = NULL;
  iotc_globals.globals_ref_count += 1;

  if (1 == iotc_globals.globals_ref_count) {
    iotc_globals.evtd_instance = iotc_evtd_create_instance();

    IOTC_CHECK_STATE(iotc_backoff_configure_using_data(
        (iotc_vector_elem_t*)IOTC_BACKOFF_LUT,
        (iotc_vector_elem_t*)IOTC_DECAY_LUT, IOTC_ARRAYSIZE(IOTC_BACKOFF_LUT),
        IOTC_MEMORY_TYPE_UNMANAGED));

    IOTC_CHECK_MEMORY(iotc_globals.evtd_instance, state);

    /* Note: this is NULL if thread module is disabled. */
    iotc_globals.main_threadpool = iotc_threadpool_create_instance(1);

    iotc_globals.context_handles_vector = iotc_vector_create();
    iotc_globals.timed_tasks_container = iotc_make_timed_task_container();
  }

  /* Allocate the structure to store new context. */
  IOTC_ALLOC_AT(iotc_context_t, *context, state);

  /* Create io timeout vector. */
  (*context)->context_data.io_timeouts = iotc_vector_create();

  IOTC_CHECK_MEMORY((*context)->context_data.io_timeouts, state);
  /* Set the event dispatcher to the global one, if none is provided. */
  (*context)->context_data.evtd_instance = (NULL == event_dispatcher)
                                               ? iotc_globals.evtd_instance
                                               : event_dispatcher;

  /* copy given numeric parameters as is */
  (*context)->protocol = IOTC_MQTT;

  (*context)->layer_chain = iotc_layer_chain_create(
      layer_chain, layer_chain_size, &(*context)->context_data, layer_config);

  IOTC_CHECK_STATE(state = iotc_register_handle_for_object(
                       iotc_globals.context_handles_vector,
                       IOTC_MAX_NUM_CONTEXTS, *context));

  return IOTC_STATE_OK;

err_handling:
  /* @TODO release any allocated buffers. */
  IOTC_SAFE_FREE(*context);
  return state;
}

iotc_state_t iotc_create_context_with_custom_layers(
    iotc_context_t** context, iotc_layer_type_t layer_config[],
    iotc_layer_type_id_t layer_chain[], size_t layer_chain_size) {
  return iotc_create_context_with_custom_layers_and_evtd(
      context, layer_config, layer_chain, layer_chain_size, NULL);
}

iotc_context_handle_t iotc_create_context() {
  iotc_context_t* context;
  iotc_state_t state = IOTC_STATE_OK;

  IOTC_CHECK_STATE(state = iotc_create_context_with_custom_layers(
                       &context, iotc_layer_types_g, IOTC_LAYER_CHAIN_DEFAULT,
                       IOTC_LAYER_CHAIN_DEFAULTSIZE_SUFFIX));

  iotc_context_handle_t context_handle;
  IOTC_CHECK_STATE(
      state = iotc_find_handle_for_object(iotc_globals.context_handles_vector,
                                          context, &context_handle));

  goto end;
err_handling:
  return -state;
end:
  return context_handle;
}

/**
 * @brief Helper function used to clean and free the protocol specific
 *in-context data.
 **/
static void iotc_free_context_data(iotc_context_t* context) {
  assert(NULL != context);

  iotc_context_data_t* context_data =
      (iotc_context_data_t*)&context->context_data;

  assert(NULL != context_data);

  /* Destroy timeout. */
  iotc_vector_destroy(context_data->io_timeouts);

  /* See comment in iotc_types_internal.h. */
  if (context_data->copy_of_handlers_for_topics) {
    iotc_vector_for_each(context_data->copy_of_handlers_for_topics,
                         &iotc_mqtt_task_spec_data_free_subscribe_data_vec,
                         NULL, 0);

    context_data->copy_of_handlers_for_topics =
        iotc_vector_destroy(context_data->copy_of_handlers_for_topics);
  }

  if (context_data->copy_of_q12_unacked_messages_queue) {
    /* This pointer must be present otherwise we are not going to be able to
     * release mqtt specific data. */
    assert(NULL != context_data->copy_of_q12_unacked_messages_queue_dtor_ptr);
    context_data->copy_of_q12_unacked_messages_queue_dtor_ptr(
        &context_data->copy_of_q12_unacked_messages_queue);
  }

  {
    uint16_t id_file = 0;
    for (; id_file < context_data->updateable_files_count; ++id_file) {
      IOTC_SAFE_FREE(context_data->updateable_files[id_file]);
    }

    IOTC_SAFE_FREE(context_data->updateable_files);
  }

  iotc_free_connection_data(&context_data->connection_data);

  /* Remember: event dispatcher ownership is not taken, this is why we don't
   * delete it. */
  context_data->evtd_instance = NULL;
}

iotc_state_t iotc_delete_context_with_custom_layers(
    iotc_context_t** context, iotc_layer_type_t layer_config[],
    size_t layer_chain_size) {
  if (NULL == *context) {
    return IOTC_INVALID_PARAMETER;
  }

  iotc_state_t state = IOTC_STATE_OK;

  state = iotc_delete_handle_for_object(iotc_globals.context_handles_vector,
                                        *context);

  if (IOTC_STATE_OK != state) {
    return IOTC_ELEMENT_NOT_FOUND;
  }

  iotc_layer_chain_delete(&((*context)->layer_chain), layer_chain_size,
                          layer_config);

  iotc_free_context_data(*context);

  IOTC_SAFE_FREE(*context);

  iotc_globals.globals_ref_count -= 1;

  if (0 == iotc_globals.globals_ref_count) {
    iotc_cancel_backoff_event();
    iotc_backoff_release();
    iotc_evtd_destroy_instance(iotc_globals.evtd_instance);
    iotc_globals.evtd_instance = NULL;
    iotc_threadpool_destroy_instance(&iotc_globals.main_threadpool);

    iotc_vector_destroy(iotc_globals.context_handles_vector);
    iotc_globals.context_handles_vector = NULL;

    iotc_destroy_timed_task_container(iotc_globals.timed_tasks_container);
    iotc_globals.timed_tasks_container = NULL;
  }

  return IOTC_STATE_OK;
}

iotc_state_t iotc_delete_context(iotc_context_handle_t context_handle) {
  iotc_context_t* context = iotc_object_for_handle(
      iotc_globals.context_handles_vector, context_handle);
  assert(context != NULL);

  return iotc_delete_context_with_custom_layers(
      &context, iotc_layer_types_g,
      IOTC_LAYER_CHAIN_SCHEME_LENGTH(IOTC_LAYER_CHAIN_DEFAULT));
}

void iotc_default_client_callback(iotc_context_handle_t in_context_handle,
                                  void* data, iotc_state_t state) {
  IOTC_UNUSED(in_context_handle);
  IOTC_UNUSED(data);
  IOTC_UNUSED(state);
}

iotc_state_t iotc_user_callback_wrapper(void* context, void* data,
                                        iotc_state_t in_state,
                                        void* client_callback) {
  assert(NULL != client_callback);
  assert(NULL != context);

  iotc_state_t state = IOTC_STATE_OK;
  iotc_context_handle_t context_handle;
  IOTC_CHECK_STATE(
      state = iotc_find_handle_for_object(iotc_globals.context_handles_vector,
                                          context, &context_handle));

  ((iotc_user_callback_t*)(client_callback))(context_handle, data, in_state);

err_handling:
  return state;
}

extern uint8_t iotc_is_context_connected(iotc_context_handle_t iotc_h) {
  if (IOTC_INVALID_CONTEXT_HANDLE == iotc_h) {
    return 0;
  }

  iotc_context_t* iotc = (iotc_context_t*)iotc_object_for_handle(
      iotc_globals.context_handles_vector, iotc_h);

  if (NULL == iotc || NULL == iotc->context_data.connection_data) {
    return 0;
  }

  return IOTC_CONNECTION_STATE_OPENED ==
             iotc->context_data.connection_data->connection_state &&
         IOTC_SHUTDOWN_UNINITIALISED == iotc->context_data.shutdown_state;
}

void iotc_events_stop() { iotc_evtd_stop(iotc_globals.evtd_instance); }

void iotc_events_process_blocking() {
  iotc_event_loop_with_evtds(0, &iotc_globals.evtd_instance, 1);
}

iotc_state_t iotc_events_process_tick() {
  if (iotc_evtd_dispatcher_continue(iotc_globals.evtd_instance) == 1) {
    iotc_event_loop_with_evtds(1, &iotc_globals.evtd_instance, 1);
    return IOTC_STATE_OK;
  }

  return IOTC_EVENT_PROCESS_STOPPED;
}

iotc_state_t iotc_connect(
  iotc_context_handle_t iotc_h, const char* username,
  const char* password, const char* client_id,
  uint16_t connection_timeout, uint16_t keepalive_timeout,
  iotc_user_callback_t* client_callback ) {

  typedef struct iotc_static_host_desc_s {
    char* name;
    uint16_t port;
  } iotc_static_host_desc_t;

#define IOTC_MQTT_HOST_ACCESSOR ((iotc_static_host_desc_t)IOTC_MQTT_HOST)

  return iotc_connect_to(
      iotc_h, IOTC_MQTT_HOST_ACCESSOR.name, IOTC_MQTT_HOST_ACCESSOR.port,
      username, password, client_id, connection_timeout,
      keepalive_timeout, client_callback);
}

iotc_state_t iotc_connect_to(
  iotc_context_handle_t iotc_h, const char* host, uint16_t port,
  const char* username, const char* password,
  const char* client_id, uint16_t connection_timeout,
  uint16_t keepalive_timeout, iotc_user_callback_t* client_callback) {

  iotc_state_t state = IOTC_STATE_OK;
  iotc_context_t* iotc = NULL;
  iotc_event_handle_t event_handle = iotc_make_empty_event_handle();
  iotc_layer_t* input_layer = NULL;
  uint32_t new_backoff = 0;

  IOTC_CHECK_CND_DBGMESSAGE(NULL == host, IOTC_NULL_HOST, state,
                            "ERROR: NULL host provided");

  IOTC_CHECK_CND_DBGMESSAGE(NULL == client_id, IOTC_NULL_CLIENT_ID_ERROR, state,
                            "ERROR: NULL client_id provided");

  IOTC_CHECK_CND_DBGMESSAGE(IOTC_INVALID_CONTEXT_HANDLE >= iotc_h,
                            IOTC_NULL_CONTEXT, state,
                            "ERROR: invalid context handle provided");

  iotc = iotc_object_for_handle(iotc_globals.context_handles_vector, iotc_h);

  IOTC_CHECK_CND_DBGMESSAGE(NULL == iotc, IOTC_NULL_CONTEXT, state,
                            "ERROR: NULL context provided");

  assert(NULL != client_callback);

  event_handle = iotc_make_threaded_handle(
      IOTC_THREADID_THREAD_0, &iotc_user_callback_wrapper, iotc, NULL,
      IOTC_STATE_OK, (void*)client_callback);

  /* Guard against adding two connection requests. */
  if (NULL != iotc->context_data.connect_handler.ptr_to_position) {
    iotc_debug_format(
        "Connect could not be performed due to connection state = %d,"
        "check if connect operation hasn't been already started.",
        iotc->context_data.connection_data->connection_state);
    return IOTC_ALREADY_INITIALIZED;
  }

  /* If the connection state isn't one of the final states it means that the
   * connection already has been started. */
  if (NULL != iotc->context_data.connection_data &&
      (IOTC_CONNECTION_STATE_CLOSED !=
           iotc->context_data.connection_data->connection_state &&
       IOTC_CONNECTION_STATE_OPEN_FAILED !=
           iotc->context_data.connection_data->connection_state &&
       IOTC_CONNECTION_STATE_UNINITIALIZED !=
           iotc->context_data.connection_data->connection_state)) {
    iotc_debug_format(
        "Connect could not be performed due to connection state = %d,"
        "check if connect operation hasn't been already started.",
        iotc->context_data.connection_data->connection_state);
    return IOTC_ALREADY_INITIALIZED;
  }

  input_layer = iotc->layer_chain.top;
  iotc->protocol = IOTC_MQTT;

  if (NULL != iotc->context_data.connection_data) {
    IOTC_CHECK_STATE(iotc_connection_data_update_lastwill(
        iotc->context_data.connection_data, host, port, username, password,
        client_id,  connection_timeout, keepalive_timeout,
        IOTC_SESSION_CLEAN, NULL, NULL, (iotc_mqtt_qos_t)0,
        (iotc_mqtt_retain_t)0));
  } else {
    iotc->context_data.connection_data = iotc_alloc_connection_data_lastwill(
        host, port, username, password, client_id, connection_timeout,
        keepalive_timeout, IOTC_SESSION_CLEAN, NULL, NULL, (iotc_mqtt_qos_t)0,
        (iotc_mqtt_retain_t)0);

    IOTC_CHECK_MEMORY(iotc->context_data.connection_data, state);
  }

  iotc_debug_format("New host:port [%s]:[%hu]",
                    iotc->context_data.connection_data->host,
                    iotc->context_data.connection_data->port);

  /* Reset the connection state. */
  iotc->context_data.connection_data->connection_state =
      IOTC_CONNECTION_STATE_UNINITIALIZED;

  /* Reset shutdown state. */
  iotc->context_data.shutdown_state = IOTC_SHUTDOWN_UNINITIALISED;

  /* Set the connection callback. */
  iotc->context_data.connection_callback = event_handle;

  new_backoff = iotc_get_backoff_penalty();

  iotc_debug_format("new backoff value: %d", new_backoff);

  /* Register the execution in next init. */
  state = iotc_evtd_execute_in(
      iotc->context_data.evtd_instance,
      iotc_make_handle(input_layer->layer_connection.self->layer_funcs->init,
                       &input_layer->layer_connection,
                       iotc->context_data.connection_data, IOTC_STATE_OK),
      new_backoff, &iotc->context_data.connect_handler);

  IOTC_CHECK_STATE(state);

  return IOTC_STATE_OK;

err_handling:
  return state;
}

iotc_state_t iotc_publish_data_impl(iotc_context_handle_t iotc_h,
                                    const char* topic, iotc_data_desc_t* data,
                                    const iotc_mqtt_qos_t qos,
                                    iotc_user_callback_t* callback,
                                    void* user_data) {
  /* PRE-CONDITIONS */
  assert(IOTC_INVALID_CONTEXT_HANDLE < iotc_h);
  iotc_context_t* iotc = (iotc_context_t*)iotc_object_for_handle(
      iotc_globals.context_handles_vector, iotc_h);
  assert(NULL != iotc);

  if (NULL == callback) {
    callback = &iotc_default_client_callback;
    user_data = NULL;
  }

  iotc_event_handle_t event_handle = iotc_make_threaded_handle(
      IOTC_THREADID_THREAD_0, &iotc_user_callback_wrapper, iotc, user_data,
      IOTC_STATE_OK, (void*)callback);

  assert(IOTC_EVENT_HANDLE_ARGC4 == event_handle.handle_type ||
         IOTC_EVENT_HANDLE_UNSET == event_handle.handle_type);

  if (IOTC_BACKOFF_CLASS_NONE != iotc_globals.backoff_status.backoff_class) {
    iotc_free_desc(&data);
    return IOTC_BACKOFF_TERMINAL;
  }

  iotc_mqtt_qos_t effective_qos = qos;

  iotc_mqtt_logic_task_t* task = NULL;
  iotc_state_t state = IOTC_STATE_OK;
  iotc_layer_t* input_layer = iotc->layer_chain.top;

  iotc_mqtt_logic_layer_data_t* layer_data =
      (iotc_mqtt_logic_layer_data_t*)input_layer->user_data;

  IOTC_UNUSED(layer_data);

  task = iotc_mqtt_logic_make_publish_task(topic, data, effective_qos,
                                           (iotc_mqtt_retain_t)0, event_handle);

  IOTC_CHECK_MEMORY(task, state);

  return IOTC_PROCESS_PUSH_ON_THIS_LAYER(&input_layer->layer_connection, task,
                                         IOTC_STATE_OK);

err_handling:
  if (task) {
    iotc_mqtt_logic_free_task(&task);
  }

  return state;
}

iotc_state_t iotc_publish(iotc_context_handle_t iotc_h, const char* topic,
                          const char* msg, const iotc_mqtt_qos_t qos,
                          iotc_user_callback_t* callback, void* user_data) {
  /* PRE-CONDITIONS */
  assert(NULL != topic);
  assert(NULL != msg);

  iotc_state_t state = IOTC_STATE_OK;

  iotc_data_desc_t* data_desc = iotc_make_desc_from_string_copy(msg);

  IOTC_CHECK_MEMORY(data_desc, state);

  return iotc_publish_data_impl(iotc_h, topic, data_desc, qos, callback,
                                user_data);

err_handling:
  return state;
}

iotc_state_t iotc_publish_data(iotc_context_handle_t iotc_h, const char* topic,
                               const uint8_t* data, size_t data_len,
                               const iotc_mqtt_qos_t qos,
                               iotc_user_callback_t* callback,
                               void* user_data) {
  /* PRE-CONDITIONS */
  assert(NULL != topic);
  assert(NULL != data);
  assert(0 != data_len);

  iotc_state_t state = IOTC_STATE_OK;

  iotc_data_desc_t* data_desc = iotc_make_desc_from_buffer_copy(data, data_len);

  IOTC_CHECK_MEMORY(data_desc, state);

  return iotc_publish_data_impl(iotc_h, topic, data_desc, qos, callback,
                                user_data);

err_handling:
  return state;
}

iotc_state_t iotc_subscribe(iotc_context_handle_t iotc_h, const char* topic,
                            const iotc_mqtt_qos_t qos,
                            iotc_user_subscription_callback_t* callback,
                            void* user_data) {
  if ((IOTC_INVALID_CONTEXT_HANDLE == iotc_h) || (NULL == topic) ||
      (NULL == callback)) {
    return IOTC_INVALID_PARAMETER;
  }

  iotc_state_t state = IOTC_STATE_OK;
  iotc_mqtt_logic_task_t* task = NULL;
  char* internal_topic = NULL;
  iotc_layer_t* input_layer = NULL;
  iotc_event_handle_t event_handle = iotc_make_empty_event_handle();

  iotc_context_t* iotc = (iotc_context_t*)iotc_object_for_handle(
      iotc_globals.context_handles_vector, iotc_h);

  IOTC_CHECK_MEMORY(iotc, state);

  event_handle = iotc_make_threaded_handle(
      IOTC_THREADID_THREAD_0, &iotc_user_sub_call_wrapper, iotc, NULL,
      IOTC_STATE_OK, (void*)callback, (void*)user_data, (void*)NULL);

  if (IOTC_BACKOFF_CLASS_NONE != iotc_globals.backoff_status.backoff_class) {
    return IOTC_BACKOFF_TERMINAL;
  }

  /* Copy the topic memory */
  internal_topic = iotc_str_dup(topic);

  IOTC_CHECK_MEMORY(internal_topic, state);

  input_layer = iotc->layer_chain.top;

  event_handle.handlers.h6.a1 = iotc;

  task = iotc_mqtt_logic_make_subscribe_task(internal_topic, qos, event_handle);
  IOTC_CHECK_MEMORY(task, state);

  /* Pass the partial ownership of the task data to the handler (in case of
   * subscription failure it will release the memory.) */
  task->data.data_u->subscribe.handler.handlers.h6.a6 = task->data.data_u;

  return IOTC_PROCESS_PUSH_ON_THIS_LAYER(&input_layer->layer_connection, task,
                                         IOTC_STATE_OK);

err_handling:
  if (task) {
    iotc_mqtt_logic_free_task(&task);
  }

  IOTC_SAFE_FREE(internal_topic);

  return state;
}

iotc_state_t iotc_shutdown_connection(iotc_context_handle_t iotc_h) {
  assert(IOTC_INVALID_CONTEXT_HANDLE < iotc_h);
  iotc_context_t* itoc =
      iotc_object_for_handle(iotc_globals.context_handles_vector, iotc_h);
  assert(NULL != itoc);

  iotc_state_t state = IOTC_STATE_OK;
  iotc_layer_t* input_layer = itoc->layer_chain.top;
  iotc_mqtt_logic_task_t* task = NULL;

  /* check if connect operation has been finished */
  if (NULL == itoc->context_data.connect_handler.ptr_to_position) {
    /* check if the connection is not established for any reason */
    if (NULL == itoc->context_data.connection_data ||
        IOTC_CONNECTION_STATE_OPENED !=
            itoc->context_data.connection_data->connection_state) {
      return IOTC_SOCKET_NO_ACTIVE_CONNECTION_ERROR;
    }
  } else {
    /* If the connect operation has been scheduled but not executed. */
    state = iotc_evtd_cancel(itoc->context_data.evtd_instance,
                             &itoc->context_data.connect_handler);

    IOTC_CHECK_STATE(state);

    return IOTC_STATE_OK;
  }

  switch (itoc->context_data.shutdown_state) {
    case IOTC_SHUTDOWN_UNINITIALISED:
      itoc->context_data.shutdown_state = IOTC_SHUTDOWN_STARTED;
      break;
    case IOTC_SHUTDOWN_STARTED:
      iotc_debug_logger("IOTC_ALREADY_INITIALIZED");
      return IOTC_ALREADY_INITIALIZED;
    default:
      return IOTC_INTERNAL_ERROR;
  }

  task = iotc_mqtt_logic_make_shutdown_task();

  IOTC_CHECK_MEMORY(task, state);

  return IOTC_PROCESS_PUSH_ON_THIS_LAYER(&input_layer->layer_connection, task,
                                         IOTC_STATE_OK);

err_handling:
  if (task) {
    iotc_mqtt_logic_free_task(&task);
  }
  return state;
}

iotc_timed_task_handle_t iotc_schedule_timed_task(
    iotc_context_handle_t iotc_h, iotc_user_task_callback_t* callback,
    const iotc_time_t seconds_from_now, const uint8_t repeats_forever,
    void* data) {
  return iotc_add_timed_task(iotc_globals.timed_tasks_container,
                             iotc_globals.evtd_instance, iotc_h, callback,
                             seconds_from_now, repeats_forever, data);
}

void iotc_cancel_timed_task(iotc_timed_task_handle_t timed_task_handle) {
  iotc_remove_timed_task(iotc_globals.timed_tasks_container, timed_task_handle);
}

iotc_state_t iotc_set_maximum_heap_usage(const size_t max_bytes) {
#ifndef IOTC_MEMORY_LIMITER_ENABLED
  IOTC_UNUSED(max_bytes);
  return IOTC_NOT_SUPPORTED;
#else
  return iotc_memory_limiter_set_limit(max_bytes);
#endif
}

iotc_state_t iotc_get_heap_usage(size_t* const heap_usage) {
#ifndef IOTC_MEMORY_LIMITER_ENABLED
  IOTC_UNUSED(heap_usage);
  return IOTC_NOT_SUPPORTED;
#else
  if (NULL == heap_usage) {
    return IOTC_INVALID_PARAMETER;
  }

  *heap_usage = iotc_memory_limiter_get_allocated_space();
  return IOTC_STATE_OK;
#endif
}

#ifdef IOTC_EXPOSE_FS
iotc_state_t iotc_set_fs_functions(const iotc_fs_functions_t fs_functions) {
  /* check the size of the passed structure */
  if (sizeof(iotc_fs_functions_t) != fs_functions.fs_functions_size) {
    return IOTC_INTERNAL_ERROR;
  }

  /* check if any of function pointer is NULL */
  if (NULL == fs_functions.stat_resource) {
    return IOTC_INVALID_PARAMETER;
  } else if (NULL == fs_functions.open_resource) {
    return IOTC_INVALID_PARAMETER;
  } else if (NULL == fs_functions.read_resource) {
    return IOTC_INVALID_PARAMETER;
  } else if (NULL == fs_functions.write_resource) {
    return IOTC_INVALID_PARAMETER;
  } else if (NULL == fs_functions.remove_resource) {
    return IOTC_INVALID_PARAMETER;
  }

  /* discuss this as this may be potentially dangerous */
  memcpy(&iotc_internals.fs_functions, &fs_functions,
         sizeof(iotc_fs_functions_t));

  return IOTC_STATE_OK;
}

#endif /* IOTC_EXPOSE_FS */

#ifdef __cplusplus
}
#endif
