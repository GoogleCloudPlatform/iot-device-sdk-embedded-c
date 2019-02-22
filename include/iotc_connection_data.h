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

#ifndef __IOTC_CONNECTION_DATA_H__
#define __IOTC_CONNECTION_DATA_H__

#include <iotc_mqtt.h>
#include "iotc_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @enum iotc_connection_state_e
 * @brief Defines all possible states of the connection process and lifecycle.
 */
typedef enum iotc_connection_state_e {
  IOTC_CONNECTION_STATE_UNINITIALIZED =
      0,                            /** the connection is not
                                       established and there is no pending
                                       connect operation in libiotc. */
  IOTC_CONNECTION_STATE_OPENING,    /** the connect operation has been started.
                                     */
  IOTC_CONNECTION_STATE_OPENED,     /** connect operation has been finished
                                     successfully. */
  IOTC_CONNECTION_STATE_CLOSING,    /** disconnect operation has been started */
  IOTC_CONNECTION_STATE_CLOSED,     /** connection is closed */
  IOTC_CONNECTION_STATE_OPEN_FAILED /**< there was an error during connect
                                     operation */
} iotc_connection_state_t;

/**
 * @enum iotc_session_type_e
 * @brief MQTT session types. Note that CONTINUE isn't currently supported.
 */
typedef enum iotc_session_type_e {
  IOTC_SESSION_CLEAN,   /**< MQTT clean session */
  IOTC_SESSION_CONTINUE /**< MQTT unclean session */
} iotc_session_type_t;

/**
 * @struct  iotc_connection_data_t
 * @brief   Connection parameters received by iotc_connect's callback function.
 * These values may be reused to request another connection from within the
 * callback.
 */
typedef struct {
  char* host;
  char* username;
  char* password;
  char* client_id;
  uint16_t port;
  uint16_t connection_timeout;
  uint16_t keepalive_timeout;
  iotc_connection_state_t connection_state;
  iotc_session_type_t session_type;
  char* will_topic;               /* UNUSED */
  char* will_message;             /* UNUSED */
  iotc_mqtt_qos_t will_qos;       /* UNUSED */
  iotc_mqtt_retain_t will_retain; /* UNUSED */
} iotc_connection_data_t;

#ifdef __cplusplus
}
#endif

#endif /* __IOTC_CONNECTION_DATA_H__ */
