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

#include "iotc_memory_checks.h"
#include "iotc_tt_testcase_management.h"
#include "tinytest.h"
#include "tinytest_macros.h"

#include "iotc_timed_task.h"
#include "iotc_vector.h"

#ifndef IOTC_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN

iotc_context_handle_t iotc_utest_timed_task_last_context_handle = -1;
iotc_timed_task_handle_t iotc_utest_timed_task_last_timed_task_handle = -1;
void* iotc_utest_timed_task_user_data = NULL;

void iotc_utest_timed_task_reset_globals() {
  iotc_utest_timed_task_last_context_handle = -1;
  iotc_utest_timed_task_last_timed_task_handle = -1;
  iotc_utest_timed_task_user_data = NULL;
}

void iotc_utest_timed_task_callback(
    const iotc_context_handle_t context_handle,
    const iotc_timed_task_handle_t timed_task_handle, void* user_data) {
  iotc_utest_timed_task_last_context_handle = context_handle;
  iotc_utest_timed_task_last_timed_task_handle = timed_task_handle;
  iotc_utest_timed_task_user_data = user_data;
}

void iotc_utest_timed_task_callback_remove_timed_task(
    const iotc_context_handle_t context_handle,
    const iotc_timed_task_handle_t timed_task_handle, void* user_data) {
  iotc_timed_task_container_t* container =
      (iotc_timed_task_container_t*)user_data;
  iotc_remove_timed_task(container, timed_task_handle);
  iotc_utest_timed_task_last_context_handle = context_handle;
  iotc_utest_timed_task_last_timed_task_handle = timed_task_handle;
  iotc_utest_timed_task_user_data = user_data;
}

#endif

IOTC_TT_TESTGROUP_BEGIN(utest_timed_task)

// ------------------------------ SINGLE TIMED TASK TESTS
// ------------------------------
// //

IOTC_TT_TESTCASE(utest__iotc_make_timed_task_container__ctor_dtor, {
  iotc_timed_task_container_t* container = iotc_make_timed_task_container();
  tt_want_ptr_op(NULL, !=, container);
  iotc_destroy_timed_task_container(container);
  tt_int_op(iotc_is_whole_memory_deallocated(), >, 0);
end:;
})

IOTC_TT_TESTCASE(utest__add_timed_task__returns_valid_handle, {
  iotc_evtd_instance_t* dispatcher = iotc_evtd_create_instance();
  iotc_timed_task_container_t* container = iotc_make_timed_task_container();

  iotc_context_handle_t context_handle = 1;

  iotc_timed_task_handle_t task_handle =
      iotc_add_timed_task(container, dispatcher, context_handle,
                          &iotc_utest_timed_task_callback, 0, 0, NULL);
  tt_want_int_op(task_handle, >=, 0);

  iotc_remove_timed_task(container, task_handle);
  iotc_destroy_timed_task_container(container);
  iotc_evtd_destroy_instance(dispatcher);
  tt_int_op(iotc_is_whole_memory_deallocated(), >, 0);
end:;
})

IOTC_TT_TESTCASE(utest__add_timed_task__callback_called, {
  iotc_utest_timed_task_reset_globals();
  iotc_evtd_instance_t* dispatcher = iotc_evtd_create_instance();
  iotc_timed_task_container_t* container = iotc_make_timed_task_container();

  iotc_context_handle_t context_handle = 1;
  void* user_data = (void*)(intptr_t)2;

  iotc_timed_task_handle_t task_handle =
      iotc_add_timed_task(container, dispatcher, context_handle,
                          &iotc_utest_timed_task_callback, 0, 0, user_data);

  tt_want_int_op(context_handle, !=, iotc_utest_timed_task_last_context_handle);
  tt_want_int_op(task_handle, !=, iotc_utest_timed_task_last_timed_task_handle);
  tt_want_ptr_op(user_data, !=, iotc_utest_timed_task_user_data);

  iotc_evtd_step(dispatcher, 1);

  tt_want_int_op(context_handle, ==, iotc_utest_timed_task_last_context_handle);
  tt_want_int_op(task_handle, ==, iotc_utest_timed_task_last_timed_task_handle);
  tt_want_ptr_op(user_data, ==, iotc_utest_timed_task_user_data);

  iotc_destroy_timed_task_container(container);
  iotc_evtd_destroy_instance(dispatcher);
  tt_int_op(iotc_is_whole_memory_deallocated(), >, 0);
end:;
})

