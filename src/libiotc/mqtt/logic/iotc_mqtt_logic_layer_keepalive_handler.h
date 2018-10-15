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

#ifndef __IOTC_MQTT_LOGIC_LAYER_KEEPALIVE_HANDLER_H__
#define __IOTC_MQTT_LOGIC_LAYER_KEEPALIVE_HANDLER_H__

#include "iotc_layer_api.h"

#ifdef __cplusplus
extern "C" {
#endif

iotc_state_t do_mqtt_keepalive_once(void* data);

iotc_state_t do_mqtt_keepalive_task(void* context, void* task,
                                    iotc_state_t state, void* msg_memory);

static inline iotc_state_t on_keepalive_timeout_expiry(void* context,
                                                       void* task,
                                                       iotc_state_t state,
                                                       void* msg_memory) {
  IOTC_UNUSED(state);

  do_mqtt_keepalive_task(context, task, IOTC_STATE_TIMEOUT, msg_memory);

  return IOTC_STATE_OK;
}

#ifdef __cplusplus
}
#endif

#endif /* __IOTC_MQTT_LOGIC_LAYER_KEEPALIVE_HANDLER_H__ */
