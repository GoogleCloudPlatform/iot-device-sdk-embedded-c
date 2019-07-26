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

#ifndef __IOTC_ERROR_H__
#define __IOTC_ERROR_H__

#ifdef __cplusplus
extern "C" {
#endif

/*! \file
 * @brief Defines state messages and their numeric codes.
 */

/**
 * @typedef iotc_state_t
 * @brief The internal state.
 */
typedef enum {
  /** The Device SDK function succeeded. Numeric code: 0 */ IOTC_STATE_OK = 0,
  /** A timeout occurred. Numeric code: 1 */ IOTC_STATE_TIMEOUT,
  /** The BSP file management functions Numeric code: 2 */ IOTC_STATE_WANT_READ,
  /** Numeric code: 3 */ IOTC_STATE_WANT_WRITE,
  /** Numeric code: 4 */ IOTC_STATE_WRITTEN,
  /** Numeric code: 5 */ IOTC_STATE_FAILED_WRITING,
  /** Numeric code: 6 */ IOTC_BACKOFF_TERMINAL,
  /** Numeric code: 7 */ IOTC_OUT_OF_MEMORY,
  /** Numeric code: 8 */ IOTC_SOCKET_INITIALIZATION_ERROR,
  /** Numeric code: 9 */ IOTC_SOCKET_GETHOSTBYNAME_ERROR,
  /** Numeric code: 10 */ IOTC_SOCKET_GETSOCKOPT_ERROR,
  /** Numeric code: 11 */ IOTC_SOCKET_ERROR,
  /** Numeric code: 12 */ IOTC_SOCKET_CONNECTION_ERROR,
  /** Numeric code: 13 */ IOTC_SOCKET_SHUTDOWN_ERROR,
  /** Numeric code: 14 */ IOTC_SOCKET_WRITE_ERROR,
  /** Numeric code: 15 */ IOTC_SOCKET_READ_ERROR,
  /** Numeric code: 16 */ IOTC_SOCKET_NO_ACTIVE_CONNECTION_ERROR,
  /** Numeric code: 17 */ IOTC_CONNECTION_RESET_BY_PEER_ERROR,
  /** Numeric code: 18 */ IOTC_FD_HANDLER_NOT_FOUND,
  /** Numeric code: 19 */ IOTC_TLS_INITALIZATION_ERROR,
  /** Numeric code: 20 */ IOTC_TLS_FAILED_LOADING_CERTIFICATE,
  /** Numeric code: 21 */ IOTC_TLS_CONNECT_ERROR,
  /** Numeric code: 22 */ IOTC_TLS_WRITE_ERROR,
  /** Numeric code: 23 */ IOTC_TLS_READ_ERROR,
  /** Numeric code: 24 */ IOTC_MQTT_SERIALIZER_ERROR,
  /** Numeric code: 25 */ IOTC_MQTT_PARSER_ERROR,
  /** Numeric code: 26 */ IOTC_MQTT_UNKNOWN_MESSAGE_ID,
  /** Numeric code: 27 */ IOTC_MQTT_LOGIC_UNKNOWN_TASK_ID,
  /** Numeric code: 28 */ IOTC_MQTT_LOGIC_WRONG_SCENARIO_TYPE,
  /** Numeric code: 29 */ IOTC_MQTT_LOGIC_WRONG_MESSAGE_RECEIVED,
  /** Numeric code: 30 */ IOTC_MQTT_UNACCEPTABLE_PROTOCOL_VERSION,
  /** Numeric code: 31 */ IOTC_MQTT_IDENTIFIER_REJECTED,
  /** Numeric code: 32 */ IOTC_MQTT_SERVER_UNAVAILIBLE,
  /** Numeric code: 33 */ IOTC_MQTT_BAD_USERNAME_OR_PASSWORD,
  /** Numeric code: 34 */ IOTC_MQTT_NOT_AUTHORIZED,
  /** Numeric code: 35 */ IOTC_MQTT_CONNECT_UNKNOWN_RETURN_CODE,
  /** Numeric code: 36 */ IOTC_MQTT_MESSAGE_CLASS_UNKNOWN_ERROR,
  /** Numeric code: 37 */ IOTC_MQTT_PAYLOAD_SIZE_TOO_LARGE,
  /** Numeric code: 38 */ IOTC_MQTT_SUBSCRIPTION_FAILED,
  /** Numeric code: 39 */ IOTC_MQTT_SUBSCRIPTION_SUCCESSFULL,
  /** Numeric code: 40 */ IOTC_INTERNAL_ERROR,
  /** Numeric code: 41 */ IOTC_NOT_INITIALIZED,
  /** Numeric code: 42 */ IOTC_FAILED_INITIALIZATION,
  /** Numeric code: 43 */ IOTC_ALREADY_INITIALIZED,
  /** Numeric code: 44 */ IOTC_INVALID_PARAMETER,
  /** Numeric code: 45 */ IOTC_UNSET_HANDLER_ERROR,
  /** Numeric code: 46 */ IOTC_NOT_IMPLEMENTED,
  /** Numeric code: 47 */ IOTC_ELEMENT_NOT_FOUND,
  /** Numeric code: 48 */ IOTC_SERIALIZATION_ERROR,
  /** Numeric code: 49 */ IOTC_TRUNCATION_WARNING,
  /** Numeric code: 50 */ IOTC_BUFFER_OVERFLOW,
  /** Numeric code: 51 */ IOTC_THREAD_ERROR,
  /** Numeric code: 52 */ IOTC_NULL_CONTEXT,
  /** Numeric code: 53 */ IOTC_NULL_WILL_TOPIC,
  /** Numeric code: 54 */ IOTC_NULL_WILL_MESSAGE,
  /** Numeric code: 55 */ IOTC_NO_MORE_RESOURCE_AVAILABLE,
  /** Numeric code: 56 */ IOTC_FS_RESOURCE_NOT_AVAILABLE,
  /** Numeric code: 57 */ IOTC_FS_ERROR,
  /** Numeric code: 58 */ IOTC_NOT_SUPPORTED,
  /** Numeric code: 59 */ IOTC_EVENT_PROCESS_STOPPED,
  /** Numeric code: 60 */ IOTC_STATE_RESEND,
  /** Numeric code: 61 */ IOTC_NULL_HOST,
  /** Numeric code: 62 */ IOTC_TLS_FAILED_CERT_ERROR,
  /** Numeric code: 63 */ IOTC_FS_OPEN_ERROR,
  /** Numeric code: 64 */ IOTC_FS_OPEN_READ_ONLY,
  /** Numeric code: 65 */ IOTC_FS_READ_ERROR,
  /** Numeric code: 66 */ IOTC_FS_WRITE_ERROR,
  /** Numeric code: 67 */ IOTC_FS_CLOSE_ERROR,
  /** Numeric code: 68 */ IOTC_FS_REMOVE_ERROR,
  /** Numeric code: 69 */ IOTC_NULL_PROJECT_ID_ERROR,
  /** Numeric code: 70 */ IOTC_ALG_NOT_SUPPORTED_ERROR,
  /** Numeric code: 71 */ IOTC_JWT_FORMATTION_ERROR,
  /** Numeric code: 72 */ IOTC_JWT_PROJECTID_TOO_LONG_ERROR,
  /** Numeric code: 73 */ IOTC_NULL_DEVICE_PATH_ERROR,
  /** Numeric code: 74 */ IOTC_BUFFER_TOO_SMALL_ERROR,
  /** Numeric code: 75 */ IOTC_NULL_KEY_DATA_ERROR,
  /** Numeric code: 76 */ IOTC_NULL_CLIENT_ID_ERROR,

  IOTC_ERROR_COUNT /* Add errors above this line; this should always be last line. */
} iotc_state_t;

/**
 * @brief A flag to translate between {::iotc_state_t state messages} to their numeric codes.
 */
#define IOTC_OPT_NO_ERROR_STRINGS 1

/**
 * @brief Returns the {@link ::iotc_state_t error code} as a string.
 *
 * @param [in] e The numeric {@link ::iotc_state_t error code}.
 */
extern const char* iotc_get_state_string(iotc_state_t e);

#ifdef __cplusplus
}
#endif

#endif /* __IOTC_ERR_H__ */
