/* Copyright 2018-2019 Google LLC
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

#ifndef __IOTC_CONNECTION_DATA_INTERNAL_H__
#define __IOTC_CONNECTION_DATA_INTERNAL_H__

#include <stdint.h>

#include "iotc_allocator.h"
#include "iotc_config.h"
#include "iotc_macros.h"
#include "iotc_types.h"

#include "iotc_event_dispatcher_api.h"
#include "iotc_event_handle_typedef.h"

#include <iotc_connection_data.h>
#include "iotc_mqtt_message.h"

#ifdef __cplusplus
extern "C" {
#endif

extern iotc_connection_data_t* iotc_alloc_connection_data(
    const char* host, uint16_t port, const char* username,
    const char* password, const char* client_id, uint16_t connection_timeout,
    uint16_t keepalive_timeout, iotc_session_type_t session_type);

extern iotc_connection_data_t* iotc_alloc_connection_data_lastwill(
    const char* host, uint16_t port, const char* username,
    const char* password, const char* client_id, uint16_t keepalive_timeout,
    uint16_t connection_timeout, iotc_session_type_t session_type,
    const char* will_topic, const char* will_apssword,
    iotc_mqtt_qos_t will_qios, iotc_mqtt_retain_t will_retain);

iotc_state_t iotc_connection_data_update(
    iotc_connection_data_t* conn_data, const char* host, uint16_t port,
    const char* username, const char* password, const char* client_id,
    uint16_t connection_timeout, uint16_t keepalive_timeout,
    iotc_session_type_t session_type);

iotc_state_t iotc_connection_data_update_lastwill(
    iotc_connection_data_t* conn_data, const char* host, uint16_t port,
    const char* username, const char* password, const char* client_id,
    uint16_t connection_timeout, uint16_t keepalive_timeout,
    iotc_session_type_t session_type, const char* will_topic,
    const char* will_message, iotc_mqtt_qos_t will_qos,
    iotc_mqtt_retain_t will_retain);

extern void iotc_free_connection_data(iotc_connection_data_t** data);

#ifdef __cplusplus
}
#endif

#endif /* __IOTC_CONNECTION_DATA_INTERNAL_H__ */
