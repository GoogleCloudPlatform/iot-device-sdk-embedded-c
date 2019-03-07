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

#define IOTC_INVALID_CONTEXT_HANDLE -1
#define IOTC_INVALID_TIMED_TASK_HANDLE -1

/**
 * @name iotc_context_handle_t
 * @brief Internal context handle. No user action required.
 */
typedef int32_t iotc_context_handle_t;

/**
 * @name iotc_timed_task_handle_t
 * @brief Timed task handle to identify timed tasks.
 */
typedef int32_t iotc_timed_task_handle_t;

/**
 * @name iotc_user_task_callback_t
 * @brief Custom callback for timed tasks.
 * @detailed This is a user-defined custom callback. Pass the callback to
 * <code>iotc_schedule_timed_task()</code> repeatedly execute the callback at
 * the given interval.
 *
 * @param [in] in_context_handle the context handle provided to
 * <code>iotc_schedule_timed_task()</code>.
 * @param [in] timed_task_handle the handle that identifies the timed task.
 * @param [in] user_data the data provided to <code>iotc_schedule_timed_task()</code>.
 */
typedef void(iotc_user_task_callback_t)(
    const iotc_context_handle_t context_handle,
    const iotc_timed_task_handle_t timed_task_handle, void* user_data);

/**
 * @name     iotc_user_callback_t
 * @brief    Custom callback.
 * @detailed Callbacks are passed to <code>iotc_user_callback_t</code> Callbacks
 *           notify the client application after a function executes.
 *
 * @param [in] in_context_handle the context handle provided to the original
 * API call.
 * @param [in] data API-specific information. Cast to a specific type depending
 * on the original API call. For example, the <code>iotc_connect()</code>
 * callback returns the iotc_connection_data_t* type so <code>iotc_connection_data_t* conn_data = (iotc_connection_data_t*)data;</code>.
 * @param [in] state IOTC_STATE_OK if the connection succeeds. See <code><a href="~/include/iotc_error.h">iotc_error.h</a></code>
 * for more error codes.
 */
typedef void(iotc_user_callback_t)(iotc_context_handle_t in_context_handle,
                                   void* data, iotc_state_t state);

/**
 * @enum iotc_sub_call_type_t
 * @brief Determine the subscription callback type.
 * @detailed Establish the data type that <code>iotc_subscription_data_t()</code> 
 * passes to the custom callback.
 *
 *    - IOTC_SUB_UNKNOWN: unknown callback type. Check the state value.
 *    - IOTC_SUBSCRIPTION_DATA_SUBACK: callback is a SUBACK notification.
 *    - IOTC_SUBSCRIPTION_DATA_MESSAGE: callback is a MESSAGE notification.
 */
typedef enum iotc_subscription_data_type_e {
  IOTC_SUB_CALL_UNKNOWN = 0,
  IOTC_SUB_CALL_SUBACK,
  IOTC_SUB_CALL_MESSAGE
} iotc_sub_call_type_t;

/**
 * @union iotc_sub_call_params_u
 * @brief A union that describes data type passed to the subscription callback.
 *
 *    - suback: callback is a SUBACK notification.
 *    - message: callback is a MESSAGE notification.
 */
typedef union iotc_sub_call_params_u {
  struct {
    const char* topic;
    iotc_mqtt_suback_status_t suback_status;
  } suback;

  struct {
    const char* topic;
    const uint8_t* temporary_payload_data; /* automatically freed when the
                                              callback returns */
    size_t temporary_payload_data_length;
    iotc_mqtt_retain_t retain;
    iotc_mqtt_qos_t qos;
    iotc_mqtt_dup_t dup_flag;
  } message;
} iotc_sub_call_params_t;

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
 * @name     iotc_user_subscription_callback_t
 * @brief    Subscription callback.
 * @detailed The subscription callback notifies the user that the subscribed
 * topic recieved messages.
 *
 * @param [in] in_context_handle context on which the callback is invoked.
 * @param [in] call_type data type of the data parameter.
 * @param [in] params pointer to a structure that holds parameter details
 * @param [in] topic name of the topic
 * @param [in] user_data pointer specified when registering the subscription
 * callback.
 */
