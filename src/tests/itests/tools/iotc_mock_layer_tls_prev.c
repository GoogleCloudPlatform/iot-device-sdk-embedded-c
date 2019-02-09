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

#include "iotc_mock_layer_tls_prev.h"
#include "iotc_globals.h"
#include "iotc_itest_helpers.h"
#include "iotc_itest_mock_broker_layerchain.h"
#include "iotc_layer_macros.h"

#ifdef __cplusplus
extern "C" {
#endif

extern iotc_context_t* iotc_context_mockbroker;  // test mock broker context

iotc_mock_broker_control_t
iotc_mock_layer_tls_prev__check_expected__LAYER_LEVEL() {
  return mock_type(iotc_mock_broker_control_t);
}

#define IOTC_MOCK_BROKER_CONDITIONAL__CHECK_EXPECTED(variable_to_check, level) \
  if (CONTROL_SKIP_CHECK_EXPECTED !=                                           \
      iotc_mock_layer_tls_prev__check_expected__##level()) {                   \
    check_expected(variable_to_check);                                         \
  }

iotc_state_t iotc_mock_layer_tls_prev_push(void* context, void* data,
                                           iotc_state_t in_out_state) {
  IOTC_LAYER_FUNCTION_PRINT_FUNCTION_DIGEST();

  IOTC_UNUSED(itest_mock_broker_codec_layer_chain);
  IOTC_UNUSED(IOTC_LAYER_CHAIN_MOCK_BROKER_CODECSIZE_SUFFIX);

  IOTC_MOCK_BROKER_CONDITIONAL__CHECK_EXPECTED(in_out_state, LAYER_LEVEL);

  const iotc_mock_layer_tls_prev_control_t mock_control_directive =
      mock_type(iotc_mock_layer_tls_prev_control_t);

  iotc_data_desc_t* data_desc = (iotc_data_desc_t*)data;
  iotc_free_desc(&data_desc);

  switch (mock_control_directive) {
    case CONTROL_TLS_PREV_CONTINUE:
      break;
    case CONTROL_TLS_PREV_PUSH__RETURN_MESSAGE: {
      const char* string_to_send_back = mock_type(const char*);

      if (string_to_send_back != 0) {
        iotc_data_desc_t* message_to_send_back =
            iotc_make_desc_from_string_copy(string_to_send_back);

        IOTC_PROCESS_PULL_ON_THIS_LAYER(context, message_to_send_back,
                                        IOTC_STATE_OK);
      }
    } break;
    case CONTROL_TLS_PREV_PUSH__WRITE_ERROR: {
      iotc_state_t state_to_return = mock_type(iotc_state_t);

      return IOTC_PROCESS_PUSH_ON_NEXT_LAYER(context, NULL, state_to_return);
    } break;
    case CONTROL_TLS_PREV_CLOSE:
      return IOTC_PROCESS_CLOSE_ON_THIS_LAYER(context, NULL,
                                              mock_type(iotc_state_t));
      break;
    default:
      break;
  }

  // pretend always successful send
  return IOTC_PROCESS_PUSH_ON_NEXT_LAYER(context, NULL, IOTC_STATE_WRITTEN);
}

iotc_state_t iotc_mock_layer_tls_prev_pull(void* context, void* data,
                                           iotc_state_t in_out_state) {
  IOTC_LAYER_FUNCTION_PRINT_FUNCTION_DIGEST();

  IOTC_MOCK_BROKER_CONDITIONAL__CHECK_EXPECTED(in_out_state, LAYER_LEVEL);

  return IOTC_PROCESS_PULL_ON_NEXT_LAYER(context, data, in_out_state);
}

iotc_state_t iotc_mock_layer_tls_prev_close(void* context, void* data,
                                            iotc_state_t in_out_state) {
  IOTC_LAYER_FUNCTION_PRINT_FUNCTION_DIGEST();

  IOTC_MOCK_BROKER_CONDITIONAL__CHECK_EXPECTED(in_out_state, LAYER_LEVEL);

  /* Call close on the mockbroker chain */
  if (NULL != iotc_context_mockbroker) {
    iotc_evtd_execute_in(
        iotc_globals.evtd_instance,
        iotc_make_handle(iotc_itest_find_layer(iotc_context_mockbroker,
                                               IOTC_LAYER_TYPE_MOCKBROKER_TOP)
                             ->layer_funcs->close,
                         &iotc_itest_find_layer(iotc_context_mockbroker,
                                                IOTC_LAYER_TYPE_MOCKBROKER_TOP)
                              ->layer_connection,
                         data, in_out_state),
        1, NULL);
  }

  return IOTC_PROCESS_CLOSE_EXTERNALLY_ON_THIS_LAYER(context, data,
                                                     in_out_state);
}

iotc_state_t iotc_mock_layer_tls_prev_close_externally(
    void* context, void* data, iotc_state_t in_out_state) {
  IOTC_LAYER_FUNCTION_PRINT_FUNCTION_DIGEST();

  IOTC_MOCK_BROKER_CONDITIONAL__CHECK_EXPECTED(in_out_state, LAYER_LEVEL);

  return IOTC_PROCESS_CLOSE_EXTERNALLY_ON_NEXT_LAYER(context, data,
                                                     in_out_state);
}

iotc_state_t iotc_mock_layer_tls_prev_init(void* context, void* data,
                                           iotc_state_t in_out_state) {
  IOTC_LAYER_FUNCTION_PRINT_FUNCTION_DIGEST();

  IOTC_MOCK_BROKER_CONDITIONAL__CHECK_EXPECTED(in_out_state, LAYER_LEVEL);

  return IOTC_PROCESS_CONNECT_ON_THIS_LAYER(context, data, in_out_state);
}

iotc_state_t iotc_mock_layer_tls_prev_connect(void* context, void* data,
                                              iotc_state_t in_out_state) {
  IOTC_LAYER_FUNCTION_PRINT_FUNCTION_DIGEST();

  IOTC_MOCK_BROKER_CONDITIONAL__CHECK_EXPECTED(in_out_state, LAYER_LEVEL);

  return IOTC_PROCESS_CONNECT_ON_NEXT_LAYER(context, data, in_out_state);
}

#ifdef __cplusplus
}
#endif
