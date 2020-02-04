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

#ifndef __IOTC_CONNECTION_DATA_H__
#define __IOTC_CONNECTION_DATA_H__

#include <iotc_mqtt.h>
#include "iotc_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/*! \file
 * @brief Defines connection states.
 */

/**
 * @typedef iotc_connection_state_t
 * @brief Connection process and lifecycle states.
 *
 * @see iotc_connection_state_e
 */
typedef enum iotc_connection_state_e {
  /** The connection is not established and there is no pending
   * connection request. */
  IOTC_CONNECTION_STATE_UNINITIALIZED = 0,
  /** The connect operation started. */
  IOTC_CONNECTION_STATE_OPENING,
  /** The client connected to an MQTT broker. */
  IOTC_CONNECTION_STATE_OPENED,
  /** The disconnect operation started. */
  IOTC_CONNECTION_STATE_CLOSING,
  /** The connection is closed. */
  IOTC_CONNECTION_STATE_CLOSED,
  /** Can't open connection. */
  IOTC_CONNECTION_STATE_OPEN_FAILED
} iotc_connection_state_t;

/**
 * @typedef iotc_session_type_t
 * @brief MQTT session types.
 *
 * @see iotc_session_type_e
 */
typedef enum iotc_session_type_e {
  /** MQTT clean session. */
  IOTC_SESSION_CLEAN,
  /** MQTT unclean session. */
  IOTC_SESSION_CONTINUE
} iotc_session_type_t;

/**
 * @typedef iotc_connection_data_t
 * @brief The MQTT CONNECT parameters.
 * @see iotc_connection_data_s
 *
 * @struct iotc_connection_data_s
 * @brief The MQTT CONNECT parameters.
 */
typedef struct iotc_connection_data_s {
  /** The MQTT broker hostname. */
  char* host;
  /** The MQTT client username. */
  char* username;
  /** The MQTT client password. */
  char* password;
  /** The MQTT client ID. */
  char* client_id;
  /** The port on which the MQTT broker listens. */
  uint16_t port;
  /** The connection timeout in seconds. */
  uint16_t connection_timeout;
  /** MQTT keepalive in seconds. */
  uint16_t keepalive_timeout;
  /** The CONNACK message from the MQTT broker. */
  iotc_connection_state_t connection_state;
  /** The MQTT client session. */
  iotc_session_type_t session_type;
  /** Unused. */
  char* will_topic;
  /** Unused. */
  char* will_message;
  /** Unused. */
  iotc_mqtt_qos_t will_qos;
  /** Unused. */
  iotc_mqtt_retain_t will_retain;
} iotc_connection_data_t;

#ifdef __cplusplus
}
#endif

#endif /* __IOTC_CONNECTION_DATA_H__ */
