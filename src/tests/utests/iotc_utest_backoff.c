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

#include "iotc_tt_testcase_management.h"
#include "iotc_utest_basic_testcase_frame.h"
#include "tinytest.h"
#include "tinytest_macros.h"

#include "iotc.h"
#include "iotc_backoff_lut_config.h"
#include "iotc_backoff_status_api.h"
#include "iotc_err.h"
#include "iotc_globals.h"
#include "iotc_helpers.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifndef IOTC_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN

static const iotc_vector_elem_t iotc_utest_backoff_lut_test_1[] = {
    IOTC_VEC_ELEM(IOTC_VEC_VALUE_UI32(1))};

static const iotc_vector_elem_t iotc_utest_decay_lut_test_1[] = {
    IOTC_VEC_ELEM(IOTC_VEC_VALUE_UI32(123))};

static const iotc_vector_elem_t iotc_utest_backoff_lut_test_2[] = {
    IOTC_VEC_ELEM(IOTC_VEC_VALUE_UI32(2)),
    IOTC_VEC_ELEM(IOTC_VEC_VALUE_UI32(4)),
    IOTC_VEC_ELEM(IOTC_VEC_VALUE_UI32(8)),
    IOTC_VEC_ELEM(IOTC_VEC_VALUE_UI32(16))};

static const iotc_vector_elem_t iotc_utest_decay_lut_test_2[] = {
    IOTC_VEC_ELEM(IOTC_VEC_VALUE_UI32(2)),
    IOTC_VEC_ELEM(IOTC_VEC_VALUE_UI32(4)),
    IOTC_VEC_ELEM(IOTC_VEC_VALUE_UI32(8)),
    IOTC_VEC_ELEM(IOTC_VEC_VALUE_UI32(30))};

typedef struct iotc_utest_backoff_data_test_case_s {
  const iotc_vector_elem_t* backoff_data;
  const iotc_vector_elem_t* decay_data;
  size_t data_len;
} iotc_utest_backoff_data_test_case_t;

static const iotc_utest_backoff_data_test_case_t
    iotc_utest_backoff_test_cases[] = {
        {IOTC_BACKOFF_LUT, IOTC_DECAY_LUT, IOTC_ARRAYSIZE(IOTC_BACKOFF_LUT)},
        {iotc_utest_backoff_lut_test_1, iotc_utest_decay_lut_test_1,
         IOTC_ARRAYSIZE(iotc_utest_backoff_lut_test_1)},
        {iotc_utest_backoff_lut_test_2, iotc_utest_decay_lut_test_2,
         IOTC_ARRAYSIZE(iotc_utest_backoff_lut_test_2)}};

void iotc__utest__reset_backoff_penalty() {
  iotc_globals.backoff_status.backoff_lut_i = 0;
  iotc_cancel_backoff_event();
}

#endif

IOTC_TT_TESTGROUP_BEGIN(utest_backoff)

IOTC_TT_TESTCASE(utest__backoff_and_decay_lut_sizes__no_data__must_be_equal, {
  tt_int_op(IOTC_ARRAYSIZE(IOTC_BACKOFF_LUT), ==,
            IOTC_ARRAYSIZE(IOTC_DECAY_LUT));

  tt_int_op(IOTC_ARRAYSIZE(iotc_utest_backoff_lut_test_1), ==,
            IOTC_ARRAYSIZE(iotc_utest_decay_lut_test_1));

  tt_int_op(IOTC_ARRAYSIZE(iotc_utest_backoff_lut_test_2), ==,
            IOTC_ARRAYSIZE(iotc_utest_decay_lut_test_2));
end:;
})

