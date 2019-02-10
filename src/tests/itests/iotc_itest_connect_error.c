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

#include "iotc_itest_connect_error.h"
#include "iotc_backoff_status_api.h"

#include "iotc_debug.h"
#include "iotc_globals.h"
#include "iotc_handle.h"

#include "iotc_bsp_time.h"
#include "iotc_itest_layerchain_ct_ml_mc.h"
#include "iotc_itest_mock_broker_layerchain.h"
#include "iotc_memory_checks.h"

/**
 * @brief These test cases are for testing the wrong usage of connect and
 * disconnect functions. Motivation is to make the library's API as robust as
 * it's possible. Thanks to these tests we are protecting our library from
 * running connect or disconnect logic twice in a row.
 *
 * This integration test setup is similar to iotc_itest_tls_error.c. It is using
 * same setup of layers so the SUT layer chain is:
 * - Control Topic (CT)
 * - Mqtt Logic (ML)
 * - Mqtt Codec (MC)
 *
 * In order to perform tests on SUT layers it is using additional mock layers
 * such as:
 * - Mock Broker - MB
 * - TLSPREV
 *
 * @note MB layer is used twice which requires to double to requirements for
 * functions of this layer in all of the tests. Keep that in mind while reading
 * the test's code.
 *
 * This test consists two layer chains that communicates through the MB layer.
 *
 *        CT - ML - MC - MB - TLSPREV
 *                       |
 *                       MC
 *                       |
 *                       MBS
 *
 * The horizontal layer chain acts like libiotc, MB intercepts messages from the
 * 3 SUT layers, forwards it to two directions: TLSPREV which behaves like IO
 * layer and MC for decoding mqtt messages. The horizontal layer chain's MB and
 * TLSPREV is "programmed" to mimic errors at various stages of functioning.
 *
 */

/* Depends on the iotc_itest_tls_error.c */
extern iotc_context_t* iotc_context;
extern iotc_context_handle_t iotc_context_handle;
extern iotc_context_t* iotc_context_mockbroker;
/* end of dependency */

/*********************************************************************************
 * test fixture
 *******************************************************************
 ********************************************************************************/
typedef struct iotc_itest_connect_error__test_fixture_s {
  const char* test_topic_name;
  const char* test_full_topic_name;
  const char* control_topic_name;
} iotc_itest_connect_error__test_fixture_t;

iotc_itest_connect_error__test_fixture_t*
iotc_itest_connect_error__generate_fixture() {
  iotc_state_t iotc_state = IOTC_STATE_OK;

  IOTC_ALLOC(iotc_itest_connect_error__test_fixture_t, fixture, iotc_state);

  fixture->test_topic_name = ("test/topic/name");
  fixture->test_full_topic_name =
      ("xi/blue/v1/iotc_itest_connect_error_account_id/d/"
       "iotc_itest_connect_error_device_id/test/topic/name");

  return fixture;

err_handling:
  fail();

  return NULL;
}

