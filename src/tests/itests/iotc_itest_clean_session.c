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

#include "iotc_itest_clean_session.h"

#include <time.h>

#include <iotc_allocator.h>
#include <iotc_bsp_time.h>
#include <iotc_globals.h>
#include <iotc_helpers.h>
#include <iotc_layer_macros.h>

// dummy io layer
#include <iotc_io_dummy_layer.h>

#include "iotc_handle.h"
#include "iotc_layer_default_functions.h"
#include "iotc_memory_checks.h"

/*-----------------------------------------------------------------------*/
#ifndef IOTC_NO_TLS_LAYER
#define IOTC_DEFAULT_LAYER_CHAIN                     \
  IOTC_LAYER_TYPE_IO                                 \
  , IOTC_LAYER_TYPE_TLS, IOTC_LAYER_TYPE_MQTT_CODEC, \
      IOTC_LAYER_TYPE_MQTT_LOGIC, IOTC_LAYER_TYPE_CONTROL_TOPIC
#else
#define IOTC_DEFAULT_LAYER_CHAIN                            \
  IOTC_LAYER_TYPE_IO                                        \
  , IOTC_LAYER_TYPE_MQTT_CODEC, IOTC_LAYER_TYPE_MQTT_LOGIC, \
      IOTC_LAYER_TYPE_CONTROL_TOPIC
#endif

IOTC_DECLARE_LAYER_CHAIN_SCHEME(IOTC_LAYER_CHAIN_DEFAULT,
                                IOTC_DEFAULT_LAYER_CHAIN);

// some helpers
iotc_mqtt_task_specific_data_t handlers_b[] = {
    {.subscribe = {"test",
                   {IOTC_EVENT_HANDLE_UNSET, .handlers.h0 = {0}, 0},
                   IOTC_MQTT_QOS_AT_LEAST_ONCE}},
    {.subscribe = {"test2",
                   {IOTC_EVENT_HANDLE_UNSET, .handlers.h0 = {0}, 0},
                   IOTC_MQTT_QOS_AT_LEAST_ONCE}},
    {.subscribe = {NULL,
                   {IOTC_EVENT_HANDLE_UNSET, .handlers.h0 = {0}, 0},
                   IOTC_MQTT_QOS_AT_MOST_ONCE}}};

#ifndef IOTC_NO_TLS_LAYER
#include "iotc_tls_layer.h"
#include "iotc_tls_layer_state.h"
#endif
#include "iotc_control_topic_layer.h"

/*-----------------------------------------------------------------------*/
#include "iotc_mqtt_codec_layer.h"
#include "iotc_mqtt_codec_layer_data.h"
#include "iotc_mqtt_logic_layer.h"
#include "iotc_mqtt_logic_layer_data.h"

IOTC_DECLARE_LAYER_TYPES_BEGIN(itest_cyassl_context)
IOTC_LAYER_TYPES_ADD(IOTC_LAYER_TYPE_IO, &iotc_io_dummy_layer_push,
                     &iotc_io_dummy_layer_pull, &iotc_io_dummy_layer_close,
                     &iotc_io_dummy_layer_close_externally,
                     &iotc_io_dummy_layer_init, &iotc_io_dummy_layer_connect,
                     &iotc_layer_default_post_connect)
#ifndef IOTC_NO_TLS_LAYER
, IOTC_LAYER_TYPES_ADD(IOTC_LAYER_TYPE_TLS, &iotc_tls_layer_push,
                       &iotc_tls_layer_pull, &iotc_tls_layer_close,
                       &iotc_tls_layer_close_externally, &iotc_tls_layer_init,
                       &iotc_tls_layer_connect,
                       &iotc_layer_default_post_connect)
