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

#ifndef __IOTC_MQTT_LOGIC_LAYER_CONNECTION_HELPERS_H__
#define __IOTC_MQTT_LOGIC_LAYER_CONNECTION_HELPERS_H__

#include "iotc_layer_api.h"
#include "iotc_mqtt_logic_layer_data.h"

#ifdef __cplusplus
extern "C" {
#endif

static inline iotc_state_t do_reconnect(void* context, void* data,
                                        iotc_state_t in_out_state) {
  return IOTC_PROCESS_CLOSE_ON_PREV_LAYER(context, data, in_out_state);
}

static inline iotc_state_t do_shutdown(
    void* ctx /* Should be the context of the logic layer. */
    ,
    void* data, iotc_state_t state, void* msg_data) {
  iotc_layer_connectivity_t* context = (iotc_layer_connectivity_t*)ctx;
  iotc_mqtt_logic_task_t* task = (iotc_mqtt_logic_task_t*)data;
  iotc_mqtt_message_t* msg_memory = (iotc_mqtt_message_t*)msg_data;

  iotc_mqtt_logic_layer_data_t* layer_data =
      (iotc_mqtt_logic_layer_data_t*)IOTC_THIS_LAYER(context)->user_data;

  if (layer_data == 0) {
    /* Means that the layer has been shutted down. */
    iotc_mqtt_message_free(&msg_memory);
    return IOTC_STATE_OK;
  }

  IOTC_UNUSED(state);

  assert(task != 0);

  IOTC_CR_START(task->cs);

  IOTC_ALLOC_AT(iotc_mqtt_message_t, msg_memory, state);

  IOTC_CHECK_STATE(state = fill_with_disconnect_data(msg_memory));

  iotc_debug_logger("message memory filled with disconnect data");

  IOTC_CR_YIELD(task->cs, IOTC_PROCESS_PUSH_ON_PREV_LAYER(context, msg_memory,
                                                          IOTC_STATE_OK));

  if (state == IOTC_STATE_WRITTEN) {
    iotc_debug_logger(
        "disconnect message has been sent "
        "continue with shutting down");
  } else {
    iotc_debug_logger(
        "disconnect message has not been "
        "sent continue with shutting down");
  }

  IOTC_SAFE_FREE(task->data.data_u);

  /* Let's close. */
  iotc_mqtt_logic_layer_close(ctx, 0, IOTC_STATE_OK);

  IOTC_CR_EXIT(task->cs, iotc_mqtt_logic_layer_finalize_task(context, task));

  IOTC_CR_END();

err_handling:
  return state;
}

#ifdef __cplusplus
}
#endif

#endif /* __IOTC_MQTT_LOGIC_LAYER_CONNECTION_HELPERS_H__ */