IOTC_TT_TESTCASE(
    utest__remove_timed_task__called_after_fired_callback_wont_break, {
      iotc_utest_timed_task_reset_globals();
      iotc_evtd_instance_t* dispatcher = iotc_evtd_create_instance();
      iotc_timed_task_container_t* container = iotc_make_timed_task_container();

      iotc_context_handle_t context_handle = 1;
      void* user_data = (void*)(intptr_t)2;

      iotc_timed_task_handle_t task_handle =
          iotc_add_timed_task(container, dispatcher, context_handle,
                              &iotc_utest_timed_task_callback, 0, 0, user_data);

      tt_want_int_op(context_handle, !=,
                     iotc_utest_timed_task_last_context_handle);
      tt_want_int_op(task_handle, !=,
                     iotc_utest_timed_task_last_timed_task_handle);
      tt_want_ptr_op(user_data, !=, iotc_utest_timed_task_user_data);

      iotc_evtd_step(dispatcher, 1);
      iotc_remove_timed_task(container, task_handle);

      tt_want_int_op(context_handle, ==,
                     iotc_utest_timed_task_last_context_handle);
      tt_want_int_op(task_handle, ==,
                     iotc_utest_timed_task_last_timed_task_handle);
      tt_want_ptr_op(user_data, ==, iotc_utest_timed_task_user_data);

      iotc_destroy_timed_task_container(container);
      iotc_evtd_destroy_instance(dispatcher);
      tt_int_op(iotc_is_whole_memory_deallocated(), >, 0);
    end:;
    })

IOTC_TT_TESTCASE(utest__remove_timed_task__wont_call_callback, {
  iotc_utest_timed_task_reset_globals();
  iotc_evtd_instance_t* dispatcher = iotc_evtd_create_instance();
  iotc_timed_task_container_t* container = iotc_make_timed_task_container();

  iotc_context_handle_t context_handle = 1;
  void* user_data = (void*)(intptr_t)2;

  iotc_timed_task_handle_t task_handle =
      iotc_add_timed_task(container, dispatcher, context_handle,
                          &iotc_utest_timed_task_callback, 0, 0, user_data);

  tt_want_int_op(context_handle, !=, iotc_utest_timed_task_last_context_handle);
  tt_want_int_op(task_handle, !=, iotc_utest_timed_task_last_timed_task_handle);
  tt_want_ptr_op(user_data, !=, iotc_utest_timed_task_user_data);

  iotc_remove_timed_task(container, task_handle);
  iotc_evtd_step(dispatcher, 1);

  tt_want_int_op(context_handle, !=, iotc_utest_timed_task_last_context_handle);
  tt_want_int_op(task_handle, !=, iotc_utest_timed_task_last_timed_task_handle);
  tt_want_ptr_op(user_data, !=, iotc_utest_timed_task_user_data);

  iotc_destroy_timed_task_container(container);
  iotc_evtd_destroy_instance(dispatcher);
  tt_int_op(iotc_is_whole_memory_deallocated(), >, 0);
end:;
})

