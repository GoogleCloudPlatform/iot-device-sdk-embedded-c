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

#include "iotc_tt_testcase_management.h"
#include "iotc_utest_basic_testcase_frame.h"
#include "tinytest.h"
#include "tinytest_macros.h"

#include "iotc.h"
#include "iotc_err.h"
#include "iotc_helpers.h"
#include "iotc_memory_checks.h"
#include "iotc_types_internal.h"
#include "iotc_version.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifndef IOTC_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN

iotc_state_t h0(void) {
  tt_assert(0 == 0);
end:
  return 0;
}

iotc_state_t h1(iotc_event_handle_arg1_t a1) {
  tt_assert(a1 == 0);
end:
  return 0;
}

iotc_state_t h2(iotc_event_handle_arg1_t a1, iotc_event_handle_arg2_t a2) {
  tt_assert(a1 == 0);
  tt_assert(*((int*)a2) == 22);
end:
  return 0;
}

iotc_state_t h3(iotc_event_handle_arg1_t a1, iotc_event_handle_arg2_t a2,
                iotc_event_handle_arg3_t a3) {
  tt_assert(a1 == 0);
  tt_assert(*((int*)a2) == 222);
  tt_assert(a3 == 0);
end:
  return 0;
}

iotc_state_t h4(iotc_event_handle_arg1_t a1, iotc_event_handle_arg2_t a2,
                iotc_event_handle_arg3_t a3, iotc_event_handle_arg4_t a4) {
  tt_assert(a1 == 0);
  tt_assert(*((int*)a2) == 2222);
  tt_assert(a3 == 0);
  tt_assert(*((int*)a4) == 4444);
end:
  return 0;
}

iotc_state_t h5(iotc_event_handle_arg1_t a1, iotc_event_handle_arg2_t a2,
                iotc_event_handle_arg3_t a3, iotc_event_handle_arg4_t a4,
                iotc_event_handle_arg5_t a5) {
  tt_assert(a1 == 0);
  tt_assert(*((int*)a2) == 22222);
  tt_assert(a3 == 0);
  tt_assert(*((int*)a4) == 44444);
  tt_assert(*((int*)a5) == 55555);
end:
  return 0;
}

iotc_state_t h6(iotc_event_handle_arg1_t a1, iotc_event_handle_arg2_t a2,
                iotc_event_handle_arg3_t a3, iotc_event_handle_arg4_t a4,
                iotc_event_handle_arg5_t a5, iotc_event_handle_arg6_t a6) {
  tt_assert(a1 == 0);
  tt_assert(*((int*)a2) == 222222);
  tt_assert(a3 == 0);
  tt_assert(*((int*)a4) == 444444);
  tt_assert(*((int*)a5) == 555555);
  tt_assert(*((int*)a6) == 666666);
end:
  return 0;
}

#endif

IOTC_TT_TESTGROUP_BEGIN(utest_core)

IOTC_TT_TESTCASE(test_make_handles, {
  iotc_event_handle_t eh;

  int val_a2 = 22;
  iotc_event_handle_arg2_t a2 = &val_a2;
  int val_a4 = 4444;
  iotc_event_handle_arg4_t a4 = &val_a4;
  int val_a5 = 55555;
  iotc_event_handle_arg5_t a5 = &val_a5;
  int val_a6 = 666666;
  iotc_event_handle_arg6_t a6 = &val_a6;

  eh = iotc_make_handle(h0);
  tt_assert(eh.handle_type == IOTC_EVENT_HANDLE_ARGC0);
  iotc_evtd_execute_handle(&eh);

  eh = iotc_make_handle(h1, 0);
  tt_assert(eh.handle_type == IOTC_EVENT_HANDLE_ARGC1);
  iotc_evtd_execute_handle(&eh);

  eh = iotc_make_handle(h2, 0, a2);
  tt_assert(eh.handle_type == IOTC_EVENT_HANDLE_ARGC2);
  iotc_evtd_execute_handle(&eh);

  val_a2 += 200;

  eh = iotc_make_handle(h3, 0, a2, 0);
  tt_assert(eh.handle_type == IOTC_EVENT_HANDLE_ARGC3);
  iotc_evtd_execute_handle(&eh);

  val_a2 += 2000;

  eh = iotc_make_handle(h4, 0, a2, 0, a4);
  tt_assert(eh.handle_type == IOTC_EVENT_HANDLE_ARGC4);
  iotc_evtd_execute_handle(&eh);

  val_a2 += 20000;
  val_a4 += 40000;

  eh = iotc_make_handle(h5, 0, a2, 0, a4, a5);
  tt_assert(eh.handle_type == IOTC_EVENT_HANDLE_ARGC5);
  iotc_evtd_execute_handle(&eh);

  val_a2 += 200000;
  val_a4 += 400000;
  val_a5 += 500000;

  eh = iotc_make_handle(h6, 0, a2, 0, a4, a5, a6);
  tt_assert(eh.handle_type == IOTC_EVENT_HANDLE_ARGC6);
  iotc_evtd_execute_handle(&eh);

end:;
})

IOTC_TT_TESTGROUP_END

#ifndef IOTC_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#define IOTC_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#include __FILE__
#undef IOTC_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#endif
