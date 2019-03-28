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

#ifndef __IOTC_MQTT_LOGIC_LAYER_PUBLISH_Q0_COMMAND_H__
#define __IOTC_MQTT_LOGIC_LAYER_PUBLISH_Q0_COMMAND_H__

#include "iotc_coroutine.h"
#include "iotc_globals.h"
#include "iotc_layer_api.h"
#include "iotc_mqtt_logic_layer_data.h"
#include "iotc_mqtt_logic_layer_data_helpers.h"
#include "iotc_mqtt_logic_layer_task_helpers.h"
#include "iotc_mqtt_message.h"

#ifdef __cplusplus
extern "C" {
#endif

static inline iotc_state_t do_mqtt_publish_q0(
    void* ctx, /* Should be the context of the logic layer. */
    void* data, iotc_state_t state, void* msg_data) {
  IOTC_UNUSED(state);
  IOTC_UNUSED(msg_data);

  /* PRECONDITIONS */
  assert(NULL == msg_data);

  iotc_layer_connectivity_t* context = (iotc_layer_connectivity_t*)ctx;
  iotc_mqtt_logic_task_t* task = (iotc_mqtt_logic_task_t*)data;

  assert(NULL != task);
  assert(NULL != context);

  iotc_mqtt_logic_layer_data_t* layer_data =
      (iotc_mqtt_logic_layer_data_t*)IOTC_THIS_LAYER(context)->user_data;

  iotc_mqtt_message_t* msg_memory = NULL;
  iotc_state_t callback_state = IOTC_STATE_OK;

  /* Means that we are shutting down. */
  if (IOTC_THIS_LAYER_NOT_OPERATIONAL(context) || NULL == layer_data) {
    return IOTC_STATE_OK;
  }

  IOTC_CR_START(task->cs);

  iotc_debug_logger("publish preparing message...");

  IOTC_ALLOC_AT(iotc_mqtt_message_t, msg_memory, state);

  IOTC_CHECK_STATE(
      state = fill_with_publish_data(
          msg_memory, task->data.data_u->publish.topic,
          task->data.data_u->publish.data, IOTC_MQTT_QOS_AT_MOST_ONCE,
          task->data.data_u->publish.retain, IOTC_MQTT_DUP_FALSE, 0));

  iotc_debug_logger("publish sending message...");

  /* Wait till it is sent. */
  IOTC_CR_YIELD(task->cs, IOTC_PROCESS_PUSH_ON_PREV_LAYER(context, msg_memory,
                                                          IOTC_STATE_OK));

  callback_state = state;

  if (IOTC_STATE_WRITTEN == state) {
    callback_state = IOTC_STATE_OK;
    iotc_debug_logger("publish message has been sent...");
  } else {
    iotc_debug_logger("publish message has not been sent...");
  }

  iotc_mqtt_logic_free_task_data(task);

  iotc_mqtt_logic_task_defer_users_callback(context, task, callback_state);

  IOTC_CR_EXIT(task->cs, iotc_mqtt_logic_layer_finalize_task(context, task));

err_handling:
  iotc_mqtt_logic_task_defer_users_callback(context, task, state);

  iotc_mqtt_logic_free_task_data(task);

  IOTC_CR_EXIT(task->cs, iotc_mqtt_logic_layer_finalize_task(context, task));

  IOTC_CR_END();
}

#ifdef __cplusplus
}
#endif

#endif /* __IOTC_MQTT_LOGIC_LAYER_PUBLISH_Q0_COMMAND_H__ */
