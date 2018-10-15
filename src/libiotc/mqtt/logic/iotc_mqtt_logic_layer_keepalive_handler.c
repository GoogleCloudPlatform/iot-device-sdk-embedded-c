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

#include "iotc_mqtt_logic_layer_keepalive_handler.h"
#include "iotc_coroutine.h"
#include "iotc_globals.h"
#include "iotc_layer_api.h"
#include "iotc_mqtt_logic_layer.h"
#include "iotc_mqtt_logic_layer_data.h"
#include "iotc_mqtt_logic_layer_helpers.h"

#ifdef __cplusplus
extern "C" {
#endif

iotc_state_t do_mqtt_keepalive_once(void* data) {
  iotc_layer_connectivity_t* context = data;
  iotc_state_t state = IOTC_STATE_OK;

  iotc_mqtt_logic_layer_data_t* layer_data =
      (iotc_mqtt_logic_layer_data_t*)IOTC_THIS_LAYER(context)->user_data;

  if (layer_data == 0) {
    return IOTC_STATE_OK;
  }

  layer_data->keepalive_event.ptr_to_position = NULL;

  IOTC_ALLOC(iotc_mqtt_logic_task_t, task, state);

  task->data.mqtt_settings.scenario = IOTC_MQTT_KEEPALIVE;
  task->data.mqtt_settings.qos = IOTC_MQTT_QOS_AT_MOST_ONCE;

  iotc_debug_logger("do_mqtt_keepalive_once");

  return iotc_mqtt_logic_layer_push(context, task, IOTC_STATE_OK);

err_handling:
  IOTC_SAFE_FREE(task);
  return state;
}

iotc_state_t do_mqtt_keepalive_task(void* ctx, void* data, iotc_state_t state,
                                    void* msg_data) {
  iotc_layer_connectivity_t* context = ctx;
  iotc_mqtt_logic_task_t* task = data;
  iotc_mqtt_message_t* msg_memory = msg_data;
  iotc_evtd_instance_t* event_dispatcher =
      IOTC_CONTEXT_DATA(context)->evtd_instance;

  iotc_mqtt_logic_layer_data_t* layer_data =
      (iotc_mqtt_logic_layer_data_t*)IOTC_THIS_LAYER(context)->user_data;

  if (layer_data == 0) {
    iotc_mqtt_message_free(&msg_memory);
    return IOTC_STATE_OK;
  }

  assert(task != 0);

  IOTC_CR_START(task->cs);

  IOTC_ALLOC_AT(iotc_mqtt_message_t, msg_memory, state);

  IOTC_CHECK_STATE(state = fill_with_pingreq_data(msg_memory));

  iotc_debug_logger("message memory filled with pingreq data");

  IOTC_CR_YIELD(task->cs, IOTC_PROCESS_PUSH_ON_PREV_LAYER(context, msg_memory,
                                                          IOTC_STATE_OK));

  if (state == IOTC_STATE_WRITTEN) {
    iotc_debug_logger("pingreq message sent... waiting for response");
  } else {
    iotc_debug_format("pingreq message has not been sent... %d", (int)state);
  }

  /* Wait for an interval of keepalive. */
  {
    assert(NULL == task->timeout.ptr_to_position);

    state = iotc_evtd_execute_in(
        event_dispatcher,
        iotc_make_handle(&on_keepalive_timeout_expiry, context, task, state,
                         msg_memory),
        IOTC_CONTEXT_DATA(context)->connection_data->keepalive_timeout,
        &task->timeout);

    IOTC_CHECK_STATE(state);

    /* For a message. */
    IOTC_CR_YIELD(task->cs, IOTC_STATE_OK);
  }

  if (state == IOTC_STATE_TIMEOUT) {
    iotc_debug_logger("keepalive timeout passed!");
    assert(NULL == task->timeout.ptr_to_position);
    IOTC_CR_EXIT(task->cs, do_reconnect(context, 0, IOTC_STATE_TIMEOUT));
  } else if (state != IOTC_STATE_OK) {
    iotc_debug_logger("error while waiting for pingresp!");
    cancel_task_timeout(task, context);
    IOTC_CR_EXIT(task->cs, iotc_mqtt_logic_layer_finalize_task(context, task));
  }

  cancel_task_timeout(task, context);

  if (msg_memory->common.common_u.common_bits.type == IOTC_MQTT_TYPE_PINGRESP) {
    iotc_debug_logger("PINGRESP received...");
  } else {
    iotc_debug_format("PINGRESP expected got: %d",
                      msg_memory->common.common_u.common_bits.type);
    state = IOTC_MQTT_LOGIC_WRONG_MESSAGE_RECEIVED;
    goto err_handling;
  }

  /* Only if it is connected. */
  if (IOTC_CONTEXT_DATA(context)->connection_data->connection_state ==
      IOTC_CONNECTION_STATE_OPENED) {
    state = iotc_evtd_execute_in(
        event_dispatcher, iotc_make_handle(&do_mqtt_keepalive_once, context),
        IOTC_CONTEXT_DATA(context)->connection_data->keepalive_timeout,
        &layer_data->keepalive_event);
    IOTC_CHECK_STATE(state);
  }

  iotc_mqtt_message_free(&msg_memory);

  IOTC_CR_EXIT(task->cs, iotc_mqtt_logic_layer_finalize_task(context, task));

  IOTC_CR_END();

err_handling:
  iotc_mqtt_message_free(&msg_memory);
  IOTC_CR_RESET(task->cs);
  iotc_mqtt_logic_layer_finalize_task(context, task);

  return state;
}

#ifdef __cplusplus
}
#endif
