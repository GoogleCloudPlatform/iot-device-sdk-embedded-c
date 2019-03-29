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

#ifndef __IOTC_ITEST_MOCK_LAYER_MQTTLOGIC_NEXT_H__
#define __IOTC_ITEST_MOCK_LAYER_MQTTLOGIC_NEXT_H__

#include "iotc_layer.h"

#ifdef __cplusplus
extern "C" {
#endif

iotc_state_t iotc_mock_layer_mqttlogic_next_push(void* context, void* data,
                                                 iotc_state_t state);

iotc_state_t iotc_mock_layer_mqttlogic_next_pull(void* context, void* data,
                                                 iotc_state_t state);

iotc_state_t iotc_mock_layer_mqttlogic_next_close(void* context, void* data,
                                                  iotc_state_t state);

iotc_state_t iotc_mock_layer_mqttlogic_next_close_externally(
    void* context, void* data, iotc_state_t state);

iotc_state_t iotc_mock_layer_mqttlogic_next_init(void* context, void* data,
                                                 iotc_state_t state);

iotc_state_t iotc_mock_layer_mqttlogic_next_connect(void* context, void* data,
                                                    iotc_state_t state);

#ifdef __cplusplus
}
#endif

#endif /* __IOTC_ITEST_MOCK_LAYER_MQTTLOGIC_NEXT_H__ */
