/* Copyright 2018-2019 Google LLC
 *
 * This is part of the Google Cloud IoT Device SDK for Embedded C,
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

#ifndef __IOTC_MQTT_LOGIC_LAYER_PUBLISH_Q1_COMMAND_H__
#define __IOTC_MQTT_LOGIC_LAYER_PUBLISH_Q1_COMMAND_H__

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

static iotc_state_t do_mqtt_publish_q1(
    void* ctx /* Should be the context of the logic layer. */
    ,
    void* data, iotc_state_t state, void* msg_data) {
  iotc_layer_connectivity_t* context = (iotc_layer_connectivity_t*)ctx;
  iotc_mqtt_logic_task_t* task = (iotc_mqtt_logic_task_t*)data;

  /* Pre-conditions */
  assert(NULL != context);
  assert(NULL != task);

  iotc_mqtt_logic_layer_data_t* layer_data =
      (iotc_mqtt_logic_layer_data_t*)IOTC_THIS_LAYER(context)->user_data;

  iotc_evtd_instance_t* event_dispatcher =
      IOTC_CONTEXT_DATA(context)->evtd_instance;
  iotc_mqtt_message_t* msg_memory = (iotc_mqtt_message_t*)msg_data;

  if (IOTC_THIS_LAYER_NOT_OPERATIONAL(context) || NULL == layer_data) {
    cancel_task_timeout(task, context);

    iotc_mqtt_message_free(&msg_memory);

    IOTC_CR_RESET(task->cs);
    return IOTC_STATE_OK;
  }

  IOTC_CR_START(task->cs);

  assert(NULL == task->timeout.ptr_to_position);

  do {
    iotc_debug_format("[m.id[%d]]publish q1 preparing message", task->msg_id);

    IOTC_ALLOC_AT(iotc_mqtt_message_t, msg_memory, state);

    /* Note on memory - here the data ptr's are shared, so no data copy. */
    IOTC_CHECK_STATE(state = fill_with_publish_data(
                         msg_memory, task->data.data_u->publish.topic,
                         task->data.data_u->publish.data,
                         IOTC_MQTT_QOS_AT_LEAST_ONCE,
                         task->data.data_u->publish.retain,
                         state == IOTC_STATE_RESEND ? IOTC_MQTT_DUP_TRUE
                                                    : IOTC_MQTT_DUP_FALSE,
                         task->msg_id));

    iotc_debug_format("[m.id[%d]]publish q1 sending message", task->msg_id);

    IOTC_CR_YIELD(task->cs, IOTC_PROCESS_PUSH_ON_PREV_LAYER(context, msg_memory,
                                                            IOTC_STATE_OK));

    if (state == IOTC_STATE_WRITTEN) {
      iotc_debug_format("[m.id[%d]]publish q1 has been sent", task->msg_id);
      task->session_state =
          task->session_state == IOTC_MQTT_LOGIC_TASK_SESSION_UNSET
              ? IOTC_MQTT_LOGIC_TASK_SESSION_STORE
              : task->session_state;
    } else {
      iotc_debug_format("[m.id[%d]]publish q1 has not been sent", task->msg_id);
      state = IOTC_STATE_RESEND;
      continue;
    }

    /* Add a timeout for waiting for the response. */
    assert(NULL == task->timeout.ptr_to_position);

    /* @TODO change it to use the defined timeout. */
    if (IOTC_CONTEXT_DATA(context)->connection_data->keepalive_timeout > 0) {
      state = iotc_evtd_execute_in(
          event_dispatcher,
          iotc_make_handle(&do_mqtt_publish_q1, context, task,
                           IOTC_STATE_TIMEOUT, NULL),
          IOTC_CONTEXT_DATA(context)->connection_data->keepalive_timeout,
          &task->timeout);
      IOTC_CHECK_STATE(state);
    }

    /* Wait for the puback. */
    IOTC_CR_YIELD(task->cs, IOTC_STATE_OK);

    if (IOTC_STATE_TIMEOUT == state) {
      iotc_debug_format("[m.id[%d]]publish q1 timeout occured", task->msg_id);

      /* Clear if it was timeout. */
      assert(NULL == task->timeout.ptr_to_position);

      /* Let's change the actual state to resend as the coroutine has to resend
       * the message. */
      state = IOTC_STATE_RESEND;
    } else {
      cancel_task_timeout(task, context);
    }

    if (state == IOTC_STATE_RESEND) {
      iotc_debug_format("[m.id[%d]]publish q1 resend", task->msg_id);
    }

    /* Post-loop condition. */
    assert(NULL == task->timeout.ptr_to_position);

  } while (IOTC_STATE_RESEND == state);
  /* Try to send the message until timeout occurs. */

  assert(NULL == task->timeout.ptr_to_position);

  if (IOTC_STATE_OK != state) {
    iotc_debug_format("[m.id[%d]]publish q1 error while waiting for PUBACK",
                      task->msg_id);
    goto err_handling;
  }

  assert(NULL != msg_memory);

  if (msg_memory->common.common_u.common_bits.type != IOTC_MQTT_TYPE_PUBACK) {
    iotc_debug_format("[m.id[%d]]publish q1 error was expecting puback got %d!",
                      task->msg_id,
                      msg_memory->common.common_u.common_bits.type);

    state = IOTC_MQTT_LOGIC_WRONG_MESSAGE_RECEIVED;
    goto err_handling;
  }

  iotc_debug_format("[m.id[%d]]publish q1 publish puback received",
                    task->msg_id);

  iotc_mqtt_logic_task_defer_users_callback(context, task, state);

  iotc_mqtt_message_free(&msg_memory);

  iotc_mqtt_logic_free_task_data(task);

  IOTC_CR_EXIT(task->cs, iotc_mqtt_logic_layer_finalize_task(context, task));

  IOTC_CR_END();

err_handling:
  iotc_mqtt_logic_task_defer_users_callback(context, task, state);

  iotc_mqtt_message_free(&msg_memory);

  if (task->data.data_u) {
    iotc_mqtt_logic_free_task_data(task);
  }

  IOTC_CR_RESET(task->cs);

  iotc_mqtt_logic_layer_finalize_task(context, task);

  return state;
}

#ifdef __cplusplus
}
#endif

#endif /* __IOTC_MQTT_LOGIC_LAYER_PUBLISH_Q1_COMMAND_H__ */
