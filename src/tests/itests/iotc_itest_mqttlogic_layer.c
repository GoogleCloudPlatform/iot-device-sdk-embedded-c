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

#include "iotc_itest_mqttlogic_layer.h"
#include "iotc_connection_data_internal.h"
#include "iotc_globals.h"
#include "iotc_handle.h"
#include "iotc_itest_helpers.h"
#include "iotc_itest_layerchain_mqttlogic.h"
#include "iotc_memory_checks.h"
#include "iotc_mqtt_logic_layer_data_helpers.h"

#include <time.h>

iotc_context_t* iotc_context__itest_mqttlogic_layer = NULL;
const iotc_state_t iotc_itest_mqttlogic__backoff_terminal_class_errors[] = {
    IOTC_CONNECTION_RESET_BY_PEER_ERROR,
    IOTC_MQTT_UNACCEPTABLE_PROTOCOL_VERSION, IOTC_MQTT_IDENTIFIER_REJECTED,
    IOTC_MQTT_BAD_USERNAME_OR_PASSWORD, IOTC_MQTT_NOT_AUTHORIZED};

typedef enum {
  IOTC_MQTT_QOS_0 = 0,
  IOTC_MQTT_QOS_1 = 1,
  IOTC_MQTT_QOS_TEST_LEVELS_COUNT
} iotc_itest_mqttlogic_qos_levels_t;

typedef struct iotc_itest_mqttlogic_test_msg_what_to_check_s {
  unsigned char retain : 1;
  unsigned char qos : 1;
  unsigned char dup : 1;
  unsigned char type : 1;
} iotc_itest_mqttlogic_test_msg_what_to_check_t;

typedef struct iotc_itest_mqttlogic_test_msg_common_bits_check_values_s {
  unsigned int retain : 1;
  unsigned int qos : 2;
  unsigned int dup : 1;
  unsigned int type : 4;
} iotc_itest_mqttlogic_test_msg_common_bits_check_values_t;

typedef struct iotc_itest_mqttlogic_test_msg_s {
  iotc_itest_mqttlogic_test_msg_what_to_check_t what_to_check;
  iotc_itest_mqttlogic_test_msg_common_bits_check_values_t values_to_check;
} iotc_itest_mqttlogic_test_msg_t;

iotc_itest_mqttlogic_test_msg_t* iotc_itest_mqttlogic_make_msg_test_matrix(
    iotc_itest_mqttlogic_test_msg_what_to_check_t what_to_check,
    iotc_itest_mqttlogic_test_msg_common_bits_check_values_t values_to_check) {
  iotc_itest_mqttlogic_test_msg_t* test_values =
      (iotc_itest_mqttlogic_test_msg_t*)malloc(
          sizeof(iotc_itest_mqttlogic_test_msg_t));

  test_values->what_to_check = what_to_check;
  test_values->values_to_check = values_to_check;

  return test_values;
}

#define iotc_itest_mqttlogic_test(what, msg, test_msg_data, ret) \
  if (test_msg_data->what_to_check.what) {                       \
    if (msg->common.common_u.common_bits.what !=                 \
        test_msg_data->values_to_check.what) {                   \
      fprintf(stderr,                                            \
              "msg bit - [%d] " #what                            \
              " does not match the expected value [%d]\n",       \
              msg->common.common_u.common_bits.what,             \
              test_msg_data->values_to_check.what);              \
      fflush(stderr);                                            \
      ret = 0;                                                   \
    }                                                            \
  }

int check_msg(const LargestIntegralType data,
              const LargestIntegralType check_value_data) {
  iotc_itest_mqttlogic_test_msg_t* test_msg_data =
      (iotc_itest_mqttlogic_test_msg_t*)check_value_data;

  iotc_mqtt_message_t* msg = (iotc_mqtt_message_t*)data;

  int ret = 1;

  assert_non_null(msg);
  assert_non_null(test_msg_data);

  iotc_itest_mqttlogic_test(retain, msg, test_msg_data, ret);
  iotc_itest_mqttlogic_test(qos, msg, test_msg_data, ret);
  iotc_itest_mqttlogic_test(dup, msg, test_msg_data, ret);
  iotc_itest_mqttlogic_test(type, msg, test_msg_data, ret);

  free(test_msg_data);
  return ret;
}

