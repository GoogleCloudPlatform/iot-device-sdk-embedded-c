/* Copyright 2018 - 2019 Google LLC
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

#include "iotc_mock_layer_mqttlogic_prev.h"
#include "iotc_itest_helpers.h"
#include "iotc_layer_macros.h"
#include "iotc_tuples.h"

#ifdef __cplusplus
extern "C" {
#endif

iotc_state_t iotc_mock_layer_mqttlogic_prev_push(void* context, void* data,
                                                 iotc_state_t in_out_state) {
  IOTC_LAYER_FUNCTION_PRINT_FUNCTION_DIGEST();

  iotc_mqtt_message_t* msg = (iotc_mqtt_message_t*)data;

  /* iotc_debug_mqtt_message_dump( msg ); */

  check_expected(in_out_state);

  iotc_state_t response_state = mock_type(iotc_state_t);

  if (IOTC_STATE_OK == response_state) {
    /* mimic successful message send */
    const uint16_t msg_id = iotc_mqtt_get_message_id(msg);
    const iotc_mqtt_type_t msg_type =
        (iotc_mqtt_type_t)msg->common.common_u.common_bits.type;

    check_expected(data);

    iotc_mqtt_written_data_t* written_data =
        iotc_alloc_make_tuple(iotc_mqtt_written_data_t, msg_id, msg_type);

    iotc_mqtt_message_free(&msg);

    return IOTC_PROCESS_PUSH_ON_NEXT_LAYER(context, written_data,
                                           IOTC_STATE_WRITTEN);
  } else if (IOTC_CONNECTION_RESET_BY_PEER_ERROR ==
             response_state) /* this is just one error but we could create more
                                different scenarios using that system */
  {
    iotc_mqtt_message_free(&msg);

    return IOTC_PROCESS_CLOSE_EXTERNALLY_ON_NEXT_LAYER(
        context, NULL, IOTC_CONNECTION_RESET_BY_PEER_ERROR);
  }

  return IOTC_STATE_OK;
}

iotc_state_t iotc_mock_layer_mqttlogic_prev_pull(void* context, void* data,
                                                 iotc_state_t in_out_state) {
  IOTC_LAYER_FUNCTION_PRINT_FUNCTION_DIGEST();

  IOTC_UNUSED(context);
  IOTC_UNUSED(data);

  check_expected(in_out_state);

  return IOTC_STATE_OK;
}

iotc_state_t iotc_mock_layer_mqttlogic_prev_close(void* context, void* data,
                                                  iotc_state_t in_out_state) {
  IOTC_LAYER_FUNCTION_PRINT_FUNCTION_DIGEST();

  check_expected(in_out_state);

  return IOTC_PROCESS_CLOSE_EXTERNALLY_ON_THIS_LAYER(context, data,
                                                     in_out_state);
}

iotc_state_t iotc_mock_layer_mqttlogic_prev_close_externally(
    void* context, void* data, iotc_state_t in_out_state) {
  IOTC_LAYER_FUNCTION_PRINT_FUNCTION_DIGEST();

  check_expected(in_out_state);

  return IOTC_PROCESS_CLOSE_EXTERNALLY_ON_NEXT_LAYER(context, data,
                                                     in_out_state);
}

iotc_state_t iotc_mock_layer_mqttlogic_prev_init(void* context, void* data,
                                                 iotc_state_t in_out_state) {
  IOTC_LAYER_FUNCTION_PRINT_FUNCTION_DIGEST();

  check_expected(in_out_state);

  return IOTC_PROCESS_CONNECT_ON_THIS_LAYER(context, data, in_out_state);
}

iotc_state_t iotc_mock_layer_mqttlogic_prev_connect(void* context, void* data,
                                                    iotc_state_t in_out_state) {
  IOTC_LAYER_FUNCTION_PRINT_FUNCTION_DIGEST();

  check_expected(in_out_state);

  return IOTC_PROCESS_CONNECT_ON_NEXT_LAYER(context, data, in_out_state);
}

#ifdef __cplusplus
}
#endif
