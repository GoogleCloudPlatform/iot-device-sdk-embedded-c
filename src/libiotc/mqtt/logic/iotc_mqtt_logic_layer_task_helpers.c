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

#include "iotc_mqtt_logic_layer_task_helpers.h"
#include "iotc_event_thread_dispatcher.h"
#include "iotc_layer_api.h"
#include "iotc_list.h"
#include "iotc_mqtt_logic_layer_data.h"

#ifdef __cplusplus
extern "C" {
#endif

iotc_state_t iotc_mqtt_logic_layer_run_next_q0_task(void* data) {
  iotc_layer_connectivity_t* context = data;

  iotc_mqtt_logic_layer_data_t* layer_data =
      (iotc_mqtt_logic_layer_data_t*)IOTC_THIS_LAYER(context)->user_data;

  assert(layer_data != 0);

  /* cancel the timeout of the tasks if it is still registered */
  if (layer_data->current_q0_task != 0) {
    if (NULL != layer_data->current_q0_task->timeout.ptr_to_position) {
      cancel_task_timeout(layer_data->current_q0_task, context);
    }

    iotc_mqtt_logic_free_task(&layer_data->current_q0_task);
  }

  iotc_mqtt_logic_task_t* task = 0;

  if (layer_data->q0_tasks_queue != 0) {
    IOTC_LIST_POP(iotc_mqtt_logic_task_t, layer_data->q0_tasks_queue, task);

    /* prevent execution of other tasks while connecting */
    if (IOTC_CONTEXT_DATA(context)->connection_data->connection_state ==
        IOTC_CONNECTION_STATE_OPENING) {
      /* we only allow connect while client is disconnected */
      if (task->data.mqtt_settings.scenario != IOTC_MQTT_CONNECT) {
        IOTC_LIST_PUSH_BACK(iotc_mqtt_logic_task_t, layer_data->q0_tasks_queue,
                            task);

        return IOTC_STATE_OK;
      }
    }

    assert(layer_data->current_q0_task == 0); /* sanity check */

    layer_data->current_q0_task = task;
    return iotc_evtd_execute_handle(&task->logic);
  }

  return IOTC_STATE_OK;
}

void iotc_mqtt_logic_task_defer_users_callback(void* context,
                                               iotc_mqtt_logic_task_t* task,
                                               iotc_state_t state) {
  /* PRECONDITION */
  assert(context != NULL);
  assert(task != NULL);

  /* if callback is not disposed */
  if (iotc_handle_disposed(&task->callback) == 0) {
    /* prepare handle */
    iotc_event_handle_t handle = task->callback;
    handle.handlers.h3.a3 = state;

    iotc_evttd_execute(IOTC_CONTEXT_DATA(context)->evtd_instance, handle);
  }
}

iotc_state_t iotc_mqtt_logic_layer_finalize_task(
    iotc_layer_connectivity_t* context, iotc_mqtt_logic_task_t* task) {
  /* PRECONDITION */
  assert(NULL != task);

  /* let's make sure that the task timeout has been finished / cancelled
   * for now just a assertion so that we can check if it has been correctly
   * implemented */
  assert(NULL == task->timeout.ptr_to_position);

  iotc_mqtt_logic_layer_data_t* layer_data =
      (iotc_mqtt_logic_layer_data_t*)IOTC_THIS_LAYER(context)->user_data;

  if (task->data.mqtt_settings.qos == IOTC_MQTT_QOS_AT_MOST_ONCE) {
    return iotc_mqtt_logic_layer_run_next_q0_task(context);
  } else /* I left it for better code readability */
  {
    /* detach the task from the qos 1 and 2 queue */
    IOTC_LIST_DROP(iotc_mqtt_logic_task_t, layer_data->q12_tasks_queue, task);

    /* release task's memory */
    iotc_mqtt_logic_free_task(&task);
  }

  return IOTC_STATE_OK;
}

#ifdef __cplusplus
}
#endif
