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

#include "iotc_itest_tls_layer.h"
#include "iotc_connection_data_internal.h"
#include "iotc_globals.h"
#include "iotc_itest_helpers.h"
#include "iotc_itest_layerchain_tls.h"
#include "iotc_memory_checks.h"

#include <time.h>

iotc_context_t* iotc_context__itest_tls_layer = NULL;

/**
 * iotc_itest_tls_layer test suit description
 *
 * System Under Test: 1 layer: TLS layer
 *
 * Test consists of an artificial layer chain: TLSNEXT - TLS - TLSPREV
 * TLSNEXT and TLSPREV are mock layers. They drive and monitor the SUT: TLS
 * layer.
 */

/*********************************************************************************
 * test fixture
 *******************************************************************
 ********************************************************************************/
typedef struct iotc_itest_tls_layer__test_fixture_s {
  uint8_t loop_id__manual_close;
  uint8_t max_loop_count;
} iotc_itest_tls_layer__test_fixture_t;

iotc_itest_tls_layer__test_fixture_t* iotc_itest_tls_layer__generate_fixture() {
  iotc_state_t iotc_state = IOTC_STATE_OK;

  IOTC_ALLOC(iotc_itest_tls_layer__test_fixture_t, fixture, iotc_state);

  /* 114 iterations are needed to load roots.pem (117kB) */
  fixture->loop_id__manual_close = 115;
  fixture->max_loop_count = 120;

  return fixture;

err_handling:
  fail();

  return NULL;
}

int iotc_itest_tls_layer_setup(void** fixture_void) {
  iotc_memory_limiter_tearup();

  *fixture_void = iotc_itest_tls_layer__generate_fixture();

  assert_int_equal(IOTC_STATE_OK, iotc_initialize());

  IOTC_CHECK_STATE(iotc_create_context_with_custom_layers(
      &iotc_context__itest_tls_layer, itest_layer_chain_tls,
      IOTC_LAYER_CHAIN_TLS,
      IOTC_LAYER_CHAIN_SCHEME_LENGTH(IOTC_LAYER_CHAIN_TLS)));

  return 0;

err_handling:
  fail();

  return 1;
}

int iotc_itest_tls_layer_teardown(void** fixture_void) {
  iotc_itest_tls_layer__test_fixture_t* fixture =
      (iotc_itest_tls_layer__test_fixture_t*)*fixture_void;

  IOTC_SAFE_FREE(fixture);

  iotc_delete_context_with_custom_layers(
      &iotc_context__itest_tls_layer, itest_layer_chain_tls,
      IOTC_LAYER_CHAIN_SCHEME_LENGTH(IOTC_LAYER_CHAIN_TLS));

  iotc_shutdown();

  return !iotc_memory_limiter_teardown();
}

/**
 * @name    iotc_itest_tls_layer__act
 * @brief   mimics a libiotc live environment: drives event dispatcher
 *
 *
 * Does a limited number of iterations, processes some events. Acts
 * like a live libiotc environment.
 *
 * @param [in] init_connection_data_flag    if not set an uninitialized
 *                                          libiotc behavior can be tested
 */
void iotc_itest_tls_layer__act(void** fixture_void,
                               char init_connection_data_flag,
                               char close_layer_chain_flag) {
  iotc_itest_tls_layer__test_fixture_t* fixture =
      (iotc_itest_tls_layer__test_fixture_t*)*fixture_void;

  const iotc_crypto_private_key_data_t dummy_private_key = {
      .private_key_signature_algorithm =
          IOTC_JWT_PRIVATE_KEY_SIGNATURE_ALGORITHM_ES256,
      .private_key_union_type = IOTC_CRYPTO_KEY_UNION_TYPE_PEM,
      .private_key_union.key_pem.key = "dummy_key"};

  if (init_connection_data_flag != 0) {
    iotc_context__itest_tls_layer->context_data.connection_data =
        iotc_alloc_connection_data("target.broker.com", /*port=*/8883,
                                   "itest_projectid", "itest_device_path",
                                   &dummy_private_key,
                                   /*jwt_expiration_period_sec=*/600,
                                   /*connection_timeout=*/20,
                                   /*keepalive_timeout=*/5, IOTC_SESSION_CLEAN);
  }

  iotc_layer_t* input_layer = iotc_context__itest_tls_layer->layer_chain.top;
  iotc_layer_t* output_layer =
      iotc_context__itest_tls_layer->layer_chain.bottom;

  IOTC_PROCESS_INIT_ON_THIS_LAYER(
      &input_layer->layer_connection,
      iotc_context__itest_tls_layer->context_data.connection_data,
      IOTC_STATE_OK);

  size_t loop_counter = 0;
  while (1 == iotc_evtd_dispatcher_continue(iotc_globals.evtd_instance) &&
         loop_counter < fixture->max_loop_count) {
    iotc_evtd_step(iotc_globals.evtd_instance, time(NULL) + loop_counter);
    iotc_evtd_update_file_fd_events(iotc_globals.evtd_instance);
    ++loop_counter;

    if (close_layer_chain_flag &&
        loop_counter == fixture->loop_id__manual_close) {
      IOTC_PROCESS_CLOSE_EXTERNALLY_ON_THIS_LAYER(
          &output_layer->layer_connection, NULL, IOTC_STATE_OK);
    }
  }
}

