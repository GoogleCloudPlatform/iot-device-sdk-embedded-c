/* Copyright 2018 - 2019 Google LLC
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

#ifndef __IOTC_USER_SUB_CALL_WRAPPER_H__
#define __IOTC_USER_SUB_CALL_WRAPPER_H__

#include "iotc_globals.h"
#include "iotc_handle.h"
#include "iotc_mqtt_logic_layer_data.h"
#include "iotc_types.h"

iotc_state_t iotc_user_sub_call_wrapper(void* context, void* data,
                                        iotc_state_t in_state,
                                        void* client_callback, void* user_data,
                                        void* task_data);

#endif /* __IOTC_USER_SUB_CALL_WRAPPER_H__ */
