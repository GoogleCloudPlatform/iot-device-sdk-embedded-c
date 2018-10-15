/* Copyright 2018 Google LLC
 *
 * This is part of the Google Cloud IoT Edge Embedded C Client,
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
 * @name    iotc_context_handle_t
 * @brief   Internal context handle, no user action required on these type
 * variables apart from passing them around properly through API calls.
 */
typedef int32_t iotc_context_handle_t;

/**
 * @name    iotc_timed_task_handle_t
 * @brief   Timed task handle to facilitate timed tasks identification.
 */
typedef int32_t iotc_timed_task_handle_t;

/**
 * @name    iotc_user_task_callback_t
 * @brief   User custom action which can be passed to the
 * iotc_schedule_timed_task to have it executed in the given time intervals.
 *
 * @param [in]  in_context_handle the context handle provided to
 * iotc_schedule_timed_task.
 * @param [in]  timed_task_handle is the handle that identifies timed task and
 * it allows to manipulate the timed task from within the callback e.g. to
 * cancel the task if it's no longer needed.
 * @param [in]  user_data is the data provided to iotc_schedule_timed_task.
 */
typedef void(iotc_user_task_callback_t)(
    const iotc_context_handle_t context_handle,
    const iotc_timed_task_handle_t timed_task_handle, void* user_data);

/**
 * @name    iotc_user_callback_t
 * @brief   Some API functions notify the application about the result through
 *          callbacks, these callbacks can be passed as iotc_user_callback_t to
 *          the API.
 *
 * @param [in]  in_context_handle the context handle which was provided to the
 *              original API call.
 * @param [in]  data contains API specific information. Before usage it should
 *              be cast to specific type depending on the original API call.
 *              E.g. iotc_connect's callback receives iotc_connection_data_t*
 *              in the data attribute. Thus its usage is as follows:
 *              iotc_connection_data_t* conn_data =
 *              (iotc_connection_data_t*)data;
 * @param [in]  state is the result of the API call. IOTC_STATE_OK in case of
 *              success. For other error codes please see the IoTC User Guide
 *              or Examples.
 */
typedef void(iotc_user_callback_t)(iotc_context_handle_t in_context_handle,
                                   void* data, iotc_state_t state);

/**
 * @enum iotc_sub_call_type_t
 * @brief determines the subscription callback type and the data passed to the
 * user callback through iotc_subscription_data_t.
 *
 *    - IOTC_SUB_UNKNOWN - whenever the type of the call is not known (user
 * should check the state value.)
 *    - IOTC_SUBSCRIPTION_DATA_SUBACK - callback is a SUBACK notification thus
 * suback part should be used from the params.
 * IOTC_SUBSCRIPTION_DATA_MESSAGE - callback is a MESSAGE notification thus
 * message part should be used from the params.
 */
typedef enum iotc_subscription_data_type_e {
  IOTC_SUB_CALL_UNKNOWN = 0,
  IOTC_SUB_CALL_SUBACK,
  IOTC_SUB_CALL_MESSAGE
} iotc_sub_call_type_t;

/**
 * @union iotc_sub_call_params_u
 * @brief A union which describes the type of data that is passed through the
 * user subscription callback
 *
 * suback - contains information important from perspective of processing mqtt
 * suback by user
 *
 * message - contains information important from perspective of processing mqtt
 * message on subscribed topic
 */
