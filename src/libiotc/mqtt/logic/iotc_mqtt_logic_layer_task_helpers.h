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

#ifndef __IOTC_MQTT_LOGIC_LAYER_TASK_HELPERS_H__
#define __IOTC_MQTT_LOGIC_LAYER_TASK_HELPERS_H__

#include "iotc_event_dispatcher_api.h"
#include "iotc_globals.h"
#include "iotc_io_timeouts.h"
#include "iotc_layer_api.h"
#include "iotc_list.h"
#include "iotc_mqtt_logic_layer_data.h"

#ifdef __cplusplus
extern "C" {
#endif

iotc_state_t iotc_mqtt_logic_layer_finalize_task(
    iotc_layer_connectivity_t* context, iotc_mqtt_logic_task_t* task);

iotc_state_t iotc_mqtt_logic_layer_run_next_q0_task(void* data);

void iotc_mqtt_logic_task_defer_users_callback(void* context,
                                               iotc_mqtt_logic_task_t* task,
                                               iotc_state_t state);

#define CMP_TASK_MSG_ID(task, id) (task->msg_id == id)

static inline void cancel_task_timeout(iotc_mqtt_logic_task_t* task,
                                       iotc_layer_connectivity_t* context) {
  /* PRE-CONDITIONS */
  assert(task != NULL);
  assert(context != NULL);

  iotc_evtd_instance_t* event_dispatcher =
      IOTC_CONTEXT_DATA(context)->evtd_instance;
  iotc_vector_t* io_timeouts = IOTC_CONTEXT_DATA(context)->io_timeouts;

  assert(event_dispatcher != NULL);
  assert(io_timeouts != NULL);

  if (NULL != task->timeout.ptr_to_position) {
    /* check if timeout is in timeouts vector, and remove it */
    iotc_io_timeouts_remove(&task->timeout, io_timeouts);
    iotc_state_t local_state =
        iotc_evtd_cancel(event_dispatcher, &task->timeout);

    if (IOTC_STATE_OK != local_state) {
      //@TODO add proper implementation and error handling
      iotc_debug_logger("error while canceling task timeout");
    }
    assert(IOTC_STATE_OK == local_state);
  }

  /* POST-CONDITIONS */
  assert(NULL == task->timeout.ptr_to_position);
}

static inline void signal_task(iotc_mqtt_logic_task_t* task,
                               iotc_state_t state) {
  task->logic.handlers.h4.a3 = state;
  iotc_evtd_execute_handle(&task->logic);
}

/* if sent after layer_data cleared
 * will stop each task and free tasks memory */
static inline void abort_task(iotc_mqtt_logic_task_t* task) {
  /* call the task handler with ok status */
  signal_task(task, IOTC_STATE_OK);
}

static inline void resend_task(iotc_mqtt_logic_task_t* task) {
  signal_task(task, IOTC_STATE_RESEND);
}

static inline void timeout_task(iotc_mqtt_logic_task_t* task) {
  signal_task(task, IOTC_STATE_TIMEOUT);
}

static inline void set_new_context_and_call_resend(iotc_mqtt_logic_task_t* task,
                                                   void* context) {
  if (task->session_state == IOTC_MQTT_LOGIC_TASK_SESSION_STORE) {
    task->logic.handlers.h4.a1 = context;
    resend_task(task);
  }
}

static inline iotc_state_t run_task(iotc_layer_connectivity_t* context,
                                    iotc_mqtt_logic_task_t* task) {
  /* PRECONDITION */
  assert(context != 0);
  assert(task != 0);

  iotc_mqtt_logic_layer_data_t* layer_data =
      (iotc_mqtt_logic_layer_data_t*)IOTC_THIS_LAYER(context)->user_data;

  if (task->data.mqtt_settings.qos ==
      IOTC_MQTT_QOS_AT_MOST_ONCE) /* qos zero tasks
                                   * must be enqued */
  {
    /* task on qos0 can be prioritized */
    switch (task->priority) {
      case IOTC_MQTT_LOGIC_TASK_NORMAL:
        IOTC_LIST_PUSH_BACK(iotc_mqtt_logic_task_t, layer_data->q0_tasks_queue,
                            task);
        break;
      case IOTC_MQTT_LOGIC_TASK_IMMEDIATE:
        IOTC_LIST_PUSH_FRONT(iotc_mqtt_logic_task_t, layer_data->q0_tasks_queue,
                             task);
        break;
    }

    if (layer_data->current_q0_task == 0) {
      return iotc_mqtt_logic_layer_run_next_q0_task(context);
    }
  } else {
    /* generate the new id this id will be used to communicate with the server
     * and to demultiplex msgs */
    task->msg_id = ++layer_data->last_msg_id;

#ifdef IOTC_DEBUG_EXTRA_INFO
    iotc_mqtt_logic_task_t* needle = NULL;
    IOTC_LIST_FIND(iotc_mqtt_logic_task_t, layer_data->q12_tasks_queue,
                   CMP_TASK_MSG_ID,
                   task->msg_id, /* this is linear search so we have O(n)
                                    complexity it can be optimized but for the
                                    small n it is acceptable complexity */
                   needle);
    assert(NULL == needle && "task with the same id already exist");
#endif

    /* add it to the queue which is really a multiplexer of message id's */
    IOTC_LIST_PUSH_BACK(iotc_mqtt_logic_task_t, layer_data->q12_tasks_queue,
                        task);

    /* execute it immediately
     * @TODO concider a different strategy of execution in order to minimize the
     * device overload we could execute only a certain amount per one loop
     *  but let's keep it simple for now */
    if (IOTC_CONTEXT_DATA(context)->connection_data->connection_state ==
        IOTC_CONNECTION_STATE_OPENED) {
      return iotc_evtd_execute_handle(&task->logic);
    }
  }

  return IOTC_STATE_OK;
}

#ifdef __cplusplus
}
#endif

#endif /* __IOTC_MQTT_LOGIC_LAYER_TASK_HELPERS_H__ */
