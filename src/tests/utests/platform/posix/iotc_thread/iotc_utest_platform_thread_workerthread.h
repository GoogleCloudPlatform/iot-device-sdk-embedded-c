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

#include "iotc_list.h"
#include "iotc_thread_posix_workerthread.h"
#include "tinytest.h"
#include "tinytest_macros.h"

#ifndef IOTC_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN

#include "../../../iotc_utest_thread_util_actions.h"

void iotc_utest_local__add_handler__workerthread(
    iotc_evtd_instance_t* evtd, iotc_event_handle_func_argc1 fn,
    iotc_event_handle_arg1_t* arg1, size_t add_handler_multiplicity) {
  if (evtd == NULL) return;

  iotc_event_handle_t evtd_handle = {
      IOTC_EVENT_HANDLE_ARGC1,
      .handlers.h1 = {fn, (iotc_event_handle_arg1_t)arg1}, 0};

  size_t counter_added = 0;
  for (; counter_added < add_handler_multiplicity; ++counter_added) {
    iotc_evtd_execute(evtd, evtd_handle);
  }
}

#endif  // IOTC_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN

IOTC_TT_TESTCASE(
    utest__posix__iotc_workerthread_create_instance__no_input__fields_are_initialized,
    {
      iotc_evtd_instance_t* evtd = iotc_evtd_create_instance();
      iotc_evtd_instance_t* test_cases_evtd_ptrs[] = {
          (iotc_evtd_instance_t*)NULL, evtd};

      uint8_t counter_testcase = 0;
      for (; counter_testcase < IOTC_ARRAYSIZE(test_cases_evtd_ptrs);
           ++counter_testcase) {
        iotc_workerthread_t* new_workerthread_instance =
            iotc_workerthread_create_instance(
                test_cases_evtd_ptrs[counter_testcase]);

        tt_ptr_op(NULL, !=, new_workerthread_instance);
        tt_ptr_op(NULL, !=, new_workerthread_instance->thread_evtd);
        tt_ptr_op(test_cases_evtd_ptrs[counter_testcase], ==,
                  new_workerthread_instance->thread_evtd_secondary);

        iotc_workerthread_destroy_instance(&new_workerthread_instance);

        tt_ptr_op(NULL, ==, new_workerthread_instance);
      }

    end:
      iotc_evtd_destroy_instance(evtd);

      return;
    })

IOTC_TT_TESTCASE(
    utest__posix__iotc_workerthread__add_onetype_handler_multiple_times__all_handlers_are_executed,
    {
      const size_t nb_handler_additions[] = {1, 2, 23, 124, 0};

      iotc_init_critical_section(&iotc_uteset_local_action_store_cs);

      size_t id_handler_adds = 0;
      for (; id_handler_adds < IOTC_ARRAYSIZE(nb_handler_additions) - 4;
           ++id_handler_adds) {
        iotc_workerthread_t* workerthread =
            iotc_workerthread_create_instance(NULL);

        // const uint8_t sync_point_reached =
        // iotc_workerthread_wait_sync_point(workerthread);
        // tt_want_int_op(1, ==, sync_point_reached);

        uint32_t value_shared_between_threads = 111;

        iotc_utest_local__add_handler__workerthread(
            workerthread->thread_evtd, &iotc_utest_local_action_increase_by_one,
            (iotc_event_handle_arg1_t)&value_shared_between_threads,
            nb_handler_additions[id_handler_adds]);

        iotc_workerthread_destroy_instance(&workerthread);

#if 0
		printf("current number of handler additions: %lu, result = %u\n"
			, nb_handler_additions[id_handler_adds]
			, value_shared_between_threads);
#endif
        tt_want_int_op(111 + nb_handler_additions[id_handler_adds], ==,
                       value_shared_between_threads);
      }

      iotc_destroy_critical_section(&iotc_uteset_local_action_store_cs);
    })

IOTC_TT_TESTCASE(
    utest__posix__iotc_workerthread__add_twotypes_handler_multiple_times__all_handlers_are_executed,
    {
      const size_t nb_handler_additions_A[] = {67, 4, 3, 7, 1, 0};
      const size_t nb_handler_additions_B[] = {
          7, 4, 22, 0, 6, 7};  // keep arrays same sized!

      iotc_init_critical_section(&iotc_uteset_local_action_store_cs);

      size_t id_handler_adds = 0;
      for (; id_handler_adds < IOTC_ARRAYSIZE(nb_handler_additions_A) - 5;
           ++id_handler_adds) {
        iotc_workerthread_t* workerthread =
            iotc_workerthread_create_instance(NULL);

        // const uint8_t sync_point_reached =
        // iotc_workerthread_wait_sync_point(workerthread);
        // tt_want_int_op(1, ==, sync_point_reached);

        uint32_t value_shared_between_threads = 77777;

        iotc_utest_local__add_handler__workerthread(
            workerthread->thread_evtd, &iotc_utest_local_action_increase_by_one,
            (iotc_event_handle_arg1_t)&value_shared_between_threads,
            nb_handler_additions_A[id_handler_adds]);

        iotc_utest_local__add_handler__workerthread(
            workerthread->thread_evtd, &iotc_utest_local_action_decrease_by_11,
            (iotc_event_handle_arg1_t)&value_shared_between_threads,
            nb_handler_additions_B[id_handler_adds]);

        iotc_workerthread_destroy_instance(&workerthread);

#if 0
		printf("current number of handler additions: %lu - %lu, result = %u\n"
			, nb_handler_additions_A[id_handler_adds]
			, nb_handler_additions_B[id_handler_adds]
			, value_shared_between_threads);
#endif

        tt_want_int_op(77777 + nb_handler_additions_A[id_handler_adds] -
                           11 * nb_handler_additions_B[id_handler_adds],
                       ==, value_shared_between_threads);
      }

      iotc_destroy_critical_section(&iotc_uteset_local_action_store_cs);
    })

