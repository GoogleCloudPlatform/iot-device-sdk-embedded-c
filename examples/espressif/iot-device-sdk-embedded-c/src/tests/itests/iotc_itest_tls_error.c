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

#include "iotc_itest_tls_error.h"
#include "iotc_backoff_status_api.h"
#include "iotc_globals.h"
#include "iotc_itest_helpers.h"

#include "iotc_bsp_time.h"
#include "iotc_handle.h"
#include "iotc_itest_layerchain_ct_ml_mc.h"
#include "iotc_itest_mock_broker_layerchain.h"
#include "iotc_memory_checks.h"

extern char iotc_test_load_level;

iotc_context_t* iotc_context = NULL;
iotc_context_handle_t iotc_context_handle = IOTC_INVALID_CONTEXT_HANDLE;
iotc_context_t* iotc_context_mockbroker = NULL;

/**
 * iotc_itest_tls_error test suit description
 *
 * System Under Test: 3 layers: Control Topic (CT), Mqtt Logic (ML), Mqtt Codec
 * (MC)
 *
 * Test consists of two layer chains:
 *   - SUT layer chain: CT-ML-MC-MB-TLSPREV, where
 *      - MB      = Mock Broker / Mock TLS layer (in production MC is preceded
 * by TLS) TLSPREV = an artificial mock layer serving as a previous layer for
 * TLS layer
 *   - Mock Broker (MB) layer chain: MB-MC-MBS, where
 *      - MBS     = Mock Broker Secondary layer, a "helper layer"
 *      - here MC is the libiotc's Mqtt Codec layer. Mock Broker uses this
 * production version Mqtt Codec layer to encode / decode mqtt messages
 *
 * The two layer chains intersect each other:
 *
 * iotc_itest_layerchain_ct_ml_mc.h
 *  |
 *  L->   CT - ML - MC - MB - TLSPREV
 *                       |
 *                       MC
 *                       |
 *                       MBS
 *
 *                       A
 *                       |
 *                       |
 *                  iotc_itest_mock_broker_layerchain.h
 *
 *
 * The horizontal layer chain acts like libiotc, MB intercepts messages from the
 * 3 SUT layers, forwards it to two directions: TLSPREV which behaves like IO
 * layer and MC for decoding mqtt messages. The horizontal layer chain's MB and
 * TLSPREV is "programmed" to mimic errors at various stages of functioning.
 *
 * NOTE: The iotc_mock_broker_layer is instantiated twice. That's why we expect
 * to see two invocations of iotc_mock_broker_layer_close function. One per each
 * layer chain.
 */

/*********************************************************************************
 * test fixture
 *******************************************************************
 ********************************************************************************/
typedef struct iotc_itest_tls_error__test_fixture_s {
  const char* test_topic_name;
  const char* test_full_topic_name;
  const char* control_topic_name;

  uint8_t loop_id__control_topic_auto_subscribe;
  uint8_t loop_id__manual_publish;
  uint8_t loop_id__manual_disconnect;

  uint8_t max_loop_count;

} iotc_itest_tls_error__test_fixture_t;

iotc_itest_tls_error__test_fixture_t* iotc_itest_tls_error__generate_fixture() {
  iotc_state_t iotc_state = IOTC_STATE_OK;

  IOTC_ALLOC(iotc_itest_tls_error__test_fixture_t, fixture, iotc_state);

  fixture->test_topic_name = ("test/topic/name");
  fixture->test_full_topic_name =
      ("xi/blue/v1/iotc_itest_tls_error_account_id/d/"
       "iotc_itest_tls_error_device_id/test/topic/name");
  fixture->control_topic_name =
      ("xi/ctrl/v1/iotc_itest_tls_error_device_id/cln");

  /* control_topic_auto_subscribe is 2 because control topic subscription
   * happens in the 3rd
   * loop, this is precise timed simulation, some test cases are sensitive for
   * timing*/
  fixture->loop_id__control_topic_auto_subscribe = 3;

  fixture->loop_id__manual_publish = 6;
  fixture->loop_id__manual_disconnect = 15;
  fixture->max_loop_count = 20;

  return fixture;

err_handling:
  fail();

  return NULL;
}

/*********************************************************************************
 * setup / teardown
 ***************************************************************
 ********************************************************************************/