#endif
      ,
    IOTC_LAYER_TYPES_ADD(
        IOTC_LAYER_TYPE_MQTT_CODEC, &iotc_mqtt_codec_layer_push,
        &iotc_mqtt_codec_layer_pull, &iotc_mqtt_codec_layer_close,
        &iotc_mqtt_codec_layer_close_externally, &iotc_mqtt_codec_layer_init,
        &iotc_mqtt_codec_layer_connect, &iotc_layer_default_post_connect),
    IOTC_LAYER_TYPES_ADD(
        IOTC_LAYER_TYPE_MQTT_LOGIC, &iotc_mqtt_logic_layer_push,
        &iotc_mqtt_logic_layer_pull, &iotc_mqtt_logic_layer_close,
        &iotc_mqtt_logic_layer_close_externally, &iotc_mqtt_logic_layer_init,
        &iotc_mqtt_logic_layer_connect, &iotc_layer_default_post_connect),
    IOTC_LAYER_TYPES_ADD(IOTC_LAYER_TYPE_CONTROL_TOPIC,
                         &iotc_control_topic_layer_push,
                         &iotc_control_topic_layer_pull,
                         &iotc_control_topic_layer_close,
                         &iotc_control_topic_layer_close_externally,
                         &iotc_control_topic_layer_init,
                         &iotc_control_topic_layer_connect,
                         &iotc_layer_default_post_connect)
        IOTC_DECLARE_LAYER_TYPES_END()

            static void iotc_inject_subscribe_handler(
                iotc_vector_t** handler_vector,
                iotc_mqtt_task_specific_data_t* subs) {
  int i = 0;

  if (*handler_vector == NULL) {
    *handler_vector = iotc_vector_create();
  }

  while (subs[i].subscribe.topic != NULL) {
    size_t len = sizeof(subs[i]);
    iotc_mqtt_task_specific_data_t* data = iotc_alloc(len);

    memcpy(data, &subs[i], sizeof(subs[i]));
    data->subscribe.topic = iotc_str_dup(subs[i].subscribe.topic);

    iotc_vector_push(*handler_vector,
                     IOTC_VEC_VALUE_PARAM(IOTC_VEC_VALUE_PTR(data)));

    i++;
  }
  printf("\n----> Done with the inject\n");
}

static iotc_context_t* iotc_context = NULL;
static iotc_context_handle_t iotc_context_handle = IOTC_INVALID_CONTEXT_HANDLE;

int iotc_itest_clean_session_setup(void** state) {
  IOTC_UNUSED(state);

  iotc_memory_limiter_tearup();

  assert_int_equal(IOTC_STATE_OK, iotc_initialize());

  IOTC_CHECK_STATE(iotc_create_context_with_custom_layers(
      &iotc_context, itest_cyassl_context, IOTC_LAYER_CHAIN_DEFAULT,
      IOTC_LAYER_CHAIN_DEFAULTSIZE_SUFFIX));

  iotc_find_handle_for_object(iotc_globals.context_handles_vector, iotc_context,
                              &iotc_context_handle);

  return 0;

err_handling:
  fail();

  return 1;
}

int iotc_itest_clean_session_teardown(void** state) {
  IOTC_UNUSED(state);

  iotc_delete_context_with_custom_layers(
      &iotc_context, itest_cyassl_context,
      IOTC_LAYER_CHAIN_SCHEME_LENGTH(IOTC_LAYER_CHAIN_DEFAULT));
  iotc_shutdown();

  return !iotc_memory_limiter_teardown();
}

iotc_state_t iotc_mockfunction__layerfunction_init(void* context, void* data,
                                                   iotc_state_t in_out_state) {
  IOTC_UNUSED(context);
  IOTC_UNUSED(data);
  IOTC_UNUSED(in_out_state);

#if 0
    printf("in_out_state = %d\n", in_out_state);
    printf("this layer type id   = %d\n", IOTC_THIS_LAYER( context )->layer_type_id);
    printf("this layer user_data = %p\n", IOTC_THIS_LAYER( context )->user_data);
    printf("next layer type id   = %d\n", IOTC_NEXT_LAYER( context )->layer_type_id);
    printf("next layer user_data = %p\n", IOTC_NEXT_LAYER( context )->user_data);
#endif

  // check_expected_ptr(((iotc_mqtt_logic_layer_data_t*)IOTC_PREV_LAYER(context)->user_data)->handlers_for_topics);
  check_expected(in_out_state);

  const iotc_mqtt_logic_layer_data_t* mqtt_logic_layer_user_data =
      (iotc_mqtt_logic_layer_data_t*)IOTC_NEXT_LAYER(context)->user_data;
  check_expected(mqtt_logic_layer_user_data);

  if (mqtt_logic_layer_user_data != NULL) {
    /* printf("_init next layer handlers_for_topics = %p, size = %d\n",
        mqtt_logic_layer_user_data->handlers_for_topics,
        mqtt_logic_layer_user_data->handlers_for_topics->elem_no); */

    const iotc_vector_t* handlers_for_topics =
        mqtt_logic_layer_user_data->handlers_for_topics;

    check_expected(handlers_for_topics);
    check_expected(handlers_for_topics->elem_no);
  }

  // extension related to the next phase of processing
  enum PROC_TYPE proc_type = (enum PROC_TYPE)mock();

  if (proc_type == PROC_TYPE_DO) {
    // this is where we are going to pass the desired input to the next
    // layer
    void* next_data = (void*)mock();
    iotc_state_t next_state = (iotc_state_t)mock();

    IOTC_PROCESS_CONNECT_ON_NEXT_LAYER(context, next_data, next_state);
  }

  return IOTC_STATE_OK;
}

