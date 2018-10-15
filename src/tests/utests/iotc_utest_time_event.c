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

#include "iotc_bsp_rng.h"
#include "iotc_memory_checks.h"
#include "iotc_tt_testcase_management.h"
#include "iotc_utest_basic_testcase_frame.h"
#include "tinytest.h"
#include "tinytest_macros.h"

#include "iotc.h"
#include "iotc_handle.h"
#include "iotc_time_event.h"
#include "iotc_vector.h"

#ifndef IOTC_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN

#define TEST_TIME_EVENT_TEST_SIZE 64

typedef iotc_time_t(time_event_container_element_generator)(int index);

static iotc_time_t index_generator(int index) { return index; }

static iotc_time_t random_generator_0_1000(int index) {
  IOTC_UNUSED(index);

  return iotc_bsp_rng_get() % 1000;
}

static iotc_state_t fill_vector_with_heap_elements_using_generator(
    iotc_vector_t* vector,
    iotc_time_event_t (*time_events)[TEST_TIME_EVENT_TEST_SIZE],
    iotc_time_event_handle_t (*time_event_handles)[TEST_TIME_EVENT_TEST_SIZE],
    time_event_container_element_generator* generator_fn) {
  iotc_state_t state = IOTC_STATE_OK;

  int i = 0;
  for (; i < TEST_TIME_EVENT_TEST_SIZE; ++i) {
    time_events[0][i].time_of_execution = generator_fn(i);

    iotc_state_t ret_state = iotc_time_event_add(vector, &time_events[0][i],
                                                 &time_event_handles[0][i]);

    IOTC_CHECK_STATE(ret_state);

    tt_assert(ret_state == IOTC_STATE_OK);
    tt_assert(time_event_handles[0][i].ptr_to_position != NULL);
  }

end:
err_handling:
  return state;
}

#endif

IOTC_TT_TESTGROUP_BEGIN(utest_time_event)

IOTC_TT_TESTCASE_WITH_SETUP(
    utest__iotc_time_event_execute_handle_in__single_time_event_added,
    iotc_utest_setup_basic, iotc_utest_teardown_basic, NULL, {
      iotc_vector_t* vector = iotc_vector_create();
      iotc_time_event_t time_event = iotc_make_empty_time_event();
      iotc_time_event_handle_t time_event_handle =
          iotc_make_empty_time_event_handle();

      iotc_state_t ret_state =
          iotc_time_event_add(vector, &time_event, &time_event_handle);

      tt_assert(ret_state == IOTC_STATE_OK);
      tt_assert(time_event_handle.ptr_to_position != NULL);

      iotc_vector_destroy(vector);
    end:;
    })

IOTC_TT_TESTCASE_WITH_SETUP(
    utest__iotc_time_event_execute_handle_in__add_TEST_TIME_EVENT_TEST_SIZE_random_time_events,
    iotc_utest_setup_basic, iotc_utest_teardown_basic, NULL, {
      iotc_bsp_rng_init();

      iotc_vector_t* vector = iotc_vector_create();

      iotc_time_event_handle_t time_event_handles[TEST_TIME_EVENT_TEST_SIZE] = {
          iotc_make_empty_time_event_handle()};
      iotc_time_event_t time_events[TEST_TIME_EVENT_TEST_SIZE] = {
          iotc_make_empty_time_event()};

      iotc_state_t ret_state = fill_vector_with_heap_elements_using_generator(
          vector, &time_events, &time_event_handles, &random_generator_0_1000);

      tt_assert(IOTC_STATE_OK == ret_state);

      /* counter for tracking the number of elements */
      int no_elements = 0;
      /* for tracking the order of the container */
      int last_element_value = 0;

      /* at this point we should have a heap constructed, we can test if taking
       * elements from the top we will receive them in a sorted order */
      do {
        iotc_time_event_t* time_event = iotc_time_event_get_top(vector);
        tt_assert(time_event->time_of_execution >= last_element_value);
        last_element_value = time_event->time_of_execution;
        ++no_elements;
      } while (vector->elem_no != 0);

      /* and we can check if all of them has been received */
      tt_assert(no_elements == TEST_TIME_EVENT_TEST_SIZE);

      iotc_vector_destroy(vector);
    end:
      iotc_bsp_rng_shutdown();
    })

