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

#ifndef __IOTC_MQTT_CODEC_LAYER_DATA_H__
#define __IOTC_MQTT_CODEC_LAYER_DATA_H__

#include "iotc_mqtt_parser.h"
#include "iotc_vector.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct iotc_mqtt_codec_layer_task_s {
  struct iotc_mqtt_codec_layer_task_s* __next;
  iotc_mqtt_message_t* msg;
  uint16_t msg_id;
  iotc_mqtt_type_t msg_type;
} iotc_mqtt_codec_layer_task_t;

typedef struct iotc_mqtt_codec_layer_data_s {
  iotc_mqtt_message_t* msg;
  iotc_mqtt_codec_layer_task_t* task_queue;
  iotc_mqtt_parser_t parser;
  iotc_state_t local_state;
  uint16_t msg_id;
  iotc_mqtt_type_t msg_type;
  uint16_t pull_cs;
  uint16_t push_cs;
} iotc_mqtt_codec_layer_data_t;

/**
 * @brief iotc_mqtt_code_layer_make_task
 *
 * Helper ctor like function to create mqtt codec layer's task.
 *
 * @param msg
 * @return
 */
extern iotc_mqtt_codec_layer_task_t* iotc_mqtt_codec_layer_make_task(
    iotc_mqtt_message_t* msg);

/**
 * @brief iotc_mqtt_codec_layer_activate_task
 *
 * After the task activation certain steps must be performed
 * like detaching the msg from the task so that the msg memory
 * won't be double deleted in case of an error or system shutdown.
 *
 * @param task
 * @return msg
 */
iotc_mqtt_message_t* iotc_mqtt_codec_layer_activate_task(
    iotc_mqtt_codec_layer_task_t* task);

/**
 * @brief iotc_mqtt_codec_layer_continue_task
 *
 * When the task continues execution the msg memory must be restored
 * so that msg lifetime can be controled by the task.
 *
 * @param task
 * @param msg
 */
void iotc_mqtt_codec_layer_continue_task(iotc_mqtt_codec_layer_task_t* task,
                                         iotc_mqtt_message_t* msg);

/**
 * @brief iotc_mqtt_code_layer_free_task
 *
 * Helper dtor like function.
 *
 * @param task
 * @return
 */
extern void iotc_mqtt_codec_layer_free_task(
    iotc_mqtt_codec_layer_task_t** task);

#ifdef __cplusplus
}
#endif

#endif /* __IOTC_MQTT_CODEC_LAYER_DATA_H__ */
