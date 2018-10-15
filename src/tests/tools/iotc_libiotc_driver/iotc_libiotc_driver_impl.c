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

#include "iotc_libiotc_driver_impl.h"
#include "iotc_connection_data_internal.h"
#include "iotc_control_channel_protocol.pb-c.h"
#include "iotc_driver_control_channel_layerchain.h"
#include "iotc_globals.h"

iotc_libiotc_driver_t* libiotc_driver = NULL;

// todo: move this function predeclaration to an internal header
iotc_state_t iotc_create_context_with_custom_layers_and_evtd(
    iotc_context_t** context, iotc_layer_type_t layer_config[],
    iotc_layer_type_id_t layer_chain[], size_t layer_chain_size,
    iotc_evtd_instance_t* event_dispatcher);

iotc_state_t iotc_delete_context_with_custom_layers(
    iotc_context_t** context, iotc_layer_type_t layer_config[],
    size_t layer_chain_size);

iotc_state_t iotc_driver_free_protobuf_callback(
    struct _XiClientFtestFw__XiClientCallback* callback);

iotc_libiotc_driver_t* iotc_libiotc_driver_create_instance() {
  iotc_state_t state = IOTC_STATE_OK;

  IOTC_ALLOC(iotc_libiotc_driver_t, driver, state);

  driver->evtd_instance = iotc_evtd_create_instance();

  IOTC_CHECK_CND_DBGMESSAGE(
      driver->evtd_instance == NULL, IOTC_OUT_OF_MEMORY, state,
      "could not instantiate event dispatcher for libiotc driver");

  IOTC_CHECK_STATE(iotc_create_context_with_custom_layers_and_evtd(
      &driver->context, iotc_driver_control_channel_layerchain,
      IOTC_LAYER_CHAIN_DRIVER_CONTROL_CHANNEL,
      IOTC_LAYER_CHAIN_SCHEME_LENGTH(IOTC_LAYER_CHAIN_DRIVER_CONTROL_CHANNEL),
      driver->evtd_instance));

  IOTC_CHECK_CND_DBGMESSAGE(driver->context == NULL, IOTC_OUT_OF_MEMORY, state,
                            "could not instantiate context for libiotc driver");

  return driver;

err_handling:

  IOTC_SAFE_FREE(driver);
  return NULL;
}

iotc_state_t iotc_libiotc_driver_destroy_instance(
    iotc_libiotc_driver_t** driver) {
  iotc_delete_context_with_custom_layers(
      &(*driver)->context, iotc_driver_control_channel_layerchain,
      IOTC_LAYER_CHAIN_SCHEME_LENGTH(IOTC_LAYER_CHAIN_DRIVER_CONTROL_CHANNEL));

  iotc_evtd_destroy_instance((*driver)->evtd_instance);

  IOTC_SAFE_FREE(*driver);

  return IOTC_STATE_OK;
}

iotc_state_t iotc_libiotc_driver_connect_with_callback(
    iotc_libiotc_driver_t* driver, const char* const host, uint16_t port,
    iotc_libiotc_driver_callback_function_t on_connected) {
  IOTC_UNUSED(on_connected);

  if (driver == NULL || driver->context == NULL) {
    return IOTC_INVALID_PARAMETER;
  }

  driver->context->context_data.connection_data = iotc_alloc_connection_data(
      host, port, DEFAULT_PROJECT_ID, DEFAULT_DEVICE_PATH, &DEFAULT_PRIVATE_KEY,
      DEFAULT_JWT_EXPIRATION,
      /*connection_timeout=*/10, /*keepalive_timeout=*/20, IOTC_SESSION_CLEAN);

  iotc_layer_t* input_layer = driver->context->layer_chain.top;

  iotc_evtd_execute(
      driver->context->context_data.evtd_instance,
      iotc_make_handle(input_layer->layer_connection.self->layer_funcs->init,
                       &input_layer->layer_connection,
                       driver->context->context_data.connection_data,
                       IOTC_STATE_OK));

  return IOTC_STATE_OK;
}

