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

#include "iotc.h"
#include "iotc_critical_section.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifndef IOTC_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN

#endif  // IOTC_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN

IOTC_TT_TESTGROUP_BEGIN(utest_thread)

#include "iotc_utest_platform_thread.h"

IOTC_TT_TESTCASE(
    utest__iotc_init_critical_section_valid_input__create_critical_section, {
      struct iotc_critical_section_s* cs = 0;
      iotc_state_t state = iotc_init_critical_section(&cs);

      tt_want_int_op(IOTC_STATE_OK, ==, state);
      tt_want_ptr_op(NULL, !=, cs);

      iotc_destroy_critical_section(&cs);
    })

IOTC_TT_TESTCASE(
    utest__iotc_destroy_critical_section_valid_input__critical_section_freed_nullyfied,
    {
      struct iotc_critical_section_s* cs = 0;
      iotc_init_critical_section(&cs);

      iotc_destroy_critical_section(&cs);

      tt_want_ptr_op(NULL, ==, cs);
    })

IOTC_TT_TESTGROUP_END

#ifndef IOTC_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#define IOTC_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#include __FILE__
#undef IOTC_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#endif  // IOTC_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
