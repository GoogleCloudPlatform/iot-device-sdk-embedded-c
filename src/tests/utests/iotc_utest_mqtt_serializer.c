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

#include "iotc_tt_testcase_management.h"
#include "tinytest.h"
#include "tinytest_macros.h"

#include "iotc.h"
#include "iotc_err.h"
#include "iotc_globals.h"
#include "iotc_helpers.h"
#include "iotc_mqtt_serialiser.h"

#include "iotc_memory_checks.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifndef IOTC_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN

typedef struct utest_mqtt_size_expectations_s {
  size_t message_buffer_length;
  size_t remaining_bytes_length;
  uint8_t* reference_buffer;
} utest_mqtt_expectations_t;

typedef struct utest_mqtt_test_case_s {
  iotc_mqtt_message_t msg;
  utest_mqtt_expectations_t test_expectations;
} utest_mqtt_test_case_t;

static char topic_name[] =
    "xi/blue/v1/de289e01-cc13-11e4-a698-0a1f2727d969/d/"
    "48d0cf57-cc13-11e4-a698-0a1f2727d969/channel_0/"
    "dummyname";
static char content[] = "4 13 4 00 00 00 012";

// reference buffer generated via node.js mqtt serialiser for above data
static uint8_t reference_message_content[] = {
    0x30, 0x7f, 0x0,  0x6a, 0x78, 0x69, 0x2f, 0x62, 0x6c, 0x75, 0x65, 0x2f,
    0x76, 0x31, 0x2f, 0x64, 0x65, 0x32, 0x38, 0x39, 0x65, 0x30, 0x31, 0x2d,
    0x63, 0x63, 0x31, 0x33, 0x2d, 0x31, 0x31, 0x65, 0x34, 0x2d, 0x61, 0x36,
    0x39, 0x38, 0x2d, 0x30, 0x61, 0x31, 0x66, 0x32, 0x37, 0x32, 0x37, 0x64,
    0x39, 0x36, 0x39, 0x2f, 0x64, 0x2f, 0x34, 0x38, 0x64, 0x30, 0x63, 0x66,
    0x35, 0x37, 0x2d, 0x63, 0x63, 0x31, 0x33, 0x2d, 0x31, 0x31, 0x65, 0x34,
    0x2d, 0x61, 0x36, 0x39, 0x38, 0x2d, 0x30, 0x61, 0x31, 0x66, 0x32, 0x37,
    0x32, 0x37, 0x64, 0x39, 0x36, 0x39, 0x2f, 0x63, 0x68, 0x61, 0x6e, 0x6e,
    0x65, 0x6c, 0x5f, 0x30, 0x2f, 0x64, 0x75, 0x6d, 0x6d, 0x79, 0x6e, 0x61,
    0x6d, 0x65, 0x34, 0x20, 0x31, 0x33, 0x20, 0x34, 0x20, 0x30, 0x30, 0x20,
    0x30, 0x30, 0x20, 0x30, 0x30, 0x20, 0x30, 0x31, 0x32};

#define make_static_desc(data, size) \
  { (uint8_t*)data, NULL, size, size, 0, IOTC_MEMORY_TYPE_UNMANAGED }

iotc_data_desc_t topic_name_desc =
    make_static_desc(topic_name, sizeof(topic_name) - 1);
iotc_data_desc_t content_desc = make_static_desc(content, sizeof(content) - 1);

utest_mqtt_test_case_t array_of_test_case[] = {
    {// test case 0
     {.publish = {.common = {{.common_bits = {0, 0, 0, IOTC_MQTT_TYPE_PUBLISH}},
                             0},
                  &topic_name_desc,
                  0,
                  &content_desc}},
     {129, 127, (uint8_t*)&reference_message_content}}};

void utest__serialize_publish__valid_data_border_case__size_is_correct_impl(
    void) {
  iotc_state_t local_state = IOTC_STATE_OK;
  IOTC_UNUSED(local_state);

  size_t i = 0;
  for (; i < IOTC_ARRAYSIZE(array_of_test_case); ++i) {
    utest_mqtt_test_case_t* currrent_test_case = &array_of_test_case[i];

    //
    iotc_state_t local_state = IOTC_STATE_OK;
    iotc_mqtt_serialiser_t serializer;
    iotc_mqtt_serialiser_init(&serializer);

    //
    iotc_mqtt_message_t* msg = &currrent_test_case->msg;

    size_t message_len, remaining_len, payload_size = 0;
    local_state = iotc_mqtt_serialiser_size(&message_len, &remaining_len,
                                            &payload_size, NULL, msg);

    tt_int_op(message_len, ==,
              currrent_test_case->test_expectations.message_buffer_length);

    tt_int_op(remaining_len, ==,
              currrent_test_case->test_expectations.remaining_bytes_length);

    iotc_data_desc_t* buffer =
        iotc_make_empty_desc_alloc(message_len - payload_size);
    IOTC_CHECK_MEMORY(buffer, local_state);

    iotc_mqtt_serialiser_rc_t rc = iotc_mqtt_serialiser_write(
        NULL, msg, buffer, message_len, remaining_len);

    tt_int_op(buffer->length, ==, message_len - payload_size);
    tt_int_op(rc, ==, IOTC_MQTT_SERIALISER_RC_SUCCESS);
    tt_int_op(memcmp(buffer->data_ptr,
                     currrent_test_case->test_expectations.reference_buffer,
                     buffer->length),
              ==, 0);

    iotc_free_desc(&buffer);
  }

  tt_want_int_op(iotc_is_whole_memory_deallocated(), >, 0);

  return;
end:
err_handling:;
}

#endif

IOTC_TT_TESTGROUP_BEGIN(utest_mqtt_serializer)

IOTC_TT_TESTCASE(
    utest__serialize_publish__valid_data_border_case__size_is_correct, {
      utest__serialize_publish__valid_data_border_case__size_is_correct_impl();
    })

IOTC_TT_TESTGROUP_END

#ifndef IOTC_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#define IOTC_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#include __FILE__
#undef IOTC_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#endif
