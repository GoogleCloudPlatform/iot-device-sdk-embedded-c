/* Copyright 2021 Google LLC
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

#include "iotc_macros.h"
#include "iotc_tt_testcase_management.h"
#include "tinytest.h"
#include "tinytest_macros.h"

#include "iotc.h"
#include "iotc_allocator.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifndef IOTC_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN

#endif  // IOTC_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN

IOTC_TT_TESTGROUP_BEGIN(utest_memory_calloc)

IOTC_TT_TESTCASE(
    utest__iotc_memory_calloc,
    {
      void* ptr = __iotc_calloc( /*num=*/1, /*size=*/8);
      tt_want_ptr_op(NULL, !=, ptr);
      __iotc_free(ptr);
    })
    
IOTC_TT_TESTCASE(
    utest__iotc_memory_calloc__zero_num,
    {
      void* ptr = __iotc_calloc( /*num=*/0, /*size=*/8);
      tt_want_ptr_op(NULL, ==, ptr);
      __iotc_free(ptr);
    })

IOTC_TT_TESTCASE(
    utest__iotc_memory_calloc__zero_size,
    {
      void* ptr = __iotc_calloc( /*num=*/1, /*size=*/0);
      tt_want_ptr_op(NULL, ==, ptr);
      __iotc_free(ptr);
    })

IOTC_TT_TESTCASE(
    utest__iotc_memory_calloc__zero_num__zero_size,
    {
      void* ptr = __iotc_calloc( /*num=*/1, /*size=*/0);
      tt_want_ptr_op(NULL, ==, ptr);
      __iotc_free(ptr);
    })

IOTC_TT_TESTCASE(
    utest__iotc_memory_calloc__max_num__zero_size,
    {
      void* ptr = __iotc_calloc( /*num=*/SIZE_MAX, /*size=*/0);
      tt_want_ptr_op(NULL, ==, ptr);
      __iotc_free(ptr);
    })

IOTC_TT_TESTCASE(
    utest__iotc_memory_calloc__zero_num__max_size,
    {
      void* ptr = __iotc_calloc( /*num=*/0, /*size=*/SIZE_MAX);
      tt_want_ptr_op(NULL, ==, ptr);
      __iotc_free(ptr);
    })

/* Tests disabled due to varying calloc behaviors across platforms:
   - utest__iotc_memory_calloc_max_num_one_size
   - utest__iotc_memory_calloc_one_num_max_size
   - utest__iotc_memory_calloc__max_num_minus_one__size_one
*/

IOTC_TT_TESTCASE(
    utest__iotc_memory_calloc__overflow__half_max__size_two,
    {
      void* ptr = __iotc_calloc( /*num=*/SIZE_MAX/2, /*size=*/2);
      tt_want_ptr_op(NULL, ==, ptr);
      __iotc_free(ptr);
    })

IOTC_TT_TESTCASE(
    utest__iotc_memory_calloc__overflow__num_two__half_max_size,
    {
      void* ptr = __iotc_calloc( /*num=*/2, /*size=*/SIZE_MAX/2);
      tt_want_ptr_op(NULL, ==, ptr);
      __iotc_free(ptr);
    })

IOTC_TT_TESTCASE(
    utest__iotc_memory_calloc__overflow__num_three__half_max_size,
    {
      void* ptr = __iotc_calloc( /*num=*/3, /*size=*/SIZE_MAX/2);
      tt_want_ptr_op(NULL, ==, ptr);
      __iotc_free(ptr);
    })

IOTC_TT_TESTGROUP_END

#ifndef IOTC_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#define IOTC_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#include __FILE__
#undef IOTC_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#endif  // IOTC_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
