/* Copyright 2018-2020 Google LLC
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

#include "iotc_thread_ids.h"
#include "iotc_thread_threadpool.h"

#ifndef IOTC_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN

#include "iotc_utest_thread_util_actions.h"

void iotc_utest_local__add_handler__threadpool(
    iotc_threadpool_t* threadpool, uint8_t id_target_thread,
    iotc_event_handle_func_argc1 fn, iotc_event_handle_arg1_t* arg1,
    size_t add_handler_multiplicity) {
  if (threadpool == NULL) return;

  iotc_event_handle_t evtd_handle = {
      IOTC_EVENT_HANDLE_ARGC1,
      .handlers.h1 = {fn, (iotc_event_handle_arg1_t)arg1}, 0};

  size_t counter_added = 0;
  for (; counter_added < add_handler_multiplicity; ++counter_added) {
    (id_target_thread == IOTC_THREADID_ANYTHREAD)
        ? iotc_threadpool_execute(threadpool, evtd_handle)
        : iotc_threadpool_execute_on_thread(threadpool, evtd_handle,
                                            id_target_thread);
  }
}

#endif  // IOTC_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN

IOTC_TT_TESTGROUP_BEGIN(utest_thread_threadpool)

IOTC_TT_TESTCASE(
    utest__iotc_threadpool__create_and_destroy_instance__not_null_output_and_null_after_destroy,
    {
      const int num_of_threads_reqested_list[] = {0, 1, 7, 10, 10000};
      const int num_of_threads_expected_list[] = {
          1, 1, 7, 10, IOTC_THREADPOOL_MAXNUMOFTHREADS};

      uint8_t counter_testcase = 0;
      for (; counter_testcase < IOTC_ARRAYSIZE(num_of_threads_reqested_list);
           ++counter_testcase) {
        iotc_threadpool_t* threadpool = iotc_threadpool_create_instance(
            num_of_threads_reqested_list[counter_testcase]);

        tt_ptr_op(NULL, !=, threadpool);

        tt_ptr_op(NULL, !=, threadpool->workerthreads);
        tt_want_int_op(num_of_threads_expected_list[counter_testcase], ==,
                       threadpool->workerthreads->elem_no);
        tt_want_int_op(num_of_threads_expected_list[counter_testcase], ==,
                       threadpool->workerthreads->capacity);

        iotc_threadpool_destroy_instance(&threadpool);

        tt_ptr_op(NULL, ==, threadpool);
      }
    end:
      return;
    })

IOTC_TT_TESTCASE(
    utest__iotc_threadpool_execute_on_thread__invalid_thread_request__handle_is_executed_as_an_anythread_request,
    {
      // arrange
      iotc_threadpool_t* threadpool = iotc_threadpool_create_instance(3);
      uint32_t value_shared_between_threads = 12;

      iotc_init_critical_section(&iotc_uteset_local_action_store_cs);

      // act
      iotc_utest_local__add_handler__threadpool(
          threadpool,
          4  // 4 is invalid tid, there are only 3 threads, valid ids: 0,1,2
          ,
          &iotc_utest_local_action_increase_by_one,
          (iotc_event_handle_arg1_t)&value_shared_between_threads, 1);

      iotc_threadpool_destroy_instance(&threadpool);

      iotc_destroy_critical_section(&iotc_uteset_local_action_store_cs);

      // assert
      tt_want_int_op(13, ==, value_shared_between_threads);
    })

IOTC_TT_TESTCASE(
    utest__iotc_threadpool_execute_on_thread__singlethread_multievents__alleventsexecuted,
    {
      const size_t test_cases_handler_multiplicity[] = {1, 2, 3, 30, 300};

      iotc_init_critical_section(&iotc_uteset_local_action_store_cs);

      uint8_t counter_testcase = 0;
      for (; counter_testcase < IOTC_ARRAYSIZE(test_cases_handler_multiplicity);
           ++counter_testcase) {
        iotc_threadpool_t* threadpool = iotc_threadpool_create_instance(1);

        uint32_t value_shared_between_threads = 13579;

        iotc_utest_local__add_handler__threadpool(
            threadpool, 0, &iotc_utest_local_action_increase_by_one,
            (iotc_event_handle_arg1_t)&value_shared_between_threads,
            test_cases_handler_multiplicity[counter_testcase]);

        iotc_threadpool_destroy_instance(&threadpool);

        tt_want_int_op(
            13579 + test_cases_handler_multiplicity[counter_testcase], ==,
            value_shared_between_threads);
      }

      iotc_destroy_critical_section(&iotc_uteset_local_action_store_cs);
    })

IOTC_TT_TESTCASE(
    utest__iotc_threadpool_execute_on_thread__multithreads_singleevent__alleventsexecuted,
    {
      const uint8_t test_cases_thread_multiplicity[] = {1, 2, 7, 30};

      iotc_init_critical_section(&iotc_uteset_local_action_store_cs);

      uint8_t counter_testcase = 0;
      for (; counter_testcase < IOTC_ARRAYSIZE(test_cases_thread_multiplicity);
           ++counter_testcase) {
        uint32_t values_will_be_increased_by_threads
            [IOTC_THREADPOOL_MAXNUMOFTHREADS] = {10, 11, 12, 13, 14,
                                                 15, 16, 17, 18, 19};

        const uint8_t desired_num_of_threads =
            test_cases_thread_multiplicity[counter_testcase];
        iotc_threadpool_t* threadpool =
            iotc_threadpool_create_instance(desired_num_of_threads);
        const uint8_t actual_num_of_threads =
            threadpool->workerthreads->elem_no;

        // add handler to each workerthread
        uint8_t counter_tid = 0;
        for (; counter_tid < actual_num_of_threads; ++counter_tid) {
          iotc_utest_local__add_handler__threadpool(
              threadpool, counter_tid, &iotc_utest_local_action_increase_by_one,
              (iotc_event_handle_arg1_t)(values_will_be_increased_by_threads +
                                         counter_tid),
              1);
        }

        iotc_threadpool_destroy_instance(&threadpool);

        counter_tid = 0;
        for (; counter_tid < actual_num_of_threads; ++counter_tid) {
          // every thread added 1 to the original value
          tt_want_int_op(10 + counter_tid + 1, ==,
                         values_will_be_increased_by_threads[counter_tid]);
        }
      }

      iotc_destroy_critical_section(&iotc_uteset_local_action_store_cs);
    })

IOTC_TT_TESTCASE(
    utest__iotc_threadpool_execute_on_thread__multithreads__alleventsexecuted_each_on_different_thread,
    {
      const uint8_t test_cases_thread_multiplicity[] = {1, 2, 7, 30};

      uint8_t counter_testcase = 0;
      for (; counter_testcase < IOTC_ARRAYSIZE(test_cases_thread_multiplicity);
           ++counter_testcase) {
        uint32_t tids_filled_by_thread_action[IOTC_THREADPOOL_MAXNUMOFTHREADS] =
            {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

        const uint8_t desired_num_of_threads =
            test_cases_thread_multiplicity[counter_testcase];
        iotc_threadpool_t* threadpool =
            iotc_threadpool_create_instance(desired_num_of_threads);
        const uint8_t actual_num_of_threads =
            threadpool->workerthreads->elem_no;

        // add handler to each workerthread
        uint8_t counter_tid = 0;
        for (; counter_tid < actual_num_of_threads; ++counter_tid) {
          iotc_utest_local__add_handler__threadpool(
              threadpool, counter_tid, &iotc_utest_local_action_store_tid,
              (iotc_event_handle_arg1_t)(tids_filled_by_thread_action +
                                         counter_tid),
              1);
        }

        iotc_threadpool_destroy_instance(&threadpool);

        // assert: all handles are executed on different thread
        counter_tid = 0;
        for (; counter_tid < actual_num_of_threads; ++counter_tid) {
          uint8_t counter_tid_2 = counter_tid + 1;
          for (; counter_tid_2 < actual_num_of_threads; ++counter_tid_2) {
            tt_want_int_op(tids_filled_by_thread_action[counter_tid], !=,
                           tids_filled_by_thread_action[counter_tid_2]);
            if (tids_filled_by_thread_action[counter_tid] ==
                tids_filled_by_thread_action[counter_tid_2]) {
              // printf("checking %d - %d, actual num of threads = %d \n",
              // counter_tid, counter_tid_2, actual_num_of_threads);
            }
          }
        }
      }
    })

IOTC_TT_TESTCASE(
    utest__iotc_threadpool_execute__multievents__alleventsexecuted, {
      const size_t test_cases_handler_multiplicity[] = {0, 2, 3, 30, 300, 5000};

      iotc_init_critical_section(&iotc_uteset_local_action_store_cs);

      uint8_t counter_testcase = 0;
      for (; counter_testcase <
             (iotc_test_load_level
                  ? IOTC_ARRAYSIZE(test_cases_handler_multiplicity)
                  : 4);
           ++counter_testcase) {
        iotc_threadpool_t* threadpool = iotc_threadpool_create_instance(3);

        uint32_t value_shared_between_threads = 222;

        iotc_utest_local__add_handler__threadpool(
            threadpool, IOTC_THREADID_ANYTHREAD,
            &iotc_utest_local_action_increase_by_one,
            (iotc_event_handle_arg1_t)&value_shared_between_threads,
            test_cases_handler_multiplicity[counter_testcase]);

        iotc_threadpool_destroy_instance(&threadpool);

        tt_want_int_op(222 + test_cases_handler_multiplicity[counter_testcase],
                       ==, value_shared_between_threads);
      }

      iotc_destroy_critical_section(&iotc_uteset_local_action_store_cs);
    })

IOTC_TT_TESTGROUP_END

#ifndef IOTC_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#define IOTC_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#include __FILE__
#undef IOTC_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#endif  // IOTC_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
