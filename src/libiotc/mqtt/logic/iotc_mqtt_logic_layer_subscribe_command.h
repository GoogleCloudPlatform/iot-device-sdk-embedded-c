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

#ifndef __IOTC_MQTT_LOGIC_LAYER_SUBSCRIBE_COMMAND_H__
#define __IOTC_MQTT_LOGIC_LAYER_SUBSCRIBE_COMMAND_H__

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

static inline iotc_state_t do_mqtt_subscribe(void* ctx, void* data,
                                             iotc_state_t state, void* msg) {
  iotc_layer_connectivity_t* context = (iotc_layer_connectivity_t*)ctx;
  iotc_mqtt_logic_task_t* task = (iotc_mqtt_logic_task_t*)data;

  assert(NULL != context);
  assert(NULL != task);

  iotc_mqtt_message_t* msg_memory = (iotc_mqtt_message_t*)msg;
  iotc_evtd_instance_t* event_dispatcher =
      IOTC_CONTEXT_DATA(ctx)->evtd_instance;
  iotc_state_t local_state = IOTC_STATE_OK;

  iotc_mqtt_logic_layer_data_t* layer_data =
      (iotc_mqtt_logic_layer_data_t*)IOTC_THIS_LAYER(context)->user_data;

  IOTC_UNUSED(state);

  if (NULL == layer_data) {
    cancel_task_timeout(task, context);
    iotc_mqtt_message_free(&msg_memory);
    return IOTC_STATE_OK;
  }

  IOTC_CR_START(task->cs);

  do {
    iotc_debug_format("[m.id[%d]]subscribe preparing message", task->msg_id);

    IOTC_ALLOC_AT(iotc_mqtt_message_t, msg_memory, state);

    IOTC_CHECK_STATE(state = fill_with_subscribe_data(
                         msg_memory, task->data.data_u->subscribe.topic,
                         task->msg_id, task->data.data_u->subscribe.qos,
                         IOTC_STATE_RESEND == state ? IOTC_MQTT_DUP_TRUE
                                                    : IOTC_MQTT_DUP_FALSE));

    iotc_debug_format("[m.id[%d]]subscribe sending message", task->msg_id);

    IOTC_CR_YIELD(task->cs, IOTC_PROCESS_PUSH_ON_PREV_LAYER(context, msg_memory,
                                                            IOTC_STATE_OK));

    if (IOTC_STATE_WRITTEN == state) {
      iotc_debug_format("[m.id[%d]]subscribe has been sent", task->msg_id);
      assert(NULL == task->timeout.ptr_to_position);
      task->session_state =
          task->session_state == IOTC_MQTT_LOGIC_TASK_SESSION_UNSET
              ? IOTC_MQTT_LOGIC_TASK_SESSION_STORE
              : task->session_state;
    } else {
      iotc_debug_format("[m.id[%d]]subscribe has not been sent", task->msg_id);

      assert(NULL == task->timeout.ptr_to_position);

      local_state =
          iotc_evtd_execute_in(event_dispatcher,
                               iotc_make_handle(&do_mqtt_subscribe, context,
                                                task, IOTC_STATE_RESEND, NULL),
                               1, &task->timeout);

      IOTC_CHECK_STATE(local_state);

      IOTC_CR_YIELD(task->cs, IOTC_STATE_OK);

      /* sanity checks */
      assert(NULL == task->timeout.ptr_to_position);
      assert(IOTC_STATE_RESEND == state);

      continue;
    }

    /* add a timeout for waiting for the response */
    assert(NULL == task->timeout.ptr_to_position);

    /* @TODO change it to use the defined timeout */
    if (IOTC_CONTEXT_DATA(context)->connection_data->keepalive_timeout > 0) {
      iotc_state_t local_state = iotc_evtd_execute_in(
          event_dispatcher,
          iotc_make_handle(&do_mqtt_subscribe, context, task,
                           IOTC_STATE_TIMEOUT, NULL),
          IOTC_CONTEXT_DATA(context)->connection_data->keepalive_timeout,
          &task->timeout);
      IOTC_CHECK_STATE(local_state);
    }

    /* wait for the suback */
    IOTC_CR_YIELD(task->cs, IOTC_STATE_OK);

    /* clear timeout if it was timeout */
    if (IOTC_STATE_TIMEOUT == state) {
      iotc_debug_format("[m.id[%d]]subscribe timeout occured", task->msg_id);
      assert(NULL == task->timeout.ptr_to_position);
      state = IOTC_STATE_RESEND;
    } else {
      cancel_task_timeout(task, context);
    }

    if (IOTC_STATE_RESEND == state) {
      iotc_debug_format("[m.id[%d]]subscribe resend", task->msg_id);
    }

    assert(NULL == task->timeout.ptr_to_position);

  } while (IOTC_STATE_RESEND == state);

  assert(NULL == task->timeout.ptr_to_position);

  if (state == IOTC_STATE_OK) {
    if (msg_memory->common.common_u.common_bits.type != IOTC_MQTT_TYPE_SUBACK) {
      iotc_debug_format(
          "[m.id[%d]]subscribe error was expecting suback got %d!",
          task->msg_id, msg_memory->common.common_u.common_bits.type);

      state = IOTC_MQTT_LOGIC_WRONG_MESSAGE_RECEIVED;
      goto err_handling;
    }

    iotc_debug_format("[m.id[%d]]subscribe suback received", task->msg_id);

    iotc_mqtt_suback_status_t suback_status =
        msg_memory->suback.topics->iotc_mqtt_topic_pair_payload_u.status;

    iotc_mqtt_message_free(&msg_memory);

    task->data.data_u->subscribe.handler.handlers.h6.a2 =
        (void*)(intptr_t)suback_status;

    task->data.data_u->subscribe.handler.handlers.h6.a3 =
        suback_status == IOTC_MQTT_SUBACK_FAILED
            ? IOTC_MQTT_SUBSCRIPTION_FAILED
            : IOTC_MQTT_SUBSCRIPTION_SUCCESSFULL;

    /* check if the suback registration was successfull */
    if (IOTC_MQTT_SUBACK_FAILED != suback_status) {
      /* now it can be registered - we are passing the ownership of the
       * data.data_u to the vector */
      IOTC_CHECK_MEMORY(
          iotc_vector_push(
              layer_data->handlers_for_topics,
              IOTC_VEC_VALUE_PARAM(IOTC_VEC_VALUE_PTR(task->data.data_u))),
          state);
    }

    IOTC_CHECK_MEMORY(iotc_evtd_execute(event_dispatcher,
                                        task->data.data_u->subscribe.handler),
                      state);

    /* now it's safe to nullify this pointer because the ownership of this
     * memory block is now passed either to a subscription callback or the
     * handlers_for_topics vector if the subscription was succesfull */
    task->data.data_u = NULL;

    IOTC_CR_EXIT(task->cs, iotc_mqtt_logic_layer_finalize_task(context, task));
  }

  IOTC_CR_END();

err_handling:
  iotc_mqtt_message_free(&msg_memory);
  iotc_mqtt_logic_layer_finalize_task(context, task);

  IOTC_CR_RESET(task->cs);

  return state;
}

#ifdef __cplusplus
}
#endif

#endif /* __IOTC_MQTT_LOGIC_LAYER_SUBSCRIBE_COMMAND_H__ */
