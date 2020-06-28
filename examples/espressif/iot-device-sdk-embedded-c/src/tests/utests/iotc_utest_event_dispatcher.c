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

#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "iotc_memory_checks.h"
#include "iotc_tt_testcase_management.h"
#include "tinytest.h"
#include "tinytest_macros.h"

#include "iotc_event_dispatcher_api.h"

#ifndef IOTC_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN

static uint32_t g_cont0_test = 0;

iotc_state_t continuation(void) {
  g_cont0_test = 127;
  return 0;
}

iotc_state_t continuation1(iotc_event_handle_arg1_t a) {
  *((uint32_t*)a) = 127;
  return 0;
}

iotc_state_t continuation1_1(iotc_event_handle_arg1_t a) {
  *((uint32_t*)a) += 1;
  return 0;
}

iotc_state_t continuation1_3(iotc_event_handle_arg1_t a) {
  *((uint32_t*)a) += 3;
  return 0;
}

iotc_state_t continuation1_5(iotc_event_handle_arg1_t a) {
  *((uint32_t*)a) += 5;
  return 0;
}

static iotc_evtd_instance_t* evtd_g_i = 0;
static iotc_event_handle_t evtd_handle_g;

iotc_state_t proc_loop(iotc_event_handle_arg1_t a) {
  *((uint32_t*)a) -= 1;

  if (*((uint32_t*)a) > 0) {
    iotc_evtd_execute_in(evtd_g_i, evtd_handle_g, 1, NULL);
  }
  return 0;
}

void test_time_overflow_function(void) {
  evtd_g_i = iotc_evtd_create_instance();

  tt_assert(NULL != evtd_g_i);

  /* counter that will be used by this test */
  uint32_t counter = 0;

  const iotc_time_t time_max = LONG_MAX;
  const iotc_time_t time_hole = 10;

  printf("%ld\n", time_max);

  evtd_g_i->current_step = time_max - time_hole;

  /* now let's add some events to be executed */
  /* we will add some events before the MAX( iotc_time_t ) */
  {
    iotc_time_t i = 0;
    for (; i < time_hole; ++i) {
      const iotc_state_t ret_state = iotc_evtd_execute_in(
          evtd_g_i, iotc_make_handle(&continuation1_1, &counter), i, NULL);

      iotc_debug_format("ret_state = %d", ret_state);

      tt_int_op(IOTC_STATE_OK, ==, ret_state);
    }
  }

  /* and some after the MIN( iotc_time_t ) in order to check the reaction on
   * time overlap */
  {
    iotc_time_t i = 0;
    for (; i < time_hole; ++i) {
      const iotc_state_t ret_state = iotc_evtd_execute_in(
          evtd_g_i, iotc_make_handle(&continuation1_1, &counter), time_hole + i,
          NULL);
      tt_int_op(IOTC_STATE_OK, ==, ret_state);
    }
  }

  /* let's execute first half */
  iotc_evtd_step(evtd_g_i, time_max - (time_hole / 2));

  /* expectation is that only the half of the events added before the MAX(
   * iotc_time_t ) will be executed */
  tt_int_op(counter, ==, (time_hole / 2));

  iotc_evtd_destroy_instance(evtd_g_i);

  tt_int_op(iotc_is_whole_memory_deallocated(), >, 0);

  return;
end:;
}

#endif

/*-----------------------------------------------------------------------*/
// DISPATCHER TESTS
/*-----------------------------------------------------------------------*/
IOTC_TT_TESTGROUP_BEGIN(utest_event_dispatcher)

#ifdef IOTC_MODULE_THREAD_ENABLED
#include "iotc_utest_platform_dispatcher.h"
#endif

IOTC_TT_TESTCASE(utest__continuation0, {
  iotc_evtd_instance_t* evtd_i = iotc_evtd_create_instance();

  {
    iotc_event_handle_t evtd_handle = {IOTC_EVENT_HANDLE_ARGC0,
                                       .handlers.h0 = {&continuation}};
    iotc_evtd_execute_handle(&evtd_handle);
  }

  tt_assert(g_cont0_test == 127);

end:
  iotc_evtd_destroy_instance(evtd_i);
})

IOTC_TT_TESTCASE(utest__continuation1, {
  iotc_evtd_instance_t* evtd_i = iotc_evtd_create_instance();

  uint32_t counter = 0;

  {
    iotc_event_handle_t evtd_handle = {
        IOTC_EVENT_HANDLE_ARGC1,
        .handlers.h1 = {&continuation1, (iotc_event_handle_arg1_t)&counter}};
    iotc_evtd_execute_handle(&evtd_handle);
  }

  tt_assert(counter == 127);

end:
  iotc_evtd_destroy_instance(evtd_i);
})

