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

#include "iotc_control_topic_layer.h"
#include "iotc_event_handle.h"
#include "iotc_event_thread_dispatcher.h"
#include "iotc_globals.h"
#include "iotc_layer_api.h"
#include "iotc_layer_macros.h"
#include "iotc_list.h"
#include "iotc_macros.h"
#include "iotc_mqtt_logic_layer_data.h"
#include "iotc_mqtt_message.h"
#include "iotc_types_internal.h"

#ifdef __cplusplus
extern "C" {
#endif
  
iotc_state_t iotc_control_topic_connection_state_changed(void* context,
                                                         iotc_state_t state);

iotc_state_t iotc_control_topic_connection_state_changed(void* context,
                                                         iotc_state_t state) {
  iotc_debug_printf("%s\n", __FUNCTION__);

  IOTC_CONTEXT_DATA(context)->connection_callback.handlers.h3.a2 =
      IOTC_CONTEXT_DATA(context)->connection_data;

  IOTC_CONTEXT_DATA(context)->connection_callback.handlers.h3.a3 = state;

  iotc_evttd_execute(IOTC_CONTEXT_DATA(context)->evtd_instance,
                     IOTC_CONTEXT_DATA(context)->connection_callback);

  if (state == IOTC_STATE_OK &&
      IOTC_CONTEXT_DATA(context)->connection_data->connection_state ==
          IOTC_CONNECTION_STATE_OPENED) {
    IOTC_PROCESS_POST_CONNECT_ON_THIS_LAYER(context, NULL, state);
  }

  return state;
}

iotc_state_t iotc_control_topic_layer_push(void* context, void* data,
                                           iotc_state_t in_out_state) {
  IOTC_LAYER_FUNCTION_PRINT_FUNCTION_DIGEST();

  IOTC_UNUSED(data);

  return IOTC_PROCESS_PUSH_ON_PREV_LAYER(context, data, in_out_state);
}

iotc_state_t iotc_control_topic_layer_pull(void* context, void* data,
                                           iotc_state_t in_out_state) {
  IOTC_LAYER_FUNCTION_PRINT_FUNCTION_DIGEST();

  IOTC_UNUSED(data);
  IOTC_UNUSED(context);

  return in_out_state;
}

iotc_state_t iotc_control_topic_layer_init(void* context, void* data,
                                           iotc_state_t in_out_state) {
  IOTC_LAYER_FUNCTION_PRINT_FUNCTION_DIGEST();

  iotc_debug_logger("control topic layer initializing.. ");

  return IOTC_PROCESS_INIT_ON_PREV_LAYER(context, data, in_out_state);
}

iotc_state_t iotc_control_topic_layer_connect(void* context, void* data,
                                              iotc_state_t in_out_state) {
  IOTC_LAYER_FUNCTION_PRINT_FUNCTION_DIGEST();

  IOTC_UNUSED(data);

  if (IOTC_STATE_OK == in_out_state) {
    return iotc_control_topic_connection_state_changed(context, in_out_state);
  }

  return IOTC_PROCESS_CLOSE_ON_PREV_LAYER(context, 0, in_out_state);
}

iotc_state_t iotc_control_topic_layer_close(void* context, void* data,
                                            iotc_state_t in_out_state) {
  IOTC_LAYER_FUNCTION_PRINT_FUNCTION_DIGEST();

  IOTC_UNUSED(data);

  /* perform complete shutdown */
  IOTC_CONTEXT_DATA(context)->connection_data->connection_state =
      IOTC_CONNECTION_STATE_CLOSING;

  return IOTC_PROCESS_CLOSE_ON_PREV_LAYER(context, data, in_out_state);
}

iotc_state_t iotc_control_topic_layer_close_externally(
    void* context, void* data, iotc_state_t in_out_state) {
  IOTC_LAYER_FUNCTION_PRINT_FUNCTION_DIGEST();

  IOTC_UNUSED(data);

  if (IOTC_CONTEXT_DATA(context)->connection_data->connection_state ==
      IOTC_CONNECTION_STATE_OPENING) {
    IOTC_CONTEXT_DATA(context)->connection_data->connection_state =
        IOTC_CONNECTION_STATE_OPEN_FAILED;
  } else {
    IOTC_CONTEXT_DATA(context)->connection_data->connection_state =
        IOTC_CONNECTION_STATE_CLOSED;
  }

  /* call the connection callback to notify the user */
  return iotc_control_topic_connection_state_changed(context, in_out_state);
}

#ifdef __cplusplus
}
#endif
