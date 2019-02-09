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

#include <iotc.h>
#include <iotc_itest_mock_broker_layerchain.h>
#include <iotc_macros.h>
#include "iotc_backoff_status_api.h"
#include "iotc_globals.h"
#include "iotc_handle.h"
#include "iotc_itest_helpers.h"
#include "iotc_itest_layerchain_ct_ml_mc.h"
#include "iotc_memory_checks.h"

/* Depends on the iotc_itest_tls_error.c */
extern iotc_context_t* iotc_context;
extern iotc_context_handle_t iotc_context_handle;
extern iotc_context_t* iotc_context_mockbroker;
/* end of dependency */

int iotc_itest_mqtt_keepalive_setup(void** fixture_void) {
  IOTC_UNUSED(fixture_void);

  iotc_memory_limiter_tearup();

  iotc_globals.backoff_status.backoff_lut_i = 0;
  iotc_cancel_backoff_event();

  iotc_initialize();

  IOTC_CHECK_STATE(iotc_create_context_with_custom_layers(
      &iotc_context, itest_ct_ml_mc_layer_chain, IOTC_LAYER_CHAIN_CT_ML_MC,
      IOTC_LAYER_CHAIN_SCHEME_LENGTH(IOTC_LAYER_CHAIN_CT_ML_MC)));

  iotc_find_handle_for_object(iotc_globals.context_handles_vector, iotc_context,
                              &iotc_context_handle);

  IOTC_CHECK_STATE(iotc_create_context_with_custom_layers(
      &iotc_context_mockbroker, itest_mock_broker_codec_layer_chain,
      IOTC_LAYER_CHAIN_MOCK_BROKER_CODEC,
      IOTC_LAYER_CHAIN_SCHEME_LENGTH(IOTC_LAYER_CHAIN_MOCK_BROKER_CODEC)));

  return 0;

err_handling:
  fail();

  return 1;
}

int iotc_itest_mqtt_keepalive_teardown(void** fixture_void) {
  IOTC_UNUSED(fixture_void);

  iotc_delete_context(iotc_context_handle);
  iotc_delete_context_with_custom_layers(
      &iotc_context_mockbroker, itest_mock_broker_codec_layer_chain,
      IOTC_LAYER_CHAIN_SCHEME_LENGTH(IOTC_LAYER_CHAIN_MOCK_BROKER_CODEC));

  iotc_shutdown();

  return !iotc_memory_limiter_teardown();
}

void _iotc_itest_mqtt_keepalive__on_connection_state_changed(
    iotc_context_handle_t in_context_handle, void* data, iotc_state_t state) {
  IOTC_UNUSED(in_context_handle);
  IOTC_UNUSED(data);
  IOTC_UNUSED(state);

#ifndef IOTC_MODULE_THREAD_ENABLED
  check_expected(state);
#endif
}

/*********************************************************************************
 * act
 ****************************************************************************
 ********************************************************************************/
static void iotc_itest_mqtt_keepalive__act(
    void** fixture_void, const uint16_t loop_id_reset_by_peer) {
  IOTC_UNUSED(fixture_void);
  {
    /* turn off LAYER and MQTT LEVEL expectation checks to concentrate only on
     * SFT protocol messages */
    will_return_always(iotc_mock_broker_layer__check_expected__LAYER_LEVEL,
                       CONTROL_SKIP_CHECK_EXPECTED);

    will_return_always(iotc_mock_broker_layer__check_expected__MQTT_LEVEL,
                       CONTROL_SKIP_CHECK_EXPECTED);

    will_return_always(iotc_mock_layer_tls_prev__check_expected__LAYER_LEVEL,
                       CONTROL_SKIP_CHECK_EXPECTED);
  }

  IOTC_PROCESS_INIT_ON_THIS_LAYER(
      &iotc_context_mockbroker->layer_chain.top->layer_connection, NULL,
      IOTC_STATE_OK);

  iotc_evtd_step(iotc_globals.evtd_instance,
                 iotc_bsp_time_getcurrenttime_seconds());

  const uint16_t loop_counter_max = 23;
  const uint16_t loop_counter_disconnect = 18;

  iotc_connect(iotc_context_handle, "itest_username", "itest_password",
               "itest_client_id", /*connection_timeout=*/20,
               /*keepalive_timeout=*/5,
               &_iotc_itest_mqtt_keepalive__on_connection_state_changed);

  uint16_t loop_counter = 1;
  while (iotc_evtd_dispatcher_continue(iotc_globals.evtd_instance) == 1 &&
         loop_counter < loop_counter_max) {
    // printf( "loop_counter = %d\n", loop_counter );

    iotc_evtd_step(iotc_globals.evtd_instance,
                   iotc_bsp_time_getcurrenttime_seconds() + loop_counter);
    ++loop_counter;

    if (loop_id_reset_by_peer == loop_counter) {
      IOTC_PROCESS_CLOSE_ON_THIS_LAYER(
          &iotc_context->layer_chain.bottom->layer_connection, NULL,
          IOTC_CONNECTION_RESET_BY_PEER_ERROR);
    }

    if (loop_counter_disconnect == loop_counter) {
      iotc_shutdown_connection(iotc_context_handle);
    }
  }
}