typedef struct iotc_itest_mqttlogic_persistant_session_test_sample_s {
  iotc_mqtt_type_t msg_type;
  iotc_state_t (*api_call)(iotc_context_handle_t xih, const char* topic_name,
                           const char* payload);
} iotc_itest_mqttlogic_persistant_session_test_sample_t;

iotc_state_t iotc_itest_mqttlogic_call_publish(iotc_context_handle_t xih,
                                               const char* topic_name,
                                               const char* payload) {
  return iotc_publish(xih, topic_name, payload, IOTC_MQTT_QOS_AT_LEAST_ONCE,
                      NULL, NULL);
}

void iotc_itest_mqttlogic_subscribe_callback(
    iotc_context_handle_t in_context_handle, iotc_sub_call_type_t call_type,
    const iotc_sub_call_params_t* const params, iotc_state_t state,
    void* user_data) {
  IOTC_UNUSED(in_context_handle);
  IOTC_UNUSED(call_type);
  IOTC_UNUSED(params);
  IOTC_UNUSED(state);
  IOTC_UNUSED(user_data);
}

iotc_state_t iotc_itest_mqttlogic_call_subscribe(iotc_context_handle_t xih,
                                                 const char* topic_name,
                                                 const char* payload) {
  IOTC_UNUSED(payload);
  return iotc_subscribe(xih, topic_name, IOTC_MQTT_QOS_AT_LEAST_ONCE,
                        &iotc_itest_mqttlogic_subscribe_callback, NULL);
}

int iotc_itest_mqttlogic_layer_setup(void** state) {
  IOTC_UNUSED(state);

  iotc_memory_limiter_tearup();

  assert_int_equal(IOTC_STATE_OK, iotc_initialize());

  IOTC_CHECK_STATE(iotc_create_context_with_custom_layers(
      &iotc_context__itest_mqttlogic_layer, iotc_itest_layer_chain_mqttlogic,
      IOTC_LAYER_CHAIN_MQTTLOGIC,
      IOTC_LAYER_CHAIN_SCHEME_LENGTH(IOTC_LAYER_CHAIN_MQTTLOGIC)));

  return 0;

err_handling:
  fail();

  return 1;
}

int iotc_itest_mqttlogic_layer_teardown(void** state) {
  IOTC_UNUSED(state);

  iotc_delete_context_with_custom_layers(
      &iotc_context__itest_mqttlogic_layer, iotc_itest_layer_chain_mqttlogic,
      IOTC_LAYER_CHAIN_SCHEME_LENGTH(IOTC_LAYER_CHAIN_MQTTLOGIC));

  iotc_shutdown();

  return !iotc_memory_limiter_teardown();
}

void iotc_itest_mqttlogic_init_layer(iotc_layer_t* top_layer) {
  // default init process expectations
  expect_value(iotc_mock_layer_mqttlogic_prev_init, in_out_state,
               IOTC_STATE_OK);
  expect_value(iotc_mock_layer_mqttlogic_prev_connect, in_out_state,
               IOTC_STATE_OK);
  // mqtt logic layer pushes the CONNECT message immediately
  expect_value(iotc_mock_layer_mqttlogic_prev_push, in_out_state,
               IOTC_STATE_OK);

  expect_check(iotc_mock_layer_mqttlogic_prev_push, data, check_msg,
               iotc_itest_mqttlogic_make_msg_test_matrix(
                   (iotc_itest_mqttlogic_test_msg_what_to_check_t){
                       .retain = 0, .qos = 0, .dup = 0, .type = 1},
                   (iotc_itest_mqttlogic_test_msg_common_bits_check_values_t){
                       .retain = 0,
                       .qos = 0,
                       .dup = 0,
                       .type = IOTC_MQTT_TYPE_CONNECT}));

  iotc_context__itest_mqttlogic_layer->context_data.connection_data =
      iotc_alloc_connection_data("target.broker.com", 8883, "itest_username",
                                 "itest_password", "itest_client_id",
                                 /*connection_timeout=*/0,
                                 /*keepalive_timeout=*/0, IOTC_SESSION_CLEAN);

  IOTC_PROCESS_INIT_ON_PREV_LAYER(&top_layer->layer_connection, NULL,
                                  IOTC_STATE_OK);

  iotc_evtd_step(iotc_globals.evtd_instance, time(NULL));
}