IOTC_TT_TESTCASE_WITH_SETUP(
    utest__iotc_time_event_restart_first_element__event_key_and_position_changed_positive,
    iotc_utest_setup_basic, iotc_utest_teardown_basic, NULL, {
      iotc_time_t original_position = 0;
      for (; original_position < TEST_TIME_EVENT_TEST_SIZE;
           ++original_position) {
        iotc_time_event_handle_t time_event_handles[TEST_TIME_EVENT_TEST_SIZE] =
            {iotc_make_empty_time_event_handle()};
        iotc_time_event_t time_events[TEST_TIME_EVENT_TEST_SIZE] = {
            iotc_make_empty_time_event()};

        iotc_vector_t* vector = iotc_vector_create();

        iotc_state_t ret_state = fill_vector_with_heap_elements_using_generator(
            vector, &time_events, &time_event_handles, &index_generator);

        int i = 0;
        for (; i < TEST_TIME_EVENT_TEST_SIZE; ++i) {
          tt_assert(*time_event_handles[i].ptr_to_position == i);
        }

        tt_assert(IOTC_STATE_OK == ret_state);

        const iotc_time_t new_test_time = TEST_TIME_EVENT_TEST_SIZE + 12;

        ret_state = iotc_time_event_restart(
            vector, &time_event_handles[original_position], new_test_time);

        tt_assert(IOTC_STATE_OK == ret_state);

        iotc_time_event_t* time_event =
            (iotc_time_event_t*)vector->array[TEST_TIME_EVENT_TEST_SIZE - 1]
                .selector_t.ptr_value;

        tt_assert(time_event->time_of_execution == new_test_time);
        tt_assert(time_event->position ==
                  *time_event_handles[original_position].ptr_to_position);

        iotc_vector_destroy(vector);
      }
    end:;
    })

IOTC_TT_TESTCASE_WITH_SETUP(
    utest__iotc_time_event_restart_first_element__event_key_and_position_changed_negative,
    iotc_utest_setup_basic, iotc_utest_teardown_basic, NULL, {
      iotc_time_t original_position = 0;
      for (; original_position < TEST_TIME_EVENT_TEST_SIZE;
           ++original_position) {
        iotc_time_event_handle_t time_event_handles[TEST_TIME_EVENT_TEST_SIZE] =
            {iotc_make_empty_time_event_handle()};
        iotc_time_event_t time_events[TEST_TIME_EVENT_TEST_SIZE] = {
            iotc_make_empty_time_event()};

        iotc_vector_t* vector = iotc_vector_create();

        iotc_state_t ret_state = fill_vector_with_heap_elements_using_generator(
            vector, &time_events, &time_event_handles, &index_generator);

        int i = 0;
        for (; i < TEST_TIME_EVENT_TEST_SIZE; ++i) {
          tt_assert(*time_event_handles[i].ptr_to_position == i);
        }

        tt_assert(IOTC_STATE_OK == ret_state);

        const iotc_time_t new_test_time = -1;

        ret_state = iotc_time_event_restart(
            vector, &time_event_handles[original_position], new_test_time);

        tt_assert(IOTC_STATE_OK == ret_state);

        iotc_time_event_t* time_event =
            (iotc_time_event_t*)vector->array[0].selector_t.ptr_value;

        tt_assert(time_event->time_of_execution == new_test_time);
        tt_assert(time_event->position ==
                  *time_event_handles[original_position].ptr_to_position);

        iotc_vector_destroy(vector);
      }
    end:;
    })

IOTC_TT_TESTCASE_WITH_SETUP(
    utest__iotc_time_event_cancel_all_elements__elements_removed_their_handlers_cleared,
    iotc_utest_setup_basic, iotc_utest_teardown_basic, NULL, {
      iotc_vector_t* vector = iotc_vector_create();

      iotc_time_event_handle_t time_event_handles[TEST_TIME_EVENT_TEST_SIZE] = {
          iotc_make_empty_time_event_handle()};
      iotc_time_event_t time_events[TEST_TIME_EVENT_TEST_SIZE] = {
          iotc_make_empty_time_event()};

      iotc_state_t ret_state = fill_vector_with_heap_elements_using_generator(
          vector, &time_events, &time_event_handles, &index_generator);

      tt_assert(IOTC_STATE_OK == ret_state);

      {
        size_t i = 0;
        for (; i < TEST_TIME_EVENT_TEST_SIZE; ++i) {
          iotc_time_event_t* cancelled_time_event = NULL;
          const iotc_state_t local_state = iotc_time_event_cancel(
              vector, &time_event_handles[i], &cancelled_time_event);

          tt_assert(IOTC_STATE_OK == local_state);
        }
      }

      /* vector should be empty */
      tt_assert(0 == vector->elem_no);

      {
        size_t i = 0;
        for (; i < TEST_TIME_EVENT_TEST_SIZE; ++i) {
          tt_assert(NULL == time_event_handles[i].ptr_to_position);
        }
      }

      iotc_vector_destroy(vector);
    end:;
    })

IOTC_TT_TESTGROUP_END

#ifndef IOTC_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#define IOTC_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#include __FILE__
#undef IOTC_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#endif
