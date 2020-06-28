/* Copyright 2018-2019 Google LLC
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

#include "iotc_mock_layer_mqttlogic_next.h"
#include "iotc_itest_helpers.h"
#include "iotc_layer_macros.h"

#ifdef __cplusplus
extern "C" {
#endif

iotc_state_t iotc_mock_layer_mqttlogic_next_push(void* context, void* data,
                                                 iotc_state_t in_out_state) {
  IOTC_LAYER_FUNCTION_PRINT_FUNCTION_DIGEST();

  IOTC_UNUSED(context);
  IOTC_UNUSED(data);

  check_expected(in_out_state);

  if (NULL != data) {
    IOTC_PROCESS_PUSH_ON_PREV_LAYER(context, data, in_out_state);
  }

  return IOTC_STATE_OK;
}

iotc_state_t iotc_mock_layer_mqttlogic_next_pull(void* context, void* data,
                                                 iotc_state_t in_out_state) {
  IOTC_LAYER_FUNCTION_PRINT_FUNCTION_DIGEST();

  IOTC_UNUSED(context);
  IOTC_UNUSED(data);

  check_expected(in_out_state);

  return IOTC_STATE_OK;
}

iotc_state_t iotc_mock_layer_mqttlogic_next_close(void* context, void* data,
                                                  iotc_state_t in_out_state) {
  IOTC_LAYER_FUNCTION_PRINT_FUNCTION_DIGEST();

  IOTC_UNUSED(context);
  IOTC_UNUSED(data);

  check_expected(in_out_state);

  return IOTC_STATE_OK;
}

iotc_state_t iotc_mock_layer_mqttlogic_next_close_externally(
    void* context, void* data, iotc_state_t in_out_state) {
  IOTC_LAYER_FUNCTION_PRINT_FUNCTION_DIGEST();

  IOTC_UNUSED(context);
  IOTC_UNUSED(data);

  check_expected(in_out_state);

  return IOTC_STATE_OK;
}

iotc_state_t iotc_mock_layer_mqttlogic_next_init(void* context, void* data,
                                                 iotc_state_t in_out_state) {
  IOTC_LAYER_FUNCTION_PRINT_FUNCTION_DIGEST();

  check_expected(in_out_state);

  return IOTC_PROCESS_INIT_ON_PREV_LAYER(context, data, in_out_state);
}

iotc_state_t iotc_mock_layer_mqttlogic_next_connect(void* context, void* data,
                                                    iotc_state_t in_out_state) {
  IOTC_LAYER_FUNCTION_PRINT_FUNCTION_DIGEST();

  IOTC_UNUSED(context);
  IOTC_UNUSED(data);

  check_expected(in_out_state);

  if (IOTC_STATE_OK == in_out_state) {
    return IOTC_PROCESS_POST_CONNECT_ON_PREV_LAYER(context, NULL, in_out_state);
  } else {
    return IOTC_PROCESS_CLOSE_ON_PREV_LAYER(context, NULL, in_out_state);
  }

  return IOTC_STATE_OK;
}

#ifdef __cplusplus
}
#endif
