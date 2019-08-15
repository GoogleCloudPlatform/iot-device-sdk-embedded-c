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
  /** @cond Numeric code: 2 */ IOTC_STATE_WANT_READ, /** @endcond */
  /** @cond Numeric code: 3 */ IOTC_STATE_WANT_WRITE, /** @endcond */
  /** @cond Numeric code: 4 */ IOTC_STATE_WRITTEN, /** @endcond */
  /** @cond Numeric code: 5 */ IOTC_STATE_FAILED_WRITING, /** @endcond */
  /** The backoff was applied. Numeric code: 6 */ IOTC_BACKOFF_TERMINAL,
  /** The device doesn't have enough memory to fulfull the request. Numeric code: 7 */ IOTC_OUT_OF_MEMORY,
  /** @cond Numeric code: 8 */ IOTC_SOCKET_INITIALIZATION_ERROR, /** @endcond */
  /** @cond Numeric code: 9 */ IOTC_SOCKET_GETHOSTBYNAME_ERROR, /** @endcond */
  /** @cond Numeric code: 10 */ IOTC_SOCKET_GETSOCKOPT_ERROR, /** @endcond */
  /** @cond Numeric code: 11 */ IOTC_SOCKET_ERROR, /** @endcond */
  /** @cond Numeric code: 12 */ IOTC_SOCKET_CONNECTION_ERROR, /** @endcond */
  /** @cond Numeric code: 13 */ IOTC_SOCKET_SHUTDOWN_ERROR, /** @endcond */
  /** @cond Numeric code: 14 */ IOTC_SOCKET_WRITE_ERROR, /** @endcond */
  /** @cond Numeric code: 15 */ IOTC_SOCKET_READ_ERROR, /** @endcond */
  /** The device isn't connected to an MQTT broker. Numeric code: 16 */ IOTC_SOCKET_NO_ACTIVE_CONNECTION_ERROR,
  /** @cond Numeric code: 17 */ IOTC_CONNECTION_RESET_BY_PEER_ERROR, /** @endcond */
  /** @cond Numeric code: 18 */ IOTC_FD_HANDLER_NOT_FOUND, /** @endcond */
  /** @cond Numeric code: 19 */ IOTC_TLS_INITALIZATION_ERROR, /** @endcond */
  /** @cond Numeric code: 20 */ IOTC_TLS_FAILED_LOADING_CERTIFICATE, /** @endcond */
  /** @cond Numeric code: 21 */ IOTC_TLS_CONNECT_ERROR, /** @endcond */
  /** @cond Numeric code: 22 */ IOTC_TLS_WRITE_ERROR, /** @endcond */
  /** @cond Numeric code: 23 */ IOTC_TLS_READ_ERROR, /** @endcond */
  /** @cond Numeric code: 24 */ IOTC_MQTT_SERIALIZER_ERROR, /** @endcond */
  /** @cond Numeric code: 25 */ IOTC_MQTT_PARSER_ERROR, /** @endcond */
  /** @cond Numeric code: 26 */ IOTC_MQTT_UNKNOWN_MESSAGE_ID, /** @endcond */
  /** @cond Numeric code: 27 */ IOTC_MQTT_LOGIC_UNKNOWN_TASK_ID, /** @endcond */
  /** @cond Numeric code: 28 */ IOTC_MQTT_LOGIC_WRONG_SCENARIO_TYPE, /** @endcond */
  /** @cond Numeric code: 29 */ IOTC_MQTT_LOGIC_WRONG_MESSAGE_RECEIVED, /** @endcond */
  /** @cond Numeric code: 30 */ IOTC_MQTT_UNACCEPTABLE_PROTOCOL_VERSION, /** @endcond */
  /** @cond Numeric code: 31 */ IOTC_MQTT_IDENTIFIER_REJECTED, /** @endcond */
  /** The MQTT server is not available. Numeric code: 32 */ IOTC_MQTT_SERVER_UNAVAILIBLE,
  /** The MQTT username or password is not correct. Numeric code: 33 */ IOTC_MQTT_BAD_USERNAME_OR_PASSWORD,
  /** @cond Numeric code: 34 */ IOTC_MQTT_NOT_AUTHORIZED, /** @endcond */
  /** @cond Numeric code: 35 */ IOTC_MQTT_CONNECT_UNKNOWN_RETURN_CODE, /** @endcond */
  /** @cond Numeric code: 36 */ IOTC_MQTT_MESSAGE_CLASS_UNKNOWN_ERROR, /** @endcond */
  /** @cond Numeric code: 37 */ IOTC_MQTT_PAYLOAD_SIZE_TOO_LARGE, /** @endcond */
  /** Can't subscribe to the MQTT topic. Numeric code: 38 */ IOTC_MQTT_SUBSCRIPTION_FAILED,
  /** @cond Numeric code: 39 */ IOTC_MQTT_SUBSCRIPTION_SUCCESSFULL, /** @endcond */
  /** Something went wrong. Numeric code: 40 */ IOTC_INTERNAL_ERROR,
  /** The SDK is not initialized. Run iotc_initialize() and try again. Numeric code: 41 */ IOTC_NOT_INITIALIZED,
  /** @cond Numeric code: 42 */ IOTC_FAILED_INITIALIZATION, /** @endcond */
  /** @cond Numeric code: 43 */ IOTC_ALREADY_INITIALIZED, /** @endcond */
  /** A parameter is missing or invalid. Numeric code: 44 */ IOTC_INVALID_PARAMETER,
  /** @cond Numeric code: 45 */ IOTC_UNSET_HANDLER_ERROR, /** @endcond */
  /** Can't create a JSON Web Token because the cryptography library in the Board Support Package (BSP) isn't ported to your platform. To port the cryptography library, customize the <a href="../../bsp/html/d6/d01/iotc__bsp__crypto_8h.html">hardware-specific drivers and routines</a> for your device and then <a href="../../../porting_guide#bsp-code-porting-process">link the library to the client application</a>. <br> Numeric code: 46 */ IOTC_NOT_IMPLEMENTED,
  /** @cond Numeric code: 47 */ IOTC_ELEMENT_NOT_FOUND, /** @endcond */
  /** @cond Numeric code: 48 */ IOTC_SERIALIZATION_ERROR, /** @endcond */
  /** @cond Numeric code: 49 */ IOTC_TRUNCATION_WARNING, /** @endcond */
  /** @cond Numeric code: 50 */ IOTC_BUFFER_OVERFLOW, /** @endcond */
  /** @cond Numeric code: 51 */ IOTC_THREAD_ERROR, /** @endcond */
  /** @cond Numeric code: 52 */ IOTC_NULL_CONTEXT, /** @endcond */
  /** @cond Numeric code: 53 */ IOTC_NULL_WILL_TOPIC, /** @endcond */
  /** @cond Numeric code: 54 */ IOTC_NULL_WILL_MESSAGE, /** @endcond */
  /** @cond Numeric code: 55 */ IOTC_NO_MORE_RESOURCE_AVAILABLE, /** @endcond */
  /** @cond Numeric code: 56 */ IOTC_FS_RESOURCE_NOT_AVAILABLE, /** @endcond */
  /** @cond Numeric code: 57 */ IOTC_FS_ERROR, /** @endcond */
  /** The memory limiter module isn't installed. To enable the memory limiter, set the <a href="../../../porting_guide.md#development-flags"><code>memory_limiter</code></a> flag in a <a href="../../../porting_guide.md#config">CONFIG</a> argument. <br> Numeric code: 58 */ IOTC_NOT_SUPPORTED,
  /** The event processor stopped because the client application {@link iotc_events_stop() shut down the event engine}. Numeric code: 59 */ IOTC_EVENT_PROCESS_STOPPED,
  /** @cond Numeric code: 60 */ IOTC_STATE_RESEND, /** @endcond */
  /** @cond Numeric code: 61 */ IOTC_NULL_HOST, /** @endcond */
  /** @cond Numeric code: 62 */ IOTC_TLS_FAILED_CERT_ERROR, /** @endcond */
  /** @cond Numeric code: 63 */ IOTC_FS_OPEN_ERROR, /** @endcond */
  /** @cond Numeric code: 64 */ IOTC_FS_OPEN_READ_ONLY, /** @endcond */
  /** @cond Numeric code: 65 */ IOTC_FS_READ_ERROR, /** @endcond */
  /** @cond Numeric code: 66 */ IOTC_FS_WRITE_ERROR, /** @endcond */
  /** @cond Numeric code: 67 */ IOTC_FS_CLOSE_ERROR, /** @endcond */
  /** @cond Numeric code: 68 */ IOTC_FS_REMOVE_ERROR, /** @endcond */
  /** @cond Numeric code: 69 */ IOTC_NULL_PROJECT_ID_ERROR, /** @endcond */
  /** The private key isn't signed with ES256. Numeric code: 70 */ IOTC_ALG_NOT_SUPPORTED_ERROR,
  /** @cond Numeric code: 71 */ IOTC_JWT_FORMATTION_ERROR, /** @endcond */
  /** @cond Numeric code: 72 */ IOTC_JWT_PROJECTID_TOO_LONG_ERROR, /** @endcond */
  /** @cond Numeric code: 73 */ IOTC_NULL_DEVICE_PATH_ERROR, /** @endcond */
  /** The buffer is too small for the data. Numeric code: 74 */ IOTC_BUFFER_TOO_SMALL_ERROR,
  /** The buffer for storing formatted and signed JWTs is null. Numeric code: 75 */ IOTC_NULL_KEY_DATA_ERROR,
  /** @cond Numeric code: 76 */ IOTC_NULL_CLIENT_ID_ERROR, /** @endcond */

  /** @cond */IOTC_ERROR_COUNT /** @endcond Add errors above this line; this should always be last line. */
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