IOTC_TT_TESTCASE(
    utest__iotc_backoff_configure_using_data__valid_data__must_contain_proper_data,
    {
      size_t itests = IOTC_ARRAYSIZE(iotc_utest_backoff_test_cases);

      size_t i = 0;
      for (; i < itests; ++i) {
        const iotc_utest_backoff_data_test_case_t* curr_test_case =
            (iotc_utest_backoff_data_test_case_t*)&iotc_utest_backoff_test_cases
                [i];

        iotc_backoff_configure_using_data(
            (iotc_vector_elem_t*)curr_test_case->backoff_data,
            (iotc_vector_elem_t*)curr_test_case->decay_data,
            curr_test_case->data_len, IOTC_MEMORY_TYPE_UNMANAGED);

        tt_ptr_op(iotc_globals.backoff_status.backoff_lut->array, !=, NULL);
        tt_ptr_op(iotc_globals.backoff_status.backoff_lut->array, ==,
                  curr_test_case->backoff_data);

        tt_ptr_op(iotc_globals.backoff_status.decay_lut->array, !=, NULL);
        tt_ptr_op(iotc_globals.backoff_status.decay_lut->array, ==,
                  curr_test_case->decay_data);
      }

    end:
      iotc_backoff_release();
    })

IOTC_TT_TESTCASE_WITH_SETUP(
    utest__iotc_inc_backoff_penalty__no_data__use_proper_progression,
    iotc_utest_setup_basic, iotc_utest_teardown_basic, NULL, {
      iotc_context_handle_t iotc_context_handle = iotc_create_context();
      if (IOTC_INVALID_CONTEXT_HANDLE >= iotc_context_handle) {
        tt_fail_msg("Failed to create default context!");
        return;
      }

      size_t itests = IOTC_ARRAYSIZE(iotc_utest_backoff_test_cases);

      size_t i = 0;
      for (; i < itests; ++i) {
        const iotc_utest_backoff_data_test_case_t* curr_test_case =
            (iotc_utest_backoff_data_test_case_t*)&iotc_utest_backoff_test_cases
                [i];

        iotc_backoff_configure_using_data(
            (iotc_vector_elem_t*)curr_test_case->backoff_data,
            (iotc_vector_elem_t*)curr_test_case->decay_data,
            curr_test_case->data_len, IOTC_MEMORY_TYPE_UNMANAGED);

        tt_want_int_op(iotc_globals.backoff_status.next_update.ptr_to_position,
                       ==, 0);
        iotc_globals.backoff_status.backoff_lut_i = 0;

        int i = 0;
        for (; i < iotc_globals.backoff_status.backoff_lut->elem_no - 1; ++i) {
          uint32_t prev_backoff_lut_i =
              iotc_globals.backoff_status.backoff_lut_i;
          iotc_inc_backoff_penalty();

          if (curr_test_case->data_len > 1) {
            tt_want_int_op(iotc_globals.backoff_status.backoff_lut_i, >, 0);
          }

          tt_want_int_op(prev_backoff_lut_i, <,
                         iotc_globals.backoff_status.backoff_lut_i);
        }

        uint32_t prev_backoff_lut_i = iotc_globals.backoff_status.backoff_lut_i;

        iotc_inc_backoff_penalty();

        if (curr_test_case->data_len > 1) {
          tt_want_int_op(iotc_globals.backoff_status.backoff_lut_i, >, 0);
        }

        tt_want_int_op(prev_backoff_lut_i, ==,
                       iotc_globals.backoff_status.backoff_lut_i);

        iotc_globals.backoff_status.next_update.ptr_to_position = 0;
      }

      iotc_delete_context(iotc_context_handle);
      iotc_backoff_release();
    })

IOTC_TT_TESTCASE_WITH_SETUP(
    utest__iotc_dec_backoff_penalty__no_data__use_proper_equation,
    iotc_utest_setup_basic, iotc_utest_teardown_basic, NULL, {
      iotc_context_handle_t iotc_context_handle = iotc_create_context();
      if (IOTC_INVALID_CONTEXT_HANDLE >= iotc_context_handle) {
        tt_fail_msg("Failed to create default context!");
        return;
      }

      size_t itests = IOTC_ARRAYSIZE(iotc_utest_backoff_test_cases);

      size_t i = 0;
      for (; i < itests; ++i) {
        const iotc_utest_backoff_data_test_case_t* curr_test_case =
            (iotc_utest_backoff_data_test_case_t*)&iotc_utest_backoff_test_cases
                [i];

        iotc_backoff_configure_using_data(
            (iotc_vector_elem_t*)curr_test_case->backoff_data,
            (iotc_vector_elem_t*)curr_test_case->decay_data,
            curr_test_case->data_len, IOTC_MEMORY_TYPE_UNMANAGED);

        size_t last_index = IOTC_ARRAYSIZE(iotc_utest_backoff_lut_test_2) - 1;
        iotc_globals.backoff_status.backoff_lut_i = last_index;

        int backoff_i = last_index;

        for (; backoff_i > 0; --backoff_i) {
          uint32_t prev_backoff_i = iotc_globals.backoff_status.backoff_lut_i;

          iotc_dec_backoff_penalty();

          tt_want_int_op(iotc_globals.backoff_status.backoff_lut_i, ==,
                         prev_backoff_i - 1);
        }

        tt_want_int_op(iotc_globals.backoff_status.backoff_lut_i, ==, 0);
      }

      iotc_delete_context(iotc_context_handle);
      iotc_backoff_release();
    })