IOTC_TT_TESTCASE(
    utest__posix__iotc_workerthread__usageof_secondary_evtd__one_or_two_handlers_are_executed,
    {
      const size_t nb_handler_additions[] = {0, 1, 2, 5, 11};

      iotc_init_critical_section(&iotc_uteset_local_action_store_cs);

      size_t id_handler_adds = 0;
      for (; id_handler_adds <
             (iotc_test_load_level ? IOTC_ARRAYSIZE(nb_handler_additions) : 3);
           ++id_handler_adds) {
        iotc_evtd_instance_t* evtd = iotc_evtd_create_instance();
        iotc_workerthread_t* workerthread =
            iotc_workerthread_create_instance(evtd);

        // const uint8_t sync_point_reached =
        // iotc_workerthread_wait_sync_point(workerthread);
        // tt_want_int_op(1, ==, sync_point_reached);

        uint32_t value_shared_between_threads = 1111;

        iotc_utest_local__add_handler__workerthread(
            workerthread->thread_evtd_secondary,
            &iotc_utest_local_action_decrease_by_11,
            (iotc_event_handle_arg1_t)&value_shared_between_threads,
            nb_handler_additions[id_handler_adds]);

        while (
            !IOTC_LIST_EMPTY(iotc_event_handle_t,
                             workerthread->thread_evtd_secondary->call_queue)) {
          IOTC_TIME_MILLISLEEP(10, deltatime);
        }

        iotc_workerthread_destroy_instance(&workerthread);
        iotc_evtd_destroy_instance(evtd);

#if 0
		printf("current number of handler additions: %lu, result = %u\n"
			, nb_handler_additions[id_handler_adds]
			, value_shared_between_threads);
#endif
        tt_want_int_op(1111 - nb_handler_additions[id_handler_adds] * 11, ==,
                       value_shared_between_threads);
      }

      iotc_destroy_critical_section(&iotc_uteset_local_action_store_cs);
    })

IOTC_TT_TESTCASE(
    utest__posix__iotc_workerthread__usageof_both_evtds__handlers_are_executed_properly,
    {
      const size_t nb_handler_additions[] = {0, 2, 5, 9};

      iotc_init_critical_section(&iotc_uteset_local_action_store_cs);

      size_t id_handler_adds = 0;
      for (; id_handler_adds <
             (iotc_test_load_level ? IOTC_ARRAYSIZE(nb_handler_additions) : 2);
           ++id_handler_adds) {
        iotc_evtd_instance_t* evtd = iotc_evtd_create_instance();
        iotc_workerthread_t* workerthread =
            iotc_workerthread_create_instance(evtd);

        // const uint8_t sync_point_reached =
        // iotc_workerthread_wait_sync_point(workerthread);
        // tt_want_int_op(1, ==, sync_point_reached);

        uint32_t value_shared_between_threads = 1111;

        iotc_utest_local__add_handler__workerthread(
            workerthread->thread_evtd_secondary,
            &iotc_utest_local_action_decrease_by_11,
            (iotc_event_handle_arg1_t)&value_shared_between_threads,
            nb_handler_additions[id_handler_adds]);

        iotc_utest_local__add_handler__workerthread(
            workerthread->thread_evtd, &iotc_utest_local_action_increase_by_one,
            (iotc_event_handle_arg1_t)&value_shared_between_threads,
            nb_handler_additions[id_handler_adds]);

        while (
            !IOTC_LIST_EMPTY(iotc_event_handle_t,
                             workerthread->thread_evtd_secondary->call_queue)) {
          IOTC_TIME_MILLISLEEP(10, deltatime);
        }

        iotc_workerthread_destroy_instance(&workerthread);
        iotc_evtd_destroy_instance(evtd);

#if 0
		printf("current number of handler additions: %lu, result = %u\n"
			, nb_handler_additions[id_handler_adds]
			, value_shared_between_threads);
#endif
        tt_want_int_op(1111 + (1 - 11) * nb_handler_additions[id_handler_adds],
                       ==, value_shared_between_threads);
      }

      iotc_destroy_critical_section(&iotc_uteset_local_action_store_cs);
    })
