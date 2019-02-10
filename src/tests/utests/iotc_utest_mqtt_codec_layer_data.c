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
#include "iotc_data_desc.h"
#include "iotc_err.h"
#include "iotc_helpers.h"
#include "iotc_mqtt_codec_layer_data.h"
#include "iotc_mqtt_logic_layer_data_helpers.h"

#include "iotc_memory_checks.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifndef IOTC_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN

#endif

IOTC_TT_TESTGROUP_BEGIN(utest_mqtt_codec_layer_data)

IOTC_TT_TESTCASE(
    utest__iotc_mqtt_codec_layer_make_task__valid_data__new_task_created, {
      iotc_state_t state = IOTC_STATE_OK;

      iotc_mqtt_message_t* msg = NULL;

      IOTC_ALLOC_AT(iotc_mqtt_message_t, msg, state);

      state = fill_with_pingreq_data(msg);

      IOTC_CHECK_STATE(state);

      iotc_mqtt_codec_layer_task_t* task = iotc_mqtt_codec_layer_make_task(msg);

      tt_ptr_op(NULL, !=, task);
      tt_ptr_op(task->msg, ==, msg);

      iotc_mqtt_codec_layer_free_task(&task);

      tt_ptr_op(NULL, ==, task);

      tt_int_op(iotc_is_whole_memory_deallocated(), >, 0);

      return;
    err_handling:
      tt_fail();
    end:;
    })

IOTC_TT_TESTCASE(
    utest__iotc_mqtt_codec_layer_activate_task__valid_data__msg_zeroed, {
      iotc_state_t state = IOTC_STATE_OK;

      iotc_mqtt_message_t* msg = NULL;

      IOTC_ALLOC_AT(iotc_mqtt_message_t, msg, state);

      state = fill_with_pingreq_data(msg);

      IOTC_CHECK_STATE(state);

      iotc_mqtt_codec_layer_task_t* task = iotc_mqtt_codec_layer_make_task(msg);

      iotc_mqtt_message_t* detached_msg =
          iotc_mqtt_codec_layer_activate_task(task);

      tt_ptr_op(msg, ==, detached_msg);
      tt_ptr_op(NULL, ==, task->msg);

      iotc_mqtt_codec_layer_free_task(&task);
      iotc_mqtt_message_free(&detached_msg);

      tt_ptr_op(NULL, ==, task);

      tt_int_op(iotc_is_whole_memory_deallocated(), >, 0);

      return;

    err_handling:
      tt_fail();
    end:;
    })

IOTC_TT_TESTCASE(
    utest__iotc_mqtt_codec_layer_continue_task__valid_data__msg_restored, {
      iotc_state_t state = IOTC_STATE_OK;

      iotc_mqtt_message_t* msg = NULL;

      IOTC_ALLOC_AT(iotc_mqtt_message_t, msg, state);

      state = fill_with_pingreq_data(msg);

      IOTC_CHECK_STATE(state);

      iotc_mqtt_codec_layer_task_t* task = iotc_mqtt_codec_layer_make_task(msg);

      iotc_mqtt_message_t* detached_msg =
          iotc_mqtt_codec_layer_activate_task(task);

      tt_ptr_op(msg, ==, detached_msg);
      tt_ptr_op(NULL, ==, task->msg);

      iotc_mqtt_codec_layer_continue_task(task, detached_msg);

      tt_ptr_op(task->msg, ==, detached_msg);

      iotc_mqtt_codec_layer_free_task(&task);

      tt_ptr_op(NULL, ==, task);

      tt_int_op(iotc_is_whole_memory_deallocated(), >, 0);

      return;

    err_handling:
      tt_fail();
    end:;
    })

IOTC_TT_TESTCASE(
    utest__iotc_mqtt_codec_layer_free_task__valid_data__task_released, {
      iotc_state_t state = IOTC_STATE_OK;

      iotc_mqtt_message_t* msg = NULL;

      IOTC_ALLOC_AT(iotc_mqtt_message_t, msg, state);

      state = fill_with_pingreq_data(msg);

      IOTC_CHECK_STATE(state);

      iotc_mqtt_codec_layer_task_t* task = iotc_mqtt_codec_layer_make_task(msg);

      iotc_mqtt_codec_layer_free_task(&task);

      tt_ptr_op(NULL, ==, task);

      tt_int_op(iotc_is_whole_memory_deallocated(), >, 0);

      return;

    err_handling:
      tt_fail();
    end:;
    })

IOTC_TT_TESTCASE(
    utest__iotc_mqtt_codec_layer_free_task__valid_data_no_msg__task_released, {
      iotc_state_t state = IOTC_STATE_OK;

      iotc_mqtt_message_t* msg = NULL;

      IOTC_ALLOC_AT(iotc_mqtt_message_t, msg, state);

      state = fill_with_pingreq_data(msg);

      IOTC_CHECK_STATE(state);

      iotc_mqtt_codec_layer_task_t* task = iotc_mqtt_codec_layer_make_task(msg);

      iotc_mqtt_message_t* detached_msg =
          iotc_mqtt_codec_layer_activate_task(task);

      iotc_mqtt_message_free(&detached_msg);
      iotc_mqtt_codec_layer_free_task(&task);

      tt_ptr_op(NULL, ==, task);

      tt_int_op(iotc_is_whole_memory_deallocated(), >, 0);

      return;

    err_handling:
      tt_fail();
    end:;
    })

IOTC_TT_TESTGROUP_END

#ifndef IOTC_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#define IOTC_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#include __FILE__
#undef IOTC_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#endif
