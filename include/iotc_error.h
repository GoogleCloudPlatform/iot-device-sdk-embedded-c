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
  /** @internal Numeric code: 2 */ IOTC_STATE_WANT_READ,
  /** @internal Numeric code: 3 */ IOTC_STATE_WANT_WRITE,
  /** @internal Numeric code: 4 */ IOTC_STATE_WRITTEN,
  /** @internal Numeric code: 5 */ IOTC_STATE_FAILED_WRITING,
  /** The backoff was applied. Numeric code: 6 */ IOTC_BACKOFF_TERMINAL,
  /** The device doesn't have enough memory to fulfull the request. Numeric code: 7 */ IOTC_OUT_OF_MEMORY,
  /** @internal Numeric code: 8 */ IOTC_SOCKET_INITIALIZATION_ERROR,
  /** @internal Numeric code: 9 */ IOTC_SOCKET_GETHOSTBYNAME_ERROR,
  /** @internal Numeric code: 10 */ IOTC_SOCKET_GETSOCKOPT_ERROR,
  /** @internal Numeric code: 11 */ IOTC_SOCKET_ERROR,
  /** @internal Numeric code: 12 */ IOTC_SOCKET_CONNECTION_ERROR,
  /** @internal Numeric code: 13 */ IOTC_SOCKET_SHUTDOWN_ERROR,
  /** @internal Numeric code: 14 */ IOTC_SOCKET_WRITE_ERROR,
  /** @internal Numeric code: 15 */ IOTC_SOCKET_READ_ERROR,
  /** The device isn't connected to an MQTT broker. Numeric code: 16 */ IOTC_SOCKET_NO_ACTIVE_CONNECTION_ERROR,
  /** @internal Numeric code: 17 */ IOTC_CONNECTION_RESET_BY_PEER_ERROR,
  /** @internal Numeric code: 18 */ IOTC_FD_HANDLER_NOT_FOUND,
  /** @internal Numeric code: 19 */ IOTC_TLS_INITALIZATION_ERROR,
  /** @internal Numeric code: 20 */ IOTC_TLS_FAILED_LOADING_CERTIFICATE,
  /** @internal Numeric code: 21 */ IOTC_TLS_CONNECT_ERROR,
  /** @internal Numeric code: 22 */ IOTC_TLS_WRITE_ERROR,
  /** @internal Numeric code: 23 */ IOTC_TLS_READ_ERROR,
  /** @internal Numeric code: 24 */ IOTC_MQTT_SERIALIZER_ERROR,
  /** @internal Numeric code: 25 */ IOTC_MQTT_PARSER_ERROR,
  /** @internal Numeric code: 26 */ IOTC_MQTT_UNKNOWN_MESSAGE_ID,
  /** @internal Numeric code: 27 */ IOTC_MQTT_LOGIC_UNKNOWN_TASK_ID,
  /** @internal Numeric code: 28 */ IOTC_MQTT_LOGIC_WRONG_SCENARIO_TYPE,
  /** @internal Numeric code: 29 */ IOTC_MQTT_LOGIC_WRONG_MESSAGE_RECEIVED,
  /** @internal Numeric code: 30 */ IOTC_MQTT_UNACCEPTABLE_PROTOCOL_VERSION,
  /** @internal Numeric code: 31 */ IOTC_MQTT_IDENTIFIER_REJECTED,
  /** The MQTT server is not available. Numeric code: 32 */ IOTC_MQTT_SERVER_UNAVAILIBLE,
  /** The MQTT username or password is not correct. Numeric code: 33 */ IOTC_MQTT_BAD_USERNAME_OR_PASSWORD,
  /** @internal Numeric code: 34 */ IOTC_MQTT_NOT_AUTHORIZED,
  /** @internal Numeric code: 35 */ IOTC_MQTT_CONNECT_UNKNOWN_RETURN_CODE,
  /** @internal Numeric code: 36 */ IOTC_MQTT_MESSAGE_CLASS_UNKNOWN_ERROR,
  /** @internal Numeric code: 37 */ IOTC_MQTT_PAYLOAD_SIZE_TOO_LARGE,
  /** Can't subscribe to the MQTT topic. Numeric code: 38 */ IOTC_MQTT_SUBSCRIPTION_FAILED,
  /** @internal Numeric code: 39 */ IOTC_MQTT_SUBSCRIPTION_SUCCESSFULL,
  /** Something went wrong. Numeric code: 40 */ IOTC_INTERNAL_ERROR,
  /** The SDK is not initialized. Run iotc_initialize() and try again. Numeric code: 41 */ IOTC_NOT_INITIALIZED,
  /** @internal Numeric code: 42 */ IOTC_FAILED_INITIALIZATION,
  /** @internal Numeric code: 43 */ IOTC_ALREADY_INITIALIZED,
  /** A parameter is missing or invalid. Numeric code: 44 */ IOTC_INVALID_PARAMETER,
  /** @internal Numeric code: 45 */ IOTC_UNSET_HANDLER_ERROR,
  /** Can't create a JSON Web Token because the cryptography library in the Board Support Package (BSP) isn't ported to your platform. To port the cryptography library, customize the <a href="../../bsp/html/d6/d01/iotc__bsp__crypto_8h.html">hardware-specific drivers and routines</a> for your device and then < a href="../../../porting_guide#bsp-code-porting-process">link the library to the client application</a>. Numeric code: 46 */ IOTC_NOT_IMPLEMENTED,
  /** @internal Numeric code: 47 */ IOTC_ELEMENT_NOT_FOUND,
  /** @internal Numeric code: 48 */ IOTC_SERIALIZATION_ERROR,
  /** @internal Numeric code: 49 */ IOTC_TRUNCATION_WARNING,
  /** @internal Numeric code: 50 */ IOTC_BUFFER_OVERFLOW,
  /** @internal Numeric code: 51 */ IOTC_THREAD_ERROR,
  /** @internal Numeric code: 52 */ IOTC_NULL_CONTEXT,
  /** @internal Numeric code: 53 */ IOTC_NULL_WILL_TOPIC,
  /** @internal Numeric code: 54 */ IOTC_NULL_WILL_MESSAGE,
  /** @internal Numeric code: 55 */ IOTC_NO_MORE_RESOURCE_AVAILABLE,
  /** @internal Numeric code: 56 */ IOTC_FS_RESOURCE_NOT_AVAILABLE,
  /** @internal Numeric code: 57 */ IOTC_FS_ERROR,
  /** The memory limiter module isn't installed. To enable the memory limiter, set the <a href="../../../porting_guide.md#development-flags"><code>memory_limiter</code></a> flag in a <a href="../../../porting_guide.md#config">CONFIG</a> argument. Numeric code: 58 */ IOTC_NOT_SUPPORTED,
  /** The event processor stopped because the client application {@link iotc_events_stop() shut down the event engine}. Numeric code: 59 */ IOTC_EVENT_PROCESS_STOPPED,
  /** @internal Numeric code: 60 */ IOTC_STATE_RESEND,
  /** @internal Numeric code: 61 */ IOTC_NULL_HOST,
  /** @internal Numeric code: 62 */ IOTC_TLS_FAILED_CERT_ERROR,
  /** @internal Numeric code: 63 */ IOTC_FS_OPEN_ERROR,
  /** @internal Numeric code: 64 */ IOTC_FS_OPEN_READ_ONLY,
  /** @internal Numeric code: 65 */ IOTC_FS_READ_ERROR,
  /** @internal Numeric code: 66 */ IOTC_FS_WRITE_ERROR,
  /** @internal Numeric code: 67 */ IOTC_FS_CLOSE_ERROR,
  /** @internal Numeric code: 68 */ IOTC_FS_REMOVE_ERROR,
  /** @internal Numeric code: 69 */ IOTC_NULL_PROJECT_ID_ERROR,
  /** The private key isn't signed with ES256. Numeric code: 70 */ IOTC_ALG_NOT_SUPPORTED_ERROR,
  /** @internal Numeric code: 71 */ IOTC_JWT_FORMATTION_ERROR,
  /** @internal Numeric code: 72 */ IOTC_JWT_PROJECTID_TOO_LONG_ERROR,
  /** @internal Numeric code: 73 */ IOTC_NULL_DEVICE_PATH_ERROR,
  /** The buffer is too small for the data. Numeric code: 74 */ IOTC_BUFFER_TOO_SMALL_ERROR,
  /** The buffer for storing formatted and signed JWTs is null. Numeric code: 75 */ IOTC_NULL_KEY_DATA_ERROR,
  /** @internal Numeric code: 76 */ IOTC_NULL_CLIENT_ID_ERROR,

  IOTC_ERROR_COUNT /* Add errors above this line; this should always be last line. */
} iotc_state_t;

/**
 * @brief A flag to translate between {::iotc_state_t state messages} to their numeric codes.
 */
#define IOTC_OPT_NO_ERROR_STRINGS 1

/**
 * @brief Gets the {@link ::iotc_state_t state message} associated with a numeric code.
 *
 * @param [in] e The numeric code of the state message.
 */
extern const char* iotc_get_state_string(iotc_state_t e);

#ifdef __cplusplus
}
#endif

#endif /* __IOTC_ERR_H__ */