void iotc_itest_mqtt_keepalive__PINGREQ_failed_to_send__client_disconnects_after_keepalive_seconds(
    void** state) {
  /* let CONNECT and SUBSCRIBE writes succeed */
#ifdef IOTC_CONTROL_TOPIC_ENABLED
  will_return_count(iotc_mock_layer_tls_prev_push, CONTROL_TLS_PREV_CONTINUE,
                    2);
#else
  will_return_count(iotc_mock_layer_tls_prev_push, CONTROL_TLS_PREV_CONTINUE,
                    1);
#endif

  /* make first PINGREQ fail to write on the socket */
  will_return(iotc_mock_layer_tls_prev_push,
              CONTROL_TLS_PREV_PUSH__WRITE_ERROR);
  /* set the specific error to be returned by the mock IO layer (here tls_prev)
   */
  will_return(iotc_mock_layer_tls_prev_push, IOTC_STATE_FAILED_WRITING);

  /* make mock broker not to reply on PINGREQ, this is necessary since the
   * mock broker catches the message before IO layer */
  will_return(iotc_mock_broker_layer_pull,
              CONTROL_PULL_PINGREQ_SUPPRESS_RESPONSE);

#ifndef IOTC_MODULE_THREAD_ENABLED
  expect_value(_iotc_itest_mqtt_keepalive__on_connection_state_changed, state,
               IOTC_STATE_OK);
  expect_value(_iotc_itest_mqtt_keepalive__on_connection_state_changed, state,
               IOTC_STATE_TIMEOUT);
#endif

  iotc_itest_mqtt_keepalive__act(state, 0);
}

void iotc_itest_mqtt_keepalive__PINGREQ_failed_to_send__broker_disconnects_first(
    void** state) {
  /* let CONNECT and SUBSCRIBE writes succeed */
#ifdef IOTC_CONTROL_TOPIC_ENABLED
  will_return_count(iotc_mock_layer_tls_prev_push, CONTROL_TLS_PREV_CONTINUE,
                    2);
#else
  will_return_count(iotc_mock_layer_tls_prev_push, CONTROL_TLS_PREV_CONTINUE,
                    1);
#endif

  /* make first PINGREQ fail to write on the socket */
  will_return(iotc_mock_layer_tls_prev_push,
              CONTROL_TLS_PREV_PUSH__WRITE_ERROR);
  /* set the specific error to be returned by the mock IO layer (here tls_prev)
   */
  will_return(iotc_mock_layer_tls_prev_push, IOTC_STATE_FAILED_WRITING);

  /* make mock broker not to reply on PINGREQ, this is necessary since the
   * mock broker catches the message before IO layer */
  will_return(iotc_mock_broker_layer_pull,
              CONTROL_PULL_PINGREQ_SUPPRESS_RESPONSE);

#ifndef IOTC_MODULE_THREAD_ENABLED
  expect_value(_iotc_itest_mqtt_keepalive__on_connection_state_changed, state,
               IOTC_STATE_OK);
  expect_value(_iotc_itest_mqtt_keepalive__on_connection_state_changed, state,
               IOTC_CONNECTION_RESET_BY_PEER_ERROR);
  expect_value(_iotc_itest_mqtt_keepalive__on_connection_state_changed, state,
               IOTC_STATE_TIMEOUT);
#endif

  iotc_itest_mqtt_keepalive__act(state, 11);
}

void iotc_itest_mqtt_keepalive__2nd_PINGREQ_failed_to_send__broker_disconnects_first(
    void** state) {
  /* let CONNECT and SUBSCRIBE writes succeed */
#ifdef IOTC_CONTROL_TOPIC_ENABLED
  will_return_count(iotc_mock_layer_tls_prev_push, CONTROL_TLS_PREV_CONTINUE,
                    3);
#else
  will_return_count(iotc_mock_layer_tls_prev_push, CONTROL_TLS_PREV_CONTINUE,
                    2);
#endif

  /* make first PINGREQ fail to write on the socket */
  will_return(iotc_mock_layer_tls_prev_push,
              CONTROL_TLS_PREV_PUSH__WRITE_ERROR);
  /* set the specific error to be returned by the mock IO layer (here tls_prev)
   */
  will_return(iotc_mock_layer_tls_prev_push, IOTC_STATE_FAILED_WRITING);

  /* mock broker responds with proper PINGRESP on the first PINGREQ */
  will_return(iotc_mock_broker_layer_pull, CONTROL_CONTINUE);
  /* make mock broker not to reply on PINGREQ, this is necessary since the
   * mock broker catches the message before IO layer */
  will_return(iotc_mock_broker_layer_pull,
              CONTROL_PULL_PINGREQ_SUPPRESS_RESPONSE);

#ifndef IOTC_MODULE_THREAD_ENABLED
  expect_value(_iotc_itest_mqtt_keepalive__on_connection_state_changed, state,
               IOTC_STATE_OK);
  expect_value(_iotc_itest_mqtt_keepalive__on_connection_state_changed, state,
               IOTC_CONNECTION_RESET_BY_PEER_ERROR);
  expect_value(_iotc_itest_mqtt_keepalive__on_connection_state_changed, state,
               IOTC_STATE_TIMEOUT);
#endif

  iotc_itest_mqtt_keepalive__act(state, 18);
}
