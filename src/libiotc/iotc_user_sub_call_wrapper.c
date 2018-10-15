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

#include "iotc_user_sub_call_wrapper.h"

#include "iotc_globals.h"
#include "iotc_handle.h"
#include "iotc_mqtt_logic_layer_data.h"
#include "iotc_types.h"

iotc_state_t iotc_user_sub_call_wrapper(void* context, void* data,
                                        iotc_state_t in_state,
                                        void* client_callback, void* user_data,
                                        void* task_data) {
  /* Check for the existance of parameters - user data may be NULL. */
  if ((NULL == client_callback) || (NULL == task_data)) {
    return IOTC_INVALID_PARAMETER;
  }

  /* Local variables. */
  iotc_state_t state = IOTC_STATE_OK;
  iotc_context_handle_t context_handle = IOTC_INVALID_CONTEXT_HANDLE;
  iotc_sub_call_params_t params = IOTC_EMPTY_SUB_CALL_PARAMS;
  iotc_mqtt_message_t* msg = NULL;
  iotc_mqtt_suback_status_t status = IOTC_MQTT_SUBACK_FAILED;
  iotc_mqtt_task_specific_data_t* sub_data =
      (iotc_mqtt_task_specific_data_t*)task_data;

  /* Only if the library context is not null. */
  if (NULL != context) {
    state = iotc_find_handle_for_object(iotc_globals.context_handles_vector,
                                        context, &context_handle);
    IOTC_CHECK_STATE(state);
  }

  switch (in_state) {
    case IOTC_MQTT_SUBSCRIPTION_FAILED: {
      status = (iotc_mqtt_suback_status_t)(intptr_t)data;
      params.suback.suback_status = status;
      params.suback.topic = (const char*)sub_data->subscribe.topic;

      ((iotc_user_subscription_callback_t*)(client_callback))(
          context_handle, IOTC_SUB_CALL_SUBACK, &params, in_state, user_data);

      /* Now it's ok to free the data as it's no longer needed.*/
      iotc_mqtt_task_spec_data_free_subscribe_data(&sub_data);
    } break;
    case IOTC_MQTT_SUBSCRIPTION_SUCCESSFULL: {
      status = (iotc_mqtt_suback_status_t)(intptr_t)data;
      params.suback.suback_status = status;
      params.suback.topic = (const char*)sub_data->subscribe.topic;

      ((iotc_user_subscription_callback_t*)(client_callback))(
          context_handle, IOTC_SUB_CALL_SUBACK, &params, in_state, user_data);
    } break;
    case IOTC_STATE_OK: {
      msg = (iotc_mqtt_message_t*)data;

      params.message.temporary_payload_data =
          msg->publish.content ? msg->publish.content->data_ptr : NULL;
      params.message.temporary_payload_data_length =
          msg->publish.content ? msg->publish.content->length : 0;
      params.message.topic = (const char*)sub_data->subscribe.topic;

      in_state = iotc_mqtt_convert_to_qos(msg->common.common_u.common_bits.qos,
                                          &params.message.qos);
      IOTC_CHECK_STATE(in_state);

      in_state = iotc_mqtt_convert_to_dup(msg->common.common_u.common_bits.dup,
                                          &params.message.dup_flag);
      IOTC_CHECK_STATE(in_state);

      in_state = iotc_mqtt_convert_to_retain(
          msg->common.common_u.common_bits.retain, &params.message.retain);
      IOTC_CHECK_STATE(in_state);

      ((iotc_user_subscription_callback_t*)(client_callback))(
          context_handle, IOTC_SUB_CALL_MESSAGE, &params, in_state, user_data);
    } break;
    default: {
      ((iotc_user_subscription_callback_t*)(client_callback))(
          context_handle, IOTC_SUB_CALL_UNKNOWN, NULL, in_state, user_data);
    } break;
  };

  iotc_debug_format("user callback returned with state: %s, (%d)",
                    iotc_get_state_string(state), state);

err_handling:
  /* Call "destructor" on message. */
  iotc_mqtt_message_free(&msg);

  return state;
}