int iotc_itest_connect_error_setup(void** fixture_void) {
  /* clear the external dependencies */
  iotc_context = NULL;
  iotc_context_handle = IOTC_INVALID_CONTEXT_HANDLE;
  iotc_context_mockbroker = NULL;

  iotc_memory_limiter_tearup();

  *fixture_void = iotc_itest_connect_error__generate_fixture();

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

int iotc_itest_connect_error_teardown(void** fixture_void) {
  iotc_delete_context_with_custom_layers(
      &iotc_context, itest_ct_ml_mc_layer_chain,
      IOTC_LAYER_CHAIN_SCHEME_LENGTH(IOTC_LAYER_CHAIN_CT_ML_MC));

  iotc_delete_context_with_custom_layers(
      &iotc_context_mockbroker, itest_mock_broker_codec_layer_chain,
      IOTC_LAYER_CHAIN_SCHEME_LENGTH(IOTC_LAYER_CHAIN_MOCK_BROKER_CODEC));

  iotc_shutdown();

  iotc_itest_connect_error__test_fixture_t* fixture =
      (iotc_itest_connect_error__test_fixture_t*)*fixture_void;

  IOTC_SAFE_FREE(fixture);

  return !iotc_memory_limiter_teardown();
}

void connect_error_on_connection_state_changed(
    iotc_context_handle_t in_context_handle, void* data, iotc_state_t state) {
  IOTC_UNUSED(in_context_handle);
  IOTC_UNUSED(data);
  IOTC_UNUSED(state);
}

static void iotc_itest_connect_error__call_connect(void** fixture_void) {
  IOTC_UNUSED(fixture_void);

  const iotc_state_t local_state =
      iotc_connect(iotc_context_handle, "itest_username", "itest_password",
                   "itest_client_id", /*connection_timeout=*/20,
                   /*keepalive_timeout=*/20,
                   &connect_error_on_connection_state_changed);

  check_expected(local_state);
}

static void iotc_itest_connect_error__trigger_connect(
    void** fixture_void, uint8_t init_mock_broker) {
  if (1 == init_mock_broker) {
    IOTC_PROCESS_INIT_ON_THIS_LAYER(
        &iotc_context_mockbroker->layer_chain.top->layer_connection, NULL,
        IOTC_STATE_OK);

    iotc_evtd_step(iotc_globals.evtd_instance,
                   iotc_bsp_time_getcurrenttime_seconds());
  }

  /* here we expect to connect succesfully */
  expect_value(iotc_itest_connect_error__call_connect, local_state,
               IOTC_STATE_OK);
  iotc_itest_connect_error__call_connect(fixture_void);
}

static void iotc_itest_connect_error__trigger_event_dispatcher(
    void** fixture_void, uint8_t max_evtd_iterations) {
  IOTC_UNUSED(fixture_void);

  uint8_t loop_counter = 0;
  while (iotc_evtd_dispatcher_continue(iotc_globals.evtd_instance) == 1 &&
         loop_counter < max_evtd_iterations) {
    iotc_evtd_step(iotc_globals.evtd_instance,
                   iotc_bsp_time_getcurrenttime_seconds() + loop_counter);
    ++loop_counter;
  }
}

static void iotc_itest_connect_error__trigger_shutdown(void** fixture_void) {
  IOTC_UNUSED(fixture_void);

  const iotc_state_t local_state =
      iotc_shutdown_connection(iotc_context_handle);
  check_expected(local_state);
}

void iotc_itest_test_valid_flow__call_connect_function_twice_in_a_row__second_call_returns_error(
    void** fixture_void) {
  expect_value(iotc_itest_connect_error__call_connect, local_state,
               IOTC_STATE_OK);
  iotc_itest_connect_error__call_connect(fixture_void);
  expect_value(iotc_itest_connect_error__call_connect, local_state,
               IOTC_ALREADY_INITIALIZED);
  iotc_itest_connect_error__call_connect(fixture_void);
}

void iotc_itest_test_valid_flow__call_connect_function_twice__second_call_returns_error(
    void** fixture_void) {
  uint8_t evtd_loop_count_between_connect_calls = 0;
  for (; evtd_loop_count_between_connect_calls < 10;
       ++evtd_loop_count_between_connect_calls) {
    const iotc_itest_connect_error__test_fixture_t* const fixture =
        (iotc_itest_connect_error__test_fixture_t*)*fixture_void;

    IOTC_UNUSED(fixture);

    iotc_debug_format("Number of evtd calls: %d",
                      evtd_loop_count_between_connect_calls);

    /* one call for mock broker layer chain init*/
    expect_value(iotc_mock_broker_layer_init, in_out_state, IOTC_STATE_OK);
    expect_value(iotc_mock_layer_tls_prev_init, in_out_state, IOTC_STATE_OK);
    expect_value(iotc_mock_layer_tls_prev_connect, in_out_state, IOTC_STATE_OK);
    expect_value(iotc_mock_broker_layer_connect, in_out_state, IOTC_STATE_OK);

    expect_value(iotc_mock_broker_layer_init, in_out_state, IOTC_STATE_OK);
    expect_value(iotc_mock_broker_layer_connect, in_out_state, IOTC_STATE_OK);

    /* no problem during CONNECT*/
    expect_value(iotc_mock_broker_layer_push, in_out_state, IOTC_STATE_OK);
    expect_value(iotc_mock_broker_layer_push, in_out_state, IOTC_STATE_WRITTEN);
    expect_value(iotc_mock_layer_tls_prev_push, in_out_state, IOTC_STATE_OK);

    /* CONNECT message arrives at mock broker*/
    expect_value(iotc_mock_broker_layer_pull, in_out_state, IOTC_STATE_OK);
    expect_value(iotc_mock_broker_layer_pull, recvd_msg_type,
                 IOTC_MQTT_TYPE_CONNECT);

    /* CONNACK sent*/
    expect_value(iotc_mock_broker_secondary_layer_push, in_out_state,
                 IOTC_STATE_OK);
    will_return(iotc_mock_broker_secondary_layer_push, CONTROL_CONTINUE);
    expect_value(iotc_mock_broker_layer_push, in_out_state, IOTC_STATE_WRITTEN);

    /* DISCONNECT MESSAGE */
    expect_value(iotc_mock_broker_layer_push, in_out_state, IOTC_STATE_OK);
    expect_value(iotc_mock_layer_tls_prev_push, in_out_state, IOTC_STATE_OK);
    expect_value(iotc_mock_broker_layer_push, in_out_state, IOTC_STATE_WRITTEN);
    expect_value(iotc_mock_broker_layer_pull, in_out_state, IOTC_STATE_OK);
    expect_value(iotc_mock_broker_layer_pull, recvd_msg_type,
                 IOTC_MQTT_TYPE_DISCONNECT);

    /* SHUTDOWN */
    expect_value(iotc_mock_broker_layer_close, in_out_state, IOTC_STATE_OK);
    expect_value(iotc_mock_broker_layer_close, in_out_state, IOTC_STATE_OK);
    expect_value(iotc_mock_layer_tls_prev_close, in_out_state, IOTC_STATE_OK);
    expect_value(iotc_mock_layer_tls_prev_close_externally, in_out_state,
                 IOTC_STATE_OK);

    iotc_itest_connect_error__trigger_connect(fixture_void, 1);
    iotc_itest_connect_error__trigger_event_dispatcher(
        fixture_void, evtd_loop_count_between_connect_calls);

    /* TRY TO CONNECT ONE MORE TIME, IT SHOULD FAIL */
    expect_value(iotc_itest_connect_error__call_connect, local_state,
                 IOTC_ALREADY_INITIALIZED);
    iotc_itest_connect_error__call_connect(fixture_void);
    iotc_itest_connect_error__trigger_event_dispatcher(fixture_void, 6);

    expect_value(iotc_itest_connect_error__trigger_shutdown, local_state,
                 IOTC_STATE_OK);
    iotc_itest_connect_error__trigger_shutdown(fixture_void);
    iotc_itest_connect_error__trigger_event_dispatcher(fixture_void, 10);

    /* artificially reset test case*/
    iotc_itest_connect_error_teardown(fixture_void);
    iotc_itest_connect_error_setup(fixture_void);
  }
}

void iotc_itest_test_valid_flow__call_disconnect_twice_on_connected_context__second_call_should_return_error(
    void** fixture_void) {
  uint8_t evtd_loop_count_between_connect_calls = 0;
  for (; evtd_loop_count_between_connect_calls < 10;
       ++evtd_loop_count_between_connect_calls) {
    iotc_debug_printf("gap = %d \n", evtd_loop_count_between_connect_calls);

    const iotc_itest_connect_error__test_fixture_t* const fixture =
        (iotc_itest_connect_error__test_fixture_t*)*fixture_void;

    IOTC_UNUSED(fixture);

    /* one call for mock broker layer chain init*/
    expect_value(iotc_mock_broker_layer_init, in_out_state, IOTC_STATE_OK);
    expect_value(iotc_mock_layer_tls_prev_init, in_out_state, IOTC_STATE_OK);
    expect_value(iotc_mock_layer_tls_prev_connect, in_out_state, IOTC_STATE_OK);
    expect_value(iotc_mock_broker_layer_connect, in_out_state, IOTC_STATE_OK);

    expect_value(iotc_mock_broker_layer_init, in_out_state, IOTC_STATE_OK);
    expect_value(iotc_mock_broker_layer_connect, in_out_state, IOTC_STATE_OK);

    /* no problem during CONNECT*/
    expect_value(iotc_mock_broker_layer_push, in_out_state, IOTC_STATE_OK);
    expect_value(iotc_mock_broker_layer_push, in_out_state, IOTC_STATE_WRITTEN);
    expect_value(iotc_mock_layer_tls_prev_push, in_out_state, IOTC_STATE_OK);

    /* CONNECT message arrives at mock broker*/
    expect_value(iotc_mock_broker_layer_pull, in_out_state, IOTC_STATE_OK);
    expect_value(iotc_mock_broker_layer_pull, recvd_msg_type,
                 IOTC_MQTT_TYPE_CONNECT);

    /* CONNACK sent*/
    expect_value(iotc_mock_broker_secondary_layer_push, in_out_state,
                 IOTC_STATE_OK);
    will_return(iotc_mock_broker_secondary_layer_push, CONTROL_CONTINUE);
    expect_value(iotc_mock_broker_layer_push, in_out_state, IOTC_STATE_WRITTEN);

    /* DISCONNECT MESSAGE */
    expect_value(iotc_mock_broker_layer_push, in_out_state, IOTC_STATE_OK);
    expect_value(iotc_mock_layer_tls_prev_push, in_out_state, IOTC_STATE_OK);
    expect_value(iotc_mock_broker_layer_push, in_out_state, IOTC_STATE_WRITTEN);
    expect_value(iotc_mock_broker_layer_pull, in_out_state, IOTC_STATE_OK);
    expect_value(iotc_mock_broker_layer_pull, recvd_msg_type,
                 IOTC_MQTT_TYPE_DISCONNECT);

    /* SHUTDOWN */
    expect_value(iotc_mock_broker_layer_close, in_out_state, IOTC_STATE_OK);
    expect_value(iotc_mock_broker_layer_close, in_out_state, IOTC_STATE_OK);
    expect_value(iotc_mock_layer_tls_prev_close, in_out_state, IOTC_STATE_OK);
    expect_value(iotc_mock_layer_tls_prev_close_externally, in_out_state,
                 IOTC_STATE_OK);

    iotc_itest_connect_error__trigger_connect(fixture_void, 1);
    iotc_itest_connect_error__trigger_event_dispatcher(fixture_void, 6);

    /* TRY TO CALL SHUTDOWN TWICE IN A ROW */
    expect_value(iotc_itest_connect_error__trigger_shutdown, local_state,
                 IOTC_STATE_OK);
    iotc_itest_connect_error__trigger_shutdown(fixture_void);

    /* HERE GOES THE EVTD LOOPS */
    iotc_itest_connect_error__trigger_event_dispatcher(
        fixture_void, evtd_loop_count_between_connect_calls);

    expect_value(iotc_itest_connect_error__trigger_shutdown, local_state,
                 evtd_loop_count_between_connect_calls == 0
                     ? IOTC_ALREADY_INITIALIZED
                     : IOTC_SOCKET_NO_ACTIVE_CONNECTION_ERROR);
    iotc_itest_connect_error__trigger_shutdown(fixture_void);

    iotc_itest_connect_error__trigger_event_dispatcher(fixture_void, 6);

    /* artificially reset test case*/
    iotc_itest_connect_error_teardown(fixture_void);
    iotc_itest_connect_error_setup(fixture_void);
  }
}

void iotc_itest_test_valid_flow__call_connect_function_then_disconnect_without_making_a_connection__shutdown_should_unregister_connect(
    void** fixture_void) {
  /* call initialisation and connect without dispatching any event */
  iotc_itest_connect_error__trigger_connect(fixture_void, 0);

  /* now we trigger shutdown immedietaly */
  expect_value(iotc_itest_connect_error__trigger_shutdown, local_state,
               IOTC_STATE_OK);
  iotc_itest_connect_error__trigger_shutdown(fixture_void);
  iotc_itest_connect_error__trigger_event_dispatcher(fixture_void, 5);
}

void iotc_itest_test_valid_flow__call_is_context_connected_on_connecting_context__call_returns_false(
    void** fixture_void) {
  will_return_always(iotc_mock_broker_layer__check_expected__LAYER_LEVEL,
                     CONTROL_SKIP_CHECK_EXPECTED);
  will_return_always(iotc_mock_broker_layer__check_expected__MQTT_LEVEL,
                     CONTROL_SKIP_CHECK_EXPECTED);
  will_return_always(iotc_mock_layer_tls_prev__check_expected__LAYER_LEVEL,
                     CONTROL_SKIP_CHECK_EXPECTED);

  uint8_t evtd_loop_count_between_connect_calls = 0;
  for (; evtd_loop_count_between_connect_calls < 10;
       ++evtd_loop_count_between_connect_calls) {
    const iotc_itest_connect_error__test_fixture_t* const fixture =
        (iotc_itest_connect_error__test_fixture_t*)*fixture_void;
    IOTC_UNUSED(fixture);
    iotc_debug_format("Number of evtd calls: %d",
                      evtd_loop_count_between_connect_calls);
    iotc_itest_connect_error__trigger_connect(fixture_void, 1);

    /* TEST IT */
    int is_connected_result = iotc_is_context_connected(iotc_context_handle);

    iotc_itest_connect_error__trigger_event_dispatcher(
        fixture_void, evtd_loop_count_between_connect_calls);
    iotc_itest_connect_error__trigger_event_dispatcher(fixture_void, 6);

    expect_value(iotc_itest_connect_error__trigger_shutdown, local_state,
                 IOTC_STATE_OK);
    iotc_itest_connect_error__trigger_shutdown(fixture_void);
    iotc_itest_connect_error__trigger_event_dispatcher(fixture_void, 10);

    /* artificially reset test case*/
    iotc_itest_connect_error_teardown(fixture_void);
    iotc_itest_connect_error_setup(fixture_void);

    /* Evaluate result */
    assert_int_equal(is_connected_result, 0);
  }
}

void iotc_itest_test_valid_flow__call_is_context_connected_on_connected_context__call_returns_true(
    void** fixture_void) {
  will_return_always(iotc_mock_broker_layer__check_expected__LAYER_LEVEL,
                     CONTROL_SKIP_CHECK_EXPECTED);
  will_return_always(iotc_mock_broker_layer__check_expected__MQTT_LEVEL,
                     CONTROL_SKIP_CHECK_EXPECTED);
  will_return_always(iotc_mock_layer_tls_prev__check_expected__LAYER_LEVEL,
                     CONTROL_SKIP_CHECK_EXPECTED);
  uint8_t evtd_loop_count_between_connect_calls = 0;
  for (; evtd_loop_count_between_connect_calls < 10;
       ++evtd_loop_count_between_connect_calls) {
    const iotc_itest_connect_error__test_fixture_t* const fixture =
        (iotc_itest_connect_error__test_fixture_t*)*fixture_void;
    IOTC_UNUSED(fixture);
    iotc_debug_format("Number of evtd calls: %d",
                      evtd_loop_count_between_connect_calls);
    iotc_itest_connect_error__trigger_connect(fixture_void, 1);

    iotc_itest_connect_error__trigger_event_dispatcher(
        fixture_void, evtd_loop_count_between_connect_calls);
    iotc_itest_connect_error__trigger_event_dispatcher(fixture_void, 6);

    /* TEST IT */
    int is_connected_result = iotc_is_context_connected(iotc_context_handle);

    expect_value(iotc_itest_connect_error__trigger_shutdown, local_state,
                 IOTC_STATE_OK);
    iotc_itest_connect_error__trigger_shutdown(fixture_void);
    iotc_itest_connect_error__trigger_event_dispatcher(fixture_void, 10);

    /* artificially reset test case*/
    iotc_itest_connect_error_teardown(fixture_void);
    iotc_itest_connect_error_setup(fixture_void);

    /* Evaluate result */
    assert_int_equal(is_connected_result, 1);
  }
}

void iotc_itest_test_valid_flow__call_is_context_connected_on_disconnecting_context__call_returns_false(
    void** fixture_void) {
  will_return_always(iotc_mock_broker_layer__check_expected__LAYER_LEVEL,
                     CONTROL_SKIP_CHECK_EXPECTED);
  will_return_always(iotc_mock_broker_layer__check_expected__MQTT_LEVEL,
                     CONTROL_SKIP_CHECK_EXPECTED);
  will_return_always(iotc_mock_layer_tls_prev__check_expected__LAYER_LEVEL,
                     CONTROL_SKIP_CHECK_EXPECTED);
  uint8_t evtd_loop_count_between_connect_calls = 0;
  for (; evtd_loop_count_between_connect_calls < 10;
       ++evtd_loop_count_between_connect_calls) {
    const iotc_itest_connect_error__test_fixture_t* const fixture =
        (iotc_itest_connect_error__test_fixture_t*)*fixture_void;
    IOTC_UNUSED(fixture);
    iotc_debug_format("Number of evtd calls: %d",
                      evtd_loop_count_between_connect_calls);
    iotc_itest_connect_error__trigger_connect(fixture_void, 1);

    iotc_itest_connect_error__trigger_event_dispatcher(
        fixture_void, evtd_loop_count_between_connect_calls);
    iotc_itest_connect_error__trigger_event_dispatcher(fixture_void, 6);

    expect_value(iotc_itest_connect_error__trigger_shutdown, local_state,
                 IOTC_STATE_OK);
    iotc_itest_connect_error__trigger_shutdown(fixture_void);

    /* TEST IT */
    int is_connected_result = iotc_is_context_connected(iotc_context_handle);

    iotc_itest_connect_error__trigger_event_dispatcher(fixture_void, 10);

    /* artificially reset test case*/
    iotc_itest_connect_error_teardown(fixture_void);
    iotc_itest_connect_error_setup(fixture_void);

    /* Evaluate result */
    assert_int_equal(is_connected_result, 0);
  }
}

void iotc_itest_test_valid_flow__call_is_context_connected_on_disconnected_context__call_returns_false(
    void** fixture_void) {
  will_return_always(iotc_mock_broker_layer__check_expected__LAYER_LEVEL,
                     CONTROL_SKIP_CHECK_EXPECTED);
  will_return_always(iotc_mock_broker_layer__check_expected__MQTT_LEVEL,
                     CONTROL_SKIP_CHECK_EXPECTED);
  will_return_always(iotc_mock_layer_tls_prev__check_expected__LAYER_LEVEL,
                     CONTROL_SKIP_CHECK_EXPECTED);
  uint8_t evtd_loop_count_between_connect_calls = 0;
  for (; evtd_loop_count_between_connect_calls < 10;
       ++evtd_loop_count_between_connect_calls) {
    const iotc_itest_connect_error__test_fixture_t* const fixture =
        (iotc_itest_connect_error__test_fixture_t*)*fixture_void;
    IOTC_UNUSED(fixture);
    iotc_debug_format("Number of evtd calls: %d",
                      evtd_loop_count_between_connect_calls);
    iotc_itest_connect_error__trigger_connect(fixture_void, 1);

    iotc_itest_connect_error__trigger_event_dispatcher(
        fixture_void, evtd_loop_count_between_connect_calls);
    iotc_itest_connect_error__trigger_event_dispatcher(fixture_void, 6);

    expect_value(iotc_itest_connect_error__trigger_shutdown, local_state,
                 IOTC_STATE_OK);
    iotc_itest_connect_error__trigger_shutdown(fixture_void);
    iotc_itest_connect_error__trigger_event_dispatcher(fixture_void, 10);

    /* TEST IT */
    int is_connected_result = iotc_is_context_connected(iotc_context_handle);

    /* artificially reset test case*/
    iotc_itest_connect_error_teardown(fixture_void);
    iotc_itest_connect_error_setup(fixture_void);

    /* Evaluate result */
    assert_int_equal(is_connected_result, 0);
  }
}