typedef void(iotc_user_subscription_callback_t)(
    iotc_context_handle_t in_context_handle, iotc_sub_call_type_t call_type,
    const iotc_sub_call_params_t* const params, iotc_state_t state,
    void* user_data);

/**
 * @enum iotc_crypto_key_union_type_t
 * @brief Describe the iotc_crypto_key_union_t union structure.
 *
 *     - IOTC_CRYPTO_KEY_UNION_TYPE_PEM: the key_pem.key pointer contains
 * a null-terminated PEM key string.
 *     - IOTC_CRYPTO_KEY_UNION_TYPE_SLOT_ID: a secure element for private_key
 * storage indexes keys by slots. Signal to the BSP the slot for cryptographic
 * operations.
 *     - IOTC_CRYPTO_KEY_UNION_TYPE_CUSTOM - the union contains untyped data. 
 * The BSP must know the data format.
 */

typedef enum iotc_crypto_key_union_type_e {
  IOTC_CRYPTO_KEY_UNION_TYPE_PEM = 0,
  IOTC_CRYPTO_KEY_UNION_TYPE_SLOT_ID,
  IOTC_CRYPTO_KEY_UNION_TYPE_CUSTOM,
} iotc_crypto_key_union_type_t;

/* @union iotc_crypto_key_union_t
 * @brief Describe the private key data.
 * @detailed Describe the private key data for the crypto BSP. The crypto BSP
 * signs JWTs.
 *
 *     - key: a PEM formatted private key.
 *     - key_slot: multi-slotted secure elements.
 *     - key_custom: untyped data.
 */
typedef union iotc_crypto_key_union_u {
  struct {
    char* key;
  } key_pem;

  struct {
    uint8_t slot_id;
  } key_slot;

  struct {
    void* data;
    size_t data_size;
  } key_custom;
} iotc_crypto_key_union_t;

/**
 * @enum  iotc_crypto_key_signature_algorithm_e
 * @brief Define a key signature algorithm to sign JWTs.
 *
 * This value also determines the JWT "alg" field, which the client application
 * automatically assembles when it connects to Cloud IoT Core. Cite the same
 * algorithm in <a href="https://cloud.google.com/iot/docs/how-tos/devices#creating_device_key_pairs">Cloud IoT Core</a>.
 *
 * <b>Note</b>: RSASSA-PKCS1-v1_5 with SHA-256 (RS256) is not supported.
 *
 * @retval IOTC_CRYPTO_KEY_SIGNATURE_ALGORITHM_INVALID for development.
 * @retval IOTC_CRYPTO_KEY_SIGNATURE_ALGORITHM_ES256 ECDSA with P-256 and
 *     SHA-256.
 */
typedef enum iotc_crypto_key_signature_algorithm_e {
  IOTC_CRYPTO_KEY_SIGNATURE_ALGORITHM_INVALID = 0,
  IOTC_CRYPTO_KEY_SIGNATURE_ALGORITHM_ES256
} iotc_crypto_key_signature_algorithm_t;

/* @struct iotc_crypto_key_data_t
 * @brief Set iotc_crypto_key_union.
 * 
 * iotc_crypto_key_union is an enumeration of the current union and an
 * enumeration of the key signature algorithm.
 *
 * Refer to <code>iotc_crypto_key_params_u()</code> for more information.
 *
 * @see iotc_crypto_key_params_u
 */
typedef struct {
  iotc_crypto_key_union_type_t crypto_key_union_type;
  iotc_crypto_key_union_t crypto_key_union;
  iotc_crypto_key_signature_algorithm_t crypto_key_signature_algorithm;
} iotc_crypto_key_data_t;

#ifdef __cplusplus
}
#endif

#endif /* __IOTC_TYPES_H__ */
