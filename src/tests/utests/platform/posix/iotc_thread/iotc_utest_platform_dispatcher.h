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

#include "tinytest.h"
#include "tinytest_macros.h"

#include "iotc_event_dispatcher_api.h"

#include "iotc_critical_section_def.h"

#ifndef IOTC_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN

iotc_state_t register_evtd_handle(iotc_event_handle_arg1_t a) {
  tt_want_int_op(evtd_g_i->cs->cs_state, ==, 0);

  iotc_evtd_instance_t* instance = (iotc_evtd_instance_t*)a;

  evtd_handle_g.handle_type = IOTC_EVENT_HANDLE_ARGC1;
  evtd_handle_g.handlers.h1.fn_argc1 = &register_evtd_handle;
  evtd_handle_g.handlers.h1.a1 = a;

  iotc_evtd_register_socket_fd(instance, 10, evtd_handle_g);

  tt_want_int_op(evtd_g_i->cs->cs_state, ==, 0);

  return IOTC_STATE_OK;
}

iotc_state_t stop_evtd_handle(iotc_event_handle_arg1_t a) {
  tt_want_int_op(evtd_g_i->cs->cs_state, ==, 0);

  iotc_evtd_instance_t* instance = (iotc_evtd_instance_t*)a;

  IOTC_UNUSED(instance);

  return IOTC_MQTT_BAD_USERNAME_OR_PASSWORD;
}

iotc_state_t proc_loop_thread(iotc_event_handle_arg1_t a) {
  tt_want_int_op(evtd_g_i->cs->cs_state, ==, 0);

  *((uint32_t*)a) -= 1;

  if (*((uint32_t*)a) > 0) {
    iotc_evtd_execute_in(evtd_g_i, evtd_handle_g, 1, NULL);
  }

  tt_want_int_op(evtd_g_i->cs->cs_state, ==, 0);

  return 0;
}

#endif

IOTC_TT_TESTCASE(utest__thread_safety_clash__entities_must_not_clash, {
  evtd_g_i = iotc_evtd_create_instance();

  uint32_t counter = 10;
  iotc_time_t step = 0;

  evtd_handle_g.handle_type = IOTC_EVENT_HANDLE_ARGC1;
  evtd_handle_g.handlers.h1.fn_argc1 = &proc_loop_thread;
  evtd_handle_g.handlers.h1.a1 = (iotc_event_handle_arg1_t)&counter;

  iotc_evtd_execute_in(evtd_g_i, evtd_handle_g, 0, NULL);

  iotc_event_handle_t evtd_handle = {
      IOTC_EVENT_HANDLE_ARGC1,
      .handlers.h1 = {&register_evtd_handle,
                      (iotc_event_handle_arg1_t)&evtd_g_i}};
  tt_assert(iotc_evtd_register_socket_fd(evtd_g_i, 12, evtd_handle) != 0);

  tt_want_int_op(evtd_g_i->cs->cs_state, ==, 0);
  iotc_evtd_continue_when_evt_on_socket(evtd_g_i, IOTC_EVENT_ERROR, evtd_handle,
                                        12);
  tt_want_int_op(evtd_g_i->cs->cs_state, ==, 0);

  while (evtd_g_i->time_events_container->elem_no > 0) {
    tt_want_int_op(evtd_g_i->cs->cs_state, ==, 0);
    iotc_evtd_continue_when_evt_on_socket(evtd_g_i, IOTC_EVENT_WANT_READ,
                                          evtd_handle, 12);
    tt_want_int_op(evtd_g_i->cs->cs_state, ==, 0);

    iotc_evtd_step(evtd_g_i, step);
    step += 1;
    tt_assert(counter == 10 - step);
  }

  tt_want_int_op(evtd_g_i->cs->cs_state, ==, 0);
  tt_assert(counter == 0);
  tt_assert(step == 10);
  iotc_evtd_unregister_socket_fd(evtd_g_i, 12);

end:
  iotc_evtd_destroy_instance(evtd_g_i);
})

IOTC_TT_TESTCASE(
    utest__thread_safety_clash_time_handler_on_stop__entities_must_not_clash, {
      evtd_g_i = iotc_evtd_create_instance();

      evtd_handle_g.handle_type = IOTC_EVENT_HANDLE_ARGC1;
      evtd_handle_g.handlers.h1.fn_argc1 = &stop_evtd_handle;
      evtd_handle_g.handlers.h1.a1 = (iotc_event_handle_arg1_t)&evtd_g_i;

      iotc_evtd_execute_in(evtd_g_i, evtd_handle_g, 0, NULL);

      iotc_evtd_step(evtd_g_i, 1);

      tt_want_int_op(evtd_g_i->cs->cs_state, ==, 0);

      iotc_evtd_destroy_instance(evtd_g_i);
    })

IOTC_TT_TESTCASE(
    utest__thread_safety_clash_event_handler_on_stop__entities_must_not_clash, {
      evtd_g_i = iotc_evtd_create_instance();

      iotc_event_handle_t evtd_handle = {
          IOTC_EVENT_HANDLE_ARGC1,
          .handlers.h1 = {&stop_evtd_handle,
                          (iotc_event_handle_arg1_t)&evtd_g_i}};
      tt_assert(iotc_evtd_register_socket_fd(evtd_g_i, 12, evtd_handle) != 0);

      tt_want_int_op(evtd_g_i->cs->cs_state, ==, 0);
      iotc_evtd_continue_when_evt_on_socket(evtd_g_i, IOTC_EVENT_WANT_READ,
                                            evtd_handle, 12);
      tt_want_int_op(evtd_g_i->cs->cs_state, ==, 0);

      iotc_evtd_unregister_socket_fd(evtd_g_i, 12);
      iotc_evtd_destroy_instance(evtd_g_i);

    end:;
    })
