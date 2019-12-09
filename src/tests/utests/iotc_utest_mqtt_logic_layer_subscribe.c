/* Copyright 2018-2019 Google LLC
 *
 * This is part of the Google Cloud IoT Device SDK for Embedded C.
 * It is licensed under the BSD 3-Clause license; you may not use this file
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

#include "iotc_tt_testcase_management.h"
#include "iotc_utest_basic_testcase_frame.h"
#include "tinytest.h"
#include "tinytest_macros.h"

#include "iotc.h"
#include "iotc_globals.h"
#include "iotc_handle.h"
#include "iotc_helpers.h"
#include "iotc_memory_checks.h"
#include "iotc_mqtt_logic_layer_data.h"
#include "iotc_mqtt_logic_layer_subscribe_command.h"
#include "iotc_mqtt_message.h"
#include "iotc_user_sub_call_wrapper.h"

#include <iotc_error.h>

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifndef IOTC_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN

static uint8_t global_value_to_test = 0;

static iotc_state_t successful_subscribe_handler(
    iotc_context_handle_t in_context_handle, iotc_sub_call_type_t call_type,
    const iotc_sub_call_params_t* const params, iotc_state_t state,
    void* user_data) {
  IOTC_UNUSED(in_context_handle);
  IOTC_UNUSED(params);
  IOTC_UNUSED(state);
  IOTC_UNUSED(user_data);

  tt_want_int_op(call_type, ==, IOTC_SUB_CALL_SUBACK);
  tt_want_int_op(state, ==, IOTC_MQTT_SUBSCRIPTION_SUCCESSFULL);
  global_value_to_test = 1;

  return IOTC_STATE_OK;
}

static iotc_state_t failed_subscribe_handler(
    iotc_context_handle_t in_context_handle, iotc_sub_call_type_t call_type,
    const iotc_sub_call_params_t* const params, iotc_state_t state,
    void* user_data) {
  IOTC_UNUSED(in_context_handle);
  IOTC_UNUSED(params);
  IOTC_UNUSED(state);
  IOTC_UNUSED(user_data);

  tt_want_int_op(call_type, ==, IOTC_SUB_CALL_SUBACK);
  tt_want_int_op(state, ==, IOTC_MQTT_SUBSCRIPTION_FAILED);
  global_value_to_test = 1;

  return IOTC_STATE_OK;
}

#endif

IOTC_TT_TESTGROUP_BEGIN(utest_mqtt_logic_layer_subscribe)


IOTC_TT_TESTCASE(utest__match_topics__batch_tests, {
  iotc_event_handle_t handle = iotc_make_empty_handle();

  typedef struct {
    char* subscription_topic;
    const char* published_topic;
    const uint8_t expected_result;
  } iotc_match_topics_test_cases_t;

  iotc_match_topics_test_cases_t test_cases[] = {
    // non-wildcard tests
    {"long_topic_name_same_length_1", "long_topic_name_same_length_2", 1},
    {"long_topic_name_same_length_2", "long_topic_name_same_length_1", 1},
    {"long_topic_name", "long_topic_name_different_length", 1},
    {"long_topic_name_different_length", "short", 1},
    {"t1", "t2", 1},
    {"t", "t", 0},
    {"t", "t/", 1},
    {"t/subfolder", "t", 1},

    // wildcard tests
    {"t1/#", "t2", 1},
    {"t/#", "t", 0},
    {"t/#", "t/", 0},
    {"t/#", "t/subfolder", 0},
    {"t1/#", "t2/subfolder", 1},
    {"multi/level/#", "multi/level", 0},
    {"multi/level/#", "multi/level/", 0},
    {"multi/level/#", "multi/level/topic", 0},
    {"multi/level/#", "multi/level/topic/", 0},
    {"multi/level/#", "multi/level/topic/name", 0},
    {"multi/level/#", "multi/level/topic/name/", 0},
    {"multi/level/#", "multi/leve", 1},
    {"multi/level/#", "multi/level2", 1},
    {"multi/level/#", "multi/level2/topic/name", 1},
    {"multi/#", "multi/level/topic/name", 0},

    // NULL cases
    {NULL, "short", 1},
    {"sub", NULL, 1},
    {NULL, NULL, 1},
    {"#", NULL, 1},
    {"#", "", 1},
    {"", "", 1},

    // root wildcard tests
    {"#", "t", 0},
    {"#", "long_topic_name", 0},
    {"#", "multi/level/topic/name", 0},
  };

  uint8_t i = 0;
  for (; i < sizeof(test_cases) / sizeof(iotc_match_topics_test_cases_t); ++i) {
    iotc_match_topics_test_cases_t* test_case = test_cases + i;
    // printf("match publish topics with subscriptions, expected: %s, %s, %s\n",
           // test_case->expected_result ? "MISMATCH" : "MATCH",
           // test_case->subscription_topic,
           // test_case->published_topic);

    const size_t string_len = test_case->published_topic ? strlen(test_case->published_topic) : 0;

    iotc_mqtt_task_specific_data_t spd = {.subscribe = {test_case->subscription_topic, handle, 0}};
    iotc_data_desc_t data = {
        (unsigned char*)test_case->published_topic, NULL, string_len, string_len, string_len,
        IOTC_MEMORY_TYPE_UNMANAGED};

    union iotc_vector_selector_u a = {&spd};
    union iotc_vector_selector_u b = {&data};

    tt_want_int_op(match_topics(&a, &b), ==, test_case->expected_result);
  }
})



IOTC_TT_TESTCASE_WITH_SETUP(
    utest__do_mqtt_subscribe__valid_data__subscription_handler_registered_with_success,
    iotc_utest_setup_basic, iotc_utest_teardown_basic, NULL, {
      iotc_state_t local_state = IOTC_STATE_OK;

      iotc_context_handle_t iotc_context_handle = iotc_create_context();
      if (IOTC_INVALID_CONTEXT_HANDLE >= iotc_context_handle) {
        tt_fail_msg("Failed to create context!");
        return;
      }

      iotc_mqtt_logic_task_t* task = 0;
      iotc_mqtt_message_t* msg = 0;

      // take the pointer to context
      iotc_context_t* iotc_context = iotc_object_for_handle(
          iotc_globals.context_handles_vector, iotc_context_handle);
      tt_assert(NULL != iotc_context);

      // set the task data
      IOTC_ALLOC_AT(iotc_mqtt_logic_task_t, task, local_state);

      task->cs = 118;  // this is very hakish since it depends on the code
      // so most probably this test will fail everytime we change anything in
      // tested function which is not too good at least you know what to check
      // if the test fails

      iotc_evtd_execute_in(
          iotc_globals.evtd_instance,
          iotc_make_handle(&do_mqtt_subscribe, 0, &task, IOTC_STATE_TIMEOUT, 0),
          10, &task->timeout);

      task->data.mqtt_settings.scenario = IOTC_MQTT_SUBSCRIBE;
      task->data.mqtt_settings.qos = IOTC_MQTT_QOS_AT_LEAST_ONCE;

      IOTC_ALLOC_AT(iotc_mqtt_task_specific_data_t, task->data.data_u,
                    local_state);
      iotc_mqtt_task_specific_data_t* data_u = task->data.data_u;

      task->data.data_u->subscribe.handler = iotc_make_threaded_handle(
          IOTC_THREADID_THREAD_0, &iotc_user_sub_call_wrapper, iotc_context,
          NULL, IOTC_STATE_OK, (void*)&successful_subscribe_handler,
          (void*)NULL, (void*)task->data.data_u);

      // set the msg data
      IOTC_ALLOC_AT(iotc_mqtt_message_t, msg, local_state);

      msg->common.common_u.common_bits.type = IOTC_MQTT_TYPE_SUBACK;
      IOTC_ALLOC_AT(iotc_mqtt_topicpair_t, msg->suback.topics, local_state);
      msg->suback.topics->iotc_mqtt_topic_pair_payload_u.status =
          IOTC_MQTT_QOS_0_GRANTED;

      // set the layer data
      iotc_mqtt_logic_layer_data_t logic_layer_data;
      memset(&logic_layer_data, 0, sizeof(iotc_mqtt_logic_layer_data_t));

      logic_layer_data.q12_tasks_queue = task;

      logic_layer_data.handlers_for_topics = iotc_vector_create();

      iotc_layer_t* layer = iotc_context->layer_chain.bottom;
      layer->user_data = &logic_layer_data;

      // data prepared time to call the function
      tt_want_int_op(
          do_mqtt_subscribe(&layer->layer_connection, task, IOTC_STATE_OK, msg),
          ==, IOTC_STATE_OK);
      tt_want_int_op(logic_layer_data.handlers_for_topics->elem_no, ==, 1);
      tt_want_ptr_op(
          logic_layer_data.handlers_for_topics->array[0].selector_t.ptr_value,
          ==, data_u);

      // make the handler to be called
      iotc_evtd_step(iotc_globals.evtd_instance, 20);

      tt_want_int_op(global_value_to_test, ==, 1);
      global_value_to_test = 0;

      IOTC_SAFE_FREE(data_u);
      iotc_vector_del(logic_layer_data.handlers_for_topics, 0);

      logic_layer_data.handlers_for_topics =
          iotc_vector_destroy(logic_layer_data.handlers_for_topics);

      iotc_delete_context(iotc_context_handle);

      return;

    err_handling:
      tt_abort_msg("test should not fail");
    end:
      if (task != 0) iotc_mqtt_logic_free_task(&task);
      iotc_mqtt_message_free(&msg);
      iotc_delete_context(iotc_context_handle);
    })

IOTC_TT_TESTCASE_WITH_SETUP(
    utest__do_mqtt_subscribe__valid_data__subscription_handler_registered_with_failure,
    iotc_utest_setup_basic, iotc_utest_teardown_basic, NULL, {
      iotc_state_t local_state = IOTC_STATE_OK;

      iotc_context_handle_t iotc_context_handle = iotc_create_context();
      if (IOTC_INVALID_CONTEXT_HANDLE >= iotc_context_handle) {
        tt_fail_msg("Failed to create default context!");
        return;
      }

      iotc_mqtt_logic_task_t* task = 0;
      iotc_mqtt_message_t* msg = 0;

      // take the pointer to context
      iotc_context_t* iotc_context = iotc_object_for_handle(
          iotc_globals.context_handles_vector, iotc_context_handle);
      tt_assert(NULL != iotc_context);

      // set the task data
      IOTC_ALLOC_AT(iotc_mqtt_logic_task_t, task, local_state);

      task->cs = 118;  // this is very hakish since it depends on the code
      // so most probably this test will fail everytime we change anything in
      // tested function which is not too good at least you know what to check
      // if the test fails

      iotc_evtd_execute_in(
          iotc_globals.evtd_instance,
          iotc_make_handle(&do_mqtt_subscribe, 0, &task, IOTC_STATE_TIMEOUT, 0),
          10, &task->timeout);

      task->data.mqtt_settings.scenario = IOTC_MQTT_SUBSCRIBE;
      task->data.mqtt_settings.qos = IOTC_MQTT_QOS_AT_LEAST_ONCE;

      IOTC_ALLOC_AT(iotc_mqtt_task_specific_data_t, task->data.data_u,
                    local_state);

      task->data.data_u->subscribe.handler = iotc_make_threaded_handle(
          IOTC_THREADID_THREAD_0, &iotc_user_sub_call_wrapper, iotc_context,
          NULL, IOTC_STATE_OK, (void*)&failed_subscribe_handler, (void*)NULL,
          (void*)task->data.data_u);

      // set the msg data
      IOTC_ALLOC_AT(iotc_mqtt_message_t, msg, local_state);

      msg->common.common_u.common_bits.type = IOTC_MQTT_TYPE_SUBACK;
      IOTC_ALLOC_AT(iotc_mqtt_topicpair_t, msg->suback.topics, local_state);
      msg->suback.topics->iotc_mqtt_topic_pair_payload_u.status =
          IOTC_MQTT_SUBACK_FAILED;

      // set the layer data
      iotc_mqtt_logic_layer_data_t logic_layer_data;
      memset(&logic_layer_data, 0, sizeof(iotc_mqtt_logic_layer_data_t));

      logic_layer_data.q12_tasks_queue = task;

      logic_layer_data.handlers_for_topics = iotc_vector_create();

      iotc_layer_t* layer = iotc_context->layer_chain.bottom;
      layer->user_data = &logic_layer_data;

      // data prepared time to call the function

      tt_want_int_op(
          do_mqtt_subscribe(&layer->layer_connection, task, IOTC_STATE_OK, msg),
          ==, IOTC_STATE_OK);
      tt_want_int_op(logic_layer_data.handlers_for_topics->elem_no, ==, 0);

      // make the handler to be called
      iotc_evtd_step(iotc_globals.evtd_instance, 20);

      tt_want_int_op(global_value_to_test, ==, 1);
      global_value_to_test = 0;

      iotc_delete_context(iotc_context_handle);
      logic_layer_data.handlers_for_topics =
          iotc_vector_destroy(logic_layer_data.handlers_for_topics);

      return;

    err_handling:
      tt_abort_msg("test should not fail");
    end:
      if (task != 0) iotc_mqtt_logic_free_task(&task);
      iotc_mqtt_message_free(&msg);
      iotc_delete_context(iotc_context_handle);
      tt_int_op(iotc_is_whole_memory_deallocated(), >, 0);
    })
IOTC_TT_TESTGROUP_END

#ifndef IOTC_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#define IOTC_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#include __FILE__
#undef IOTC_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#endif
