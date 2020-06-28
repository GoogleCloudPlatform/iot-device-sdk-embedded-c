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

#include "iotc_macros.h"
#include "iotc_tt_testcase_management.h"
#include "tinytest.h"
#include "tinytest_macros.h"

#include "iotc.h"
#include "iotc_memory_checks.h"
#include "iotc_memory_limiter.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifndef IOTC_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN

const size_t utest_memory_limiter_generic_test_samples[] = {3072, 0xFAFAFAFA,
                                                            0xFFFFFFFF};

const size_t utest_memory_limiter_expected_generic_results[] = {
    3072 - IOTC_MEMORY_LIMITER_SYSTEM_MEMORY_LIMIT,
    0xFAFAFAFA - IOTC_MEMORY_LIMITER_SYSTEM_MEMORY_LIMIT,
    0xFFFFFFFF - IOTC_MEMORY_LIMITER_SYSTEM_MEMORY_LIMIT};

void test() {
  const size_t application_capacity_pre_alloc =
      iotc_memory_limiter_get_current_limit(
          IOTC_MEMORY_LIMITER_ALLOCATION_TYPE_APPLICATION);

  const size_t original_alloc_size = 0xAA;
  const size_t realloc_size = 0xAF;
  const size_t alloc_size_difference = realloc_size - original_alloc_size;

  void* ptr =
      iotc_memory_limiter_alloc(IOTC_MEMORY_LIMITER_ALLOCATION_TYPE_APPLICATION,
                                original_alloc_size, __FILE__, __LINE__);
  tt_assert(NULL != ptr);

  const size_t application_capacity_after_alloc =
      iotc_memory_limiter_get_current_limit(
          IOTC_MEMORY_LIMITER_ALLOCATION_TYPE_APPLICATION);

  memset(ptr, 0xCA, original_alloc_size);
  unsigned char* matrix[0xAA];
  memset(matrix, 0xCA, sizeof(matrix));

  void* realloc_ptr = iotc_memory_limiter_realloc(
      IOTC_MEMORY_LIMITER_ALLOCATION_TYPE_APPLICATION, ptr, 0xAF, __FILE__,
      __LINE__);
  tt_assert(NULL != realloc_ptr);

  const size_t capacity_difference =
      application_capacity_after_alloc -
      iotc_memory_limiter_get_current_limit(
          IOTC_MEMORY_LIMITER_ALLOCATION_TYPE_APPLICATION);

  tt_assert(alloc_size_difference == capacity_difference);
  tt_assert(memcmp(realloc_ptr, matrix, original_alloc_size) == 0);

  iotc_memory_limiter_free(realloc_ptr);

  tt_assert(application_capacity_pre_alloc ==
            iotc_memory_limiter_get_current_limit(
                IOTC_MEMORY_LIMITER_ALLOCATION_TYPE_APPLICATION));
  return;
end:;
}

#endif /* IOTC_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN */

IOTC_TT_TESTGROUP_BEGIN(utest_memory_limiter)

IOTC_TT_TESTCASE(
    utest__iotc_memory_limiter_set_limit__valid_data_application__new_limit_set,
    {
      size_t i = 0;

      for (; i < IOTC_ARRAYSIZE(utest_memory_limiter_generic_test_samples);
           ++i) {
        tt_want_int_op(IOTC_STATE_OK, ==,
                       iotc_memory_limiter_set_limit(
                           utest_memory_limiter_generic_test_samples[i]));
        tt_want_int_op(iotc_memory_limiter_get_capacity(
                           IOTC_MEMORY_LIMITER_ALLOCATION_TYPE_APPLICATION),
                       ==, utest_memory_limiter_expected_generic_results[i]);
        tt_want_int_op(iotc_memory_limiter_get_current_limit(
                           IOTC_MEMORY_LIMITER_ALLOCATION_TYPE_APPLICATION),
                       ==, utest_memory_limiter_expected_generic_results[i]);
      }
    })

