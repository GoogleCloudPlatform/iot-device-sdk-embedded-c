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

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "iotc_tt_testcase_management.h"
#include "tinytest.h"
#include "tinytest_macros.h"

#include "iotc_event_dispatcher_api.h"
#include "iotc_io_timeouts.h"
#include "iotc_memory_checks.h"
#include "iotc_vector.h"

#ifndef IOTC_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN

void iotc_utest_local__add_handler__evtd(iotc_evtd_instance_t* evtd,
                                         iotc_event_handle_func_argc4 fn,
                                         iotc_event_handle_arg1_t arg1,
                                         iotc_event_handle_arg2_t arg2,
                                         iotc_event_handle_arg3_t arg3,
                                         iotc_event_handle_arg4_t arg4) {
  if (evtd == NULL) return;

  iotc_event_handle_t evtd_handle = {
      IOTC_EVENT_HANDLE_ARGC4, .handlers.h4 = {fn, arg1, arg2, arg3, arg4}, 0};

  iotc_evtd_execute_in(evtd, evtd_handle, 1, NULL);
}

iotc_state_t iotc_utest_local_action__timed(
    iotc_event_handle_arg1_t target_evtd, iotc_event_handle_arg2_t exec_counter,
    iotc_event_handle_arg3_t state,
    iotc_event_handle_arg4_t seconds_since_epoch) {
  if (target_evtd == NULL || exec_counter == NULL) return IOTC_STATE_OK;
  {
    uint32_t* counter = (uint32_t*)exec_counter;

    printf("%s, counter = %u, time = %ld\n", __func__, (unsigned int)*counter,
           time(0));

    if (0 < *counter) {
      --(*counter);

      tt_want_int_op((*(time_t*)seconds_since_epoch) + 1, ==, time(0));

      *(time_t*)seconds_since_epoch = time(0);

      iotc_utest_local__add_handler__evtd(
          (iotc_evtd_instance_t*)target_evtd, &iotc_utest_local_action__timed,
          target_evtd, exec_counter, state, seconds_since_epoch);
    }
  }

  return IOTC_STATE_OK;
}

iotc_state_t iotc_utest_local_action__io_timed(void* arg1, void* vector,
                                               iotc_state_t state,
                                               void* execution_counter) {
  IOTC_UNUSED(state);

  iotc_io_timeout_t* timeout_element = (iotc_io_timeout_t*)arg1;
  iotc_time_event_handle_t* event = &timeout_element->timeout;

  // Increase execution count.
  *((int*)execution_counter) += 1;

  // Remove from vector.
  iotc_io_timeouts_remove(event, vector);

  IOTC_SAFE_FREE(timeout_element);

  return IOTC_STATE_OK;
}

#endif  // IOTC_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN

IOTC_TT_TESTGROUP_BEGIN(utest_event_dispatcher_timed)

// Skipped, because it takes to much time, enable it temporarily for aimed
// platform tests.
SKIP_IOTC_TT_TESTCASE(
    utest__iotc_evtd_execute_in__time_function_misbehaviour_upon_core_switch__test_for_multicore_archs__time_is_measured_same_on_different_cores,
    {
      // This test is for testing the function time( 0 ) whether its result
      // depends on which core it is called.
      iotc_evtd_instance_t* evtd = iotc_evtd_create_instance();

      uint32_t exec_counter = 5;
      time_t seconds_since_epoch = time(0) - 1;
      iotc_state_t state = IOTC_STATE_OK;

      iotc_utest_local__add_handler__evtd(evtd, &iotc_utest_local_action__timed,
                                          evtd, &exec_counter, state,
                                          &seconds_since_epoch);

      while (0 < exec_counter) {
        // OS might switch core of execution between two calls on function
        // 'time'
        iotc_evtd_step(evtd, time(0));
      }

      // end:
      iotc_evtd_destroy_instance(evtd);
    })

