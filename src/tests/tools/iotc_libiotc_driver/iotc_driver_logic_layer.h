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

#ifndef __IOTC_DRIVER_LOGIC_LAYER_H__
#define __IOTC_DRIVER_LOGIC_LAYER_H__

#include "iotc_layer.h"

iotc_state_t iotc_driver_logic_layer_push(void* context, void* data,
                                          iotc_state_t state);

iotc_state_t iotc_driver_logic_layer_pull(void* context, void* data,
                                          iotc_state_t state);

iotc_state_t iotc_driver_logic_layer_close(void* context, void* data,
                                           iotc_state_t state);

iotc_state_t iotc_driver_logic_layer_close_externally(void* context, void* data,
                                                      iotc_state_t state);

iotc_state_t iotc_driver_logic_layer_init(void* context, void* data,
                                          iotc_state_t state);

iotc_state_t iotc_driver_logic_layer_connect(void* context, void* data,
                                             iotc_state_t state);

#endif /* __IOTC_DRIVER_LOGIC_LAYER_H__ */