IOTC_TT_TESTCASE_WITH_SETUP(
    utest__iotc_get_backoff_penalty__check_return_value_random_range,
    iotc_utest_setup_basic, iotc_utest_teardown_basic, NULL, {
      iotc_context_handle_t iotc_context_handle = iotc_create_context();
      if (IOTC_INVALID_CONTEXT_HANDLE >= iotc_context_handle) {
        tt_fail_msg("Failed to create default context!");
        return;
      }

      size_t itests = IOTC_ARRAYSIZE(iotc_utest_backoff_test_cases);

      size_t i = 0;
      for (; i < itests; ++i) {
        const iotc_utest_backoff_data_test_case_t* curr_test_case =
            (iotc_utest_backoff_data_test_case_t*)&iotc_utest_backoff_test_cases
                [i];

        iotc_backoff_configure_using_data(
            (iotc_vector_elem_t*)curr_test_case->backoff_data,
            (iotc_vector_elem_t*)curr_test_case->decay_data,
            curr_test_case->data_len, IOTC_MEMORY_TYPE_UNMANAGED);

        size_t j = 0;
        for (; j < curr_test_case->data_len; ++j) {
          iotc_globals.backoff_status.backoff_lut_i = j;

          const iotc_backoff_lut_index_t lower_index =
              IOTC_MAX(iotc_globals.backoff_status.backoff_lut_i - 1, 0);

          const int32_t orig_value =
              iotc_globals.backoff_status.backoff_lut
                  ->array[iotc_globals.backoff_status.backoff_lut_i]
                  .selector_t.ui32_value;

          const uint32_t rand_range =
              iotc_globals.backoff_status.backoff_lut->array[lower_index]
                  .selector_t.ui32_value;

          const int32_t half_range = IOTC_MAX(rand_range / 2, 1);

          int jj = 0;
          for (; jj < 1000; ++jj) {
            const int32_t penalty = iotc_get_backoff_penalty();

            tt_int_op(penalty, <=, orig_value + half_range);
            tt_int_op(penalty, >=, orig_value - half_range);
          }
        }
      }

    end:
      iotc_delete_context(iotc_context_handle);
      iotc_backoff_release();
    })

IOTC_TT_TESTCASE_WITH_SETUP(
    utest__iotc_cancel_backoff_event__no_data__backoff_status_clean,
    iotc_utest_setup_basic, iotc_utest_teardown_basic, NULL, {
      iotc_context_handle_t iotc_context_handle = iotc_create_context();
      if (IOTC_INVALID_CONTEXT_HANDLE >= iotc_context_handle) {
        tt_fail_msg("Failed to create default context!");
        return;
      }

      size_t itests = IOTC_ARRAYSIZE(iotc_utest_backoff_test_cases);

      size_t i = 0;
      for (; i < itests; ++i) {
        const iotc_utest_backoff_data_test_case_t* curr_test_case =
            (iotc_utest_backoff_data_test_case_t*)&iotc_utest_backoff_test_cases
                [i];

        iotc_backoff_configure_using_data(
            (iotc_vector_elem_t*)curr_test_case->backoff_data,
            (iotc_vector_elem_t*)curr_test_case->decay_data,
            curr_test_case->data_len, IOTC_MEMORY_TYPE_UNMANAGED);

        iotc_globals.backoff_status.backoff_lut_i = 5;

        iotc_inc_backoff_penalty();

        iotc_cancel_backoff_event();

        tt_want_ptr_op(iotc_globals.backoff_status.next_update.ptr_to_position,
                       ==, 0);
      }

      iotc_delete_context(iotc_context_handle);
      iotc_backoff_release();
    })