void iotc_itest_mqttlogic_prepare_init_and_connect_layer(
    iotc_layer_t* top_layer, iotc_session_type_t session_type,
    uint16_t keepalive_timeout) {
  iotc_context__itest_mqttlogic_layer->context_data.shutdown_state =
      IOTC_SHUTDOWN_UNINITIALISED;

  // default init process expectations
  expect_value(iotc_mock_layer_mqttlogic_prev_init, in_out_state,
               IOTC_STATE_OK);
  expect_value(iotc_mock_layer_mqttlogic_prev_connect, in_out_state,
               IOTC_STATE_OK);

  // mqtt logic layer pushes the CONNECT message immediately
  expect_value(iotc_mock_layer_mqttlogic_prev_push, in_out_state,
               IOTC_STATE_OK);

  expect_check(iotc_mock_layer_mqttlogic_prev_push, data, check_msg,
               iotc_itest_mqttlogic_make_msg_test_matrix(
                   (iotc_itest_mqttlogic_test_msg_what_to_check_t){
                       .retain = 0, .qos = 0, .dup = 0, .type = 1},
                   (iotc_itest_mqttlogic_test_msg_common_bits_check_values_t){
                       .retain = 0,
                       .qos = 0,
                       .dup = 0,
                       .type = IOTC_MQTT_TYPE_CONNECT}));

  iotc_context__itest_mqttlogic_layer->context_data.connection_data =
      iotc_alloc_connection_data(
          "target.broker.com", 8883, "itest_username", "itest_password",
          "itest_client_id", /*connection_timeout=*/0, keepalive_timeout,
          session_type);

  IOTC_PROCESS_INIT_ON_PREV_LAYER(&top_layer->layer_connection, NULL,
                                  IOTC_STATE_OK);

  iotc_evtd_step(iotc_globals.evtd_instance, time(NULL));

  /* let's give it back the CONNACK */
  iotc_state_t state = IOTC_STATE_OK;
  IOTC_ALLOC(iotc_mqtt_message_t, connack, state);
  IOTC_CHECK_STATE(fill_with_connack_data(connack, 0));
  IOTC_PROCESS_PULL_ON_PREV_LAYER(&top_layer->layer_connection, connack,
                                  IOTC_STATE_OK);

  expect_value(iotc_mock_layer_mqttlogic_next_connect, in_out_state,
               IOTC_STATE_OK);

err_handling:
  return;
}

void iotc_itest_mqttlogic_shutdown_and_disconnect(
    iotc_context_handle_t context) {
  /* now let's disconnect the layer */
  iotc_shutdown_connection(context);

  /* it should begin the shutdown sequence */
  expect_value(iotc_mock_layer_mqttlogic_next_push, in_out_state,
               IOTC_STATE_OK);
  expect_value(iotc_mock_layer_mqttlogic_prev_push, in_out_state,
               IOTC_STATE_OK);
  expect_value(iotc_mock_layer_mqttlogic_prev_close, in_out_state,
               IOTC_STATE_OK);
  expect_value(iotc_mock_layer_mqttlogic_prev_close_externally, in_out_state,
               IOTC_STATE_OK);
  expect_value(iotc_mock_layer_mqttlogic_next_close_externally, in_out_state,
               IOTC_STATE_OK);

  expect_check(iotc_mock_layer_mqttlogic_prev_push, data, check_msg,
               iotc_itest_mqttlogic_make_msg_test_matrix(
                   (iotc_itest_mqttlogic_test_msg_what_to_check_t){0, 0, 0, 1},
                   (iotc_itest_mqttlogic_test_msg_common_bits_check_values_t){
                       0, 0, 0, IOTC_MQTT_TYPE_DISCONNECT}));

  /* let's process shutdown */
  iotc_evtd_step(iotc_globals.evtd_instance, time(NULL));

  iotc_free_connection_data(
      &iotc_context__itest_mqttlogic_layer->context_data.connection_data);
}

void iotc_itest_mqttlogic_layer_act() {
  size_t loop_counter = 0;
  while (iotc_evtd_dispatcher_continue(iotc_globals.evtd_instance) == 1 &&
         loop_counter < 5) {
    iotc_evtd_step(iotc_globals.evtd_instance, time(NULL) + loop_counter);
    ++loop_counter;
  }
}

