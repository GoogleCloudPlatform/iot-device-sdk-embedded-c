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

#ifndef __IOTC_MQTT_LOGIC_LAYER_DATA_H__
#define __IOTC_MQTT_LOGIC_LAYER_DATA_H__

#include "iotc_connection_data.h"
#include "iotc_data_desc.h"
#include "iotc_event_dispatcher_api.h"
#include "iotc_mqtt_message.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum iotc_scenario_e {
  IOTC_MQTT_CONNECT = 0,
  IOTC_MQTT_PUBLISH,
  IOTC_MQTT_PUBACK,
  IOTC_MQTT_SUBSCRIBE,
  IOTC_MQTT_KEEPALIVE,
  IOTC_MQTT_SHUTDOWN
} iotc_scenario_t;

typedef union {
  struct data_t_publish_t {
    char* topic;
    iotc_data_desc_t* data;
    iotc_mqtt_retain_t retain;
    iotc_mqtt_dup_t dup;
  } publish;

  struct data_t_subscribe_t {
    char* topic;
    iotc_event_handle_t handler;
    iotc_mqtt_qos_t qos;
  } subscribe;

  struct data_t_shutdown_t {
    char placeholder; /* This is to meet requirements of the IAR ARM
                       * compiler. */
  } shutdown;
} iotc_mqtt_task_specific_data_t;

typedef struct iotc_mqtt_logic_task_data_s {
  struct iotc_mqtt_logic_in_s {
    iotc_scenario_t scenario;
    iotc_mqtt_qos_t qos;
  } mqtt_settings;

  iotc_mqtt_task_specific_data_t* data_u;

} iotc_mqtt_logic_task_data_t;

typedef enum {
  IOTC_MQTT_LOGIC_TASK_NORMAL = 0,
  IOTC_MQTT_LOGIC_TASK_IMMEDIATE
} iotc_mqtt_logic_task_priority_t;

typedef enum {
  IOTC_MQTT_LOGIC_TASK_SESSION_UNSET = 0,
  IOTC_MQTT_LOGIC_TASK_SESSION_DO_NOT_STORE,
  IOTC_MQTT_LOGIC_TASK_SESSION_STORE,
} iotc_mqtt_logic_task_session_state_t;

typedef struct iotc_mqtt_logic_task_s {
  struct iotc_mqtt_logic_task_s* __next;
  iotc_time_event_handle_t timeout;
  iotc_event_handle_t logic;
  iotc_event_handle_t callback;
  iotc_mqtt_logic_task_data_t data;
  iotc_mqtt_logic_task_priority_t priority;
  iotc_mqtt_logic_task_session_state_t session_state;
  uint16_t cs;
  uint16_t msg_id;
} iotc_mqtt_logic_task_t;

typedef struct {
  /* Here we are going to store the mapping of the
   * handle functions versus the subscribed topics
   * so it's easy for the user to register his callback
   * for each of the subscribed topics. */

  /* Handle to the user idle function that suppose to. */
  iotc_mqtt_logic_task_t* q12_tasks_queue;
  iotc_mqtt_logic_task_t* q12_recv_tasks_queue;
  iotc_mqtt_logic_task_t* q0_tasks_queue;
  iotc_mqtt_logic_task_t* current_q0_task;
  iotc_vector_t* handlers_for_topics;
  iotc_time_event_handle_t keepalive_event;
  uint16_t last_msg_id;
} iotc_mqtt_logic_layer_data_t;

/* Pseudo constructors. */
extern iotc_mqtt_logic_task_t* iotc_mqtt_logic_make_publish_task(
    const char* topic, iotc_data_desc_t* data, const iotc_mqtt_qos_t qos,
    const iotc_mqtt_retain_t retain, iotc_event_handle_t callback);

extern iotc_mqtt_logic_task_t* iotc_mqtt_logic_make_subscribe_task(
    char* topic, const iotc_mqtt_qos_t qos, iotc_event_handle_t handler);

extern void iotc_mqtt_task_spec_data_free_publish_data(
    iotc_mqtt_task_specific_data_t** data);

extern void iotc_mqtt_task_spec_data_free_subscribe_data(
    iotc_mqtt_task_specific_data_t** data);

extern void iotc_mqtt_task_spec_data_free_subscribe_data_vec(
    union iotc_vector_selector_u* data, void* arg);

extern iotc_mqtt_logic_task_t* iotc_mqtt_logic_make_shutdown_task(void);

/* Pseudo destructor. */
extern iotc_mqtt_logic_task_t* iotc_mqtt_logic_free_task_data(
    iotc_mqtt_logic_task_t* task);

extern iotc_mqtt_logic_task_t* iotc_mqtt_logic_free_task(
    iotc_mqtt_logic_task_t** task);

#ifdef __cplusplus
}
#endif

#endif /* __IOTC_MQTT_LOGIC_LAYER_DATA_H__ */