IOTC_TT_TESTCASE(
    utest__iotc_backoff_classify_state__iotc_state_for_none__backoff_class_none,
    {
      iotc_state_t states[] = {IOTC_STATE_OK, IOTC_STATE_WRITTEN};

      size_t i = 0;
      for (; i < IOTC_ARRAYSIZE(states); ++i) {
        tt_want_int_op(iotc_backoff_classify_state(states[i]), ==,
                       IOTC_BACKOFF_CLASS_NONE);
      }
    })

IOTC_TT_TESTCASE(
    utest__iotc_backoff_classify_state__iotc_state_for_terminal__backoff_class_terminal,
    {
      iotc_state_t states[] = {IOTC_CONNECTION_RESET_BY_PEER_ERROR,
                               IOTC_MQTT_UNACCEPTABLE_PROTOCOL_VERSION,
                               IOTC_MQTT_IDENTIFIER_REJECTED,
                               IOTC_MQTT_BAD_USERNAME_OR_PASSWORD,
                               IOTC_MQTT_NOT_AUTHORIZED};

      size_t i = 0;
      for (; i < IOTC_ARRAYSIZE(states); ++i) {
        tt_want_int_op(iotc_backoff_classify_state(states[i]), ==,
                       IOTC_BACKOFF_CLASS_TERMINAL);
      }
    })

IOTC_TT_TESTCASE(
    utest__iotc_backoff_classify_state__iotc_state_for_recoverable__backoff_class_recoverable,
    {
      iotc_state_t states[] = {IOTC_MQTT_MESSAGE_CLASS_UNKNOWN_ERROR,
                               IOTC_TLS_CONNECT_ERROR,
                               IOTC_TLS_INITALIZATION_ERROR,
                               IOTC_STATE_TIMEOUT,
                               IOTC_SOCKET_WRITE_ERROR,
                               IOTC_SOCKET_READ_ERROR};

      unsigned long i = 0;
      for (; i < IOTC_ARRAYSIZE(states); ++i) {
        tt_want_int_op(iotc_backoff_classify_state(states[i]), ==,
                       IOTC_BACKOFF_CLASS_RECOVERABLE);
      }
    })

IOTC_TT_TESTCASE_WITH_SETUP(
    utest__iotc_update_backoff_penalty__iotc_state_terminal__increase_penalty,
    iotc_utest_setup_basic, iotc_utest_teardown_basic, NULL, {
      iotc_context_handle_t iotc_context_handle = iotc_create_context();
      if (IOTC_INVALID_CONTEXT_HANDLE >= iotc_context_handle) {
        tt_fail_msg("Failed to create default context!");
        return;
      }

      size_t itests = IOTC_ARRAYSIZE(iotc_utest_backoff_test_cases);

      size_t i = 0;
      for (; i < itests; ++i) {
        const iotc_utest_backoff_data_test_case_t* curr_test_case =
            (iotc_utest_backoff_data_test_case_t*)&iotc_utest_backoff_test_cases
                [i];

        iotc_backoff_configure_using_data(
            (iotc_vector_elem_t*)curr_test_case->backoff_data,
            (iotc_vector_elem_t*)curr_test_case->decay_data,
            curr_test_case->data_len, IOTC_MEMORY_TYPE_UNMANAGED);

        iotc_backoff_lut_index_t curr_index =
            iotc_globals.backoff_status.backoff_lut_i;

        iotc_update_backoff_penalty(IOTC_MQTT_BAD_USERNAME_OR_PASSWORD);

        tt_want_int_op(iotc_globals.backoff_status.backoff_class, ==,
                       IOTC_BACKOFF_CLASS_TERMINAL);

        if (curr_test_case->data_len > 1) {
          tt_want_int_op(iotc_globals.backoff_status.backoff_lut_i, >,
                         curr_index);
        } else {
          tt_want_int_op(iotc_globals.backoff_status.backoff_lut_i, ==,
                         curr_index);
          tt_want_int_op(iotc_globals.backoff_status.backoff_lut_i, ==, 0);
        }

        tt_want_ptr_op(iotc_globals.backoff_status.next_update.ptr_to_position,
                       !=, 0);

        iotc__utest__reset_backoff_penalty();
      }

      iotc_delete_context(iotc_context_handle);
      iotc_backoff_release();
    })

