/* Copyright 2018 - 2019 Google LLC
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

/*
 * Note:  These tests make no demands on platform specific code and are
 * therefore useful for initial testing on a cross-compiled embedded
 * target for the purpose of demonstrate usable linkage to the IoTC client
 * library and the functionality of the ported tinytest framework.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "tinytest.h"
#include "tinytest_macros.h"

#include "iotc_helpers.h"
#include "iotc_tt_testcase_management.h"

IOTC_TT_TESTGROUP_BEGIN(utest_helper_functions)

IOTC_TT_TESTCASE(utest_helper_bitFilter_boundry_conditions, {
  tt_assert(iotc_highest_bit_filter(0x8000) == 0x8000);
  tt_assert(iotc_highest_bit_filter(0xC000) == 0x8000);
  tt_assert(iotc_highest_bit_filter(0x0800) == 0x0800);
  tt_assert(iotc_highest_bit_filter(0x0C00) == 0x0800);
  tt_assert(iotc_highest_bit_filter(0x0002) == 0x0002);
  tt_assert(iotc_highest_bit_filter(0x0003) == 0x0002);
  tt_assert(iotc_highest_bit_filter(0x8001) == 0x8000);
  tt_assert(iotc_highest_bit_filter(0x0180) == 0x0100);
end:;
})

IOTC_TT_TESTCASE(utest_helper_str_copy_untiln, {
  char buf1[80] = "abcdefghijklmnopqrstuvwxyz0123456789";
  char buf2[80];
  int n;
  memset(buf2, 0x0, sizeof(buf2));
  n = iotc_str_copy_untiln(buf2, sizeof(buf2), buf1, 'k');
  printf("n = %d\n", n);
  tt_assert(n == 10);
  tt_str_op(buf2, ==, "abcdefghij");
end:;
})

IOTC_TT_TESTGROUP_END

#ifndef IOTC_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#define IOTC_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#include __FILE__
#undef IOTC_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#endif
