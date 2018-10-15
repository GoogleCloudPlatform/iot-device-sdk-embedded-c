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

#include "iotc_mqtt_codec_layer_data.h"
#include "iotc_mqtt_message.h"

iotc_mqtt_codec_layer_task_t* iotc_mqtt_codec_layer_make_task(
    iotc_mqtt_message_t* msg) {
  iotc_state_t state = IOTC_STATE_OK;

  assert(NULL != msg);

  IOTC_ALLOC(iotc_mqtt_codec_layer_task_t, new_task, state);

  new_task->msg_id = iotc_mqtt_get_message_id(msg);
  new_task->msg_type = (iotc_mqtt_type_t)msg->common.common_u.common_bits.type;
  new_task->msg = msg;

  return new_task;

err_handling:
  return NULL;
}

iotc_mqtt_message_t* iotc_mqtt_codec_layer_activate_task(
    iotc_mqtt_codec_layer_task_t* task) {
  assert(NULL != task);
  assert(NULL != task->msg);

  iotc_mqtt_message_t* msg = task->msg;
  task->msg = NULL;

  return msg;
}

void iotc_mqtt_codec_layer_continue_task(iotc_mqtt_codec_layer_task_t* task,
                                         iotc_mqtt_message_t* msg) {
  assert(NULL != task);
  assert(NULL != msg);
  assert(NULL == task->msg);

  task->msg = msg;
}

void iotc_mqtt_codec_layer_free_task(iotc_mqtt_codec_layer_task_t** task) {
  if (NULL == task || NULL == *task) {
    return;
  }

  iotc_mqtt_message_free(&(*task)->msg);
  IOTC_SAFE_FREE((*task));
}