IOTC_TT_TESTCASE(
    utest__iotc_evtd_io_execute_in__delayed_execution__events_should_be_executed,
    {
      // This test checks if event dispatcher executes all scheduled handles.

      iotc_evtd_instance_t* evtd = iotc_evtd_create_instance();

      int index = 0;
      uint32_t time_counter = 0;
      uint32_t execution_counter = 0;
      iotc_state_t state = IOTC_STATE_OK;
      iotc_vector_t* vector = iotc_vector_create();
      iotc_vector_t* test_vector = iotc_vector_create();
      iotc_time_event_handle_t* time_event_handle = NULL;

      // Schedule 50 handles.
      for (index = 0; index < 50; index++) {
        IOTC_ALLOC(iotc_io_timeout_t, timeout_element, state);

        time_event_handle = &timeout_element->timeout;

        iotc_event_handle_t evtd_handle = iotc_make_handle(
            &iotc_utest_local_action__io_timed, timeout_element, vector,
            IOTC_STATE_OK, &execution_counter);

        // Schedule handle execution.
        iotc_io_timeouts_create(evtd, evtd_handle, 10, vector,
                                time_event_handle);

        // Store event in the test vector.
        iotc_vector_push(test_vector, IOTC_VEC_CONST_VALUE_PARAM(
                                          IOTC_VEC_VALUE_PTR(timeout_element)));
      }

      // Check if vector contains all events.
      tt_assert(vector->elem_no == 50);

      // Check if elements are valid.
      for (index = 0; index < vector->elem_no; ++index) {
        iotc_vector_elem_t element = vector->array[index];
        iotc_vector_elem_t test_element = test_vector->array[index];
        iotc_io_timeout_t* timeout = test_element.selector_t.ptr_value;
        iotc_time_event_handle_t* event = element.selector_t.ptr_value;
        iotc_time_event_handle_t* test_event = &timeout->timeout;
        tt_assert(event == test_event);
      }

      // Simulate time stepping.
      while (time_counter < 11) {
        iotc_evtd_step(evtd, time_counter);
        time_counter += 1;
      }

      // Check if all events are executed.
      tt_assert(execution_counter == 50);

      // Check if all executed events are removed from vector.
      tt_assert(vector->elem_no == 0);

      goto end;

    err_handling:
      tt_abort_msg("test should not fail");

    end:
      iotc_vector_destroy(vector);
      iotc_vector_destroy(test_vector);
      iotc_evtd_destroy_instance(evtd);

      tt_want_int_op(iotc_is_whole_memory_deallocated(), >, 0);
    })

IOTC_TT_TESTCASE(
    utest__iotc_evtd_io_cancel__delayed_execution__events_should_be_cancelled, {
      // This test checks if event dispatcher cancels scheduled handles.

      iotc_evtd_instance_t* evtd = iotc_evtd_create_instance();

      int index = 0;
      uint32_t time_counter = 0;
      uint32_t execution_counter = 0;
      iotc_state_t state = IOTC_STATE_OK;
      iotc_vector_t* vector = iotc_vector_create();
      iotc_vector_t* test_vector = iotc_vector_create();
      iotc_time_event_handle_t* time_event_handle = NULL;

      // Schedule 50 handles.
      for (index = 0; index < 50; index++) {
        IOTC_ALLOC(iotc_io_timeout_t, timeout_element, state);

        time_event_handle = &timeout_element->timeout;

        iotc_event_handle_t evtd_handle = iotc_make_handle(
            &iotc_utest_local_action__io_timed, timeout_element, vector,
            IOTC_STATE_OK, &execution_counter);

        // Schedule handle execution.
        iotc_io_timeouts_create(evtd, evtd_handle, 10, vector,
                                time_event_handle);

        // Store event in the test vector.
        iotc_vector_push(test_vector, IOTC_VEC_CONST_VALUE_PARAM(
                                          IOTC_VEC_VALUE_PTR(timeout_element)));
      }

      // Simulate time stepping.
      while (time_counter < 11) {
        // Cancel 25 elements.
        if (time_counter == 5) {
          for (index = 0; index < 25; ++index) {
            iotc_vector_elem_t element = test_vector->array[index];
            iotc_io_timeout_t* timeout_element = element.selector_t.ptr_value;
            iotc_time_event_handle_t* event = &timeout_element->timeout;
            iotc_io_timeouts_cancel(evtd, event, vector);
            IOTC_SAFE_FREE(timeout_element);
          }
        }
        iotc_evtd_step(evtd, time_counter);
        time_counter += 1;
      }

      // Checking if remaining events are executed.
      tt_assert(execution_counter == 25);

      // Check if vector contains no remaining events, they are both executed or
      // cancelled.
      tt_assert(vector->elem_no == 0);

      goto end;

    err_handling:
      tt_abort_msg("test should not fail");

    end:
      iotc_vector_destroy(vector);
      iotc_vector_destroy(test_vector);
      iotc_evtd_destroy_instance(evtd);

      tt_want_int_op(iotc_is_whole_memory_deallocated(), >, 0);
    })