IOTC_TT_TESTCASE(
    utest__remove_timed_task__one_invocation_only_removes_one_task, {
      iotc_utest_timed_task_reset_globals();
      iotc_evtd_instance_t* dispatcher = iotc_evtd_create_instance();
      iotc_timed_task_container_t* container = iotc_make_timed_task_container();

      iotc_context_handle_t context_handle_1 = 1;
      void* user_data_1 = (void*)(intptr_t)2;
      iotc_timed_task_handle_t task_handle_1 = iotc_add_timed_task(
          container, dispatcher, context_handle_1,
          &iotc_utest_timed_task_callback, 0, 0, user_data_1);

      iotc_context_handle_t context_handle_2 = 3;
      void* user_data_2 = (void*)(intptr_t)4;
      iotc_timed_task_handle_t task_handle_2 = iotc_add_timed_task(
          container, dispatcher, context_handle_2,
          &iotc_utest_timed_task_callback, 0, 0, user_data_2);

      tt_want_int_op(context_handle_1, !=,
                     iotc_utest_timed_task_last_context_handle);
      tt_want_int_op(task_handle_1, !=,
                     iotc_utest_timed_task_last_timed_task_handle);
      tt_want_ptr_op(user_data_1, !=, iotc_utest_timed_task_user_data);
      tt_want_int_op(context_handle_2, !=,
                     iotc_utest_timed_task_last_context_handle);
      tt_want_int_op(task_handle_2, !=,
                     iotc_utest_timed_task_last_timed_task_handle);
      tt_want_ptr_op(user_data_2, !=, iotc_utest_timed_task_user_data);

      iotc_remove_timed_task(container, task_handle_1);
      iotc_evtd_step(dispatcher, 1);

      tt_want_int_op(context_handle_2, ==,
                     iotc_utest_timed_task_last_context_handle);
      tt_want_int_op(task_handle_2, ==,
                     iotc_utest_timed_task_last_timed_task_handle);
      tt_want_ptr_op(user_data_2, ==, iotc_utest_timed_task_user_data);

      iotc_destroy_timed_task_container(container);
      iotc_evtd_destroy_instance(dispatcher);
      tt_int_op(iotc_is_whole_memory_deallocated(), >, 0);
    end:;
    })

// ------------------------------ RECURRING TIMED TASK TESTS
// ------------------------------ //

IOTC_TT_TESTCASE(
    utest__recurring_timed_task__check_recurring_callbacks_by_1_timehop, {
      iotc_utest_timed_task_reset_globals();
      iotc_evtd_instance_t* dispatcher = iotc_evtd_create_instance();
      iotc_timed_task_container_t* container = iotc_make_timed_task_container();

      iotc_context_handle_t context_handle = 1;

      void* user_data = (void*)(intptr_t)0x4242;
      iotc_timed_task_handle_t task_handle =
          iotc_add_timed_task(container, dispatcher, context_handle,
                              &iotc_utest_timed_task_callback, 1, 1, user_data);

      tt_want_int_op(context_handle, !=,
                     iotc_utest_timed_task_last_context_handle);
      tt_want_int_op(task_handle, !=,
                     iotc_utest_timed_task_last_timed_task_handle);
      tt_want_ptr_op(user_data, !=, iotc_utest_timed_task_user_data);

      iotc_evtd_step(dispatcher, 0);

      tt_want_int_op(context_handle, !=,
                     iotc_utest_timed_task_last_context_handle);
      tt_want_int_op(task_handle, !=,
                     iotc_utest_timed_task_last_timed_task_handle);
      tt_want_ptr_op(user_data, !=, iotc_utest_timed_task_user_data);

      iotc_evtd_step(dispatcher, 1);

      tt_want_int_op(context_handle, ==,
                     iotc_utest_timed_task_last_context_handle);
      tt_want_int_op(task_handle, ==,
                     iotc_utest_timed_task_last_timed_task_handle);
      tt_want_ptr_op(user_data, ==, iotc_utest_timed_task_user_data);

      iotc_utest_timed_task_reset_globals();

      tt_want_int_op(context_handle, !=,
                     iotc_utest_timed_task_last_context_handle);
      tt_want_int_op(task_handle, !=,
                     iotc_utest_timed_task_last_timed_task_handle);
      tt_want_ptr_op(user_data, !=, iotc_utest_timed_task_user_data);

      iotc_evtd_step(dispatcher, 2);

      tt_want_int_op(context_handle, ==,
                     iotc_utest_timed_task_last_context_handle);
      tt_want_int_op(task_handle, ==,
                     iotc_utest_timed_task_last_timed_task_handle);
      tt_want_ptr_op(user_data, ==, iotc_utest_timed_task_user_data);

      iotc_remove_timed_task(container, task_handle);
      iotc_destroy_timed_task_container(container);
      iotc_evtd_destroy_instance(dispatcher);
      tt_int_op(iotc_is_whole_memory_deallocated(), >, 0);
    end:;
    })