/*********************************************************************************
 * test cases
 *********************************************************************
 ********************************************************************************/
void iotc_itest_mqttlogic_layer__backoff_class_error_PUSH__layerchain_closure_is_expected(
    void** state) {
  uint8_t id_error = 0;
  for (; id_error <
         IOTC_ARRAYSIZE(iotc_itest_mqttlogic__backoff_terminal_class_errors);
       ++id_error) {
    const iotc_state_t error_under_test =
        iotc_itest_mqttlogic__backoff_terminal_class_errors[id_error];

    /* __iotc_printf("--- --- --- %s, error = %d\n", __func__, error_under_test
     * ); */

    iotc_layer_t* top_layer =
        iotc_context__itest_mqttlogic_layer->layer_chain.top;

    iotc_itest_mqttlogic_init_layer(top_layer);

    // expected reaction on failure
    expect_value(iotc_mock_layer_mqttlogic_prev_close, in_out_state,
                 IOTC_BACKOFF_TERMINAL);
    expect_value(iotc_mock_layer_mqttlogic_prev_close_externally, in_out_state,
                 IOTC_BACKOFF_TERMINAL);
    // libiotc believes error occurred during connection process thus
    // it returns connect layer function with proper error
    expect_value(iotc_mock_layer_mqttlogic_next_connect, in_out_state,
                 IOTC_BACKOFF_TERMINAL);

    expect_value(iotc_mock_layer_mqttlogic_prev_close, in_out_state,
                 IOTC_BACKOFF_TERMINAL);
    expect_value(iotc_mock_layer_mqttlogic_prev_close_externally, in_out_state,
                 IOTC_BACKOFF_TERMINAL);
    expect_value(iotc_mock_layer_mqttlogic_next_close_externally, in_out_state,
                 IOTC_BACKOFF_TERMINAL);

    // FAILURE HERE: error occurs during PUSH
    IOTC_PROCESS_PUSH_ON_PREV_LAYER(&top_layer->layer_connection, NULL,
                                    error_under_test);

    iotc_itest_mqttlogic_layer_act();

    iotc_itest_mqttlogic_layer_teardown(state);
    iotc_itest_mqttlogic_layer_setup(state);
  }
}

void iotc_itest_mqttlogic_layer__backoff_class_error_PULL__layerchain_closure_is_expected(
    void** state) {
  uint8_t id_error = 0;
  for (; id_error <
         IOTC_ARRAYSIZE(iotc_itest_mqttlogic__backoff_terminal_class_errors);
       ++id_error) {
    const iotc_state_t error_under_test =
        iotc_itest_mqttlogic__backoff_terminal_class_errors[id_error];

    /* __iotc_printf("--- --- --- %s, error = %d\n", __func__, error_under_test
     * ); */

    iotc_layer_t* top_layer =
        iotc_context__itest_mqttlogic_layer->layer_chain.top;

    iotc_itest_mqttlogic_init_layer(top_layer);

    // expected reaction on failure
    expect_value(iotc_mock_layer_mqttlogic_prev_close, in_out_state,
                 IOTC_BACKOFF_TERMINAL);
    expect_value(iotc_mock_layer_mqttlogic_prev_close_externally, in_out_state,
                 IOTC_BACKOFF_TERMINAL);
    // libiotc believes error occurred during connection process thus
    // it returns connect layer function with proper error
    expect_value(iotc_mock_layer_mqttlogic_next_connect, in_out_state,
                 IOTC_BACKOFF_TERMINAL);

    expect_value(iotc_mock_layer_mqttlogic_prev_close, in_out_state,
                 IOTC_BACKOFF_TERMINAL);
    expect_value(iotc_mock_layer_mqttlogic_prev_close_externally, in_out_state,
                 IOTC_BACKOFF_TERMINAL);
    expect_value(iotc_mock_layer_mqttlogic_next_close_externally, in_out_state,
                 IOTC_BACKOFF_TERMINAL);

    // FAILURE HERE: error occurs during PULL
    IOTC_PROCESS_PULL_ON_PREV_LAYER(&top_layer->layer_connection, NULL,
                                    error_under_test);

    iotc_itest_mqttlogic_layer_act();

    iotc_itest_mqttlogic_layer_teardown(state);
    iotc_itest_mqttlogic_layer_setup(state);
  }
}