IOTC_TT_TESTCASE(
    utest__iotc_evtd_io_restart__delayed_execution__events_should_be_restarted,
    {
      // This test checks if event dispatcher reschedules handles.

      iotc_evtd_instance_t* evtd = iotc_evtd_create_instance();

      int index = 0;
      uint32_t time_counter = 0;
      uint32_t execution_counter = 0;
      iotc_state_t state = IOTC_STATE_OK;
      iotc_vector_t* vector = iotc_vector_create();
      iotc_vector_t* test_vector = iotc_vector_create();
      iotc_time_event_handle_t* time_event_handle = NULL;

      // Schedule 50 handles.
      for (index = 0; index < 50; index++) {
        IOTC_ALLOC(iotc_io_timeout_t, timeout_element, state);

        time_event_handle = &timeout_element->timeout;

        iotc_event_handle_t evtd_handle = iotc_make_handle(
            &iotc_utest_local_action__io_timed, timeout_element, vector,
            IOTC_STATE_OK, &execution_counter);

        // Schedule handle execution.
        iotc_io_timeouts_create(evtd, evtd_handle, 10, vector,
                                time_event_handle);

        // Store first 25 events in the test vector.
        if (index < 25) {
          iotc_vector_push(test_vector,
                           IOTC_VEC_CONST_VALUE_PARAM(
                               IOTC_VEC_VALUE_PTR(time_event_handle)));
        }
      }

      // Simulate time stepping.
      while (time_counter < 20) {
        // Restart first 25 elements.
        if (time_counter == 6) {
          iotc_io_timeouts_restart(evtd, 10, test_vector);
        }
        // Check if last 25 was executed.
        if (time_counter == 11) {
          tt_assert(execution_counter == 25);
        }
        // Check if first 25 was executed after restart.
        if (time_counter == 16) {
          tt_assert(execution_counter == 50);
        }
        iotc_evtd_step(evtd, time_counter);

        time_counter += 1;
      }

      goto end;

    err_handling:
      tt_abort_msg("test should not fail");

    end:
      iotc_vector_destroy(vector);
      iotc_vector_destroy(test_vector);
      iotc_evtd_destroy_instance(evtd);

      tt_want_int_op(iotc_is_whole_memory_deallocated(), >, 0);
    })

IOTC_TT_TESTCASE(
    utest__iotc_evtd_io_remove__delayed_execution__events_should_be_removed, {
      // This test checks if event dispatcher removes events from events vector.

      iotc_evtd_instance_t* evtd = iotc_evtd_create_instance();

      int index = 0;
      uint32_t execution_counter = 0;
      iotc_state_t state = IOTC_STATE_OK;
      iotc_vector_t* vector = iotc_vector_create();
      iotc_vector_t* test_vector = iotc_vector_create();
      iotc_vector_t* element_vector = iotc_vector_create();
      iotc_time_event_handle_t* time_event_handle = NULL;

      // Schedule 50 handles.
      for (index = 0; index < 50; index++) {
        IOTC_ALLOC(iotc_io_timeout_t, timeout_element, state);

        time_event_handle = &timeout_element->timeout;

        iotc_event_handle_t evtd_handle = iotc_make_handle(
            &iotc_utest_local_action__io_timed, timeout_element, vector,
            IOTC_STATE_OK, &execution_counter);

        // Schedule handle execution.
        iotc_io_timeouts_create(evtd, evtd_handle, 10, vector,
                                time_event_handle);

        // Store first 25 events in the test vector.
        if (index < 25) {
          iotc_vector_push(test_vector,
                           IOTC_VEC_CONST_VALUE_PARAM(
                               IOTC_VEC_VALUE_PTR(time_event_handle)));
        }

        // Store event in the test vector.
        iotc_vector_push(
            element_vector,
            IOTC_VEC_CONST_VALUE_PARAM(IOTC_VEC_VALUE_PTR(timeout_element)));
      }

      // Removing events.
      for (index = 0; index < 25; index++) {
        iotc_vector_elem_t element = test_vector->array[index];
        time_event_handle = element.selector_t.ptr_value;

        iotc_io_timeouts_remove(time_event_handle, vector);
      }

      // Check if the last 25 were removed.
      tt_assert(vector->elem_no == 25);

      // Cleanup.
      for (index = 0; index < 50; ++index) {
        iotc_vector_elem_t element = element_vector->array[index];
        iotc_io_timeout_t* timeout_element = element.selector_t.ptr_value;
        iotc_time_event_handle_t* event = &timeout_element->timeout;
        iotc_io_timeouts_cancel(evtd, event, vector);
        IOTC_SAFE_FREE(timeout_element);
      }

      goto end;

    err_handling:
      tt_abort_msg("test should not fail");

    end:
      iotc_vector_destroy(vector);
      iotc_vector_destroy(test_vector);
      iotc_vector_destroy(element_vector);
      iotc_evtd_destroy_instance(evtd);

      tt_want_int_op(iotc_is_whole_memory_deallocated(), >, 0);
    })

IOTC_TT_TESTGROUP_END

#ifndef IOTC_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#define IOTC_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#include __FILE__
#undef IOTC_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#endif
