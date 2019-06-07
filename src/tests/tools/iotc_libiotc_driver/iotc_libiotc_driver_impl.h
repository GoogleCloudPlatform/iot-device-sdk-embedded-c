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

#ifndef __IOTC_LIBIOTC_DRIVER_IMPL_H__
#define __IOTC_LIBIOTC_DRIVER_IMPL_H__

#include "iotc_event_dispatcher_api.h"
#include "iotc_types_internal.h"

static const uint32_t DEFAULT_JWT_EXPIRATION = 600;
static const char* DEFAULT_CLIENT_ID = "client_id";

typedef struct iotc_libiotc_driver_s {
  iotc_evtd_instance_t* evtd_instance;
  iotc_context_t* context;
} iotc_libiotc_driver_t;

extern iotc_libiotc_driver_t* libiotc_driver;

iotc_libiotc_driver_t* iotc_libiotc_driver_create_instance();

iotc_state_t iotc_libiotc_driver_destroy_instance(
    iotc_libiotc_driver_t** driver);

typedef void (*iotc_libiotc_driver_callback_function_t)();

iotc_state_t iotc_libiotc_driver_connect_with_callback(
    iotc_libiotc_driver_t* driver, const char* const host, uint16_t port,
    iotc_libiotc_driver_callback_function_t on_connected);

iotc_state_t iotc_libiotc_driver_send_on_connect_finish(
    iotc_libiotc_driver_t* driver, iotc_state_t connect_result);

iotc_state_t iotc_libiotc_driver_send_on_message_received(
    iotc_libiotc_driver_t* driver, iotc_sub_call_type_t call_type,
    const iotc_sub_call_params_t* const params, iotc_state_t receive_result);

iotc_state_t iotc_libiotc_driver_send_on_publish_finish(
    iotc_libiotc_driver_t* driver, void* data, iotc_state_t publish_result);

iotc_state_t iotc_libiotc_driver_send_on_disconnect(
    iotc_libiotc_driver_t* driver, iotc_state_t error_code);

#endif /* __IOTC_LIBIOTC_DRIVER_IMPL_H__ */