int iotc_itest_tls_error_setup(void** fixture_void) {
  IOTC_UNUSED(fixture_void);

  iotc_memory_limiter_tearup();

  *fixture_void = iotc_itest_tls_error__generate_fixture();

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

int iotc_itest_tls_error_teardown(void** fixture_void) {
  IOTC_UNUSED(fixture_void);

  iotc_delete_context(iotc_context_handle);
  iotc_delete_context_with_custom_layers(
      &iotc_context_mockbroker, itest_mock_broker_codec_layer_chain,
      IOTC_LAYER_CHAIN_SCHEME_LENGTH(IOTC_LAYER_CHAIN_MOCK_BROKER_CODEC));

  iotc_shutdown();

  iotc_itest_tls_error__test_fixture_t* fixture =
      (iotc_itest_tls_error__test_fixture_t*)*fixture_void;

  IOTC_SAFE_FREE(fixture);

  return !iotc_memory_limiter_teardown();
}

void on_publish_received(iotc_context_handle_t in_context_handle,
                         iotc_sub_call_type_t call_type,
                         const iotc_sub_call_params_t* const params,
                         iotc_state_t state, void* user_data) {
  IOTC_UNUSED(in_context_handle);
  IOTC_UNUSED(call_type);
  IOTC_UNUSED(params);
  IOTC_UNUSED(state);
  IOTC_UNUSED(user_data);
}

void tls_error_on_connection_state_changed(
    iotc_context_handle_t in_context_handle, void* data, iotc_state_t state) {
  IOTC_UNUSED(in_context_handle);
  IOTC_UNUSED(data);
  IOTC_UNUSED(state);
}

/**
 * @name    iotc_itest_tls_error__act
 * @brief   mimics a libiotc live environment: drives event dispatcher
 *
 *
 * Does a limited number of iterations, processes some events. Acts
 * like a live libiotc environment. Parameters can define custom actions:
 * publish, disconnect.
 *
 * @param [in] fixture_void         test case specific settings
 * @param [in] do_publish_flag      if set a publish is requested on the API
 * @param [in] do_publish_flag      if set a disconnect is requested on the API
 */
static void iotc_itest_tls_error__act(void** fixture_void, char do_publish_flag,
                                      char do_disconnect_flag) {
  IOTC_PROCESS_INIT_ON_THIS_LAYER(
      &iotc_context_mockbroker->layer_chain.top->layer_connection, NULL,
      IOTC_STATE_OK);

  iotc_evtd_step(iotc_globals.evtd_instance,
                 iotc_bsp_time_getcurrenttime_seconds());

  const iotc_itest_tls_error__test_fixture_t* const fixture =
      (iotc_itest_tls_error__test_fixture_t*)*fixture_void;

  const uint16_t keepalive_timeout = fixture->max_loop_count;

  iotc_connect(iotc_context_handle, "itest_username", "itest_password",
               "itest_client_id", /*connection_timeout=*/20, keepalive_timeout,
               &tls_error_on_connection_state_changed);

  uint8_t loop_counter = 0;
  while (iotc_evtd_dispatcher_continue(iotc_globals.evtd_instance) == 1 &&
         loop_counter < keepalive_timeout) {
    iotc_evtd_step(iotc_globals.evtd_instance,
                   iotc_bsp_time_getcurrenttime_seconds() + loop_counter);
    ++loop_counter;

    if (loop_counter == fixture->loop_id__control_topic_auto_subscribe) {
      iotc_subscribe(iotc_context_handle, fixture->control_topic_name,
                     IOTC_MQTT_QOS_AT_LEAST_ONCE, on_publish_received, NULL);
    }

    if (do_publish_flag && loop_counter == fixture->loop_id__manual_publish) {
      iotc_publish(iotc_context_handle, fixture->test_topic_name,
                   "test message", IOTC_MQTT_QOS_AT_LEAST_ONCE, NULL, NULL);
    }

    if (do_disconnect_flag &&
        loop_counter == fixture->loop_id__manual_disconnect) {
      iotc_shutdown_connection(iotc_context_handle);
    }
  }
}

uint8_t iotc_itest_tls_error__load_level_filter_PUSH(
    uint8_t iotc_state_error_code) {
  return (0 < iotc_test_load_level ||
          /* here probable error codes are collected to reduce runtime for
             rapid*/
          /* testing*/
          iotc_state_error_code == IOTC_STATE_FAILED_WRITING ||
          iotc_state_error_code == IOTC_CONNECTION_RESET_BY_PEER_ERROR ||
          iotc_state_error_code == IOTC_SOCKET_WRITE_ERROR)
             ? 1
             : 0;
}

/*********************************************************************************
 * test cases
 *********************************************************************
 ********************************************************************************/
void iotc_itest_tls_error__tls_pull_PUBACK_errors__graceful_error_handling(
    void** fixture_void) {
  uint8_t iotc_state_error_code = IOTC_STATE_OK + 1;
  for (; iotc_state_error_code < IOTC_ERROR_COUNT; ++iotc_state_error_code) {
    if (iotc_state_error_code != IOTC_OUT_OF_MEMORY &&
        iotc_state_error_code != IOTC_INTERNAL_ERROR &&
        iotc_state_error_code != IOTC_MQTT_UNKNOWN_MESSAGE_ID &&
        iotc_itest_tls_error__load_level_filter_PUSH(iotc_state_error_code)) {
      const iotc_itest_tls_error__test_fixture_t* const fixture =
          (iotc_itest_tls_error__test_fixture_t*)*fixture_void;

      /* one call for mock broker layer chain init*/
      expect_value(iotc_mock_broker_layer_init, in_out_state, IOTC_STATE_OK);
      expect_value(iotc_mock_layer_tls_prev_init, in_out_state, IOTC_STATE_OK);
      expect_value(iotc_mock_layer_tls_prev_connect, in_out_state,
                   IOTC_STATE_OK);
      expect_value(iotc_mock_broker_layer_connect, in_out_state, IOTC_STATE_OK);

      expect_value(iotc_mock_broker_layer_init, in_out_state, IOTC_STATE_OK);
      expect_value(iotc_mock_broker_layer_connect, in_out_state, IOTC_STATE_OK);

      /* no problem during CONNECT*/
      expect_value(iotc_mock_broker_layer_push, in_out_state, IOTC_STATE_OK);
      expect_value(iotc_mock_broker_layer_push, in_out_state,
                   IOTC_STATE_WRITTEN);
      expect_value(iotc_mock_layer_tls_prev_push, in_out_state, IOTC_STATE_OK);

      /* CONNECT message arrives at mock broker*/
      expect_value(iotc_mock_broker_layer_pull, in_out_state, IOTC_STATE_OK);
      expect_value(iotc_mock_broker_layer_pull, recvd_msg_type,
                   IOTC_MQTT_TYPE_CONNECT);

      /* CONNACK sent*/
      expect_value(iotc_mock_broker_secondary_layer_push, in_out_state,
                   IOTC_STATE_OK);
      will_return(iotc_mock_broker_secondary_layer_push, CONTROL_CONTINUE);
      expect_value(iotc_mock_broker_layer_push, in_out_state,
                   IOTC_STATE_WRITTEN);

      /* second message (probably SUBSCRIBE on a control topic)*/
      expect_value(iotc_mock_broker_layer_push, in_out_state, IOTC_STATE_OK);
      expect_value(iotc_mock_broker_layer_push, in_out_state,
                   IOTC_STATE_WRITTEN);
      expect_value(iotc_mock_layer_tls_prev_push, in_out_state, IOTC_STATE_OK);

      /* SUBSCRIBE message arrives at mock broker*/
      expect_value(iotc_mock_broker_layer_pull, in_out_state, IOTC_STATE_OK);
      expect_value(iotc_mock_broker_layer_pull, recvd_msg_type,
                   IOTC_MQTT_TYPE_SUBSCRIBE);
      expect_any(iotc_mock_broker_layer_pull, subscribe_topic_name);

      /* SUBACK sent*/
      expect_value(iotc_mock_broker_secondary_layer_push, in_out_state,
                   IOTC_STATE_OK);
      will_return(iotc_mock_broker_secondary_layer_push, CONTROL_CONTINUE);
      expect_value(iotc_mock_broker_layer_push, in_out_state,
                   IOTC_STATE_WRITTEN);

      /* PUBLISH HEADER*/
      expect_value(iotc_mock_broker_layer_push, in_out_state, IOTC_STATE_OK);
      expect_value(iotc_mock_broker_layer_push, in_out_state,
                   IOTC_STATE_WRITTEN);
      expect_value(iotc_mock_layer_tls_prev_push, in_out_state, IOTC_STATE_OK);

      /* PUBLISH PAYLOAD*/
      expect_value(iotc_mock_broker_layer_push, in_out_state, IOTC_STATE_OK);
      expect_value(iotc_mock_broker_layer_push, in_out_state,
                   IOTC_STATE_WRITTEN);
      expect_value(iotc_mock_layer_tls_prev_push, in_out_state, IOTC_STATE_OK);

      /* PUBLISH message arrives at mock broker*/
      expect_value(iotc_mock_broker_layer_pull, in_out_state, IOTC_STATE_OK);
      expect_value(iotc_mock_broker_layer_pull, recvd_msg_type,
                   IOTC_MQTT_TYPE_PUBLISH);
      const char* test_topic_name = NULL;
      if (NULL != fixture) {
#ifdef IOTC_MANGLE_TOPIC
        test_topic_name = fixture->test_full_topic_name;
#else
        test_topic_name = fixture->test_topic_name;
#endif
      }
      expect_string(iotc_mock_broker_layer_pull, publish_topic_name,
                    test_topic_name);

      /* PUBACK sent*/
      expect_value(iotc_mock_broker_layer_push, in_out_state,
                   IOTC_STATE_WRITTEN);

      /* PUBACK receive error*/
      expect_value(iotc_mock_broker_secondary_layer_push, in_out_state,
                   IOTC_STATE_OK);
      /* --- ERROR --- HERE --- ERROR --- HERE --- ERROR ---*/
      will_return(iotc_mock_broker_secondary_layer_push, CONTROL_ERROR);
      will_return(iotc_mock_broker_secondary_layer_push, iotc_state_error_code);

      /* First close is called upon  */
      expect_value(iotc_mock_broker_layer_close, in_out_state,
                   iotc_state_error_code);
      expect_value(iotc_mock_broker_layer_close, in_out_state,
                   iotc_state_error_code);
      expect_value(iotc_mock_layer_tls_prev_close, in_out_state,
                   iotc_state_error_code);
      expect_value(iotc_mock_layer_tls_prev_close_externally, in_out_state,
                   iotc_state_error_code);

      iotc_itest_tls_error__act(fixture_void, 1, 1);

      /* artificially reset test case*/
      iotc_itest_tls_error_teardown(fixture_void);
      iotc_itest_tls_error_setup(fixture_void);
    }
  }
}

void iotc_itest_tls_error__tls_pull_SUBACK_errors__graceful_error_handling(
    void** fixture_void) {
  uint8_t iotc_state_error_code = IOTC_STATE_OK + 1;
  for (; iotc_state_error_code < IOTC_ERROR_COUNT; ++iotc_state_error_code) {
    if (iotc_state_error_code != IOTC_OUT_OF_MEMORY &&
        iotc_state_error_code != IOTC_INTERNAL_ERROR &&
        iotc_state_error_code != IOTC_MQTT_UNKNOWN_MESSAGE_ID &&
        iotc_itest_tls_error__load_level_filter_PUSH(iotc_state_error_code)) {
      /* one call for mock broker layer chain init*/
      expect_value(iotc_mock_broker_layer_init, in_out_state, IOTC_STATE_OK);
      expect_value(iotc_mock_layer_tls_prev_init, in_out_state, IOTC_STATE_OK);
      expect_value(iotc_mock_layer_tls_prev_connect, in_out_state,
                   IOTC_STATE_OK);
      expect_value(iotc_mock_broker_layer_connect, in_out_state, IOTC_STATE_OK);

      expect_value(iotc_mock_broker_layer_init, in_out_state, IOTC_STATE_OK);
      expect_value(iotc_mock_broker_layer_connect, in_out_state, IOTC_STATE_OK);

      /* no problem during CONNECT*/
      expect_value(iotc_mock_broker_layer_push, in_out_state, IOTC_STATE_OK);
      expect_value(iotc_mock_broker_layer_push, in_out_state,
                   IOTC_STATE_WRITTEN);
      expect_value(iotc_mock_layer_tls_prev_push, in_out_state, IOTC_STATE_OK);

      /* CONNECT message arrives at mock broker*/
      expect_value(iotc_mock_broker_layer_pull, in_out_state, IOTC_STATE_OK);
      expect_value(iotc_mock_broker_layer_pull, recvd_msg_type,
                   IOTC_MQTT_TYPE_CONNECT);

      /* CONNACK sent*/
      expect_value(iotc_mock_broker_secondary_layer_push, in_out_state,
                   IOTC_STATE_OK);
      will_return(iotc_mock_broker_secondary_layer_push, CONTROL_CONTINUE);
      expect_value(iotc_mock_broker_layer_push, in_out_state,
                   IOTC_STATE_WRITTEN);

      /* second message (probably SUBSCRIBE on a control topic)*/
      expect_value(iotc_mock_broker_layer_push, in_out_state, IOTC_STATE_OK);
      expect_value(iotc_mock_broker_layer_push, in_out_state,
                   IOTC_STATE_WRITTEN);
      expect_value(iotc_mock_layer_tls_prev_push, in_out_state, IOTC_STATE_OK);

      /* SUBSCRIBE message arrives at mock broker*/
      expect_value(iotc_mock_broker_layer_pull, in_out_state, IOTC_STATE_OK);
      expect_value(iotc_mock_broker_layer_pull, recvd_msg_type,
                   IOTC_MQTT_TYPE_SUBSCRIBE);

      expect_any(iotc_mock_broker_layer_pull, subscribe_topic_name);

      /* SUBACK receive error*/
      expect_value(iotc_mock_broker_secondary_layer_push, in_out_state,
                   IOTC_STATE_OK);
      /* --- ERROR --- HERE --- ERROR --- HERE --- ERROR ---*/
      will_return(iotc_mock_broker_secondary_layer_push, CONTROL_ERROR);
      will_return(iotc_mock_broker_secondary_layer_push, iotc_state_error_code);
      /* SUBACK sent*/
      expect_value(iotc_mock_broker_layer_push, in_out_state,
                   IOTC_STATE_WRITTEN);
      will_return(iotc_mock_broker_layer_push, CONTROL_CONTINUE);

      expect_value(iotc_mock_broker_layer_close, in_out_state,
                   iotc_state_error_code);
      expect_value(iotc_mock_broker_layer_close, in_out_state,
                   iotc_state_error_code);
      expect_value(iotc_mock_layer_tls_prev_close, in_out_state,
                   iotc_state_error_code);
      expect_value(iotc_mock_layer_tls_prev_close_externally, in_out_state,
                   iotc_state_error_code);

      iotc_itest_tls_error__act(fixture_void, 0, 1);

      /* artificially reset test case*/
      iotc_itest_tls_error_teardown(fixture_void);
      iotc_itest_tls_error_setup(fixture_void);
    }
  }
}

void iotc_itest_tls_error__tls_pull_CONNACK_errors__graceful_error_handling(
    void** fixture_void) {
  IOTC_UNUSED(fixture_void);

  uint8_t iotc_state_error_code = IOTC_STATE_OK + 1;
  for (; iotc_state_error_code < IOTC_ERROR_COUNT; ++iotc_state_error_code) {
    if (iotc_state_error_code != IOTC_OUT_OF_MEMORY &&
        iotc_state_error_code != IOTC_INTERNAL_ERROR &&
        iotc_state_error_code != IOTC_MQTT_UNKNOWN_MESSAGE_ID &&
        iotc_itest_tls_error__load_level_filter_PUSH(iotc_state_error_code)) {
      /* one call for mock broker layer chain init*/
      expect_value(iotc_mock_broker_layer_init, in_out_state, IOTC_STATE_OK);
      expect_value(iotc_mock_layer_tls_prev_init, in_out_state, IOTC_STATE_OK);
      expect_value(iotc_mock_layer_tls_prev_connect, in_out_state,
                   IOTC_STATE_OK);
      expect_value(iotc_mock_broker_layer_connect, in_out_state, IOTC_STATE_OK);

      expect_value(iotc_mock_broker_layer_init, in_out_state, IOTC_STATE_OK);
      expect_value(iotc_mock_broker_layer_connect, in_out_state, IOTC_STATE_OK);

      /* no problem during CONNECT*/
      expect_value(iotc_mock_broker_layer_push, in_out_state, IOTC_STATE_OK);
      expect_value(iotc_mock_broker_layer_push, in_out_state,
                   IOTC_STATE_WRITTEN);
      expect_value(iotc_mock_layer_tls_prev_push, in_out_state, IOTC_STATE_OK);

      /* CONNECT message arrives at mock broker*/
      expect_value(iotc_mock_broker_layer_pull, in_out_state, IOTC_STATE_OK);
      expect_value(iotc_mock_broker_layer_pull, recvd_msg_type,
                   IOTC_MQTT_TYPE_CONNECT);

      /* CONNACK receive error*/
      expect_value(iotc_mock_broker_secondary_layer_push, in_out_state,
                   IOTC_STATE_OK);
      /* --- ERROR --- HERE --- ERROR --- HERE --- ERROR ---*/
      will_return(iotc_mock_broker_secondary_layer_push, CONTROL_ERROR);
      will_return(iotc_mock_broker_secondary_layer_push, iotc_state_error_code);
      /* CONNACK sent*/
      expect_value(iotc_mock_broker_layer_push, in_out_state,
                   IOTC_STATE_WRITTEN);
      will_return(iotc_mock_broker_layer_push, CONTROL_CONTINUE);

      expect_value(iotc_mock_broker_layer_close, in_out_state,
                   iotc_state_error_code);
      expect_value(iotc_mock_broker_layer_close, in_out_state,
                   iotc_state_error_code);
      expect_value(iotc_mock_layer_tls_prev_close, in_out_state,
                   iotc_state_error_code);
      expect_value(iotc_mock_layer_tls_prev_close_externally, in_out_state,
                   iotc_state_error_code);

      iotc_itest_tls_error__act(fixture_void, 0, 1);

      /* artificially reset test case*/
      iotc_itest_tls_error_teardown(fixture_void);
      iotc_itest_tls_error_setup(fixture_void);
    }
  }
}

void iotc_itest_tls_error__tls_push_CONNECT_errors__graceful_error_handling(
    void** fixture_void) {
  IOTC_UNUSED(fixture_void);

  uint8_t iotc_state_error_code = IOTC_STATE_OK + 1;
  for (; iotc_state_error_code < IOTC_ERROR_COUNT; ++iotc_state_error_code) {
    /* noteatigyi: this restriction is because mqtt codec layer push function is
     * not robust enough it only handles IOTC_STATE_FAILED_WRITING error,
     * otherwise it makes an assert */
    if (iotc_state_error_code == IOTC_STATE_FAILED_WRITING) {
      /* one call for mock broker layer chain init*/
      expect_value(iotc_mock_broker_layer_init, in_out_state, IOTC_STATE_OK);
      expect_value(iotc_mock_layer_tls_prev_init, in_out_state, IOTC_STATE_OK);
      expect_value(iotc_mock_layer_tls_prev_connect, in_out_state,
                   IOTC_STATE_OK);
      expect_value(iotc_mock_broker_layer_connect, in_out_state, IOTC_STATE_OK);

      expect_value(iotc_mock_broker_layer_init, in_out_state, IOTC_STATE_OK);
      expect_value(iotc_mock_broker_layer_connect, in_out_state, IOTC_STATE_OK);

      /* error during CONNECT*/
      expect_value(iotc_mock_broker_layer_push, in_out_state, IOTC_STATE_OK);
      /* --- ERROR --- HERE --- ERROR --- HERE --- ERROR ---*/
      will_return(iotc_mock_broker_layer_push, CONTROL_ERROR);
      will_return(iotc_mock_broker_layer_push__ERROR_CHANNEL,
                  iotc_state_error_code);

      expect_value(iotc_mock_broker_layer_close, in_out_state,
                   iotc_state_error_code);

      expect_value(iotc_mock_layer_tls_prev_close, in_out_state,
                   iotc_state_error_code);
      expect_value(iotc_mock_layer_tls_prev_close_externally, in_out_state,
                   iotc_state_error_code);
      expect_value(iotc_mock_broker_layer_close, in_out_state,
                   iotc_state_error_code);

      iotc_itest_tls_error__act(fixture_void, 0, 0);

      /* artificially reset test case*/
      iotc_itest_tls_error_teardown(fixture_void);
      iotc_itest_tls_error_setup(fixture_void);
    }
  }
}

void iotc_itest_tls_error__tls_push_infinite_SUBSCRIBE_errors__reSUBSCRIBE_occurs_once_in_a_second(
    void** fixture_void) {
  uint8_t iotc_state_error_code = IOTC_STATE_OK + 1;
  for (; iotc_state_error_code < IOTC_ERROR_COUNT; ++iotc_state_error_code) {
    /* noteatigyi: this restriction is because mqtt codec layer push function is
     * not robust enough it only handles IOTC_STATE_FAILED_WRITING error,
     * otherwise it makes an assert */
    if (iotc_state_error_code == IOTC_STATE_FAILED_WRITING) {
      const iotc_itest_tls_error__test_fixture_t* const fixture =
          (iotc_itest_tls_error__test_fixture_t*)*fixture_void;

      /* one call for mock broker layer chain init*/
      expect_value(iotc_mock_broker_layer_init, in_out_state, IOTC_STATE_OK);
      expect_value(iotc_mock_layer_tls_prev_init, in_out_state, IOTC_STATE_OK);
      expect_value(iotc_mock_layer_tls_prev_connect, in_out_state,
                   IOTC_STATE_OK);
      expect_value(iotc_mock_broker_layer_connect, in_out_state, IOTC_STATE_OK);

      expect_value(iotc_mock_broker_layer_init, in_out_state, IOTC_STATE_OK);
      expect_value(iotc_mock_broker_layer_connect, in_out_state, IOTC_STATE_OK);

      /* no problem during CONNECT*/
      expect_value(iotc_mock_broker_layer_push, in_out_state, IOTC_STATE_OK);
      will_return_count(iotc_mock_broker_layer_push, CONTROL_CONTINUE, 2);
      expect_value(iotc_mock_broker_layer_push, in_out_state,
                   IOTC_STATE_WRITTEN);
      expect_value(iotc_mock_layer_tls_prev_push, in_out_state, IOTC_STATE_OK);

      /* CONNECT message arrives at mock broker*/
      expect_value(iotc_mock_broker_layer_pull, in_out_state, IOTC_STATE_OK);
      expect_value(iotc_mock_broker_layer_pull, recvd_msg_type,
                   IOTC_MQTT_TYPE_CONNECT);

      /* CONNACK sent*/
      expect_value(iotc_mock_broker_secondary_layer_push, in_out_state,
                   IOTC_STATE_OK);
      expect_value(iotc_mock_broker_layer_push, in_out_state,
                   IOTC_STATE_WRITTEN);
      will_return(iotc_mock_broker_layer_push, CONTROL_CONTINUE);

      /* ERROR HERE: always drive mock TLS layer to report error upon each
       * SUBSCRIBE send*/
      will_return_always(iotc_mock_broker_layer_push, CONTROL_ERROR);
      will_return_always(iotc_mock_broker_layer_push__ERROR_CHANNEL,
                         iotc_state_error_code);

      /* EXPECTATION: here we calculate how many times we expect the
       * iotc_mock_broker_layer_push is going to be called. This value depends
       * on the rate the service will try to send data. Here we are taking into
       * account the number of SUBSCRIBES plus the number of DISCONNECTS. Number
       * of SUBSCRIBES depends on the number of loops in which the service logic
       * will try to send and re-send SUBSCRIBES. Quantity of loops depends on
       * the test setup. In our case it is the difference between the loop
       * number described by fixture->loop_id__manual_disconnect and
       * fixture->loop_id__control_topic_auto_subscribe. This difference however
       * does not take into account the additional DISCONNECT message and one
       * additional re-SUBSCRIBE that will be sent in that last loop iteration
       * that's why we have to increase this value by 2.*/
      const uint8_t expected_number_of_PUSHES =
          fixture->loop_id__manual_disconnect -
          fixture->loop_id__control_topic_auto_subscribe + 2;

      /* expecting only a certain number of message sends*/
      expect_value_count(iotc_mock_broker_layer_push, in_out_state,
                         IOTC_STATE_OK, expected_number_of_PUSHES);

      expect_value(iotc_mock_broker_layer_close, in_out_state, IOTC_STATE_OK);
      expect_value(iotc_mock_broker_layer_close, in_out_state, IOTC_STATE_OK);
      expect_value(iotc_mock_layer_tls_prev_close, in_out_state, IOTC_STATE_OK);
      expect_value(iotc_mock_layer_tls_prev_close_externally, in_out_state,
                   IOTC_STATE_OK);

      /* This suppresses warning messages, helps debugging these test cases.*/
      /*will_return( iotc_mock_broker_layer_init, CONTROL_CONTINUE );
      will_return( iotc_mock_broker_layer_init, CONTROL_CONTINUE );
      will_return( iotc_mock_layer_tls_prev_push, CONTROL_CONTINUE );
      will_return( iotc_mock_broker_secondary_layer_push, CONTROL_CONTINUE );*/

      /* In order to release the whole allocated memory we have make the test to
       * start close process on each layer that's why the manual disconnect flag
       * is set */
      iotc_itest_tls_error__act(fixture_void, 0, 1);

      /* artificially reset test case*/
      iotc_itest_tls_error_teardown(fixture_void);
      iotc_itest_tls_error_setup(fixture_void);
    }
  }
}

void iotc_itest_tls_error__tls_push_SUBSCRIBE_errors__graceful_error_handling(
    void** fixture_void) {
  uint8_t iotc_state_error_code = IOTC_STATE_OK + 1;
  for (; iotc_state_error_code < IOTC_ERROR_COUNT; ++iotc_state_error_code) {
    /* noteatigyi: this restriction is because mqtt codec layer push function is
     * not robust enough it only handles IOTC_STATE_FAILED_WRITING error,
     * otherwise it makes an assert */
    if (iotc_state_error_code == IOTC_STATE_FAILED_WRITING) {
      const iotc_itest_tls_error__test_fixture_t* const fixture =
          (iotc_itest_tls_error__test_fixture_t*)*fixture_void;

      /* one call for mock broker layer chain init*/
      expect_value(iotc_mock_broker_layer_init, in_out_state, IOTC_STATE_OK);
      expect_value(iotc_mock_layer_tls_prev_init, in_out_state, IOTC_STATE_OK);
      expect_value(iotc_mock_layer_tls_prev_connect, in_out_state,
                   IOTC_STATE_OK);
      expect_value(iotc_mock_broker_layer_connect, in_out_state, IOTC_STATE_OK);

      expect_value(iotc_mock_broker_layer_init, in_out_state, IOTC_STATE_OK);
      expect_value(iotc_mock_broker_layer_connect, in_out_state, IOTC_STATE_OK);

      /* no problem during CONNECT*/
      expect_value(iotc_mock_broker_layer_push, in_out_state, IOTC_STATE_OK);
      will_return_count(iotc_mock_broker_layer_push, CONTROL_CONTINUE, 2);
      expect_value(iotc_mock_broker_layer_push, in_out_state,
                   IOTC_STATE_WRITTEN);
      expect_value(iotc_mock_layer_tls_prev_push, in_out_state, IOTC_STATE_OK);

      /* CONNECT message arrives at mock broker*/
      expect_value(iotc_mock_broker_layer_pull, in_out_state, IOTC_STATE_OK);
      expect_value(iotc_mock_broker_layer_pull, recvd_msg_type,
                   IOTC_MQTT_TYPE_CONNECT);

      /* CONNACK sent*/
      expect_value(iotc_mock_broker_secondary_layer_push, in_out_state,
                   IOTC_STATE_OK);
      expect_value(iotc_mock_broker_layer_push, in_out_state,
                   IOTC_STATE_WRITTEN);
      will_return(iotc_mock_broker_layer_push, CONTROL_CONTINUE);

      const uint8_t expected_number_of_SUBSCRIBE_errors =
          fixture->loop_id__manual_disconnect -
          1 /* -1: DISCONNECT happens before SUBSCRIBE */
          - fixture->loop_id__control_topic_auto_subscribe -
          1; /* -1: the last SUBSCRIBE succeeds */

      assert_in_range(expected_number_of_SUBSCRIBE_errors, 1,
                      fixture->max_loop_count);

      /* error during SUBSCRIBE */
      expect_value_count(iotc_mock_broker_layer_push, in_out_state,
                         IOTC_STATE_OK, expected_number_of_SUBSCRIBE_errors);
      /* --- ERROR --- HERE --- ERROR --- HERE --- ERROR --- */
      will_return_count(iotc_mock_broker_layer_push, CONTROL_ERROR,
                        expected_number_of_SUBSCRIBE_errors);
      will_return_count(iotc_mock_broker_layer_push__ERROR_CHANNEL,
                        iotc_state_error_code,
                        expected_number_of_SUBSCRIBE_errors);

      /* SUBSCRIBE success */
      expect_value(iotc_mock_broker_layer_push, in_out_state, IOTC_STATE_OK);
      will_return_count(iotc_mock_broker_layer_push, CONTROL_CONTINUE, 2);
      expect_value(iotc_mock_broker_layer_push, in_out_state,
                   IOTC_STATE_WRITTEN);
      expect_value(iotc_mock_layer_tls_prev_push, in_out_state, IOTC_STATE_OK);

      /* SUBSCRIBE message arrives at mock broker */
      expect_value(iotc_mock_broker_layer_pull, in_out_state, IOTC_STATE_OK);
      expect_value(iotc_mock_broker_layer_pull, recvd_msg_type,
                   IOTC_MQTT_TYPE_SUBSCRIBE);
      expect_any(iotc_mock_broker_layer_pull, subscribe_topic_name);

      /* SUBACK sent*/
      expect_value(iotc_mock_broker_secondary_layer_push, in_out_state,
                   IOTC_STATE_OK);
      expect_value(iotc_mock_broker_layer_push, in_out_state,
                   IOTC_STATE_WRITTEN);
      will_return(iotc_mock_broker_layer_push, CONTROL_CONTINUE);

      /* third message (probably DISCONNECT on a control topic)*/
      expect_value(iotc_mock_broker_layer_push, in_out_state, IOTC_STATE_OK);
      will_return_count(iotc_mock_broker_layer_push, CONTROL_CONTINUE, 2);
      expect_value(iotc_mock_broker_layer_push, in_out_state,
                   IOTC_STATE_WRITTEN);
      expect_value(iotc_mock_layer_tls_prev_push, in_out_state, IOTC_STATE_OK);

      /* DISCONNECT message arrives at mock broker*/
      expect_value(iotc_mock_broker_layer_pull, in_out_state, IOTC_STATE_OK);
      expect_value(iotc_mock_broker_layer_pull, recvd_msg_type,
                   IOTC_MQTT_TYPE_DISCONNECT);

      expect_value(iotc_mock_broker_layer_close, in_out_state, IOTC_STATE_OK);
      expect_value(iotc_mock_broker_layer_close, in_out_state, IOTC_STATE_OK);
      expect_value(iotc_mock_layer_tls_prev_close, in_out_state, IOTC_STATE_OK);
      expect_value(iotc_mock_layer_tls_prev_close_externally, in_out_state,
                   IOTC_STATE_OK);

      iotc_itest_tls_error__act(fixture_void, 0, 1);

      /* artificially reset test case*/
      iotc_itest_tls_error_teardown(fixture_void);
      iotc_itest_tls_error_setup(fixture_void);
    }
  }
}

void iotc_itest_tls_error__tls_push_PUBLISH_errors__graceful_error_handling(
    void** fixture_void) {
  uint8_t iotc_state_error_code = IOTC_STATE_OK + 1;
  for (; iotc_state_error_code < IOTC_ERROR_COUNT; ++iotc_state_error_code) {
    /* noteatigyi: this restriction is because mqtt codec layer push function is
     * not robust enough it only handles IOTC_STATE_FAILED_WRITING error,
     * otherwise it makes an assert */
    if (iotc_state_error_code == IOTC_STATE_FAILED_WRITING) {
      const iotc_itest_tls_error__test_fixture_t* const fixture =
          (iotc_itest_tls_error__test_fixture_t*)*fixture_void;

      /* one call for mock broker layer chain init*/
      expect_value(iotc_mock_broker_layer_init, in_out_state, IOTC_STATE_OK);
      expect_value(iotc_mock_layer_tls_prev_init, in_out_state, IOTC_STATE_OK);
      expect_value(iotc_mock_layer_tls_prev_connect, in_out_state,
                   IOTC_STATE_OK);
      expect_value(iotc_mock_broker_layer_connect, in_out_state, IOTC_STATE_OK);

      expect_value(iotc_mock_broker_layer_init, in_out_state, IOTC_STATE_OK);
      expect_value(iotc_mock_broker_layer_connect, in_out_state, IOTC_STATE_OK);

      /* no problem during CONNECT*/
      expect_value(iotc_mock_broker_layer_push, in_out_state, IOTC_STATE_OK);
      will_return_count(iotc_mock_broker_layer_push, CONTROL_CONTINUE, 2);
      expect_value(iotc_mock_broker_layer_push, in_out_state,
                   IOTC_STATE_WRITTEN);
      expect_value(iotc_mock_layer_tls_prev_push, in_out_state, IOTC_STATE_OK);

      /* CONNECT message arrives at mock broker*/
      expect_value(iotc_mock_broker_layer_pull, in_out_state, IOTC_STATE_OK);
      expect_value(iotc_mock_broker_layer_pull, recvd_msg_type,
                   IOTC_MQTT_TYPE_CONNECT);

      /* CONNACK sent*/
      expect_value(iotc_mock_broker_secondary_layer_push, in_out_state,
                   IOTC_STATE_OK);
      expect_value(iotc_mock_broker_layer_push, in_out_state,
                   IOTC_STATE_WRITTEN);
      will_return(iotc_mock_broker_layer_push, CONTROL_CONTINUE);

      expect_value(iotc_mock_broker_layer_push, in_out_state, IOTC_STATE_OK);
      will_return_count(iotc_mock_broker_layer_push, CONTROL_CONTINUE, 2);
      expect_value(iotc_mock_broker_layer_push, in_out_state,
                   IOTC_STATE_WRITTEN);

      /* SUBSCRIBE message arrives at mock broker*/
      expect_value(iotc_mock_broker_layer_pull, in_out_state, IOTC_STATE_OK);
      expect_value(iotc_mock_broker_layer_pull, recvd_msg_type,
                   IOTC_MQTT_TYPE_SUBSCRIBE);
      expect_any(iotc_mock_broker_layer_pull, subscribe_topic_name);

      expect_value(iotc_mock_layer_tls_prev_push, in_out_state, IOTC_STATE_OK);

      /* SUBACK sent*/
      expect_value(iotc_mock_broker_secondary_layer_push, in_out_state,
                   IOTC_STATE_OK);
      expect_value(iotc_mock_broker_layer_push, in_out_state,
                   IOTC_STATE_WRITTEN);
      will_return(iotc_mock_broker_layer_push, CONTROL_CONTINUE);

      /* noteatigyi: this blocks while PUBLISH fails, this is bad I guess blocks
       * means: does not return to client application*/
      size_t loop_count = 0;
      for (; loop_count < 100; ++loop_count) {
        /* error during PUBLISH*/
        expect_value(iotc_mock_broker_layer_push, in_out_state, IOTC_STATE_OK);
        /* --- ERROR --- HERE --- ERROR --- HERE --- ERROR ---*/
        will_return(iotc_mock_broker_layer_push, CONTROL_ERROR);
        will_return(iotc_mock_broker_layer_push__ERROR_CHANNEL,
                    iotc_state_error_code);
      }

      expect_value(iotc_mock_broker_layer_push, in_out_state, IOTC_STATE_OK);
      will_return_count(iotc_mock_broker_layer_push, CONTROL_CONTINUE, 2);
      expect_value(iotc_mock_broker_layer_push, in_out_state,
                   IOTC_STATE_WRITTEN);
      expect_value(iotc_mock_layer_tls_prev_push, in_out_state, IOTC_STATE_OK);

      expect_value(iotc_mock_broker_layer_push, in_out_state, IOTC_STATE_OK);
      will_return_count(iotc_mock_broker_layer_push, CONTROL_CONTINUE, 2);
      expect_value(iotc_mock_broker_layer_push, in_out_state,
                   IOTC_STATE_WRITTEN);
      expect_value(iotc_mock_layer_tls_prev_push, in_out_state, IOTC_STATE_OK);

      /* PUBLISH message arrives at mock broker*/
      expect_value(iotc_mock_broker_layer_pull, in_out_state, IOTC_STATE_OK);
      expect_value(iotc_mock_broker_layer_pull, recvd_msg_type,
                   IOTC_MQTT_TYPE_PUBLISH);
#ifdef IOTC_MANGLE_TOPIC
      expect_string(iotc_mock_broker_layer_pull, publish_topic_name,
                    fixture->test_full_topic_name);
#else
      expect_string(iotc_mock_broker_layer_pull, publish_topic_name,
                    fixture->test_topic_name);
#endif

      /* sending PUBACK*/
      expect_value(iotc_mock_broker_secondary_layer_push, in_out_state,
                   IOTC_STATE_OK);
      expect_value(iotc_mock_broker_layer_push, in_out_state,
                   IOTC_STATE_WRITTEN);
      will_return(iotc_mock_broker_layer_push, CONTROL_CONTINUE);

      /* third message (probably DISCONNECT on a control topic)*/
      expect_value(iotc_mock_broker_layer_push, in_out_state, IOTC_STATE_OK);
      will_return_count(iotc_mock_broker_layer_push, CONTROL_CONTINUE, 2);
      expect_value(iotc_mock_broker_layer_push, in_out_state,
                   IOTC_STATE_WRITTEN);
      expect_value(iotc_mock_layer_tls_prev_push, in_out_state, IOTC_STATE_OK);

      expect_value(iotc_mock_broker_layer_close, in_out_state, IOTC_STATE_OK);
      expect_value(iotc_mock_broker_layer_close, in_out_state, IOTC_STATE_OK);
      expect_value(iotc_mock_layer_tls_prev_close, in_out_state, IOTC_STATE_OK);
      expect_value(iotc_mock_layer_tls_prev_close_externally, in_out_state,
                   IOTC_STATE_OK);

      /* DISCONNECT message arrives at mock broker*/
      expect_value(iotc_mock_broker_layer_pull, in_out_state, IOTC_STATE_OK);
      expect_value(iotc_mock_broker_layer_pull, recvd_msg_type,
                   IOTC_MQTT_TYPE_DISCONNECT);

      iotc_itest_tls_error__act(fixture_void, 1, 1);

      /* artificially reset test case*/
      iotc_itest_tls_error_teardown(fixture_void);
      iotc_itest_tls_error_setup(fixture_void);
    }
  }
}

void iotc_itest_tls_error__tls_init_and_connect_errors__graceful_error_handling(
    void** fixture_void) {
  IOTC_UNUSED(fixture_void);

  uint8_t iotc_state_error_code = IOTC_STATE_OK + 1;
  for (; iotc_state_error_code < IOTC_ERROR_COUNT; ++iotc_state_error_code) {
    if (0 < iotc_test_load_level ||
        /* here probable error codes are collected to reduce runtime for rapid*/
        /* testing*/
        iotc_state_error_code == IOTC_TLS_INITALIZATION_ERROR ||
        iotc_state_error_code == IOTC_TLS_FAILED_LOADING_CERTIFICATE ||
        iotc_state_error_code == IOTC_TLS_CONNECT_ERROR ||
        iotc_state_error_code == IOTC_SOCKET_INITIALIZATION_ERROR ||
        iotc_state_error_code == IOTC_OUT_OF_MEMORY ||
        iotc_state_error_code == IOTC_SOCKET_GETSOCKOPT_ERROR ||
        iotc_state_error_code == IOTC_SOCKET_CONNECTION_ERROR) {
      /* one call for mock broker layer chain init*/
      expect_value(iotc_mock_broker_layer_init, in_out_state, IOTC_STATE_OK);
      will_return(iotc_mock_broker_layer_init, CONTROL_CONTINUE);
      expect_value(iotc_mock_broker_layer_connect, in_out_state, IOTC_STATE_OK);

      expect_value(iotc_mock_broker_layer_init, in_out_state, IOTC_STATE_OK);
      /* --- ERROR --- HERE --- ERROR --- HERE --- ERROR ---*/
      will_return(iotc_mock_broker_layer_init, CONTROL_ERROR);
      will_return(iotc_mock_broker_layer_init, iotc_state_error_code);

      expect_value(iotc_mock_broker_layer_connect, in_out_state,
                   iotc_state_error_code);

      expect_value(iotc_mock_broker_layer_close, in_out_state,
                   iotc_state_error_code);
      expect_value(iotc_mock_layer_tls_prev_close, in_out_state,
                   iotc_state_error_code);
      expect_value(iotc_mock_layer_tls_prev_close_externally, in_out_state,
                   iotc_state_error_code);
      expect_value(iotc_mock_broker_layer_close, in_out_state,
                   iotc_state_error_code);

      iotc_itest_tls_error__act(fixture_void, 0, 0);

      /* artificially reset test case*/
      iotc_itest_tls_error_teardown(fixture_void);
      iotc_itest_tls_error_setup(fixture_void);
    }
  }
}

void iotc_itest_tls_error__connection_flow__basic_checks(void** fixture_void) {
  const iotc_itest_tls_error__test_fixture_t* const fixture =
      (iotc_itest_tls_error__test_fixture_t*)*fixture_void;

  /* one call for mock broker layer chain init*/
  expect_value(iotc_mock_broker_layer_init, in_out_state, IOTC_STATE_OK);
  expect_value(iotc_mock_layer_tls_prev_init, in_out_state, IOTC_STATE_OK);
  expect_value(iotc_mock_layer_tls_prev_connect, in_out_state, IOTC_STATE_OK);
  expect_value(iotc_mock_broker_layer_connect, in_out_state, IOTC_STATE_OK);

  /* expected activity of the libiotc's control topci + mqtt logic + mqtt codec
   * layer stack*/
  expect_value(iotc_mock_broker_layer_init, in_out_state, IOTC_STATE_OK);
  expect_value(iotc_mock_broker_layer_connect, in_out_state, IOTC_STATE_OK);

  /* first message (probably CONNECT)*/
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
  expect_value(iotc_mock_broker_layer_push, in_out_state, IOTC_STATE_WRITTEN);

  /* second message (probably SUBSCRIBE on a control topic)*/
  expect_value(iotc_mock_broker_layer_push, in_out_state, IOTC_STATE_OK);
  expect_value(iotc_mock_broker_layer_push, in_out_state, IOTC_STATE_WRITTEN);
  expect_value(iotc_mock_layer_tls_prev_push, in_out_state, IOTC_STATE_OK);

  /* SUBSCRIBE message arrives at mock broker*/
  expect_value(iotc_mock_broker_layer_pull, in_out_state, IOTC_STATE_OK);
  expect_value(iotc_mock_broker_layer_pull, recvd_msg_type,
               IOTC_MQTT_TYPE_SUBSCRIBE);
  expect_any(iotc_mock_broker_layer_pull, subscribe_topic_name);

  /* SUBACK sent*/
  expect_value(iotc_mock_broker_secondary_layer_push, in_out_state,
               IOTC_STATE_OK);
  expect_value(iotc_mock_broker_layer_push, in_out_state, IOTC_STATE_WRITTEN);

  /* PUBLISH*/
  expect_value(iotc_mock_broker_layer_push, in_out_state, IOTC_STATE_OK);
  expect_value(iotc_mock_broker_layer_push, in_out_state, IOTC_STATE_WRITTEN);
  expect_value(iotc_mock_layer_tls_prev_push, in_out_state, IOTC_STATE_OK);

  /* PUBLISH PAYLOAD*/
  expect_value(iotc_mock_broker_layer_push, in_out_state, IOTC_STATE_OK);
  expect_value(iotc_mock_broker_layer_push, in_out_state, IOTC_STATE_WRITTEN);
  expect_value(iotc_mock_layer_tls_prev_push, in_out_state, IOTC_STATE_OK);

  /* PUBLISH message arrives at mock broker*/
  expect_value(iotc_mock_broker_layer_pull, in_out_state, IOTC_STATE_OK);
  expect_value(iotc_mock_broker_layer_pull, recvd_msg_type,
               IOTC_MQTT_TYPE_PUBLISH);
#ifdef IOTC_MANGLE_TOPIC
  expect_string(iotc_mock_broker_layer_pull, publish_topic_name,
                fixture->test_full_topic_name);
#else
  expect_string(iotc_mock_broker_layer_pull, publish_topic_name,
                fixture->test_topic_name);
#endif

  /* sending PUBACK*/
  expect_value(iotc_mock_broker_secondary_layer_push, in_out_state,
               IOTC_STATE_OK);
  expect_value(iotc_mock_broker_layer_push, in_out_state, IOTC_STATE_WRITTEN);

  /* third message (probably DISCONNECT on a control topic)*/
  expect_value(iotc_mock_broker_layer_push, in_out_state, IOTC_STATE_OK);
  expect_value(iotc_mock_broker_layer_push, in_out_state, IOTC_STATE_WRITTEN);
  expect_value(iotc_mock_layer_tls_prev_push, in_out_state, IOTC_STATE_OK);

  expect_value(iotc_mock_broker_layer_close, in_out_state, IOTC_STATE_OK);
  expect_value(iotc_mock_broker_layer_close, in_out_state, IOTC_STATE_OK);
  expect_value(iotc_mock_layer_tls_prev_close, in_out_state, IOTC_STATE_OK);
  expect_value(iotc_mock_layer_tls_prev_close_externally, in_out_state,
               IOTC_STATE_OK);

  /* DISCONNECT message arrives at mock broker*/
  expect_value(iotc_mock_broker_layer_pull, in_out_state, IOTC_STATE_OK);
  expect_value(iotc_mock_broker_layer_pull, recvd_msg_type,
               IOTC_MQTT_TYPE_DISCONNECT);

  iotc_itest_tls_error__act(fixture_void, 1, 1);
}