extern int8_t iotc_cm_strict_mock;

/*********************************************************************************************
 * test cases
 *********************************************************************************
 *********************************************************************************************/
void iotc_itest_tls_layer__bad_handshake_response__graceful_closure(
    void** fixture_void) {
  iotc_cm_strict_mock = 0;

  // init
  expect_value(iotc_mock_layer_tls_next_init, in_out_state, IOTC_STATE_OK);
  expect_value(iotc_mock_layer_tls_prev_init, in_out_state, IOTC_STATE_OK);
  expect_value(iotc_mock_layer_tls_prev_connect, in_out_state, IOTC_STATE_OK);
  // handshake started by TLS layer
  expect_value(iotc_mock_layer_tls_prev_push, in_out_state, IOTC_STATE_OK);

  // INTRODUCE ERROR HERE: drive mock IO layer to return unrecognizable
  // message to TLS layer as a handshake reply
  will_return(iotc_mock_layer_tls_prev_push,
              CONTROL_TLS_PREV_PUSH__RETURN_MESSAGE);
  will_return(iotc_mock_layer_tls_prev_push, "this is litter for TLS");

  expect_value(iotc_mock_layer_tls_prev_pull, in_out_state, IOTC_STATE_OK);

#ifdef IOTC_TLS_LIB_MBEDTLS
  /* mbedTLS sends out 4 TLS handshake packets independently from the response
   * on the first packet. This test responds with litter on the first packet
   * and for wolfssl expects no continuation. But seemingly mbedTLS blindly
   * sends all 4 packets thus test case catches these, and handles extra close
   * result as well. */
  expect_value_count(iotc_mock_layer_tls_prev_push, in_out_state, IOTC_STATE_OK,
                     3);
  expect_value(iotc_mock_layer_tls_prev_close, in_out_state,
               IOTC_TLS_CONNECT_ERROR);
  expect_value(iotc_mock_layer_tls_prev_close_externally, in_out_state,
               IOTC_TLS_CONNECT_ERROR);
  expect_value_count(iotc_mock_layer_tls_next_close_externally, in_out_state,
                     IOTC_TLS_CONNECT_ERROR, 2);
#endif

  // expect error propagation through close functions too
  expect_value(iotc_mock_layer_tls_prev_close, in_out_state,
               IOTC_TLS_CONNECT_ERROR);
  expect_value(iotc_mock_layer_tls_prev_close_externally, in_out_state,
               IOTC_TLS_CONNECT_ERROR);
  expect_value(iotc_mock_layer_tls_next_connect, in_out_state,
               IOTC_TLS_CONNECT_ERROR);

  // no close_externally calls on layers above since they were not connected

  iotc_itest_tls_layer__act(fixture_void, 1, 0);
}

void iotc_itest_tls_layer__null_host__graceful_closure(void** fixture_void) {
  expect_value(iotc_mock_layer_tls_next_init, in_out_state, IOTC_STATE_OK);
  expect_value(iotc_mock_layer_tls_next_connect, in_out_state,
               IOTC_TLS_INITALIZATION_ERROR);
  expect_value(iotc_mock_layer_tls_prev_close_externally, in_out_state,
               IOTC_STATE_OK);
  expect_value(iotc_mock_layer_tls_next_close_externally, in_out_state,
               IOTC_STATE_OK);

  iotc_itest_tls_layer__act(fixture_void, 0, 1);
}

void iotc_itest_tls_layer__valid_dependencies__successful_init(
    void** fixture_void) {
  // init
  expect_value(iotc_mock_layer_tls_next_init, in_out_state, IOTC_STATE_OK);
  expect_value(iotc_mock_layer_tls_prev_init, in_out_state, IOTC_STATE_OK);
  expect_value(iotc_mock_layer_tls_prev_connect, in_out_state, IOTC_STATE_OK);
  // handshake started by TLS layer
  expect_value(iotc_mock_layer_tls_prev_push, in_out_state, IOTC_STATE_OK);
  expect_value(iotc_mock_layer_tls_prev_close_externally, in_out_state,
               IOTC_STATE_OK);
  expect_value(iotc_mock_layer_tls_next_close_externally, in_out_state,
               IOTC_STATE_OK);

  iotc_itest_tls_layer__act(fixture_void, 1, 1);
}