void iotc_itest_mqtt_logic_layer_sub_callback(
    iotc_context_handle_t in_context_handle, iotc_sub_call_type_t call_type,
    const iotc_sub_call_params_t* const params, iotc_state_t state,
    void* user_data) {
  IOTC_UNUSED(in_context_handle);
  IOTC_UNUSED(call_type);
  IOTC_UNUSED(params);
  IOTC_UNUSED(state);
  IOTC_UNUSED(user_data);

  check_expected(call_type);
  check_expected(state);
}

void iotc_itest_mqtt_logic_layer__subscribe_success__success_suback_callback_invocation(
    void** state) {
  IOTC_UNUSED(state);

  iotc_state_t local_state = IOTC_STATE_OK;
  iotc_mqtt_message_t* suback = NULL;
  iotc_context_handle_t context_handle = IOTC_INVALID_CONTEXT_HANDLE;

  int it_qos_level = 0;
  for (; it_qos_level < IOTC_MQTT_QOS_TEST_LEVELS_COUNT; ++it_qos_level) {
    iotc_debug_format("Testing QoS level: %d", it_qos_level);

    /* initialisation of the layer chain */
    iotc_layer_t* top_layer =
        iotc_context__itest_mqttlogic_layer->layer_chain.top;
    iotc_itest_mqttlogic_prepare_init_and_connect_layer(top_layer,
                                                        IOTC_SESSION_CLEAN, 0);
    iotc_itest_mqttlogic_layer_act();

    /* let's extract the context handle in order to use it against the API
     * functions
     */
    IOTC_CHECK_STATE(local_state = iotc_find_handle_for_object(
                         iotc_globals.context_handles_vector,
                         iotc_context__itest_mqttlogic_layer, &context_handle));

    /* let's use the IoTC library subscribe function */
    iotc_subscribe(context_handle, "test_topic", it_qos_level,
                   &iotc_itest_mqtt_logic_layer_sub_callback, NULL);

    expect_value(iotc_mock_layer_mqttlogic_next_push, in_out_state,
                 IOTC_STATE_OK);
    expect_value(iotc_mock_layer_mqttlogic_prev_push, in_out_state,
                 IOTC_STATE_OK);

    expect_check(iotc_mock_layer_mqttlogic_prev_push, data, check_msg,
                 iotc_itest_mqttlogic_make_msg_test_matrix(
                     (iotc_itest_mqttlogic_test_msg_what_to_check_t){
                         .retain = 0, .qos = 0, .dup = 0, .type = 1},
                     (iotc_itest_mqttlogic_test_msg_common_bits_check_values_t){
                         .retain = 0,
                         .qos = 0,
                         .dup = 0,
                         .type = IOTC_MQTT_TYPE_SUBSCRIBE}));

    /* let's start the subscription process */
    iotc_itest_mqttlogic_layer_act();

    /* let's send suback message */
    IOTC_ALLOC_AT(iotc_mqtt_message_t, suback, local_state);
    suback->common.common_u.common_bits.type = IOTC_MQTT_TYPE_SUBACK;
    suback->suback.message_id = 1;
    IOTC_ALLOC_AT(iotc_mqtt_topicpair_t, suback->suback.topics, local_state);
    suback->suback.topics->iotc_mqtt_topic_pair_payload_u.qos = 0;
    suback->suback.topics->iotc_mqtt_topic_pair_payload_u.status = it_qos_level;
    IOTC_PROCESS_PULL_ON_PREV_LAYER(&top_layer->layer_connection, suback,
                                    IOTC_STATE_OK);

    /* Expectations for the callback */
    expect_value(iotc_itest_mqtt_logic_layer_sub_callback, call_type,
                 IOTC_SUB_CALL_SUBACK);
    expect_value(iotc_itest_mqtt_logic_layer_sub_callback, state,
                 IOTC_MQTT_SUBSCRIPTION_SUCCESSFULL);

    /* let's process suback */
    iotc_itest_mqttlogic_layer_act();

    /* let's close the connection gracefully */
    iotc_itest_mqttlogic_shutdown_and_disconnect(context_handle);
  }

  return;
err_handling:
  iotc_mqtt_message_free(&suback);
  iotc_itest_mqttlogic_shutdown_and_disconnect(context_handle);
}