IOTC_TT_TESTCASE(
    utest__iotc_memory_limiter_allocate_memory__valid_data__current_limit_shrinked_down_of_an_allocation_size_return_state_ok,
    {
      /* size values */
      const size_t allocation_size = 128;
      const size_t allocation_footprint =
          allocation_size + sizeof(iotc_memory_limiter_entry_t);
      const size_t memory_limit_size =
          IOTC_MEMORY_LIMITER_SYSTEM_MEMORY_LIMIT + 1024;

      /* assumptions */
      tt_assert(IOTC_MEMORY_LIMITER_SYSTEM_MEMORY_LIMIT > allocation_footprint);

      iotc_state_t memory_set_result =
          iotc_set_maximum_heap_usage(memory_limit_size);
      tt_assert(memory_set_result == IOTC_STATE_OK);

      void* ptr =
          iotc_memory_limiter_alloc(IOTC_MEMORY_LIMITER_ALLOCATION_TYPE_SYSTEM,
                                    allocation_size, __FILE__, __LINE__);
      tt_assert(NULL != ptr);
      tt_assert(iotc_memory_limiter_get_current_limit(
                    IOTC_MEMORY_LIMITER_ALLOCATION_TYPE_SYSTEM) ==
                memory_limit_size - allocation_size -
                    sizeof(iotc_memory_limiter_entry_t));

      tt_assert(memory_limit_size - IOTC_MEMORY_LIMITER_SYSTEM_MEMORY_LIMIT ==
                iotc_memory_limiter_get_capacity(
                    IOTC_MEMORY_LIMITER_ALLOCATION_TYPE_APPLICATION));

      tt_assert(memory_limit_size ==
                iotc_memory_limiter_get_capacity(
                    IOTC_MEMORY_LIMITER_ALLOCATION_TYPE_SYSTEM));

      void* ptr2 =
          iotc_memory_limiter_alloc(IOTC_MEMORY_LIMITER_ALLOCATION_TYPE_SYSTEM,
                                    allocation_size, __FILE__, __LINE__);
      tt_assert(NULL != ptr2);
      tt_assert(memory_limit_size - 2 * allocation_size -
                    2 * sizeof(iotc_memory_limiter_entry_t) ==
                iotc_memory_limiter_get_current_limit(
                    IOTC_MEMORY_LIMITER_ALLOCATION_TYPE_SYSTEM));

      tt_assert(memory_limit_size - IOTC_MEMORY_LIMITER_SYSTEM_MEMORY_LIMIT ==
                iotc_memory_limiter_get_capacity(
                    IOTC_MEMORY_LIMITER_ALLOCATION_TYPE_APPLICATION));

      tt_assert(memory_limit_size ==
                iotc_memory_limiter_get_capacity(
                    IOTC_MEMORY_LIMITER_ALLOCATION_TYPE_SYSTEM));

      iotc_memory_limiter_free(ptr);
      iotc_memory_limiter_free(ptr2);

      const size_t new_memory_limit =
          IOTC_MEMORY_LIMITER_SYSTEM_MEMORY_LIMIT + allocation_footprint;
      iotc_memory_limiter_set_limit(new_memory_limit);

      void* ptr3 = iotc_memory_limiter_alloc(
          IOTC_MEMORY_LIMITER_ALLOCATION_TYPE_APPLICATION, allocation_size,
          __FILE__, __LINE__);
      tt_assert(NULL != ptr3);

      tt_assert(iotc_memory_limiter_get_current_limit(
                    IOTC_MEMORY_LIMITER_ALLOCATION_TYPE_APPLICATION) == 0x0);
      tt_assert(iotc_memory_limiter_get_current_limit(
                    IOTC_MEMORY_LIMITER_ALLOCATION_TYPE_SYSTEM) ==
                IOTC_MEMORY_LIMITER_SYSTEM_MEMORY_LIMIT);
      tt_assert(iotc_memory_limiter_get_capacity(
                    IOTC_MEMORY_LIMITER_ALLOCATION_TYPE_APPLICATION) ==
                allocation_size + sizeof(iotc_memory_limiter_entry_t));
      tt_assert(new_memory_limit ==
                iotc_memory_limiter_get_capacity(
                    IOTC_MEMORY_LIMITER_ALLOCATION_TYPE_SYSTEM));

      iotc_memory_limiter_free(ptr3);

      /* put things back to normal */
      iotc_memory_limiter_set_limit(
          IOTC_MEMORY_LIMITER_SYSTEM_MEMORY_LIMIT +
          IOTC_MEMORY_LIMITER_APPLICATION_MEMORY_LIMIT);
      return;
    end:
      iotc_memory_limiter_set_limit(
          IOTC_MEMORY_LIMITER_SYSTEM_MEMORY_LIMIT +
          IOTC_MEMORY_LIMITER_APPLICATION_MEMORY_LIMIT);
    })

