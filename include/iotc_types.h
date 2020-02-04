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

#ifndef __IOTC_TYPES_H__
#define __IOTC_TYPES_H__
#include <stddef.h>
#include <stdint.h>

#include <iotc_error.h>
#include <iotc_mqtt.h>

#ifdef __cplusplus
extern "C" {
#endif

/*! \file
 * @brief Defines custom data formats.
 */

/** A flag indicating an invalid
 * {@link iotc_create_context() context handle}. */
#define IOTC_INVALID_CONTEXT_HANDLE -1
/** A flag indicating an invalid
 * {@link ::iotc_timed_task_handle_t timed-task handle}. */
#define IOTC_INVALID_TIMED_TASK_HANDLE -1

/**
 * @typedef iotc_context_handle_t
 * @brief An internal context handle.
 */
typedef int32_t iotc_context_handle_t;

/**
 * @typedef iotc_timed_task_handle_t
 * @brief The handle to identify {@link ::iotc_timed_task_handle_t timed tasks}.
 */
typedef int32_t iotc_timed_task_handle_t;

/**
 * @typedef iotc_user_task_callback_t
 * @brief A custom callback for {@link ::iotc_timed_task_handle_t timed tasks}.
 *
 * @param [in] in_context_handle The
 *     {@link iotc_create_context() context handle} provided to the
 *     {@link iotc_schedule_timed_task() function that schedules timed tasks}.
 * @param [in] timed_task_handle The handle that identifies the timed task.
 * @param [in] user_data The data provided to the
 *     {@link iotc_schedule_timed_task() function that schedules timed tasks}
 */
typedef void(iotc_user_task_callback_t)(
    const iotc_context_handle_t context_handle,
    const iotc_timed_task_handle_t timed_task_handle, void* user_data);

/**
 * @typedef iotc_user_callback_t
 * @details A custom callback. The API-specific parameters are cast to the data
 * types in the API call.
 *
 * @param [in] in_context_handle The context handle provided to the original
 *     API call.
 * @param [in] data The API-specific parameters.
 * @param [in] state The {@link iotc_error.h state} on which to invoke the
 *     callback.
 */
typedef void(iotc_user_callback_t)(iotc_context_handle_t in_context_handle,
                                   void* data, iotc_state_t state);

/**
 * @typedef iotc_sub_call_type_t
 * @brief The data type of the user-defined subscription callback.
 *
 * @see #iotc_subscription_data_type_e
 */
typedef enum iotc_subscription_data_type_e {
  /** @details Unknown callback type. Check the state value. */
  IOTC_SUB_CALL_UNKNOWN = 0,
  /** @brief The callback is a SUBACK notification. */
  IOTC_SUB_CALL_SUBACK,
  /** @brief The callback is a MESSAGE notification. */
  IOTC_SUB_CALL_MESSAGE
} iotc_sub_call_type_t;

/**
 * @typedef iotc_sub_call_params_t
 * @brief The operational data for the user-defined
 * {@link ::iotc_user_subscription_callback_t subscription callback}.
 * @see #iotc_sub_call_params_u
 *
 * @union iotc_sub_call_params_u
 * @brief The operational data for the user-defined
 * {@link ::iotc_user_subscription_callback_t subscription callback}.
 */
typedef union iotc_sub_call_params_u {
  /** The MQTT
   * <a href="http://docs.oasis-open.org/mqtt/mqtt/v3.1.1/os/mqtt-v3.1.1-os.html#_Toc398718068">
   * SUBACK packet</a>.
   */
  struct {
    /** The MQTT topic. In Cloud IoT Core, you can publish telemetry events to
     * the
     * <a href="https://cloud.google.com/iot/docs/how-tos/mqtt-bridge#publishing_telemetry_events">/devices/DEVICE_ID/events</a>
     * topic and device state to the
     * <a href="https://cloud.google.com/iot/docs/how-tos/mqtt-bridge#setting_device_state">/devices/DEVICE_ID/state</a>
     * topic.
     */
    const char* topic;
    /** The MQTT 
     * <a href="http://docs.oasis-open.org/mqtt/mqtt/v3.1.1/os/mqtt-v3.1.1-os.html#_Toc398718071">
     * SUBACK payload</a>.
     */
    iotc_mqtt_suback_status_t suback_status;
  } suback;

  /** The MQTT
   * <a href="http://docs.oasis-open.org/mqtt/mqtt/v3.1.1/os/mqtt-v3.1.1-os.html#_Toc398718037">
   * PUBLISH packet</a>.
   */
  struct {
    const char* topic;
    /** @details The MQTT
     * <a href="http://docs.oasis-open.org/mqtt/mqtt/v3.1.1/os/mqtt-v3.1.1-os.html#_Toc398718040">
     * PUBLISH payload</a>. Automatically freed when the iotc_publish()
     * {@link ::iotc_user_callback_t callback} returns.
     */
    const uint8_t* temporary_payload_data;
    /** The length, in bytes, of the MQTT
     * <a href="http://docs.oasis-open.org/mqtt/mqtt/v3.1.1/os/mqtt-v3.1.1-os.html#_Toc398718040">
     * PUBLISH payload</a>.
     */
    size_t temporary_payload_data_length;
    /** The MQTT
     * <a href="http://docs.oasis-open.org/mqtt/mqtt/v3.1.1/os/mqtt-v3.1.1-os.html#_Toc398718030">
     * retain</a> flag.
     */
    iotc_mqtt_retain_t retain;
    /** The MQTT
     * <a href="http://docs.oasis-open.org/mqtt/mqtt/v3.1.1/os/mqtt-v3.1.1-os.html#_Toc398718099">
     * Quality of Service</a> levels.
     */
    iotc_mqtt_qos_t qos;
    /** The MQTT
     * <a href="http://docs.oasis-open.org/mqtt/mqtt/v3.1.1/os/mqtt-v3.1.1-os.html#_Toc398718038">
     * DUP</a> flag.
     */
    iotc_mqtt_dup_t dup_flag;
  } message;
} iotc_sub_call_params_t;

/** @brief The internal representations of empty
 * {@link iotc_subscribe() subscription} parameters. */
#define IOTC_EMPTY_SUB_CALL_PARAMS \
  (iotc_sub_call_params_t) {       \
    .message = {                   \
      NULL,                        \
      NULL,                        \
      0,                           \
      IOTC_MQTT_RETAIN_FALSE,      \
      IOTC_MQTT_QOS_AT_MOST_ONCE,  \
      IOTC_MQTT_DUP_FALSE          \
    }                              \
  }

/**
 * @typedef iotc_user_subscription_callback_t
 * @brief The {@link iotc_subscribe() subscription} callback.
 *
 * @param [in] in_context_handle The context on which the callback is invoked.
 * @param [in] call_type The data type of the data parameter.
 * @param [in] params A pointer to a structure that holds parameter details.
 * @param [in] topic The name of the topic.
 * @param [in] user_data A pointer specified when registering the subscription
 *     callback.
 */
typedef void(iotc_user_subscription_callback_t)(
    iotc_context_handle_t in_context_handle, iotc_sub_call_type_t call_type,
    const iotc_sub_call_params_t* const params, iotc_state_t state,
    void* user_data);

/**
 * @typedef iotc_crypto_key_union_type_t
 * @brief The internal code that represents the data type of the public or
 * private key.
 *
 * @see #iotc_crypto_key_union_type_e
 */
typedef enum iotc_crypto_key_union_type_e {
  /** @brief The {@link ::iotc_crypto_key_union_u public or private key data} is
      a null-terminated PEM string. */
  IOTC_CRYPTO_KEY_UNION_TYPE_PEM = 0,
  /** @details Slot IDs address the
      {@link ::iotc_crypto_key_union_u public or private key data}. These are
      the same slots from which the BSP reads the key. */
  IOTC_CRYPTO_KEY_UNION_TYPE_SLOT_ID,
  /** @details The {@link ::iotc_crypto_key_union_u public or private key data}
      is untyped. The BSP determines the data format. */
  IOTC_CRYPTO_KEY_UNION_TYPE_CUSTOM,
} iotc_crypto_key_union_type_t;

/** 
 * @typedef iotc_crypto_key_union_t
 * @brief The public or private key data.
 * @see iotc_crypto_key_union_u

 * @union iotc_crypto_key_union_u
 * @brief The public or private key data.
 */
typedef union iotc_crypto_key_union_u {
  /** A PEM-formatted public or private key. */
  struct {
    /** The text of the public or private key. */
    char* key;
  } key_pem;

  /** The slot IDs of secure elements. */
  struct {
    /** A slot ID. */
    uint8_t slot_id;
  } key_slot;

  /** @details Untyped data. The BSP determines the data format. */
  struct {
    /** The data in the format that the BSP determined. */
    void* data;
    /** The size, in bytes, of the untyped data. */
    size_t data_size;
  } key_custom;
} iotc_crypto_key_union_t;

/**
 * @typedef iotc_crypto_key_signature_algorithm_t
 * @brief The ES256 algorithm with which to sign
 * {@link iotc_create_iotcore_jwt() JWTs}.
 */
typedef enum iotc_crypto_key_signature_algorithm_e {
  /** The signature algorithm is invalid. */
  IOTC_CRYPTO_KEY_SIGNATURE_ALGORITHM_INVALID = 0,
  /** The signature algorithm is an ECDSA with P-256 and SHA-256. */
  IOTC_CRYPTO_KEY_SIGNATURE_ALGORITHM_ES256
} iotc_crypto_key_signature_algorithm_t;

/**
 * @typedef iotc_crypto_key_data_t
 * @struct iotc_crypto_key_data_t
 * @brief The parameters with which to create
 *     {@link iotc_create_iotcore_jwt() JWTs}.
 */
typedef struct {
  /** The internal code that represents the data type of the public or private
   * key. */
  iotc_crypto_key_union_type_t crypto_key_union_type;
  /** The public or private key data. */
  iotc_crypto_key_union_t crypto_key_union;
  /** The ES256 algorithm with which to sign
   * {@link iotc_create_iotcore_jwt() JWTs}. */
  iotc_crypto_key_signature_algorithm_t crypto_key_signature_algorithm;
} iotc_crypto_key_data_t;

#ifdef __cplusplus
}
#endif

#endif /* __IOTC_TYPES_H__ */
