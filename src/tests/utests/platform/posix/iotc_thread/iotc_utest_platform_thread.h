/* Copyright 2018-2019 Google LLC
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

#include "iotc_critical_section_def.h"

IOTC_TT_TESTCASE(
    utest__posix__iotc_create_critical_section__valid_input__critical_section_state_eq_zero,
    {
      struct iotc_critical_section_s* cs = 0;
      iotc_init_critical_section(&cs);

      tt_want_int_op(cs->cs_state, ==, 0);

      iotc_destroy_critical_section(&cs);
    })

IOTC_TT_TESTCASE(
    utest__posix__iotc_lock_critical_section__valid_input__critical_section_state_eq_one,
    {
      struct iotc_critical_section_s* cs = 0;
      iotc_init_critical_section(&cs);

      iotc_lock_critical_section(cs);

      tt_want_int_op(cs->cs_state, ==, 1);

      iotc_unlock_critical_section(cs);
      iotc_destroy_critical_section(&cs);
    })

IOTC_TT_TESTCASE(
    utest__posix__iotc_lock_critical_section__valid_input__critical_section_state_eq_zero,
    {
      struct iotc_critical_section_s* cs = 0;
      iotc_init_critical_section(&cs);

      iotc_lock_critical_section(cs);
      iotc_unlock_critical_section(cs);

      tt_want_int_op(cs->cs_state, ==, 0);

      iotc_destroy_critical_section(&cs);
    })
