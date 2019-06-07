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

/**
 * @name iotc_state_t
 * @brief Internal error codes.
 *
 * IOTC_STATE_OK represents success. Others represent errors.
 */
typedef enum {
  /** 0 */ IOTC_STATE_OK = 0,
  /** 1 */ IOTC_STATE_TIMEOUT,
  /** 2 */ IOTC_STATE_WANT_READ,
  /** 3 */ IOTC_STATE_WANT_WRITE,
  /** 4 */ IOTC_STATE_WRITTEN,
  /** 5 */ IOTC_STATE_FAILED_WRITING,
  /** 6 */ IOTC_BACKOFF_TERMINAL,
  /** 7 */ IOTC_OUT_OF_MEMORY,
  /** 8 */ IOTC_SOCKET_INITIALIZATION_ERROR,
  /** 9 */ IOTC_SOCKET_GETHOSTBYNAME_ERROR,
  /** 10 */ IOTC_SOCKET_GETSOCKOPT_ERROR,
  /** 11 */ IOTC_SOCKET_ERROR,
  /** 12 */ IOTC_SOCKET_CONNECTION_ERROR,
  /** 13 */ IOTC_SOCKET_SHUTDOWN_ERROR,
  /** 14 */ IOTC_SOCKET_WRITE_ERROR,
  /** 15 */ IOTC_SOCKET_READ_ERROR,
  /** 16 */ IOTC_SOCKET_NO_ACTIVE_CONNECTION_ERROR,
  /** 17 */ IOTC_CONNECTION_RESET_BY_PEER_ERROR,
  /** 18 */ IOTC_FD_HANDLER_NOT_FOUND,
  /** 19 */ IOTC_TLS_INITALIZATION_ERROR,
  /** 20 */ IOTC_TLS_FAILED_LOADING_CERTIFICATE,
  /** 21 */ IOTC_TLS_CONNECT_ERROR,
  /** 22 */ IOTC_TLS_WRITE_ERROR,
  /** 23 */ IOTC_TLS_READ_ERROR,
  /** 24 */ IOTC_MQTT_SERIALIZER_ERROR,
  /** 25 */ IOTC_MQTT_PARSER_ERROR,
  /** 26 */ IOTC_MQTT_UNKNOWN_MESSAGE_ID,
  /** 27 */ IOTC_MQTT_LOGIC_UNKNOWN_TASK_ID,
  /** 28 */ IOTC_MQTT_LOGIC_WRONG_SCENARIO_TYPE,
  /** 29 */ IOTC_MQTT_LOGIC_WRONG_MESSAGE_RECEIVED,
  /** 30 */ IOTC_MQTT_UNACCEPTABLE_PROTOCOL_VERSION,
  /** 31 */ IOTC_MQTT_IDENTIFIER_REJECTED,
  /** 32 */ IOTC_MQTT_SERVER_UNAVAILIBLE,
  /** 33 */ IOTC_MQTT_BAD_USERNAME_OR_PASSWORD,
  /** 34 */ IOTC_MQTT_NOT_AUTHORIZED,
  /** 35 */ IOTC_MQTT_CONNECT_UNKNOWN_RETURN_CODE,
  /** 36 */ IOTC_MQTT_MESSAGE_CLASS_UNKNOWN_ERROR,
  /** 37 */ IOTC_MQTT_PAYLOAD_SIZE_TOO_LARGE,
  /** 38 */ IOTC_MQTT_SUBSCRIPTION_FAILED,
  /** 39 */ IOTC_MQTT_SUBSCRIPTION_SUCCESSFULL,
  /** 40 */ IOTC_INTERNAL_ERROR,
  /** 41 */ IOTC_NOT_INITIALIZED,
  /** 42 */ IOTC_FAILED_INITIALIZATION,
  /** 43 */ IOTC_ALREADY_INITIALIZED,
  /** 44 */ IOTC_INVALID_PARAMETER,
  /** 45 */ IOTC_UNSET_HANDLER_ERROR,
  /** 46 */ IOTC_NOT_IMPLEMENTED,
  /** 47 */ IOTC_ELEMENT_NOT_FOUND,
  /** 48 */ IOTC_SERIALIZATION_ERROR,
  /** 49 */ IOTC_TRUNCATION_WARNING,
  /** 50 */ IOTC_BUFFER_OVERFLOW,
  /** 51 */ IOTC_THREAD_ERROR,
  /** 52 */ IOTC_NULL_CONTEXT,
  /** 53 */ IOTC_NULL_WILL_TOPIC,
  /** 54 */ IOTC_NULL_WILL_MESSAGE,
  /** 55 */ IOTC_NO_MORE_RESOURCE_AVAILABLE,
  /** 56 */ IOTC_FS_RESOURCE_NOT_AVAILABLE,
  /** 57 */ IOTC_FS_ERROR,
  /** 58 */ IOTC_NOT_SUPPORTED,
  /** 59 */ IOTC_EVENT_PROCESS_STOPPED,
  /** 60 */ IOTC_STATE_RESEND,
  /** 61 */ IOTC_NULL_HOST,
  /** 62 */ IOTC_TLS_FAILED_CERT_ERROR,
  /** 63 */ IOTC_FS_OPEN_ERROR,
  /** 64 */ IOTC_FS_OPEN_READ_ONLY,
  /** 65 */ IOTC_FS_READ_ERROR,
  /** 66 */ IOTC_FS_WRITE_ERROR,
  /** 67 */ IOTC_FS_CLOSE_ERROR,
  /** 68 */ IOTC_FS_REMOVE_ERROR,
  /** 69 */ IOTC_NULL_PROJECT_ID_ERROR,
  /** 70 */ IOTC_ALG_NOT_SUPPORTED_ERROR,
  /** 71 */ IOTC_JWT_FORMATTION_ERROR,
  /** 72 */ IOTC_JWT_PROJECTID_TOO_LONG_ERROR,
  /** 73 */ IOTC_NULL_DEVICE_PATH_ERROR,
  /** 74 */ IOTC_BUFFER_TOO_SMALL_ERROR,
  /** 75 */ IOTC_NULL_KEY_DATA_ERROR,
  /** 76 */ IOTC_NULL_CLIENT_ID_ERROR,

  IOTC_ERROR_COUNT /* add above this line, and this should always be last. */
} iotc_state_t;

/**
 * @def
 * @brief undefine to enable the translation of iotc_state_t to human readable
 * strings
 */
#define IOTC_OPT_NO_ERROR_STRINGS 1

/**
 * @function
 * @brief Returns a string representation of the value of iotc_state_t.
 * If the passed value is not valid it returns a string indicating such.
 *
 * NOTE: If IOTC_OPT_NO_ERROR_STRINGS is not defined always returns an empty
 * string.
 *
 * @param [in] e the iotc_state_t to be translated to a string.
 *
 * @see iotc_state_t
 * @see IOTC_OPT_NO_ERROR_STRINGS
 *
 * @retval the string representation of e
 * @retval a string indicating that e is not valid if e is not in the range
 * IOTC_STATE_OK to IOTC_ERROR_COUNT-1
 * @retval an empty string if IOTC_OPT_NO_ERROR_STRINGS is not defined
 */
extern const char* iotc_get_state_string(iotc_state_t e);

#ifdef __cplusplus
}
#endif

#endif /* __IOTC_ERR_H__ */