IOTC_TT_TESTCASE(utest__handler_processing_loop, {
  evtd_g_i = iotc_evtd_create_instance();

  uint32_t counter = 10;
  iotc_time_t step = 0;

  evtd_handle_g.handle_type = IOTC_EVENT_HANDLE_ARGC1;
  evtd_handle_g.handlers.h1.fn_argc1 = &proc_loop;
  evtd_handle_g.handlers.h1.a1 = (iotc_event_handle_arg1_t)&counter;

  iotc_evtd_execute_in(evtd_g_i, evtd_handle_g, 0, NULL);

  while (evtd_g_i->time_events_container->elem_no > 0) {
    iotc_evtd_step(evtd_g_i, step);
    step += 1;
    tt_assert(counter == 10u - step);
  }

  tt_assert(counter == 0);
  tt_assert(step == 10);

end:
  iotc_evtd_destroy_instance(evtd_g_i);
})

IOTC_TT_TESTCASE(utest__register_fd, {
  evtd_g_i = iotc_evtd_create_instance();

  iotc_event_handle_t handle = iotc_make_empty_handle();

  tt_assert(iotc_evtd_register_socket_fd(evtd_g_i, 15, handle) != 0);
  {
    iotc_evtd_fd_tuple_t* tmp =
        (iotc_evtd_fd_tuple_t*)evtd_g_i->handles_and_socket_fd->array[0]
            .selector_t.ptr_value;
    tt_assert(tmp->fd == 15);
    tt_assert(tmp->event_type == IOTC_EVENT_WANT_READ);
  }

  tt_assert(iotc_evtd_register_socket_fd(evtd_g_i, 14, handle));
  {
    iotc_evtd_fd_tuple_t* tmp =
        (iotc_evtd_fd_tuple_t*)evtd_g_i->handles_and_socket_fd->array[1]
            .selector_t.ptr_value;
    tt_assert(tmp->fd == 14);
    tt_assert(tmp->event_type == IOTC_EVENT_WANT_READ);
  }

  tt_assert(iotc_evtd_register_socket_fd(evtd_g_i, 12, handle));
  {
    iotc_evtd_fd_tuple_t* tmp =
        (iotc_evtd_fd_tuple_t*)evtd_g_i->handles_and_socket_fd->array[2]
            .selector_t.ptr_value;
    tt_assert(tmp->fd == 12);
    tt_assert(tmp->event_type == IOTC_EVENT_WANT_READ);
  }

  iotc_evtd_unregister_socket_fd(evtd_g_i, 12);
  iotc_evtd_unregister_socket_fd(evtd_g_i, 15);
  iotc_evtd_unregister_socket_fd(evtd_g_i, 14);

end:
  iotc_evtd_destroy_instance(evtd_g_i);
})

