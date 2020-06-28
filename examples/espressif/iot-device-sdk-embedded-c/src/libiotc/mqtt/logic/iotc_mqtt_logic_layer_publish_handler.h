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

#ifndef __IOTC_MQTT_LOGIC_LAYER_PUBLISH_HANDLER_H__
#define __IOTC_MQTT_LOGIC_LAYER_PUBLISH_HANDLER_H__

#include "iotc_coroutine.h"
#include "iotc_event_thread_dispatcher.h"
#include "iotc_globals.h"
#include "iotc_helpers.h"
#include "iotc_layer_api.h"
#include "iotc_mqtt_logic_layer_data.h"
#include "iotc_mqtt_logic_layer_data_helpers.h"
#include "iotc_mqtt_logic_layer_task_helpers.h"
#include "iotc_mqtt_message.h"

#ifdef __cplusplus
extern "C" {
#endif

static inline void call_topic_handler(
    void* context, /* Should be the context of the logic layer. */
    void* msg_data) {
  iotc_mqtt_logic_layer_data_t* layer_data =
      (iotc_mqtt_logic_layer_data_t*)IOTC_THIS_LAYER(context)->user_data;

  iotc_mqtt_message_t* msg_memory = (iotc_mqtt_message_t*)msg_data;

  /* Pre-conditions. */
  assert(NULL != msg_memory);

  iotc_vector_index_type_t index = 0;

  iotc_debug_format("[m.id[%d]] looking for publish message handler",
                    iotc_mqtt_get_message_id(msg_memory));

  index = iotc_vector_find(layer_data->handlers_for_topics,
                           IOTC_VEC_CONST_VALUE_PARAM(IOTC_VEC_VALUE_PTR(
                               msg_memory->publish.topic_name)),
                           cmp_topics);

  if (index != -1) {
    iotc_mqtt_task_specific_data_t* subscribe_data =
        (iotc_mqtt_task_specific_data_t*)layer_data->handlers_for_topics
            ->array[index]
            .selector_t.ptr_value;

    subscribe_data->subscribe.handler.handlers.h3.a2 = msg_memory;
    subscribe_data->subscribe.handler.handlers.h3.a3 = IOTC_STATE_OK;

    iotc_evttd_execute(IOTC_CONTEXT_DATA(context)->evtd_instance,
                       subscribe_data->subscribe.handler);
  } else {
    iotc_debug_format(
        "[m.id[%d]] received publish message for topic which "
        "is not registered",
        iotc_mqtt_get_message_id(msg_memory));
    iotc_debug_mqtt_message_dump(msg_memory);
    iotc_mqtt_message_free(&msg_memory);
  }
}

static inline iotc_state_t on_publish_q0_recieved(
    iotc_layer_connectivity_t*
        context /* Should be the context of the logic layer. */
    ,
    void* data, iotc_state_t state, void* msg_data) {
  IOTC_UNUSED(data);
  IOTC_UNUSED(state);

  iotc_mqtt_logic_layer_data_t* layer_data =
      (iotc_mqtt_logic_layer_data_t*)IOTC_THIS_LAYER(context)->user_data;

  iotc_mqtt_message_t* msg = (iotc_mqtt_message_t*)msg_data;

  if (NULL == layer_data) {
    /* Means that the layer has been shut down .*/
    iotc_mqtt_message_free(&msg);

    return IOTC_STATE_OK;
  }

  call_topic_handler(context, msg);

  return IOTC_STATE_OK;
}

static inline iotc_state_t on_publish_q1_recieved(
    void* context /* Should be the context of the logic layer. */
    ,
    void* data, iotc_state_t state, void* msg_data) {
  IOTC_UNUSED(data);
  IOTC_UNUSED(state);

  iotc_mqtt_logic_layer_data_t* layer_data =
      (iotc_mqtt_logic_layer_data_t*)IOTC_THIS_LAYER(context)->user_data;

  iotc_mqtt_message_t* msg = (iotc_mqtt_message_t*)msg_data;
  iotc_mqtt_logic_task_t* task = (iotc_mqtt_logic_task_t*)data;

  if (IOTC_THIS_LAYER_NOT_OPERATIONAL(context) || NULL == layer_data) {
    goto err_handling;
  }

  /* If task doesn't exist create and register one. */
  if (NULL == task) {
    uint16_t msg_id = msg->publish.message_id;

    IOTC_ALLOC_AT(iotc_mqtt_logic_task_t, task, state);

    task->data.mqtt_settings.scenario = IOTC_MQTT_PUBACK;
    task->data.mqtt_settings.qos = IOTC_MQTT_QOS_AT_LEAST_ONCE;

    task->logic = iotc_make_handle(&on_publish_q1_recieved, context, task,
                                   IOTC_STATE_OK, msg);

    task->msg_id = msg_id;

/* @TODO: change this into debug only compilation. */
#if 1
    {
      iotc_mqtt_logic_task_t* test_task = NULL;

      IOTC_LIST_FIND(iotc_mqtt_logic_task_t, layer_data->q12_recv_tasks_queue,
                     CMP_TASK_MSG_ID, task->msg_id, test_task);

      // there must not be similar tasks
      assert(NULL == test_task);
    }
#endif

    IOTC_LIST_PUSH_BACK(iotc_mqtt_logic_task_t,
                        layer_data->q12_recv_tasks_queue, task);
  }

  IOTC_CR_START(task->cs);

  /* PRECONDITIONS */
  assert(NULL != msg);

  do {
    /* Send puback to the server. */
    IOTC_ALLOC_AT(iotc_mqtt_message_t, msg, state);

    IOTC_CHECK_STATE(state = fill_with_puback_data(msg, task->msg_id));

    iotc_debug_format("[m.id[%d]preparing puback data", task->msg_id);

    IOTC_CR_YIELD_UNTIL(
        task->cs, (state != IOTC_STATE_WRITTEN),
        IOTC_PROCESS_PUSH_ON_PREV_LAYER(context, msg, IOTC_STATE_OK));

  } while (state != IOTC_STATE_WRITTEN);

  iotc_debug_format("[m.id[%d]]PUBACK sent", task->msg_id);

  /* Msg sent now proceed with cleaning. */

  /* Clean the created task data. */
  IOTC_LIST_DROP(iotc_mqtt_logic_task_t, layer_data->q12_recv_tasks_queue,
                 task);

  iotc_mqtt_logic_free_task(&task);

  /* Use the recv publish function. */
  call_topic_handler(context, msg);

  IOTC_CR_END();

  return IOTC_STATE_OK;

err_handling:
  iotc_mqtt_message_free(&msg);

  if (task) {
    IOTC_CR_RESET(task->cs);
  }

  return state;
}

static inline iotc_state_t on_publish_recieved(
    iotc_layer_connectivity_t*
        context, /* Should be the context of the logic layer. */
    iotc_mqtt_message_t* msg_memory, iotc_state_t state) {
  iotc_mqtt_logic_layer_data_t* layer_data =
      (iotc_mqtt_logic_layer_data_t*)IOTC_THIS_LAYER(context)->user_data;

  if (layer_data == 0) {
    iotc_mqtt_message_free(&msg_memory);
    return IOTC_STATE_OK;
  }

  switch (msg_memory->common.common_u.common_bits.qos) {
    case IOTC_MQTT_QOS_AT_MOST_ONCE:
      return on_publish_q0_recieved(context, 0, state, msg_memory);
    case IOTC_MQTT_QOS_AT_LEAST_ONCE:
      return on_publish_q1_recieved(context, 0, state, msg_memory);
    case IOTC_MQTT_QOS_EXACTLY_ONCE:
      iotc_debug_logger(
          "recv_publish for IOTC_MQTT_QOS_EXACTLY_ONCE not "
          "yet implemented!");
      break;
  }

  return IOTC_STATE_OK;
}

#ifdef __cplusplus
}
#endif

#endif /* __IOTC_MQTT_LOGIC_LAYER_PUBLISH_HANDLER_H__ */
