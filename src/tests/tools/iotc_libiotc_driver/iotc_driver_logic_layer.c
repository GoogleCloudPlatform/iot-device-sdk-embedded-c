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

#include "iotc_driver_logic_layer.h"
#include "iotc.h"
#include "iotc_control_channel_protocol.pb-c.h"
#include "iotc_debug.h"
#include "iotc_globals.h"
#include "iotc_helpers.h"
#include "iotc_layer_api.h"
#include "iotc_layer_macros.h"
#include "iotc_libiotc_driver_impl.h"
#include "iotc_types.h"

#ifdef IOTC_MULTI_LEVEL_DIRECTORY_STRUCTURE
#include "errno.h"
#include "unistd.h"
#endif

/****************************************************************************
 * libiotc callbacks ******************************************************
 ****************************************************************************/
void on_connected(iotc_context_handle_t context_handle, void* data,
                  iotc_state_t in_out_state) {
  IOTC_UNUSED(data);
  IOTC_UNUSED(in_out_state);
  IOTC_UNUSED(context_handle);

  iotc_connection_data_t* conn_data = (iotc_connection_data_t*)data;

  printf("[ driver lgc ] %s, result = %d, connection_state = %d\n", __func__,
         in_out_state, conn_data->connection_state);

  switch (conn_data->connection_state) {
    case IOTC_CONNECTION_STATE_OPENED:
    case IOTC_CONNECTION_STATE_OPEN_FAILED:
      iotc_libiotc_driver_send_on_connect_finish(libiotc_driver, in_out_state);
      break;
    case IOTC_CONNECTION_STATE_CLOSED:
      iotc_libiotc_driver_send_on_disconnect(libiotc_driver, in_out_state);
      break;
    default:
      break;
  }
}

void on_message_received(iotc_context_handle_t context_handle,
                         iotc_sub_call_type_t call_type,
                         const iotc_sub_call_params_t* const params,
                         iotc_state_t in_out_state, void* user_data) {
  IOTC_UNUSED(user_data);
  IOTC_UNUSED(params);
  IOTC_UNUSED(in_out_state);
  IOTC_UNUSED(context_handle);

  printf("[ driver lgc ] %s, state = %d, call_type = %d, user = %d\n", __func__,
         in_out_state, call_type, (int)(intptr_t)user_data);

  switch (call_type) {
    case IOTC_SUB_CALL_SUBACK:
    case IOTC_SUB_CALL_MESSAGE:
      iotc_libiotc_driver_send_on_message_received(libiotc_driver, call_type,
                                                   params, in_out_state);
      break;
    default:
      break;
  }
}

void on_publish_finish(iotc_context_handle_t context, void* data,
                       iotc_state_t in_out_state) {
  IOTC_UNUSED(context);
  IOTC_UNUSED(data);
  IOTC_UNUSED(in_out_state);

  printf("[ driver lgc ] %s, state = %d, data = %p\n", __func__, in_out_state,
         data);

  iotc_libiotc_driver_send_on_publish_finish(libiotc_driver, data,
                                             in_out_state);
}

/****************************************************************************
 * driver layer functions ***************************************************
 ****************************************************************************/
iotc_state_t iotc_driver_logic_layer_push(void* context, void* data,
                                          iotc_state_t in_out_state) {
  IOTC_LAYER_FUNCTION_PRINT_FUNCTION_DIGEST();

  if (IOTC_THIS_LAYER_NOT_OPERATIONAL(context)) {
    goto err_handling;
  }

  if (NULL == data) {
    return in_out_state;
  } else {
    return IOTC_PROCESS_PUSH_ON_PREV_LAYER(context, data, in_out_state);
  }

err_handling:
  return IOTC_PROCESS_CLOSE_ON_THIS_LAYER(context, NULL, in_out_state);
}