typedef union iotc_sub_call_params_u {
  struct {
    const char* topic;
    iotc_mqtt_suback_status_t suback_status;
  } suback;

  struct {
    const char* topic;
    const uint8_t* temporary_payload_data; /* automatically free'd when the
                                              callback returned */
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
 * @name  iotc_user_subscription_callback_t
 * @brief subscription callback used to notify user about changes in topic
 * subscription and about messages that has been received on subscribed topics
 *
 * @param in_context_handle - context on which callback has been invoked
 * @param call_type - gives user information about which information type is
 * passed in data parameter
 * @param params - pointer to a structure that holds the details
 * @param topic - name of the topic
 * @param user_data - pointer previously registered via user during the
 * subscription callback registration, may be used for identification or
 * carrying some helper data
 */
typedef void(iotc_user_subscription_callback_t)(
    iotc_context_handle_t in_context_handle, iotc_sub_call_type_t call_type,
    const iotc_sub_call_params_t* const params, iotc_state_t state,
    void* user_data);

/**
 * @enum iotc_crypto_key_union_type_t
 * @brief used to denote how the iotc_crypto_key_union_t union structure
 * should be evaluated.
 *
 *      - IOTC_CRYPTO_KEY_UNION_TYPE_PEM - if the key_pem.key pointer contains
 * a null terminated PEM key string.
 *      - IOTC_CRYPTO_KEY_UNION_TYPE_SLOT_ID - if the implementation is using
 * a secure element for private_key storage which indexes keys by slots,  then
 * this parameter will signal to the BSP which key storage slot to use for
 * cryptographic operations.
 *      - IOTC_CRYPTO_KEY_UNION_TYPE_CUSTOM - the uinion contains untyped data
 * which will be passed to the BSP for cryptographic operations. The BSP must
 * know the format of this data for its proper use.
 */

typedef enum iotc_crypto_key_union_type_e {
  IOTC_CRYPTO_KEY_UNION_TYPE_PEM = 0,
  IOTC_CRYPTO_KEY_UNION_TYPE_SLOT_ID,
  IOTC_CRYPTO_KEY_UNION_TYPE_CUSTOM,
} iotc_crypto_key_union_type_t;

/* @union iotc_crypto_key_union_t
 * @brief A union which describes the private key data which will be passed to
 * the crypto BSP to handle JWT signatures.  There are three options:
 *
 * 1. key: pass a byte array to the PEM formatted private key.
 * 2. key_slot: used for multi-slotted secure elements which can store
 * private_keys in secure storage and use them to sign data.
 * 3. key_custom: a catch all for other options where the client application
 * may pass untyped data to the BSP crypto implementation.
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
 * @enum iotc_jwt_private_key_signature_algorithm
 * @brief specifies the algorithm that will be used to sign the device's JWT.
 * This should match the algorithm type that was used to provision the device's
 * public key in Google IoT Core.
 *
 * The value provided here will also determine the "alg" field of the JWT, which
 * is automatically assembled by the IoTC Client during the connection process.
 *
 * IOTC_JWT_PRIVATE_KEY_SIGNATURE_ALGORITHM_INVALID - for internal use only.
 * IOTC_JWT_PRIVATE_KEY_SIGNATURE_ALGORITHM_ES256 - to use ECDSA using P-256
 * and SHA-256 to compute the JWT signature.
 *
 * NOTE: RSASSA-PKCS1-v1_5 using SHA-256 (aka RS256) is currently not supported.
 */
typedef enum iotc_jwt_private_key_signature_algorithm_e {
  IOTC_JWT_PRIVATE_KEY_SIGNATURE_ALGORITHM_INVALID = 0,
  IOTC_JWT_PRIVATE_KEY_SIGNATURE_ALGORITHM_ES256
} iotc_jwt_private_key_signature_algorithm_t;

/* @struct iotc_crypto_private_key_data_t
 * @brief A structure that contains a iotc_crypto_key_params union, an
 * enumeration of which configuration of the union that's being used,
 * and an enumeration of the signature algorithm of the key.
 *
 * For more information please see the fields and documentation of
 * iotc_crypto_key_params_u.
 *
 * @see iotc_crypto_key_params_u
 */
typedef struct {
  iotc_crypto_key_union_type_t private_key_union_type;
  iotc_crypto_key_union_t private_key_union;
  iotc_jwt_private_key_signature_algorithm_t private_key_signature_algorithm;
} iotc_crypto_private_key_data_t;

#ifdef __cplusplus
}
#endif

#endif /* __IOTC_TYPES_H__ */