IOTC_TT_TESTCASE_WITH_SETUP(
    utest__iotc_update_backoff_penalty__iotc_state_recoverable__increase_penalty,
    iotc_utest_setup_basic, iotc_utest_teardown_basic, NULL, {
      iotc_context_handle_t iotc_context_handle = iotc_create_context();
      if (IOTC_INVALID_CONTEXT_HANDLE >= iotc_context_handle) {
        tt_fail_msg("Failed to create default context!");
        return;
      }

      size_t itests = IOTC_ARRAYSIZE(iotc_utest_backoff_test_cases);

      size_t i = 0;
      for (; i < itests; ++i) {
        const iotc_utest_backoff_data_test_case_t* curr_test_case =
            (iotc_utest_backoff_data_test_case_t*)&iotc_utest_backoff_test_cases
                [i];

        iotc_backoff_configure_using_data(
            (iotc_vector_elem_t*)curr_test_case->backoff_data,
            (iotc_vector_elem_t*)curr_test_case->decay_data,
            curr_test_case->data_len, IOTC_MEMORY_TYPE_UNMANAGED);

        iotc_backoff_lut_index_t curr_index =
            iotc_globals.backoff_status.backoff_lut_i;

        iotc_update_backoff_penalty(IOTC_SOCKET_READ_ERROR);

        tt_want_int_op(iotc_globals.backoff_status.backoff_class, ==,
                       IOTC_BACKOFF_CLASS_RECOVERABLE);

        if (curr_test_case->data_len > 1) {
          tt_want_int_op(iotc_globals.backoff_status.backoff_lut_i, >,
                         curr_index);
        } else {
          tt_want_int_op(iotc_globals.backoff_status.backoff_lut_i, ==,
                         curr_index);
          tt_want_int_op(iotc_globals.backoff_status.backoff_lut_i, ==, 0);
        }

        tt_want_ptr_op(iotc_globals.backoff_status.next_update.ptr_to_position,
                       !=, 0);

        iotc__utest__reset_backoff_penalty();
      }

      iotc_delete_context(iotc_context_handle);
      iotc_backoff_release();
    })

IOTC_TT_TESTCASE_WITH_SETUP(
    utest__iotc_update_backoff_penalty__iotc_state_none__none_penalty,
    iotc_utest_setup_basic, iotc_utest_teardown_basic, NULL, {
      iotc_context_handle_t iotc_context_handle = iotc_create_context();
      if (IOTC_INVALID_CONTEXT_HANDLE >= iotc_context_handle) {
        tt_fail_msg("Failed to create default context!");
        return;
      }

      size_t itests = IOTC_ARRAYSIZE(iotc_utest_backoff_test_cases);

      size_t i = 0;
      for (; i < itests; ++i) {
        const iotc_utest_backoff_data_test_case_t* curr_test_case =
            (iotc_utest_backoff_data_test_case_t*)&iotc_utest_backoff_test_cases
                [i];

        iotc_backoff_configure_using_data(
            (iotc_vector_elem_t*)curr_test_case->backoff_data,
            (iotc_vector_elem_t*)curr_test_case->decay_data,
            curr_test_case->data_len, IOTC_MEMORY_TYPE_UNMANAGED);

        iotc_backoff_lut_index_t curr_index =
            iotc_globals.backoff_status.backoff_lut_i;

        iotc_update_backoff_penalty(IOTC_STATE_OK);

        tt_want_int_op(iotc_globals.backoff_status.backoff_class, ==,
                       IOTC_BACKOFF_CLASS_NONE);
        tt_want_int_op(iotc_globals.backoff_status.backoff_lut_i, ==,
                       curr_index);
        tt_want_ptr_op(iotc_globals.backoff_status.next_update.ptr_to_position,
                       ==, 0);

        iotc__utest__reset_backoff_penalty();
      }

      iotc_delete_context(iotc_context_handle);
      iotc_backoff_release();
    })