#define IOTC_DRIVER_PROTOBUF_CREATE_DEFAULT_ON_HEAP(type, out_variable_name,  \
                                                    stack_init, state)        \
  struct type* out_variable_name = NULL;                                      \
  {                                                                           \
    IOTC_ALLOC_AT(struct type, out_variable_name, state);                     \
                                                                              \
    struct type out_variable_name_local = stack_init;                         \
                                                                              \
    memcpy(out_variable_name, &out_variable_name_local, sizeof(struct type)); \
  }

iotc_state_t iotc_libiotc_driver_send_on_connect_finish(
    iotc_libiotc_driver_t* driver, iotc_state_t connect_result) {
  printf("[ driver     ] %s, state = %d\n", __func__, connect_result);

  iotc_state_t state = IOTC_STATE_OK;

  IOTC_DRIVER_PROTOBUF_CREATE_DEFAULT_ON_HEAP(
      _XiClientFtestFw__XiClientCallback, client_callback,
      IOTC_CLIENT_FTEST_FW__IOTC_CLIENT_CALLBACK__INIT, state);

  IOTC_CHECK_CND_DBGMESSAGE(driver == NULL, IOTC_INTERNAL_ERROR, state,
                            "NULL driver pointer");

  IOTC_DRIVER_PROTOBUF_CREATE_DEFAULT_ON_HEAP(
      _XiClientFtestFw__XiClientCallback__OnConnectFinish, on_connect_finish,
      IOTC_CLIENT_FTEST_FW__IOTC_CLIENT_CALLBACK__ON_CONNECT_FINISH__INIT,
      state);

  on_connect_finish->has_connect_result = 1;
  on_connect_finish->connect_result = connect_result;

  client_callback->on_connect_finish = on_connect_finish;

  iotc_layer_t* input_layer = driver->context->layer_chain.top;

  iotc_evtd_execute(
      driver->context->context_data.evtd_instance,
      iotc_make_handle(input_layer->layer_connection.self->layer_funcs->push,
                       &input_layer->layer_connection, client_callback,
                       IOTC_STATE_OK));

  return IOTC_STATE_OK;

err_handling:

  iotc_driver_free_protobuf_callback(client_callback);
  return state;
}

iotc_state_t iotc_libiotc_driver_send_on_disconnect(
    iotc_libiotc_driver_t* driver, iotc_state_t error_code) {
  printf("[ driver     ] %s, state = %d\n", __func__, error_code);

  iotc_state_t state = IOTC_STATE_OK;

  IOTC_DRIVER_PROTOBUF_CREATE_DEFAULT_ON_HEAP(
      _XiClientFtestFw__XiClientCallback, client_callback,
      IOTC_CLIENT_FTEST_FW__IOTC_CLIENT_CALLBACK__INIT, state);

  IOTC_DRIVER_PROTOBUF_CREATE_DEFAULT_ON_HEAP(
      _XiClientFtestFw__XiClientCallback__OnDisconnect, on_disconnect,
      IOTC_CLIENT_FTEST_FW__IOTC_CLIENT_CALLBACK__ON_DISCONNECT__INIT, state);

  on_disconnect->has_error_code = 1;
  on_disconnect->error_code = error_code;

  client_callback->on_disconnect = on_disconnect;

  iotc_layer_t* input_layer = driver->context->layer_chain.top;

  iotc_evtd_execute(
      driver->context->context_data.evtd_instance,
      iotc_make_handle(input_layer->layer_connection.self->layer_funcs->push,
                       &input_layer->layer_connection, client_callback,
                       IOTC_STATE_OK));

  return IOTC_STATE_OK;

err_handling:

  iotc_driver_free_protobuf_callback(client_callback);
  return state;
}

