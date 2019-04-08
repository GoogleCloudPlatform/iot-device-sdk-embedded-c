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
#include "tinytest.h"
#include "tinytest_macros.h"

#include "iotc.h"
#include "iotc_globals.h"
#include "iotc_helpers.h"
#include "iotc_mqtt_parser.h"

#include "iotc_memory_checks.h"

#include <iotc_error.h>

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifndef IOTC_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN

#endif

IOTC_TT_TESTGROUP_BEGIN(utest_mqtt_parser)

IOTC_TT_TESTCASE(utest__parse_suback_response__valid_data__qos_0_granted, {
  iotc_mqtt_suback_status_t status = (iotc_mqtt_suback_status_t)-1;
  iotc_state_t state = iotc_mqtt_parse_suback_response(&status, 0x00);

  tt_want_int_op(state, ==, IOTC_STATE_OK);
  tt_want_int_op(status, ==, IOTC_MQTT_QOS_0_GRANTED);

  tt_want_int_op(iotc_is_whole_memory_deallocated(), >, 0);
})

IOTC_TT_TESTCASE(utest__parse_suback_response__valid_data__qos_1_granted, {
  iotc_mqtt_suback_status_t status = (iotc_mqtt_suback_status_t)-1;
  iotc_state_t state = iotc_mqtt_parse_suback_response(&status, 0x01);

  tt_want_int_op(state, ==, IOTC_STATE_OK);
  tt_want_int_op(status, ==, IOTC_MQTT_QOS_1_GRANTED);

  tt_want_int_op(iotc_is_whole_memory_deallocated(), >, 0);
})

IOTC_TT_TESTCASE(utest__parse_suback_response__valid_data__qos_2_granted, {
  iotc_mqtt_suback_status_t status = (iotc_mqtt_suback_status_t)-1;
  iotc_state_t state = iotc_mqtt_parse_suback_response(&status, 0x02);

  tt_want_int_op(state, ==, IOTC_STATE_OK);
  tt_want_int_op(status, ==, IOTC_MQTT_QOS_2_GRANTED);

  tt_want_int_op(iotc_is_whole_memory_deallocated(), >, 0);
})

IOTC_TT_TESTCASE(utest__parse_suback_response__valid_data__subscription_failed,
                 {
                   iotc_mqtt_suback_status_t status =
                       (iotc_mqtt_suback_status_t)-1;
                   iotc_state_t state =
                       iotc_mqtt_parse_suback_response(&status, 0x80);

                   tt_want_int_op(state, ==, IOTC_STATE_OK);
                   tt_want_int_op(status, ==, IOTC_MQTT_SUBACK_FAILED);

                   tt_want_int_op(iotc_is_whole_memory_deallocated(), >, 0);
                 })

IOTC_TT_TESTCASE(utest__parse_suback_response__invalid_data__parser_error, {
  iotc_mqtt_suback_status_t status = (iotc_mqtt_suback_status_t)-1;

  uint8_t i = 3;

  for (; i < 0xFF; ++i) {
    if (i == 0x80) {
      continue;
    }

    iotc_state_t state = iotc_mqtt_parse_suback_response(&status, i);

    tt_want_int_op(state, ==, IOTC_MQTT_PARSER_ERROR);
  }

  tt_want_int_op(iotc_is_whole_memory_deallocated(), >, 0);
})

IOTC_TT_TESTGROUP_END

#ifndef IOTC_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#define IOTC_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#include __FILE__
#undef IOTC_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#endif