IOTC_TT_TESTCASE(
    utest__iotc_memory_limiter_allocate_memory__vald_data_allocate_more_than_capacity__return_NULL,
    {
      /* size values */
      const size_t allocation_size =
          IOTC_MEMORY_LIMITER_SYSTEM_MEMORY_LIMIT * 2;
      const size_t memory_limit_size = IOTC_MEMORY_LIMITER_SYSTEM_MEMORY_LIMIT +
                                       sizeof(iotc_memory_limiter_entry_t);

      iotc_state_t memory_set_result =
          iotc_set_maximum_heap_usage(memory_limit_size);
      tt_assert(memory_set_result == IOTC_STATE_OK);

      tt_assert(NULL == iotc_memory_limiter_alloc(
                            IOTC_MEMORY_LIMITER_ALLOCATION_TYPE_APPLICATION,
                            allocation_size, __FILE__, __LINE__));
      memory_set_result = iotc_memory_limiter_set_limit(
          IOTC_MEMORY_LIMITER_SYSTEM_MEMORY_LIMIT +
          IOTC_MEMORY_LIMITER_APPLICATION_MEMORY_LIMIT);
      tt_assert(memory_set_result == IOTC_STATE_OK);
      return;
    end:
      iotc_memory_limiter_set_limit(
          IOTC_MEMORY_LIMITER_SYSTEM_MEMORY_LIMIT +
          IOTC_MEMORY_LIMITER_APPLICATION_MEMORY_LIMIT);
      ;
    })

IOTC_TT_TESTCASE(
    utest__iotc_memory_limiter_allocate_memory_system__vald_data_allocate_more_than_capacity__return_NULL,
    {
      /* size values */
      const size_t allocation_size =
          IOTC_MEMORY_LIMITER_SYSTEM_MEMORY_LIMIT * 2;
      const size_t memory_limit_size = IOTC_MEMORY_LIMITER_SYSTEM_MEMORY_LIMIT +
                                       sizeof(iotc_memory_limiter_entry_t);

      iotc_state_t memory_set_result =
          iotc_set_maximum_heap_usage(memory_limit_size);
      tt_assert(memory_set_result == IOTC_STATE_OK);

      tt_assert(NULL == iotc_memory_limiter_alloc(
                            IOTC_MEMORY_LIMITER_ALLOCATION_TYPE_SYSTEM,
                            allocation_size, __FILE__, __LINE__));
      memory_set_result = iotc_memory_limiter_set_limit(
          IOTC_MEMORY_LIMITER_SYSTEM_MEMORY_LIMIT +
          IOTC_MEMORY_LIMITER_APPLICATION_MEMORY_LIMIT);
      tt_assert(memory_set_result == IOTC_STATE_OK);
      return;
    end:
      iotc_memory_limiter_set_limit(
          IOTC_MEMORY_LIMITER_SYSTEM_MEMORY_LIMIT +
          IOTC_MEMORY_LIMITER_APPLICATION_MEMORY_LIMIT);
      ;
    })

IOTC_TT_TESTCASE(
    utest__iotc_memory_limiter_allocate_memory__wrong_data__return_invalid_parameter,
    {
      void* ptr = iotc_memory_limiter_alloc(
          IOTC_MEMORY_LIMITER_ALLOCATION_TYPE_APPLICATION, 0, __FILE__,
          __LINE__);
      tt_assert(NULL != ptr);

      iotc_memory_limiter_free(ptr);
    end:;
    })

IOTC_TT_TESTCASE(
    utest__iotc_memory_limiter_allocate_memory__valid_data_realloc_more__realloc_previously_allocated_memory,
    { test(); })