iotc_state_t iotc_libiotc_driver_send_on_message_received(
    iotc_libiotc_driver_t* driver, iotc_sub_call_type_t call_type,
    const iotc_sub_call_params_t* const params, iotc_state_t receive_result) {
  printf("[ driver     ] %s, state = %d\n", __func__, receive_result);

  iotc_state_t state = IOTC_STATE_OK;
  iotc_layer_t* input_layer = NULL;

  IOTC_DRIVER_PROTOBUF_CREATE_DEFAULT_ON_HEAP(
      _XiClientFtestFw__XiClientCallback, client_callback,
      IOTC_CLIENT_FTEST_FW__IOTC_CLIENT_CALLBACK__INIT, state);

  switch (call_type) {
    case IOTC_SUB_CALL_SUBACK: {
      IOTC_DRIVER_PROTOBUF_CREATE_DEFAULT_ON_HEAP(
          _XiClientFtestFw__XiClientCallback__OnSubscribeFinish,
          on_subscribe_finish,
          IOTC_CLIENT_FTEST_FW__IOTC_CLIENT_CALLBACK__ON_SUBSCRIBE_FINISH__INIT,
          state);

      on_subscribe_finish->n_subscribe_result_list = 1;

      IOTC_ALLOC_BUFFER_AT(uint32_t, on_subscribe_finish->subscribe_result_list,
                           sizeof(uint32_t), state);

      on_subscribe_finish->subscribe_result_list[0] =
          params->suback.suback_status;

      client_callback->on_subscribe_finish = on_subscribe_finish;
    } break;
    case IOTC_SUB_CALL_MESSAGE: {
      printf("--- topic_name = %s\n", params->message.topic);
      printf("--- qos = %d\n", params->message.qos);
      printf("--- content length = %ld, content = %s\n",
             params->message.temporary_payload_data_length,
             params->message.temporary_payload_data);

      IOTC_DRIVER_PROTOBUF_CREATE_DEFAULT_ON_HEAP(
          _XiClientFtestFw__XiClientCallback__OnMessageReceived,
          on_message_received,
          IOTC_CLIENT_FTEST_FW__IOTC_CLIENT_CALLBACK__ON_MESSAGE_RECEIVED__INIT,
          state);

      on_message_received->topic_name = (char*)params->message.topic;
      on_message_received->has_qos = 1;
      on_message_received->qos = params->message.qos;
      on_message_received->has_payload = 1;

      /* here we have to copy the pointer data */
      IOTC_ALLOC_BUFFER_AT(uint8_t, on_message_received->payload.data,
                           params->message.temporary_payload_data_length,
                           state);
      memcpy(on_message_received->payload.data,
             params->message.temporary_payload_data,
             params->message.temporary_payload_data_length);
      on_message_received->payload.len =
          params->message.temporary_payload_data_length;

      client_callback->on_message_received = on_message_received;
    } break;
    case IOTC_SUB_CALL_UNKNOWN:
    default:
      goto err_handling;
  }

  input_layer = driver->context->layer_chain.top;

  iotc_evtd_execute(
      driver->context->context_data.evtd_instance,
      iotc_make_handle(input_layer->layer_connection.self->layer_funcs->push,
                       &input_layer->layer_connection, client_callback,
                       IOTC_STATE_OK));

  return IOTC_STATE_OK;

err_handling:

  iotc_driver_free_protobuf_callback(client_callback);
  return state;
}

iotc_state_t iotc_libiotc_driver_send_on_publish_finish(
    iotc_libiotc_driver_t* driver, void* data, iotc_state_t publish_result) {
  IOTC_UNUSED(data);

  printf("[ driver     ] %s, state = %d\n", __func__, publish_result);

  iotc_state_t state = IOTC_STATE_OK;

  IOTC_DRIVER_PROTOBUF_CREATE_DEFAULT_ON_HEAP(
      _XiClientFtestFw__XiClientCallback, client_callback,
      IOTC_CLIENT_FTEST_FW__IOTC_CLIENT_CALLBACK__INIT, state);

  IOTC_DRIVER_PROTOBUF_CREATE_DEFAULT_ON_HEAP(
      _XiClientFtestFw__XiClientCallback__OnPublishFinish, on_publish_finish,
      IOTC_CLIENT_FTEST_FW__IOTC_CLIENT_CALLBACK__ON_PUBLISH_FINISH__INIT,
      state);

  on_publish_finish->has_return_code = 1;
  on_publish_finish->return_code = publish_result;

  client_callback->on_publish_finish = on_publish_finish;

  iotc_layer_t* input_layer = driver->context->layer_chain.top;

  iotc_evtd_execute(
      driver->context->context_data.evtd_instance,
      iotc_make_handle(input_layer->layer_connection.self->layer_funcs->push,
                       &input_layer->layer_connection, client_callback,
                       IOTC_STATE_OK));

  return IOTC_STATE_OK;

err_handling:

  iotc_driver_free_protobuf_callback(client_callback);
  return state;
}