IOTC_TT_TESTCASE_WITH_SETUP(
    utest__iotc_restart_update_time__no_data__update_event_registered,
    iotc_utest_setup_basic, iotc_utest_teardown_basic, NULL, {
      iotc_context_handle_t iotc_context_handle = iotc_create_context();
      if (IOTC_INVALID_CONTEXT_HANDLE >= iotc_context_handle) {
        tt_fail_msg("Failed to create default context!");
        return;
      }

      iotc_evtd_instance_t* event_dispatcher = iotc_globals.evtd_instance;

      size_t itests = IOTC_ARRAYSIZE(iotc_utest_backoff_test_cases);

      size_t i = 0;
      for (; i < itests; ++i) {
        const iotc_utest_backoff_data_test_case_t* curr_test_case =
            (iotc_utest_backoff_data_test_case_t*)&iotc_utest_backoff_test_cases
                [i];

        iotc_backoff_configure_using_data(
            (iotc_vector_elem_t*)curr_test_case->backoff_data,
            (iotc_vector_elem_t*)curr_test_case->decay_data,
            curr_test_case->data_len, IOTC_MEMORY_TYPE_UNMANAGED);

        tt_want_ptr_op(iotc_globals.backoff_status.next_update.ptr_to_position,
                       ==, 0);

        iotc_restart_update_time(event_dispatcher);

        tt_want_ptr_op(iotc_globals.backoff_status.next_update.ptr_to_position,
                       !=, 0);

        iotc_time_event_handle_t* backoff_handler =
            &iotc_globals.backoff_status.next_update;

        iotc_restart_update_time(event_dispatcher);

        tt_want_ptr_op(iotc_globals.backoff_status.next_update.ptr_to_position,
                       ==, backoff_handler->ptr_to_position);
        tt_want_ptr_op(iotc_globals.backoff_status.next_update.ptr_to_position,
                       !=, NULL);

        iotc__utest__reset_backoff_penalty();
      }

      iotc_delete_context(iotc_context_handle);
      iotc_backoff_release();
    })

IOTC_TT_TESTCASE_WITH_SETUP(
    utest__iotc_update_backoff_penalty_time__no_data__update_update_time_and_no_backoff_penalty,
    iotc_utest_setup_basic, iotc_utest_teardown_basic, NULL, {
      iotc_context_handle_t iotc_context_handle = iotc_create_context();
      if (IOTC_INVALID_CONTEXT_HANDLE >= iotc_context_handle) {
        tt_fail_msg("Failed to create default context!");
        return;
      }

      iotc_evtd_instance_t* event_dispatcher = iotc_globals.evtd_instance;
      tt_int_op(iotc_globals.backoff_status.backoff_lut_i, ==, 0);

      size_t itests = IOTC_ARRAYSIZE(iotc_utest_backoff_test_cases);

      size_t i = 0;
      for (; i < itests; ++i) {
        const iotc_utest_backoff_data_test_case_t* curr_test_case =
            (iotc_utest_backoff_data_test_case_t*)&iotc_utest_backoff_test_cases
                [i];

        iotc_backoff_configure_using_data(
            (iotc_vector_elem_t*)curr_test_case->backoff_data,
            (iotc_vector_elem_t*)curr_test_case->decay_data,
            curr_test_case->data_len, IOTC_MEMORY_TYPE_UNMANAGED);

        iotc_inc_backoff_penalty();

        iotc_globals.backoff_status.backoff_class = IOTC_BACKOFF_CLASS_TERMINAL;

        iotc_vector_index_type_t* backoff_time_event_position =
            iotc_globals.backoff_status.next_update.ptr_to_position;

        iotc_backoff_lut_index_t curr_index =
            iotc_globals.backoff_status.backoff_lut_i;

        iotc_evtd_step(
            event_dispatcher,
            event_dispatcher->current_step +
                iotc_globals.backoff_status.decay_lut->array[curr_index]
                    .selector_t.ui32_value +
                1);

        tt_int_op(iotc_globals.backoff_status.backoff_lut_i, ==, curr_index);

        if (curr_test_case->data_len > 1) {
          tt_ptr_op(iotc_globals.backoff_status.next_update.ptr_to_position, !=,
                    NULL);
        } else {
          tt_ptr_op(iotc_globals.backoff_status.next_update.ptr_to_position, ==,
                    NULL);
        }

        tt_ptr_op(&iotc_globals.backoff_status.next_update.ptr_to_position, !=,
                  backoff_time_event_position);

        iotc__utest__reset_backoff_penalty();
      }

    end:
      iotc__utest__reset_backoff_penalty();
      iotc_delete_context(iotc_context_handle);
      iotc_backoff_release();
    })