IOTC_TT_TESTCASE(
    utest__recurring_timed_task__check_recurring_callbacks_by_2_timehop, {
      iotc_utest_timed_task_reset_globals();
      iotc_evtd_instance_t* dispatcher = iotc_evtd_create_instance();
      iotc_timed_task_container_t* container = iotc_make_timed_task_container();

      iotc_context_handle_t context_handle = 1;

      void* user_data = (void*)(intptr_t)0x4242;
      iotc_timed_task_handle_t task_handle =
          iotc_add_timed_task(container, dispatcher, context_handle,
                              &iotc_utest_timed_task_callback, 2, 1, user_data);

      tt_want_int_op(context_handle, !=,
                     iotc_utest_timed_task_last_context_handle);
      tt_want_int_op(task_handle, !=,
                     iotc_utest_timed_task_last_timed_task_handle);
      tt_want_ptr_op(user_data, !=, iotc_utest_timed_task_user_data);

      iotc_evtd_step(dispatcher, 0);

      tt_want_int_op(context_handle, !=,
                     iotc_utest_timed_task_last_context_handle);
      tt_want_int_op(task_handle, !=,
                     iotc_utest_timed_task_last_timed_task_handle);
      tt_want_ptr_op(user_data, !=, iotc_utest_timed_task_user_data);

      iotc_evtd_step(dispatcher, 1);

      tt_want_int_op(context_handle, !=,
                     iotc_utest_timed_task_last_context_handle);
      tt_want_int_op(task_handle, !=,
                     iotc_utest_timed_task_last_timed_task_handle);
      tt_want_ptr_op(user_data, !=, iotc_utest_timed_task_user_data);

      iotc_evtd_step(dispatcher, 2);

      tt_want_int_op(context_handle, ==,
                     iotc_utest_timed_task_last_context_handle);
      tt_want_int_op(task_handle, ==,
                     iotc_utest_timed_task_last_timed_task_handle);
      tt_want_ptr_op(user_data, ==, iotc_utest_timed_task_user_data);

      iotc_utest_timed_task_reset_globals();

      tt_want_int_op(context_handle, !=,
                     iotc_utest_timed_task_last_context_handle);
      tt_want_int_op(task_handle, !=,
                     iotc_utest_timed_task_last_timed_task_handle);
      tt_want_ptr_op(user_data, !=, iotc_utest_timed_task_user_data);

      iotc_evtd_step(dispatcher, 3);

      tt_want_int_op(context_handle, !=,
                     iotc_utest_timed_task_last_context_handle);
      tt_want_int_op(task_handle, !=,
                     iotc_utest_timed_task_last_timed_task_handle);
      tt_want_ptr_op(user_data, !=, iotc_utest_timed_task_user_data);

      iotc_evtd_step(dispatcher, 4);

      tt_want_int_op(context_handle, ==,
                     iotc_utest_timed_task_last_context_handle);
      tt_want_int_op(task_handle, ==,
                     iotc_utest_timed_task_last_timed_task_handle);
      tt_want_ptr_op(user_data, ==, iotc_utest_timed_task_user_data);

      iotc_remove_timed_task(container, task_handle);
      iotc_destroy_timed_task_container(container);
      iotc_evtd_destroy_instance(dispatcher);
      tt_int_op(iotc_is_whole_memory_deallocated(), >, 0);
    end:;
    })

