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

#include "iotc_mqtt_logic_layer_data.h"
#include "iotc_helpers.h"
#include "iotc_macros.h"

#ifdef __cplusplus
extern "C" {
#endif

iotc_mqtt_logic_task_t* iotc_mqtt_logic_make_publish_task(
    const char* topic, iotc_data_desc_t* data, const iotc_mqtt_qos_t qos,
    const iotc_mqtt_retain_t retain, iotc_event_handle_t callback) {
  /* PRECONDITIONS */
  assert(NULL != topic);
  assert(NULL != data);

  iotc_state_t state = IOTC_STATE_OK;

  IOTC_ALLOC(iotc_mqtt_logic_task_t, task, state);

  task->data.mqtt_settings.scenario = IOTC_MQTT_PUBLISH;
  task->data.mqtt_settings.qos = qos;

  task->callback = callback;

  IOTC_ALLOC_AT(iotc_mqtt_task_specific_data_t, task->data.data_u, state);

  task->data.data_u->publish.retain = retain;
  task->data.data_u->publish.topic = iotc_str_dup(topic);
  task->data.data_u->publish.data = data;

  return task;

err_handling:
  if (task) {
    iotc_mqtt_logic_free_task(&task);
  }
  return NULL;
}

iotc_mqtt_logic_task_t* iotc_mqtt_logic_make_subscribe_task(
    char* topic, const iotc_mqtt_qos_t qos, iotc_event_handle_t handler) {
  /* PRECONDITIONS */
  assert(NULL != topic);

  iotc_state_t state = IOTC_STATE_OK;

  IOTC_ALLOC(iotc_mqtt_logic_task_t, task, state);

  task->data.mqtt_settings.scenario = IOTC_MQTT_SUBSCRIBE;
  task->data.mqtt_settings.qos = IOTC_MQTT_QOS_AT_LEAST_ONCE;

  IOTC_ALLOC_AT(iotc_mqtt_task_specific_data_t, task->data.data_u, state);

  task->data.data_u->subscribe.topic = topic;
  task->data.data_u->subscribe.qos = qos;
  task->data.data_u->subscribe.handler = handler;

  return task;

err_handling:
  if (task) {
    iotc_mqtt_logic_free_task(&task);
  }
  return NULL;
}

iotc_mqtt_logic_task_t* iotc_mqtt_logic_make_shutdown_task(void) {
  iotc_state_t state = IOTC_STATE_OK;

  IOTC_ALLOC(iotc_mqtt_logic_task_t, task, state);

  task->data.mqtt_settings.scenario = IOTC_MQTT_SHUTDOWN;
  task->priority = IOTC_MQTT_LOGIC_TASK_IMMEDIATE;

  return task;

err_handling:
  if (task) {
    iotc_mqtt_logic_free_task(&task);
  }
  return NULL;
}

void iotc_mqtt_task_spec_data_free_publish_data(
    iotc_mqtt_task_specific_data_t** data) {
  /* PRECONDITIONS */
  assert(NULL != data);
  assert(NULL != *data);

  iotc_free_desc(&(*data)->publish.data);
  IOTC_SAFE_FREE((*data)->publish.topic);
  IOTC_SAFE_FREE((*data));
}

void iotc_mqtt_task_spec_data_free_subscribe_data(
    iotc_mqtt_task_specific_data_t** data) {
  /* PRECONDITIONS */
  assert(NULL != data);
  assert(NULL != *data);

  IOTC_SAFE_FREE((*data)->subscribe.topic);
  IOTC_SAFE_FREE((*data));
}

void iotc_mqtt_task_spec_data_free_subscribe_data_vec(
    union iotc_vector_selector_u* data, void* arg) {
  IOTC_UNUSED(arg);

  iotc_mqtt_task_spec_data_free_subscribe_data(
      (iotc_mqtt_task_specific_data_t**)&data->ptr_value);
}

iotc_mqtt_logic_task_t* iotc_mqtt_logic_free_task_data(
    iotc_mqtt_logic_task_t* task) {
  /* PRECONDITIONS */
  assert(NULL != task);
  assert(NULL != task->data.data_u);
  assert(NULL == task->timeout.ptr_to_position);

  switch (task->data.mqtt_settings.scenario) {
    case IOTC_MQTT_PUBLISH:
      iotc_mqtt_task_spec_data_free_publish_data(&task->data.data_u);
      break;
    case IOTC_MQTT_SUBSCRIBE:
      iotc_mqtt_task_spec_data_free_subscribe_data(&task->data.data_u);
      break;
    case IOTC_MQTT_SHUTDOWN:
      break;
    default:
      iotc_debug_format("unhandled task type: %d",
                        task->data.mqtt_settings.scenario);
  }

  /* POSTCONDITIONS */
  assert(task->data.data_u == 0);

  return task;
}

iotc_mqtt_logic_task_t* iotc_mqtt_logic_free_task(
    iotc_mqtt_logic_task_t** task) {
  if (NULL != task && NULL != *task) {
    if (NULL != (*task)->data.data_u) {
      iotc_mqtt_logic_free_task_data(*task);
    }

    IOTC_SAFE_FREE(*task);
  }

  return 0;
}

#ifdef __cplusplus
}
#endif
