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
 * @brief The state of the client application.
 */
typedef enum {
  /** The SDK function succeeded. @internal Numeric code: 0 @endinternal */ IOTC_STATE_OK = 0,
  /** A timeout occurred. @internal Numeric code: 1 @endinternal */ IOTC_STATE_TIMEOUT,
  /** @cond Internal. Numeric code: 2 */ IOTC_STATE_WANT_READ, /** @endcond */
  /** @cond Internal. Numeric code: 3 */ IOTC_STATE_WANT_WRITE, /** @endcond */
  /** @cond Internal. Numeric code: 4 */ IOTC_STATE_WRITTEN, /** @endcond */
  /** @cond Internal. Numeric code: 5 */ IOTC_STATE_FAILED_WRITING, /** @endcond */
  /** The backoff was applied. @internal Numeric code: 6 @endinternal */ IOTC_BACKOFF_TERMINAL,
  /** The device doesn't have enough memory to fulfull the request. @internal Numeric code: 7 @endinternal */ IOTC_OUT_OF_MEMORY,
  /** @cond Internal. Numeric code: 8 */ IOTC_SOCKET_INITIALIZATION_ERROR, /** @endcond */
  /** @cond Internal. Numeric code: 9 */ IOTC_SOCKET_GETHOSTBYNAME_ERROR, /** @endcond */
  /** @cond Internal. Numeric code: 10 */ IOTC_SOCKET_GETSOCKOPT_ERROR, /** @endcond */
  /** @cond Internal. Numeric code: 11 */ IOTC_SOCKET_ERROR, /** @endcond */
  /** @cond Internal. Numeric code: 12 */ IOTC_SOCKET_CONNECTION_ERROR, /** @endcond */
  /** @cond Internal. Numeric code: 13 */ IOTC_SOCKET_SHUTDOWN_ERROR, /** @endcond */
  /** @cond Internal. Numeric code: 14 */ IOTC_SOCKET_WRITE_ERROR, /** @endcond */
  /** @cond Internal. Numeric code: 15 */ IOTC_SOCKET_READ_ERROR, /** @endcond */
  /** The device isn't connected to an MQTT broker. @internal Numeric code: 16 @endinternal */ IOTC_SOCKET_NO_ACTIVE_CONNECTION_ERROR,
  /** @cond Internal. Numeric code: 17 */ IOTC_CONNECTION_RESET_BY_PEER_ERROR, /** @endcond */
  /** @cond Internal. Numeric code: 18 */ IOTC_FD_HANDLER_NOT_FOUND, /** @endcond */
  /** @cond Internal. Numeric code: 19 */ IOTC_TLS_INITALIZATION_ERROR, /** @endcond */
  /** @cond Internal. Numeric code: 20 */ IOTC_TLS_FAILED_LOADING_CERTIFICATE, /** @endcond */
  /** @cond Internal. Numeric code: 21 */ IOTC_TLS_CONNECT_ERROR, /** @endcond */
  /** @cond Internal. Numeric code: 22 */ IOTC_TLS_WRITE_ERROR, /** @endcond */
  /** @cond Internal. Numeric code: 23 */ IOTC_TLS_READ_ERROR, /** @endcond */
  /** @cond Internal. Numeric code: 24 */ IOTC_MQTT_SERIALIZER_ERROR, /** @endcond */
  /** @cond Internal. Numeric code: 25 */ IOTC_MQTT_PARSER_ERROR, /** @endcond */
  /** @cond Internal. Numeric code: 26 */ IOTC_MQTT_UNKNOWN_MESSAGE_ID, /** @endcond */
  /** @cond Internal. Numeric code: 27 */ IOTC_MQTT_LOGIC_UNKNOWN_TASK_ID, /** @endcond */
  /** @cond Internal. Numeric code: 28 */ IOTC_MQTT_LOGIC_WRONG_SCENARIO_TYPE, /** @endcond */
  /** @cond Internal. Numeric code: 29 */ IOTC_MQTT_LOGIC_WRONG_MESSAGE_RECEIVED, /** @endcond */
  /** @cond Internal. Numeric code: 30 */ IOTC_MQTT_UNACCEPTABLE_PROTOCOL_VERSION, /** @endcond */
  /** @cond Internal. Numeric code: 31 */ IOTC_MQTT_IDENTIFIER_REJECTED, /** @endcond */
  /** The MQTT server is not available. @internal Numeric code: 32 @endinternal */ IOTC_MQTT_SERVER_UNAVAILIBLE,
  /** The MQTT username or password is not correct. @internal Numeric code: 33 @endinternal */ IOTC_MQTT_BAD_USERNAME_OR_PASSWORD,
  /** @cond Internal. Numeric code: 34 */ IOTC_MQTT_NOT_AUTHORIZED, /** @endcond */
  /** @cond Internal. Numeric code: 35 */ IOTC_MQTT_CONNECT_UNKNOWN_RETURN_CODE, /** @endcond */
  /** @cond Internal. Numeric code: 36 */ IOTC_MQTT_MESSAGE_CLASS_UNKNOWN_ERROR, /** @endcond */
  /** @cond Internal. Numeric code: 37 */ IOTC_MQTT_PAYLOAD_SIZE_TOO_LARGE, /** @endcond */
  /** Can't subscribe to the MQTT topic. @internal Numeric code: 38 @endinternal */ IOTC_MQTT_SUBSCRIPTION_FAILED,
  /** @cond Internal. Numeric code: 39 */ IOTC_MQTT_SUBSCRIPTION_SUCCESSFULL, /** @endcond */
  /** Something went wrong. @internal Numeric code: 40 @endinternal */ IOTC_INTERNAL_ERROR,
  /** The SDK is not initialized. Run iotc_initialize() and try again. @internal Numeric code: 41 @endinternal */ IOTC_NOT_INITIALIZED,
  /** @cond Internal. Numeric code: 42 */ IOTC_FAILED_INITIALIZATION, /** @endcond */
  /** @cond Internal. Numeric code: 43 */ IOTC_ALREADY_INITIALIZED, /** @endcond */
  /** A parameter is missing or invalid. @internal Numeric code: 44 @endinternal */ IOTC_INVALID_PARAMETER,
  /** @cond Internal. Numeric code: 45 */ IOTC_UNSET_HANDLER_ERROR, /** @endcond */
  /** Can't create a JWT because the cryptography library in the Board Support Package isn't ported to your platform. To port the cryptography library, customize the <a href="../../bsp/html/d6/d01/iotc__bsp__crypto_8h.html">hardware-specific drivers and routines</a> for your device and then <a href="../../../porting_guide#bsp-code-porting-process">link the library to the client application</a>. @internal Numeric code: 46 @endinternal */ IOTC_NOT_IMPLEMENTED,
  /** @cond Internal. Numeric code: 47 */ IOTC_ELEMENT_NOT_FOUND, /** @endcond */
  /** @cond Internal. Numeric code: 48 */ IOTC_SERIALIZATION_ERROR, /** @endcond */
  /** @cond Internal. Numeric code: 49 */ IOTC_TRUNCATION_WARNING, /** @endcond */
  /** @cond Internal. Numeric code: 50 */ IOTC_BUFFER_OVERFLOW, /** @endcond */
  /** @cond Internal. Numeric code: 51 */ IOTC_THREAD_ERROR, /** @endcond */
  /** @cond Internal. Numeric code: 52 */ IOTC_NULL_CONTEXT, /** @endcond */
  /** @cond Internal. Numeric code: 53 */ IOTC_NULL_WILL_TOPIC, /** @endcond */
  /** @cond Internal. Numeric code: 54 */ IOTC_NULL_WILL_MESSAGE, /** @endcond */
  /** @cond Internal. Numeric code: 55 */ IOTC_NO_MORE_RESOURCE_AVAILABLE, /** @endcond */
  /** @cond Internal. Numeric code: 56 */ IOTC_FS_RESOURCE_NOT_AVAILABLE, /** @endcond */
  /** @cond Internal. Numeric code: 57 */ IOTC_FS_ERROR, /** @endcond */
  /** The memory limiter module isn't installed. To enable the memory limiter, set the <a href="../../../porting_guide.md#development-flags"><code>memory_limiter</code></a> flag in a <a href="../../../porting_guide.md#config">CONFIG</a> argument. @internal Numeric code: 58 @endinternal */ IOTC_NOT_SUPPORTED,
  /** The event processor stopped because the client application {@link iotc_events_stop() shut down the event engine}. @internal Numeric code: 59 @endinternal */ IOTC_EVENT_PROCESS_STOPPED,
  /** @cond Internal. Numeric code: 60 */ IOTC_STATE_RESEND, /** @endcond */
  /** @cond Internal. Numeric code: 61 */ IOTC_NULL_HOST, /** @endcond */
  /** @cond Internal. Numeric code: 62 */ IOTC_TLS_FAILED_CERT_ERROR, /** @endcond */
  /** @cond Internal. Numeric code: 63 */ IOTC_FS_OPEN_ERROR, /** @endcond */
  /** @cond Internal. Numeric code: 64 */ IOTC_FS_OPEN_READ_ONLY, /** @endcond */
  /** @cond Internal. Numeric code: 65 */ IOTC_FS_READ_ERROR, /** @endcond */
  /** @cond Internal. Numeric code: 66 */ IOTC_FS_WRITE_ERROR, /** @endcond */
  /** @cond Internal. Numeric code: 67 */ IOTC_FS_CLOSE_ERROR, /** @endcond */
  /** @cond Internal. Numeric code: 68 */ IOTC_FS_REMOVE_ERROR, /** @endcond */
  /** @cond Internal. Numeric code: 69 */ IOTC_NULL_PROJECT_ID_ERROR, /** @endcond */
  /** The algorithm of the private key for the JWT signature is not supported. @internal Numeric code: 70 @endinternal */ IOTC_ALG_NOT_SUPPORTED_ERROR,
  /** @cond Numeric code: 71 */ IOTC_JWT_FORMATTION_ERROR, /** @endcond */
  /** @cond Numeric code: 72 */ IOTC_JWT_PROJECTID_TOO_LONG_ERROR, /** @endcond */
  /** @cond Numeric code: 73 */ IOTC_NULL_DEVICE_PATH_ERROR, /** @endcond */
  /** The buffer is too small for the data. @internal Numeric code: 74 @endinternal */ IOTC_BUFFER_TOO_SMALL_ERROR,
  /** The buffer for storing formatted and signed JWTs is null. @internal Numeric code: 75 @endinternal */ IOTC_NULL_KEY_DATA_ERROR,
  /** @cond Numeric code: 76 */ IOTC_NULL_CLIENT_ID_ERROR, /** @endcond */

  /** @cond */ IOTC_ERROR_COUNT /** @endcond */ /* Add errors above this line; this should always be last line. */
} iotc_state_t;

/**
 * @brief A flag to translate between {@link ::iotc_state_t state messages} to their numeric codes.
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