void iotc_itest_mqtt_logic_layer__subscribe_failure__failed_suback_callback_invocation(
    void** state) {
  IOTC_UNUSED(state);

  iotc_state_t local_state = IOTC_STATE_OK;
  iotc_mqtt_message_t* suback = NULL;
  iotc_context_handle_t context_handle = IOTC_INVALID_CONTEXT_HANDLE;

  int it_qos_level = 0;
  for (; it_qos_level < IOTC_MQTT_QOS_TEST_LEVELS_COUNT; ++it_qos_level) {
    iotc_debug_format("Testing QoS level: %d", it_qos_level);

    /* initialisation of the layer chain */
    iotc_layer_t* top_layer =
        iotc_context__itest_mqttlogic_layer->layer_chain.top;
    iotc_itest_mqttlogic_prepare_init_and_connect_layer(top_layer,
                                                        IOTC_SESSION_CLEAN, 0);
    iotc_itest_mqttlogic_layer_act();

    /* let's extract the context handle in order to use it against the API
     * functions
     */
    IOTC_CHECK_STATE(local_state = iotc_find_handle_for_object(
                         iotc_globals.context_handles_vector,
                         iotc_context__itest_mqttlogic_layer, &context_handle));

    /* let's use the IoTC library subscribe function */
    iotc_subscribe(context_handle, "test_topic", it_qos_level,
                   &iotc_itest_mqtt_logic_layer_sub_callback, NULL);

    expect_value(iotc_mock_layer_mqttlogic_next_push, in_out_state,
                 IOTC_STATE_OK);
    expect_value(iotc_mock_layer_mqttlogic_prev_push, in_out_state,
                 IOTC_STATE_OK);

    expect_check(iotc_mock_layer_mqttlogic_prev_push, data, check_msg,
                 iotc_itest_mqttlogic_make_msg_test_matrix(
                     (iotc_itest_mqttlogic_test_msg_what_to_check_t){
                         .retain = 0, .qos = 0, .dup = 0, .type = 1},
                     (iotc_itest_mqttlogic_test_msg_common_bits_check_values_t){
                         .retain = 0,
                         .qos = 0,
                         .dup = 0,
                         .type = IOTC_MQTT_TYPE_SUBSCRIBE}));

    /* let's start the subscription process */
    iotc_itest_mqttlogic_layer_act();

    /* let's send suback message */
    IOTC_ALLOC_AT(iotc_mqtt_message_t, suback, local_state);
    suback->common.common_u.common_bits.type = IOTC_MQTT_TYPE_SUBACK;
    suback->suback.message_id = 1;
    IOTC_ALLOC_AT(iotc_mqtt_topicpair_t, suback->suback.topics, local_state);
    suback->suback.topics->iotc_mqtt_topic_pair_payload_u.qos = 0;
    suback->suback.topics->iotc_mqtt_topic_pair_payload_u.status =
        IOTC_MQTT_SUBACK_FAILED;
    IOTC_PROCESS_PULL_ON_PREV_LAYER(&top_layer->layer_connection, suback,
                                    IOTC_STATE_OK);

    /* Expectations for the callback */
    expect_value(iotc_itest_mqtt_logic_layer_sub_callback, call_type,
                 IOTC_SUB_CALL_SUBACK);
    expect_value(iotc_itest_mqtt_logic_layer_sub_callback, state,
                 IOTC_MQTT_SUBSCRIPTION_FAILED);

    /* let's process suback */
    iotc_itest_mqttlogic_layer_act();

    /* let's close the connection gracefully */
    iotc_itest_mqttlogic_shutdown_and_disconnect(context_handle);
  }

  return;
err_handling:
  iotc_mqtt_message_free(&suback);
  iotc_itest_mqttlogic_shutdown_and_disconnect(context_handle);
}

