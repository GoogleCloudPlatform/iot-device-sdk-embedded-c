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

/* Happy cases. */
IOTC_TT_TESTCASE(
    utest__iotc_memory_calloc__num_1__size_8,
    {
      void* ptr = __iotc_calloc( /*num=*/1, /*size=*/8);
      tt_want_ptr_op(NULL, !=, ptr);
      printf("DDB sizeofptr:  %lu\n", sizeof(*ptr));
      __iotc_free(ptr);
    })

IOTC_TT_TESTCASE(
    utest__iotc_memory_calloc___num_255__size_55,
    {
      void* ptr = __iotc_calloc( /*num=*/255, /*size=*/55);
      tt_want_ptr_op(NULL, !=, ptr);
      printf("DDB sizeofptr:  %lu\n", sizeof(*ptr));
      __iotc_free(ptr);
    })

IOTC_TT_TESTCASE(
    utest__iotc_memory_calloc___num_1000000__size_88,
    {
      void* ptr = __iotc_calloc( /*num=*/1000000, /*size=*/88);
      tt_want_ptr_op(NULL, !=, ptr);
      printf("DDB sizeofptr:  %lu\n", sizeof(*ptr));
      __iotc_free(ptr);
    })

/* Edge cases.
   Note: Following tests are absent due to varying calloc behaviors across
   platforms. The allocations succeed on our x86 runners but fail on AMD64
  runners:
    - utest__iotc_memory_calloc__max_num__one_size
    - utest__iotc_memory_calloc__one_num__max_size
    - utest__iotc_memory_calloc__max_num_minus_one__one_size
    - utest__iotc_memory_calloc__overflow__half_max__size_two
    - utest__iotc_memory_calloc__overflow__num_two__half_max_size
*/
IOTC_TT_TESTCASE(
    utest__iotc_memory_calloc__num_0__size_8,
    {
      void* ptr = __iotc_calloc( /*num=*/0, /*size=*/8);
      tt_want_ptr_op(NULL, ==, ptr);
      __iotc_free(ptr);
    })

IOTC_TT_TESTCASE(
    utest__iotc_memory_calloc__num_1__size_0,
    {
      void* ptr = __iotc_calloc( /*num=*/1, /*size=*/0);
      tt_want_ptr_op(NULL, ==, ptr);
      __iotc_free(ptr);
    })

IOTC_TT_TESTCASE(
    utest__iotc_memory_calloc__num_0__size_0,
    {
      void* ptr = __iotc_calloc( /*num=*/0, /*size=*/0);
      tt_want_ptr_op(NULL, ==, ptr);
      __iotc_free(ptr);
    })

IOTC_TT_TESTCASE(
    utest__iotc_memory_calloc__num_max__size_0,
    {
      void* ptr = __iotc_calloc( /*num=*/SIZE_MAX, /*size=*/0);
      tt_want_ptr_op(NULL, ==, ptr);
      __iotc_free(ptr);
    })

IOTC_TT_TESTCASE(
    utest__iotc_memory_calloc__num_0__size_max,
    {
      void* ptr = __iotc_calloc( /*num=*/0, /*size=*/SIZE_MAX);
      tt_want_ptr_op(NULL, ==, ptr);
      __iotc_free(ptr);
    })

#ifndef __aarch64__
IOTC_TT_TESTCASE(
    utest__iotc_memory_calloc__num_1__size_max_minus_1,
    {
      void* ptr = __iotc_calloc( /*num=*/1, /*size=*/SIZE_MAX-1);
      tt_want_ptr_op(NULL, ==, ptr);
      __iotc_free(ptr);
    })
#endif

IOTC_TT_TESTCASE(
    utest__iotc_memory_calloc__num_size_max_minus_1__size_1,
    {
      void* ptr = __iotc_calloc( /*num=*/SIZE_MAX-1, /*size=*/1);
      tt_want_ptr_op(NULL, ==, ptr);
      __iotc_free(ptr);
    })

/* Overflow cases. */
IOTC_TT_TESTCASE(
    utest__iotc_memory_calloc__overflow__num_3__size_half_max,
    {
      void* ptr = __iotc_calloc( /*num=*/3, /*size=*/SIZE_MAX/2);
      tt_want_ptr_op(NULL, ==, ptr);
      __iotc_free(ptr);
    })
  
IOTC_TT_TESTCASE(
    utest__iotc_memory_calloc__overflow__num_half_max__size_3,
    {
      void* ptr = __iotc_calloc( /*num=*/SIZE_MAX/2, /*size=*/3) ;
      tt_want_ptr_op(NULL, ==, ptr);
      __iotc_free(ptr);
    })

IOTC_TT_TESTCASE(
    utest__iotc_memory_calloc__overflow__num_half_max_plus_1__size_2,
    {
      void* ptr = __iotc_calloc( /*num=*/SIZE_MAX/2+1, /*size=*/2) ;
      tt_want_ptr_op(NULL, ==, ptr);
      __iotc_free(ptr);
    })

IOTC_TT_TESTCASE(
    utest__iotc_memory_calloc__overflow__num_3__size_half_max_plus_1,
    {
      void* ptr = __iotc_calloc( /*num=*/3, /*size=*/SIZE_MAX/2+1) ;
      tt_want_ptr_op(NULL, ==, ptr);
      __iotc_free(ptr);
    })

IOTC_TT_TESTGROUP_END

#ifndef IOTC_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#define IOTC_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#include __FILE__
#undef IOTC_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#endif  // IOTC_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