iotc_state_t iotc_mockfunction__layerfunction_close(void* context, void* data,
                                                    iotc_state_t in_out_state) {
  check_expected(in_out_state);

  const iotc_mqtt_logic_layer_data_t* mqtt_logic_layer_user_data =
      (iotc_mqtt_logic_layer_data_t*)IOTC_NEXT_LAYER(context)->user_data;
  check_expected(mqtt_logic_layer_user_data);

  if (mqtt_logic_layer_user_data != NULL) {
    /* printf("_close next layer handlers_for_topics = %p, size = %d\n",
        mqtt_logic_layer_user_data->handlers_for_topics,
        mqtt_logic_layer_user_data->handlers_for_topics->elem_no); */

    const iotc_vector_t* handlers_for_topics =
        mqtt_logic_layer_user_data->handlers_for_topics;

    check_expected(handlers_for_topics);
    check_expected(handlers_for_topics->elem_no);
  }

  return IOTC_PROCESS_CLOSE_EXTERNALLY_ON_THIS_LAYER(context, data,
                                                     in_out_state);
}

static void iotc_itest_clean_session_arrange(int inject_subscribe_handlers) {
  iotc_itest_inject_wraps(iotc_context, IOTC_LAYER_TYPE_MQTT_CODEC, NULL, NULL,
                          iotc_mockfunction__layerfunction_close, NULL,
                          iotc_mockfunction__layerfunction_init, NULL);

  if (inject_subscribe_handlers > 0) {
    iotc_inject_subscribe_handler(
        &iotc_context->context_data.copy_of_handlers_for_topics, handlers_b);
  }
}

void clean_session_on_connection_state_changed(
    iotc_context_handle_t in_context_handle, void* data, iotc_state_t state) {
  IOTC_UNUSED(in_context_handle);
  IOTC_UNUSED(data);
  IOTC_UNUSED(state);
}

static void iotc_itest_clean_session_act() {
  iotc_connect(iotc_context_handle, "itest_username", "itest_password",
               "itest_client_id", /*connection_timeout=*/20,
               /*keepalive_timeout=*/20,
               &clean_session_on_connection_state_changed);

  iotc_evtd_step(iotc_context->context_data.evtd_instance,
                 iotc_bsp_time_getcurrenttime_seconds() + 1);

  IOTC_PROCESS_CLOSE_EXTERNALLY_ON_THIS_LAYER(&iotc_context->layer_chain.bottom,
                                              NULL, IOTC_STATE_OK);

  iotc_evtd_step(iotc_context->context_data.evtd_instance,
                 iotc_bsp_time_getcurrenttime_seconds() + 1);

  return;
}

// test cases
void iotc_itest_test_valid_flow__handlers_vector_should_be_empty__init_with_clean_session(
    void** state) {
  IOTC_UNUSED(state);

  iotc_itest_clean_session_arrange(0);

  expect_value(iotc_mockfunction__layerfunction_init, in_out_state,
               IOTC_STATE_OK);
  expect_not_value(iotc_mockfunction__layerfunction_init,
                   mqtt_logic_layer_user_data, NULL);
  expect_not_value(iotc_mockfunction__layerfunction_init, handlers_for_topics,
                   NULL);
  expect_value(iotc_mockfunction__layerfunction_init,
               handlers_for_topics->elem_no, 0);
  will_return(iotc_mockfunction__layerfunction_init, PROC_TYPE_DONT);

  iotc_itest_clean_session_act();
}