IOTC_TT_TESTCASE(
    utest__iotc_memory_limiter_allocate_memory__valid_data_realloc_less__realloc_previously_allocated_memory,
    {
      const size_t application_capacity_pre_alloc =
          iotc_memory_limiter_get_current_limit(
              IOTC_MEMORY_LIMITER_ALLOCATION_TYPE_APPLICATION);

      const size_t original_alloc_size = 0xAA;
      const size_t original_alloc_size_footprint =
          original_alloc_size + sizeof(iotc_memory_limiter_entry_t);
      const size_t realloc_size = 0xAF;
      const size_t realloc_size_footprint =
          0xAF + sizeof(iotc_memory_limiter_entry_t);

      const size_t expected_new_application_limit_alloc =
          application_capacity_pre_alloc - original_alloc_size_footprint;
      const size_t expected_new_application_limit_realloc =
          application_capacity_pre_alloc - realloc_size_footprint;

      void* ptr = iotc_memory_limiter_alloc(
          IOTC_MEMORY_LIMITER_ALLOCATION_TYPE_APPLICATION, original_alloc_size,
          __FILE__, __LINE__);

      tt_assert(NULL != ptr);
      tt_assert(expected_new_application_limit_alloc ==
                iotc_memory_limiter_get_current_limit(
                    IOTC_MEMORY_LIMITER_ALLOCATION_TYPE_APPLICATION));

      memset(ptr, 0xCA, original_alloc_size);
      unsigned char* matrix[0xAA];
      memset(matrix, 0xCA, sizeof(matrix));

      void* realloc_ptr = iotc_memory_limiter_realloc(
          IOTC_MEMORY_LIMITER_ALLOCATION_TYPE_APPLICATION, ptr, realloc_size,
          __FILE__, __LINE__);

      tt_assert(NULL != realloc_ptr);
      tt_assert(expected_new_application_limit_realloc ==
                iotc_memory_limiter_get_current_limit(
                    IOTC_MEMORY_LIMITER_ALLOCATION_TYPE_APPLICATION));
      tt_assert(memcmp(realloc_ptr, matrix, original_alloc_size) == 0);

      iotc_memory_limiter_free(realloc_ptr);

      tt_assert(iotc_memory_limiter_get_current_limit(
                    IOTC_MEMORY_LIMITER_ALLOCATION_TYPE_APPLICATION) ==
                application_capacity_pre_alloc);

      return;
    end:;
    })

IOTC_TT_TESTCASE(
    utest__iotc_memory_limiter_free_memory__valid_data_free_previously_allocated_value__return_state_ok,
    {
      const size_t application_capacity = iotc_memory_limiter_get_current_limit(
          IOTC_MEMORY_LIMITER_ALLOCATION_TYPE_APPLICATION);
      const size_t alloc_size = 0xAA;
      const size_t alloc_size_footprint =
          alloc_size + sizeof(iotc_memory_limiter_entry_t);
      const size_t application_limit_post_alloc =
          application_capacity - alloc_size_footprint;

      void* ptr = iotc_memory_limiter_alloc(
          IOTC_MEMORY_LIMITER_ALLOCATION_TYPE_APPLICATION, alloc_size, __FILE__,
          __LINE__);
      tt_assert(NULL != ptr);
      tt_assert(application_limit_post_alloc ==
                iotc_memory_limiter_get_current_limit(
                    IOTC_MEMORY_LIMITER_ALLOCATION_TYPE_APPLICATION));
      tt_assert(application_capacity ==
                iotc_memory_limiter_get_capacity(
                    IOTC_MEMORY_LIMITER_ALLOCATION_TYPE_APPLICATION));

      iotc_memory_limiter_free(ptr);
      tt_assert(application_capacity ==
                iotc_memory_limiter_get_current_limit(
                    IOTC_MEMORY_LIMITER_ALLOCATION_TYPE_APPLICATION));
      tt_assert(application_capacity ==
                iotc_memory_limiter_get_capacity(
                    IOTC_MEMORY_LIMITER_ALLOCATION_TYPE_APPLICATION));

      ptr = iotc_memory_limiter_alloc(
          IOTC_MEMORY_LIMITER_ALLOCATION_TYPE_APPLICATION,
          application_capacity - sizeof(iotc_memory_limiter_entry_t), __FILE__,
          __LINE__);
      tt_assert(NULL != ptr);
      tt_assert(0x0 == iotc_memory_limiter_get_current_limit(
                           IOTC_MEMORY_LIMITER_ALLOCATION_TYPE_APPLICATION));
      tt_assert(application_capacity ==
                iotc_memory_limiter_get_capacity(
                    IOTC_MEMORY_LIMITER_ALLOCATION_TYPE_APPLICATION));

      iotc_memory_limiter_free(ptr);
      tt_assert(application_capacity ==
                iotc_memory_limiter_get_current_limit(
                    IOTC_MEMORY_LIMITER_ALLOCATION_TYPE_APPLICATION));
      tt_assert(application_capacity ==
                iotc_memory_limiter_get_capacity(
                    IOTC_MEMORY_LIMITER_ALLOCATION_TYPE_APPLICATION));

      return;
    end:;
    })

