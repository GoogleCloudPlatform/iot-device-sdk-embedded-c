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

#include "iotc_tt_testcase_management.h"
#include "tinytest.h"
#include "tinytest_macros.h"

#include "iotc.h"
#include "iotc_connection_data_internal.h"
#include "iotc_helpers.h"
#include "iotc_memory_checks.h"
#include "iotc_mqtt_logic_layer_data_helpers.h"

#include <errno.h>
#include <stdlib.h>
#include <string.h>

/*-----------------------------------------------------------------------*/
/* HELPER TESTS                                                          */
/*-----------------------------------------------------------------------*/
#ifndef IOTC_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN

void test_helpers_iotc_parse_payload_as_string_parse_empty_payload_help(void) {
  const char topic[] = "test_topic";
  const char content[] = "";
  char* string_payload = NULL;
  iotc_mqtt_message_t* msg = NULL;

  iotc_data_desc_t* message_payload = iotc_make_desc_from_string_copy(content);

  iotc_state_t local_state = IOTC_STATE_OK;
  IOTC_ALLOC_AT(iotc_mqtt_message_t, msg, local_state);

  tt_want_int_op(fill_with_publish_data(
                     msg, topic, message_payload, IOTC_MQTT_QOS_AT_LEAST_ONCE,
                     IOTC_MQTT_RETAIN_FALSE, IOTC_MQTT_DUP_FALSE, 17),
                 ==, IOTC_STATE_OK);

  string_payload = iotc_parse_message_payload_as_string(msg);

  tt_assert(string_payload != NULL);

  tt_want_int_op(strlen(string_payload), ==, 0);

err_handling:
end:
  IOTC_SAFE_FREE(string_payload);
  iotc_free_desc(&message_payload);
  iotc_mqtt_message_free(&msg);
  tt_want_int_op(iotc_is_whole_memory_deallocated(), >, 0);
  ;
}

#endif

IOTC_TT_TESTGROUP_BEGIN(utest_helpers)

IOTC_TT_TESTCASE(test_helpers_iotc_parse_payload_as_string_null_param, {
  char* result = iotc_parse_message_payload_as_string(NULL);

  tt_assert(NULL == result);

end:
  IOTC_SAFE_FREE(result);
  tt_want_int_op(iotc_is_whole_memory_deallocated(), >, 0);
  ;
})

IOTC_TT_TESTCASE(
    test_helpers_iotc_parse_payload_as_string_parse_legitmiate_payload, {
      const char topic[] = "test_topic";
      const char content[] = "0123456789";
      char* string_payload = NULL;
      iotc_mqtt_message_t* msg = NULL;

      iotc_data_desc_t* message_payload =
          iotc_make_desc_from_string_copy(content);

      iotc_state_t local_state = IOTC_STATE_OK;
      IOTC_ALLOC_AT(iotc_mqtt_message_t, msg, local_state);

      tt_want_int_op(fill_with_publish_data(msg, topic, message_payload,
                                            IOTC_MQTT_QOS_AT_LEAST_ONCE,
                                            IOTC_MQTT_RETAIN_FALSE,
                                            IOTC_MQTT_DUP_FALSE, 17),
                     ==, IOTC_STATE_OK);

      string_payload = iotc_parse_message_payload_as_string(msg);

      tt_assert(string_payload != NULL);

      tt_want_int_op(strncmp(content, string_payload, 10), ==, 0);
      tt_want_int_op(strlen(string_payload), ==, 10);

    err_handling:
    end:
      IOTC_SAFE_FREE(string_payload);
      iotc_free_desc(&message_payload);
      iotc_mqtt_message_free(&msg);
      tt_want_int_op(iotc_is_whole_memory_deallocated(), >, 0);
      ;
    })

IOTC_TT_TESTCASE(
    test_helpers_iotc_parse_payload_as_string_parse_empty_payload,
    { test_helpers_iotc_parse_payload_as_string_parse_empty_payload_help(); })


IOTC_TT_TESTGROUP_END

#ifndef IOTC_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#define IOTC_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#include __FILE__
#undef IOTC_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#endif