iotc_state_t iotc_driver_logic_layer_pull(void* context, void* data,
                                          iotc_state_t in_out_state) {
  IOTC_LAYER_FUNCTION_PRINT_FUNCTION_DIGEST();

  IOTC_UNUSED(context);

  if (IOTC_THIS_LAYER_NOT_OPERATIONAL(context) || NULL == data) {
    goto err_handling;
  }

  struct _XiClientFtestFw__XiClientAPI* message_API_call =
      (struct _XiClientFtestFw__XiClientAPI*)data;

  printf("[ driver lgc ] *** connect                      = %p\n",
         message_API_call->connect);
  printf("[ driver lgc ] *** disconnect                   = %p\n",
         message_API_call->disconnect);
  printf("[ driver lgc ] *** subscribe                    = %p\n",
         message_API_call->subscribe);
  printf("[ driver lgc ] *** publish_string               = %p\n",
         message_API_call->publish_string);
  printf("[ driver lgc ] *** publish_binary               = %p\n",
         message_API_call->publish_binary);
  printf("[ driver lgc ] *** publish_timeseries           = %p\n",
         message_API_call->publish_timeseries);
  printf("[ driver lgc ] *** publish_formatted_timeseries = %p\n",
         message_API_call->publish_formatted_timeseries);
  printf("[ driver lgc ] *** setup_tls                    = %p\n",
         message_API_call->setup_tls);

  if (NULL != message_API_call->connect) {
    printf("[ driver lgc ] --- connecting libiotc to [ %s : %d ]\n",
           message_API_call->connect->server_address->host,
           message_API_call->connect->server_address->port);

    iotc_connect_to(iotc_globals.default_context_handle,
                    message_API_call->connect->server_address->host,
                    message_API_call->connect->server_address->port,
                    message_API_call->connect->username,
                    message_API_call->connect->password,
                    DEFAULT_CLIENT_ID, /* TODO(ddb): add this to the protobuf */
                    message_API_call->connect->has_connection_timeout
                        ? message_API_call->connect->connection_timeout
                        : 10,
                    20, &on_connected);

    // use buffer allocated by protobuf
    message_API_call->connect->username = NULL;
    message_API_call->connect->password = NULL;
  } else if (NULL != message_API_call->disconnect) {
    printf("[ driver lgc ] --- disconnecting\n");
    iotc_shutdown_connection(iotc_globals.default_context_handle);
  } else if (NULL != message_API_call->subscribe) {
    printf("[ driver lgc ] --- subscribing\n");

    uint8_t topic_id = 0;
    for (; topic_id < message_API_call->subscribe->n_topic_qos_list;
         ++topic_id) {
      iotc_state_t state = iotc_subscribe(
          iotc_globals.default_context_handle,
          message_API_call->subscribe->topic_qos_list[topic_id]->topic_name,
          message_API_call->subscribe->topic_qos_list[topic_id]->qos,
          &on_message_received, (void*)(intptr_t)topic_id);

      printf("[ driver lgc ] --- subscribe state = %d\n", state);
    }
  } else if (NULL != message_API_call->publish_string) {
    printf("[ driver lgc ] --- publish string\n");

    iotc_publish(
        iotc_globals.default_context_handle,
        message_API_call->publish_string->publish_common_data->topic_name,
        message_API_call->publish_string->payload,
        message_API_call->publish_string->publish_common_data->qos,
        &on_publish_finish, NULL);

    message_API_call->publish_string->payload = NULL;
  } else if (NULL != message_API_call->publish_binary) {
    printf("[ driver lgc ] --- publish binary\n");

    iotc_publish_data(
        iotc_globals.default_context_handle,
        message_API_call->publish_binary->publish_common_data->topic_name,
        message_API_call->publish_binary->payload.data,
        message_API_call->publish_binary->payload.len,
        message_API_call->publish_binary->publish_common_data->qos,
        &on_publish_finish, NULL);

    message_API_call->publish_binary->has_payload = 0;
    message_API_call->publish_binary->payload.data = NULL;
    message_API_call->publish_binary->payload.len = 0;
  } else if (NULL != message_API_call->setup_tls) {
#ifdef IOTC_MULTI_LEVEL_DIRECTORY_STRUCTURE
    const int16_t cwd_buffer_size = 512;
    char cwd_buffer[512] = {0};

    if (NULL == getcwd(cwd_buffer, cwd_buffer_size)) {
      iotc_debug_format(
          "[ driver lgc ] ERROR: could not get current working "
          "directory, errno = %d",
          errno);
      in_out_state = IOTC_INTERNAL_ERROR;
      goto err_handling;
    }

    size_t cwd_length = strlen(cwd_buffer);
    const char* libiotc_cwds = "/libiotc_cwds/";

    iotc_str_copy_untiln(cwd_buffer + cwd_length, cwd_buffer_size - cwd_length,
                         libiotc_cwds, '\0');

    cwd_length += strlen(libiotc_cwds);

    iotc_str_copy_untiln(cwd_buffer + cwd_length, cwd_buffer_size - cwd_length,
                         message_API_call->setup_tls->ca_cert_file
                             ? message_API_call->setup_tls->ca_cert_file
                             : "",
                         '.');

    if (0 != chdir(cwd_buffer)) {
      iotc_debug_format(
          "[ driver lgc ] ERROR: could not change dir to %s, errno = %d",
          cwd_buffer, errno);
      in_out_state = IOTC_INTERNAL_ERROR;
      goto err_handling;
    }

    iotc_debug_format(
        "[ driver lgc ] successfully changed current working directory to %s",
        cwd_buffer);
#endif
  } else {
    printf(
        "[ driver lgc ] ERROR: unrecognized control channel message, "
        "disconnecting\n");
    iotc_shutdown_connection(iotc_globals.default_context_handle);
  }

  iotc_client_ftest_fw__iotc_client_api__free_unpacked(message_API_call, NULL);

  return in_out_state;

err_handling:

  return IOTC_PROCESS_CLOSE_ON_THIS_LAYER(context, NULL, in_out_state);
}

iotc_state_t iotc_driver_logic_layer_close(void* context, void* data,
                                           iotc_state_t in_out_state) {
  IOTC_LAYER_FUNCTION_PRINT_FUNCTION_DIGEST();

  return IOTC_PROCESS_CLOSE_ON_PREV_LAYER(context, data, in_out_state);
}

iotc_state_t iotc_driver_logic_layer_close_externally(
    void* context, void* data, iotc_state_t in_out_state) {
  IOTC_LAYER_FUNCTION_PRINT_FUNCTION_DIGEST();

  IOTC_UNUSED(data);
  IOTC_UNUSED(in_out_state);

  iotc_evtd_stop(IOTC_CONTEXT_DATA(context)->evtd_instance);

  return IOTC_STATE_OK;
}

iotc_state_t iotc_driver_logic_layer_init(void* context, void* data,
                                          iotc_state_t in_out_state) {
  IOTC_LAYER_FUNCTION_PRINT_FUNCTION_DIGEST();

  return IOTC_PROCESS_INIT_ON_PREV_LAYER(context, data, in_out_state);
}

iotc_state_t iotc_driver_logic_layer_connect(void* context, void* data,
                                             iotc_state_t in_out_state) {
  IOTC_LAYER_FUNCTION_PRINT_FUNCTION_DIGEST();

  IOTC_UNUSED(context);
  IOTC_UNUSED(data);

  return in_out_state;
}