IOTC_TT_TESTCASE(utest__iotc_memory_limiter_reset_the_limiter, {
  iotc_memory_limiter_set_limit(IOTC_MEMORY_LIMITER_APPLICATION_MEMORY_LIMIT +
                                IOTC_MEMORY_LIMITER_SYSTEM_MEMORY_LIMIT + 1);
  tt_want_int_op(iotc_is_whole_memory_deallocated(), >, 0);
  iotc_memory_limiter_set_limit(IOTC_MEMORY_LIMITER_APPLICATION_MEMORY_LIMIT +
                                IOTC_MEMORY_LIMITER_SYSTEM_MEMORY_LIMIT);
})

IOTC_TT_TESTCASE(
    utest__iotc_memory_limiter_allocate_memory__vald_data_proper_capacities__return_proper_capacity,
    {
      tt_int_op(IOTC_MEMORY_LIMITER_SYSTEM_MEMORY_LIMIT +
                    IOTC_MEMORY_LIMITER_APPLICATION_MEMORY_LIMIT,
                ==,
                iotc_memory_limiter_get_current_limit(
                    IOTC_MEMORY_LIMITER_ALLOCATION_TYPE_SYSTEM));
      tt_int_op(IOTC_MEMORY_LIMITER_APPLICATION_MEMORY_LIMIT, ==,
                iotc_memory_limiter_get_current_limit(
                    IOTC_MEMORY_LIMITER_ALLOCATION_TYPE_APPLICATION));
    end:;
    })

IOTC_TT_TESTCASE(
    utest__iotc_memory_limiter_allocate_memory__vald_data_allocate_system_and_application_to_capacity__return_NOT_NULL,
    {
      const size_t allocation_overhead = sizeof(iotc_memory_limiter_entry_t);
      const size_t system_to_alloc =
          IOTC_MEMORY_LIMITER_SYSTEM_MEMORY_LIMIT - allocation_overhead;

      const size_t application_to_alloc =
          iotc_memory_limiter_get_capacity(
              IOTC_MEMORY_LIMITER_ALLOCATION_TYPE_APPLICATION) -
          allocation_overhead;

      void *ptr1 = NULL, *ptr2 = NULL;
      ptr1 = iotc_memory_limiter_alloc(
          IOTC_MEMORY_LIMITER_ALLOCATION_TYPE_APPLICATION, application_to_alloc,
          __FILE__, __LINE__);

      tt_int_op(iotc_memory_limiter_get_allocated_space(), ==,
                application_to_alloc + allocation_overhead);

      tt_ptr_op(NULL, !=, ptr1);
      ptr2 =
          iotc_memory_limiter_alloc(IOTC_MEMORY_LIMITER_ALLOCATION_TYPE_SYSTEM,
                                    system_to_alloc, __FILE__, __LINE__);
      tt_ptr_op(NULL, !=, ptr2);
      tt_int_op(iotc_memory_limiter_get_current_limit(
                    IOTC_MEMORY_LIMITER_ALLOCATION_TYPE_SYSTEM),
                ==, 0);

      tt_int_op(iotc_memory_limiter_get_current_limit(
                    IOTC_MEMORY_LIMITER_ALLOCATION_TYPE_APPLICATION),
                ==, 0);

    end:
      iotc_memory_limiter_free(ptr1);
      iotc_memory_limiter_free(ptr2);
    })

IOTC_TT_TESTGROUP_END

#ifndef IOTC_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#define IOTC_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#include __FILE__
#undef IOTC_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#endif