IOTC_TT_TESTCASE_WITH_SETUP(
    utest__iotc_update_backoff_penalty_time__no_data__update_update_time_and_backoff_penalty,
    iotc_utest_setup_basic, iotc_utest_teardown_basic, NULL, {
      iotc_context_handle_t iotc_context_handle = iotc_create_context();
      if (IOTC_INVALID_CONTEXT_HANDLE >= iotc_context_handle) {
        tt_fail_msg("Failed to create default context!");
        return;
      }

      iotc_evtd_instance_t* event_dispatcher = iotc_globals.evtd_instance;
      size_t itests = IOTC_ARRAYSIZE(iotc_utest_backoff_test_cases);

      size_t i = 0;
      for (; i < itests; ++i) {
        const iotc_utest_backoff_data_test_case_t* curr_test_case =
            (iotc_utest_backoff_data_test_case_t*)&iotc_utest_backoff_test_cases
                [i];

        iotc_backoff_configure_using_data(
            (iotc_vector_elem_t*)curr_test_case->backoff_data,
            (iotc_vector_elem_t*)curr_test_case->decay_data,
            curr_test_case->data_len, IOTC_MEMORY_TYPE_UNMANAGED);

        iotc_inc_backoff_penalty();

        iotc_globals.backoff_status.backoff_class = IOTC_BACKOFF_CLASS_NONE;

        iotc_vector_index_type_t* backoff_time_event_position =
            iotc_globals.backoff_status.next_update.ptr_to_position;

        iotc_backoff_lut_index_t curr_index =
            iotc_globals.backoff_status.backoff_lut_i;

        iotc_evtd_step(
            event_dispatcher,
            event_dispatcher->current_step +
                iotc_globals.backoff_status.decay_lut->array[curr_index]
                    .selector_t.ui32_value +
                1);

        if (curr_test_case->data_len > 1) {
          tt_int_op(iotc_globals.backoff_status.backoff_lut_i, <, curr_index);
        } else {
          tt_int_op(iotc_globals.backoff_status.backoff_lut_i, ==, 0);
        }

        tt_ptr_op(iotc_globals.backoff_status.next_update.ptr_to_position, ==,
                  NULL);
        tt_ptr_op(iotc_globals.backoff_status.next_update.ptr_to_position, !=,
                  backoff_time_event_position);

        iotc__utest__reset_backoff_penalty();
      }

    end:
      iotc__utest__reset_backoff_penalty();
      iotc_delete_context(iotc_context_handle);
      iotc_backoff_release();
    })

IOTC_TT_TESTCASE_WITH_SETUP(
    utest__iotc_context__no_data__after_restarting_iotc_context_penalty_must_not_be_changed,
    iotc_utest_setup_basic, iotc_utest_teardown_basic, NULL, {
      // initialize xi library.
      iotc_context_handle_t iotc_context_handle = iotc_create_context();
      if (IOTC_INVALID_CONTEXT_HANDLE >= iotc_context_handle) {
        tt_fail_msg("Failed to create default context!");
        return;
      }

      iotc_inc_backoff_penalty();

      iotc_backoff_lut_index_t curr_index =
          iotc_globals.backoff_status.backoff_lut_i;

      iotc_delete_context(iotc_context_handle);

      iotc_context_handle = iotc_create_context();
      if (IOTC_INVALID_CONTEXT_HANDLE >= iotc_context_handle) {
        tt_fail_msg("Failed to create default context!");
        return;
      }

      tt_want_int_op(curr_index, ==, iotc_globals.backoff_status.backoff_lut_i);
      tt_want_int_op(iotc_globals.backoff_status.next_update.ptr_to_position,
                     ==, NULL);

      iotc_delete_context(iotc_context_handle);
    })

IOTC_TT_TESTGROUP_END

#ifndef IOTC_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#define IOTC_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#include __FILE__
#undef IOTC_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#endif