IOTC_TT_TESTCASE(utest__evtd_updates, {
  uint32_t counter = 0;

  evtd_g_i = iotc_evtd_create_instance();

  {
    iotc_event_handle_t evtd_handle = {
        IOTC_EVENT_HANDLE_ARGC1,
        .handlers.h1 = {&continuation1_1, (iotc_event_handle_arg1_t)&counter}};

    tt_assert(iotc_evtd_register_socket_fd(evtd_g_i, 15, evtd_handle) != 0);
    tt_assert(iotc_evtd_register_socket_fd(evtd_g_i, 14, evtd_handle) != 0);
    tt_assert(iotc_evtd_register_socket_fd(evtd_g_i, 12, evtd_handle) != 0);
  }

  {
    iotc_event_handle_t evtd_handle = {
        IOTC_EVENT_HANDLE_ARGC1,
        .handlers.h1 = {&continuation1_1, (iotc_event_handle_arg1_t)&counter}};
    iotc_evtd_continue_when_evt_on_socket(evtd_g_i, IOTC_EVENT_WANT_READ,
                                          evtd_handle, 15);

    iotc_evtd_fd_tuple_t* tmp =
        (iotc_evtd_fd_tuple_t*)evtd_g_i->handles_and_socket_fd->array[0]
            .selector_t.ptr_value;

    tt_assert(tmp->event_type == IOTC_EVENT_WANT_READ);
    tt_assert(tmp->handle.handle_type == IOTC_EVENT_HANDLE_ARGC1);
    tt_assert(tmp->handle.handlers.h1.a1 == (iotc_event_handle_arg1_t)&counter);
    tt_assert(tmp->handle.handlers.h1.fn_argc1 == &continuation1_1);
  }

  {
    iotc_event_handle_t evtd_handle = {
        IOTC_EVENT_HANDLE_ARGC1,
        .handlers.h1 = {&continuation1_3, (iotc_event_handle_arg1_t)&counter}};
    iotc_evtd_continue_when_evt_on_socket(evtd_g_i, IOTC_EVENT_WANT_WRITE,
                                          evtd_handle, 14);

    iotc_evtd_fd_tuple_t* tmp =
        (iotc_evtd_fd_tuple_t*)evtd_g_i->handles_and_socket_fd->array[1]
            .selector_t.ptr_value;

    tt_assert(tmp->event_type == IOTC_EVENT_WANT_WRITE);
    tt_assert(tmp->handle.handle_type == IOTC_EVENT_HANDLE_ARGC1);
    tt_assert(tmp->handle.handlers.h1.a1 == (iotc_event_handle_arg1_t)&counter);
    tt_assert(tmp->handle.handlers.h1.fn_argc1 == &continuation1_3);
  }

  {
    iotc_event_handle_t evtd_handle = {
        IOTC_EVENT_HANDLE_ARGC1,
        .handlers.h1 = {&continuation1_5, (iotc_event_handle_arg1_t)&counter}};
    iotc_evtd_continue_when_evt_on_socket(evtd_g_i, IOTC_EVENT_ERROR,
                                          evtd_handle, 12);

    iotc_evtd_fd_tuple_t* tmp =
        (iotc_evtd_fd_tuple_t*)evtd_g_i->handles_and_socket_fd->array[2]
            .selector_t.ptr_value;

    tt_assert(tmp->event_type == IOTC_EVENT_ERROR);
    tt_assert(tmp->handle.handle_type == IOTC_EVENT_HANDLE_ARGC1);
    tt_assert(tmp->handle.handlers.h1.a1 == (iotc_event_handle_arg1_t)&counter);
    tt_assert(tmp->handle.handlers.h1.fn_argc1 == &continuation1_5);
  }

  tt_assert(counter == 0);
  iotc_evtd_update_event_on_socket(evtd_g_i, 12);
  tt_assert(counter == 5);

  counter = 0;

  {
    iotc_event_handle_t evtd_handle = {
        IOTC_EVENT_HANDLE_ARGC1,
        .handlers.h1 = {&continuation1_5, (iotc_event_handle_arg1_t)&counter}};
    iotc_evtd_continue_when_evt_on_socket(evtd_g_i, IOTC_EVENT_ERROR,
                                          evtd_handle, 12);
  }

  tt_assert(counter == 0);
  iotc_evtd_update_event_on_socket(evtd_g_i, 12);
  iotc_evtd_update_event_on_socket(evtd_g_i, 15);
  tt_assert(counter == 6);

  counter = 0;

  {
    iotc_event_handle_t evtd_handle = {
        IOTC_EVENT_HANDLE_ARGC1,
        .handlers.h1 = {&continuation1_5, (iotc_event_handle_arg1_t)&counter}};
    iotc_evtd_continue_when_evt_on_socket(evtd_g_i, IOTC_EVENT_ERROR,
                                          evtd_handle, 12);
  }
  {
    iotc_event_handle_t evtd_handle = {
        IOTC_EVENT_HANDLE_ARGC1,
        .handlers.h1 = {&continuation1_3, (iotc_event_handle_arg1_t)&counter}};
    iotc_evtd_continue_when_evt_on_socket(evtd_g_i, IOTC_EVENT_WANT_WRITE,
                                          evtd_handle, 14);
  }

  tt_assert(counter == 0);
  iotc_evtd_update_event_on_socket(evtd_g_i, 12);
  iotc_evtd_update_event_on_socket(evtd_g_i, 15);
  iotc_evtd_update_event_on_socket(evtd_g_i, 14);
  tt_assert(counter == 9);

  iotc_evtd_unregister_socket_fd(evtd_g_i, 12);
  iotc_evtd_unregister_socket_fd(evtd_g_i, 15);
  iotc_evtd_unregister_socket_fd(evtd_g_i, 14);

end:
  iotc_evtd_destroy_instance(evtd_g_i);
})

/* skipped because this feature is not yet implemented */
SKIP_IOTC_TT_TESTCASE(
    utest__iotc_evtd__events_to_call_added__overlap_timer__proper_events_executed,
    { test_time_overflow_function(); })

#undef TEST_DATA_SIZE

IOTC_TT_TESTGROUP_END

#ifndef IOTC_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#define IOTC_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#include __FILE__
#undef IOTC_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#endif