IOTC_TT_TESTCASE(
    utest__remove_recurring_timed_task__wont_call_callback_after_remove, {
      iotc_utest_timed_task_reset_globals();
      iotc_evtd_instance_t* dispatcher = iotc_evtd_create_instance();
      iotc_timed_task_container_t* container = iotc_make_timed_task_container();

      iotc_context_handle_t context_handle = 1;

      void* user_data = (void*)(intptr_t)0x4242;
      iotc_timed_task_handle_t task_handle =
          iotc_add_timed_task(container, dispatcher, context_handle,
                              &iotc_utest_timed_task_callback, 1, 1, user_data);

      iotc_evtd_step(dispatcher, 0);

      tt_want_int_op(context_handle, !=,
                     iotc_utest_timed_task_last_context_handle);
      tt_want_int_op(task_handle, !=,
                     iotc_utest_timed_task_last_timed_task_handle);
      tt_want_ptr_op(user_data, !=, iotc_utest_timed_task_user_data);

      iotc_evtd_step(dispatcher, 1);

      tt_want_int_op(context_handle, ==,
                     iotc_utest_timed_task_last_context_handle);
      tt_want_int_op(task_handle, ==,
                     iotc_utest_timed_task_last_timed_task_handle);
      tt_want_ptr_op(user_data, ==, iotc_utest_timed_task_user_data);

      iotc_utest_timed_task_reset_globals();

      tt_want_int_op(context_handle, !=,
                     iotc_utest_timed_task_last_context_handle);
      tt_want_int_op(task_handle, !=,
                     iotc_utest_timed_task_last_timed_task_handle);
      tt_want_ptr_op(user_data, !=, iotc_utest_timed_task_user_data);

      iotc_evtd_step(dispatcher, 2);

      tt_want_int_op(context_handle, ==,
                     iotc_utest_timed_task_last_context_handle);
      tt_want_int_op(task_handle, ==,
                     iotc_utest_timed_task_last_timed_task_handle);
      tt_want_ptr_op(user_data, ==, iotc_utest_timed_task_user_data);

      iotc_utest_timed_task_reset_globals();

      tt_want_int_op(context_handle, !=,
                     iotc_utest_timed_task_last_context_handle);
      tt_want_int_op(task_handle, !=,
                     iotc_utest_timed_task_last_timed_task_handle);
      tt_want_ptr_op(user_data, !=, iotc_utest_timed_task_user_data);

      iotc_remove_timed_task(container, task_handle);
      iotc_evtd_step(dispatcher, 3);

      tt_want_int_op(context_handle, !=,
                     iotc_utest_timed_task_last_context_handle);
      tt_want_int_op(task_handle, !=,
                     iotc_utest_timed_task_last_timed_task_handle);
      tt_want_ptr_op(user_data, !=, iotc_utest_timed_task_user_data);

      iotc_remove_timed_task(container, task_handle);
      iotc_destroy_timed_task_container(container);
      iotc_evtd_destroy_instance(dispatcher);
      tt_int_op(iotc_is_whole_memory_deallocated(), >, 0);
    end:;
    })

IOTC_TT_TESTCASE(
    utest__remove_timed_task_inside_the_callback__wont_call_callback_after_remove,
    {
      iotc_utest_timed_task_reset_globals();
      iotc_evtd_instance_t* dispatcher = iotc_evtd_create_instance();
      iotc_timed_task_container_t* container = iotc_make_timed_task_container();

      iotc_context_handle_t context_handle = 1;

      void* user_data = (void*)container;
      iotc_timed_task_handle_t task_handle = iotc_add_timed_task(
          container, dispatcher, context_handle,
          &iotc_utest_timed_task_callback_remove_timed_task, 1, 1, user_data);

      iotc_evtd_step(dispatcher, 0);

      tt_want_int_op(context_handle, !=,
                     iotc_utest_timed_task_last_context_handle);
      tt_want_int_op(task_handle, !=,
                     iotc_utest_timed_task_last_timed_task_handle);
      tt_want_ptr_op(user_data, !=, iotc_utest_timed_task_user_data);

      iotc_evtd_step(dispatcher, 1);

      tt_want_int_op(context_handle, ==,
                     iotc_utest_timed_task_last_context_handle);
      tt_want_int_op(task_handle, ==,
                     iotc_utest_timed_task_last_timed_task_handle);
      tt_want_ptr_op(user_data, ==, iotc_utest_timed_task_user_data);

      iotc_utest_timed_task_reset_globals();

      tt_want_int_op(context_handle, !=,
                     iotc_utest_timed_task_last_context_handle);
      tt_want_int_op(task_handle, !=,
                     iotc_utest_timed_task_last_timed_task_handle);
      tt_want_ptr_op(user_data, !=, iotc_utest_timed_task_user_data);

      iotc_evtd_step(dispatcher, 2);

      tt_want_int_op(context_handle, !=,
                     iotc_utest_timed_task_last_context_handle);
      tt_want_int_op(task_handle, !=,
                     iotc_utest_timed_task_last_timed_task_handle);
      tt_want_ptr_op(user_data, !=, iotc_utest_timed_task_user_data);

      iotc_remove_timed_task(container, task_handle);
      iotc_destroy_timed_task_container(container);
      iotc_evtd_destroy_instance(dispatcher);
      tt_int_op(iotc_is_whole_memory_deallocated(), >, 0);
    end:;
    })

IOTC_TT_TESTGROUP_END

#ifndef IOTC_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#define IOTC_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#include __FILE__
#undef IOTC_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#endif