void iotc_itest_mqtt_logic_layer__subscribe_success__success_message_callback_invocation(
    void** state) {
  IOTC_UNUSED(state);

  iotc_state_t local_state = IOTC_STATE_OK;
  iotc_mqtt_message_t* suback = NULL;
  iotc_context_handle_t context_handle = IOTC_INVALID_CONTEXT_HANDLE;

  /* prepare msg payload  */
  iotc_data_desc_t* payload = iotc_make_desc_from_string_copy("some_data");
  IOTC_CHECK_MEMORY(payload, local_state);

  /* prepare msg topic */
  const char* const topic = "test_topic";

  int it_qos_level = 0;
  for (; it_qos_level < IOTC_MQTT_QOS_TEST_LEVELS_COUNT; ++it_qos_level) {
    iotc_debug_format("Testing QoS level: %d", it_qos_level);

    /* initialisation of the layer chain */
    iotc_layer_t* top_layer =
        iotc_context__itest_mqttlogic_layer->layer_chain.top;
    iotc_itest_mqttlogic_prepare_init_and_connect_layer(top_layer,
                                                        IOTC_SESSION_CLEAN, 0);
    iotc_itest_mqttlogic_layer_act();

    /* let's extract the context handle in order to use it against the API
     * functions
     */
    IOTC_CHECK_STATE(local_state = iotc_find_handle_for_object(
                         iotc_globals.context_handles_vector,
                         iotc_context__itest_mqttlogic_layer, &context_handle));

    /* let's use the IoTC library subscribe function */
    iotc_subscribe(context_handle, topic, it_qos_level,
                   &iotc_itest_mqtt_logic_layer_sub_callback, NULL);

    expect_value(iotc_mock_layer_mqttlogic_next_push, in_out_state,
                 IOTC_STATE_OK);
    expect_value(iotc_mock_layer_mqttlogic_prev_push, in_out_state,
                 IOTC_STATE_OK);

    /* let's start the subscription process */
    iotc_itest_mqttlogic_layer_act();

    /* let's send suback message */
    IOTC_ALLOC_AT(iotc_mqtt_message_t, suback, local_state);
    suback->common.common_u.common_bits.type = IOTC_MQTT_TYPE_SUBACK;
    suback->suback.message_id = 1;
    IOTC_ALLOC_AT(iotc_mqtt_topicpair_t, suback->suback.topics, local_state);
    suback->suback.topics->iotc_mqtt_topic_pair_payload_u.qos = 0;
    suback->suback.topics->iotc_mqtt_topic_pair_payload_u.status = it_qos_level;
    IOTC_PROCESS_PULL_ON_PREV_LAYER(&top_layer->layer_connection, suback,
                                    IOTC_STATE_OK);

    /* Expectations for the callback */
    expect_value(iotc_itest_mqtt_logic_layer_sub_callback, call_type,
                 IOTC_SUB_CALL_SUBACK);
    expect_value(iotc_itest_mqtt_logic_layer_sub_callback, state,
                 IOTC_MQTT_SUBSCRIPTION_SUCCESSFULL);

    /* let's process suback */
    iotc_itest_mqttlogic_layer_act();

    /* let's pretend to send a message */
    IOTC_ALLOC(iotc_mqtt_message_t, publish, local_state);

    local_state = fill_with_publish_data(
        publish, topic, payload, (iotc_mqtt_qos_t)it_qos_level,
        IOTC_MQTT_RETAIN_FALSE, IOTC_MQTT_DUP_FALSE, 3);

    IOTC_CHECK_STATE(local_state);

    IOTC_PROCESS_PULL_ON_PREV_LAYER(&top_layer->layer_connection, publish,
                                    IOTC_STATE_OK);

    expect_value(iotc_itest_mqtt_logic_layer_sub_callback, call_type,
                 IOTC_SUB_CALL_MESSAGE);
    expect_value(iotc_itest_mqtt_logic_layer_sub_callback, state,
                 IOTC_STATE_OK);

    if (IOTC_MQTT_QOS_1 == it_qos_level) {
      expect_value(iotc_mock_layer_mqttlogic_prev_push, in_out_state,
                   IOTC_STATE_OK);
    }

    iotc_itest_mqttlogic_layer_act();

    /* let's close the connection gracefully */
    iotc_itest_mqttlogic_shutdown_and_disconnect(context_handle);
  }

  iotc_free_desc(&payload);

  return;
err_handling:
  iotc_free_desc(&payload);
  iotc_mqtt_message_free(&suback);
  iotc_itest_mqttlogic_shutdown_and_disconnect(context_handle);
}
