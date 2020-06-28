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

#include "iotc_tt_testcase_management.h"
#include "iotc_utest_basic_testcase_frame.h"
#include "tinytest.h"
#include "tinytest_macros.h"

#include "iotc.h"
#include "iotc_data_desc.h"
#include "iotc_err.h"
#include "iotc_macros.h"

#include "iotc_bsp_rng.h"
#include "iotc_memory_checks.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifndef IOTC_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN

#endif

IOTC_TT_TESTGROUP_BEGIN(utest_data_desc)

IOTC_TT_TESTCASE(utest__iotc_data_desc_will_it_fit__valid_data__will_fit, {
  unsigned char buffer[32] = {'\0'};
  iotc_data_desc_t test = {buffer, NULL, 32, 0, 0, IOTC_MEMORY_TYPE_UNMANAGED};

  tt_want_int_op(iotc_data_desc_will_it_fit(&test, 16), ==, 1);

  tt_want_int_op(iotc_is_whole_memory_deallocated(), >, 0);
})

IOTC_TT_TESTCASE(utest__iotc_data_desc_will_it_fit__valid_data__will_not_fit, {
  unsigned char buffer[32] = {'\0'};
  iotc_data_desc_t test = {buffer, NULL, 32, 0, 0, IOTC_MEMORY_TYPE_UNMANAGED};

  tt_want_int_op(iotc_data_desc_will_it_fit(&test, 33), ==, 0);

  tt_want_int_op(iotc_is_whole_memory_deallocated(), >, 0);
})

IOTC_TT_TESTCASE(utest__iotc_data_desc_will_it_fit__valid_data__will_fit_border,
                 {
                   unsigned char buffer[32] = {'\0'};
                   iotc_data_desc_t test = {
                       buffer, NULL, 32, 0, 0, IOTC_MEMORY_TYPE_UNMANAGED};

                   tt_want_int_op(iotc_data_desc_will_it_fit(&test, 32), ==, 1);

                   tt_want_int_op(iotc_is_whole_memory_deallocated(), >, 0);
                 })

IOTC_TT_TESTCASE(
    utest__iotc_data_desc_will_it_fit__valid_data__will_not_fit_border, {
      unsigned char buffer[32] = {'\0'};
      iotc_data_desc_t test = {buffer, NULL, 32,
                               0,      0,    IOTC_MEMORY_TYPE_UNMANAGED};

      tt_want_int_op(iotc_data_desc_will_it_fit(&test, 33), ==, 0);

      tt_want_int_op(iotc_is_whole_memory_deallocated(), >, 0);
    })

IOTC_TT_TESTCASE(
    utest__iotc_data_desc_pow2_realloc_strategy__valid_data__will_ret_64, {
      tt_want_int_op(iotc_data_desc_pow2_realloc_strategy(32, 55), ==, 64);
      tt_want_int_op(iotc_data_desc_pow2_realloc_strategy(32, 33), ==, 64);
      tt_want_int_op(iotc_data_desc_pow2_realloc_strategy(32, 63), ==, 64);
      tt_want_int_op(iotc_data_desc_pow2_realloc_strategy(32, 64), ==, 64);

      tt_want_int_op(iotc_is_whole_memory_deallocated(), >, 0);
    })

IOTC_TT_TESTCASE(
    utest__iotc_data_desc_pow2_realloc_strategy__valid_data__will_ret_128, {
      tt_want_int_op(iotc_data_desc_pow2_realloc_strategy(32, 67), ==, 128);
      tt_want_int_op(iotc_data_desc_pow2_realloc_strategy(32, 90), ==, 128);
      tt_want_int_op(iotc_data_desc_pow2_realloc_strategy(32, 127), ==, 128);
      tt_want_int_op(iotc_data_desc_pow2_realloc_strategy(32, 128), ==, 128);

      tt_want_int_op(iotc_is_whole_memory_deallocated(), >, 0);
    })

IOTC_TT_TESTCASE(
    utest__iotc_data_desc_pow2_realloc_strategy__valid_data__will_ret_512, {
      tt_want_int_op(iotc_data_desc_pow2_realloc_strategy(2, 257), ==, 512);
      tt_want_int_op(iotc_data_desc_pow2_realloc_strategy(2, 400), ==, 512);
      tt_want_int_op(iotc_data_desc_pow2_realloc_strategy(2, 511), ==, 512);
      tt_want_int_op(iotc_data_desc_pow2_realloc_strategy(2, 512), ==, 512);

      tt_want_int_op(iotc_is_whole_memory_deallocated(), >, 0);
    })

IOTC_TT_TESTCASE(
    utest__iotc_data_desc_pow2_realloc_strategy__valid_data__will_ret_512_loop,
    {
      uint32_t i = 257;
      for (; i < 512; ++i) {
        tt_want_int_op(iotc_data_desc_pow2_realloc_strategy(2, i), ==, 512);
      }

      tt_want_int_op(iotc_is_whole_memory_deallocated(), >, 0);
    })

IOTC_TT_TESTCASE(utest__iotc_data_desc_realloc__valid_data__data_size_to_128, {
  iotc_data_desc_t* desc = iotc_make_empty_desc_alloc(32);

  iotc_state_t ret_state =
      iotc_data_desc_realloc(desc, 77, &iotc_data_desc_pow2_realloc_strategy);

  tt_want_int_op(ret_state, ==, IOTC_STATE_OK);
  tt_want_int_op(desc->capacity, ==, 128);

  iotc_free_desc(&desc);

  tt_want_int_op(iotc_is_whole_memory_deallocated(), >, 0);
})

IOTC_TT_TESTCASE_WITH_SETUP(
    utest__iotc_make_desc_from_buffer_copy__valid_data__buffer_copied,
    iotc_utest_setup_basic, iotc_utest_teardown_basic, NULL, {
      iotc_state_t ret_state = IOTC_STATE_OK;

      const size_t size = 67;

      IOTC_ALLOC_BUFFER(unsigned char, test_buffer, size, ret_state);

      size_t i = 0;
      for (; i < size; ++i) {
        test_buffer[i] = iotc_bsp_rng_get() % 256;
      }

      iotc_data_desc_t* data_desc =
          iotc_make_desc_from_buffer_copy(test_buffer, size);

      tt_ptr_op(data_desc, !=, NULL);
      tt_int_op(data_desc->capacity, ==, size);
      tt_int_op(data_desc->length, ==, size);
      tt_int_op(IOTC_MEMORY_TYPE_MANAGED, ==, data_desc->memory_type);
      tt_ptr_op(data_desc->data_ptr, !=, test_buffer);
      tt_int_op(memcmp(data_desc->data_ptr, test_buffer, size), ==, 0);

      IOTC_SAFE_FREE(test_buffer);
      iotc_free_desc(&data_desc);

      return;
    err_handling:
    end:
      tt_fail();
    })

IOTC_TT_TESTCASE_WITH_SETUP(
    utest__iotc_make_desc_from_buffer_share__valid_data__buffer_shared,
    iotc_utest_setup_basic, iotc_utest_teardown_basic, NULL, {
      iotc_state_t ret_state = IOTC_STATE_OK;

      const size_t size = 67;

      IOTC_ALLOC_BUFFER(unsigned char, test_buffer, size, ret_state);

      size_t i = 0;
      for (; i < size; ++i) {
        test_buffer[i] = iotc_bsp_rng_get() % 256;
      }

      iotc_data_desc_t* data_desc =
          iotc_make_desc_from_buffer_share(test_buffer, size);

      tt_ptr_op(data_desc, !=, NULL);
      tt_int_op(data_desc->capacity, ==, size);
      tt_int_op(data_desc->length, ==, size);
      tt_int_op(IOTC_MEMORY_TYPE_UNMANAGED, ==, data_desc->memory_type);
      tt_ptr_op(data_desc->data_ptr, ==, test_buffer);
      tt_int_op(memcmp(data_desc->data_ptr, test_buffer, size), ==, 0);

      unsigned char new_value = test_buffer[0] + 1;
      test_buffer[0] += 1;

      tt_int_op(data_desc->data_ptr[0], ==, new_value);

      iotc_free_desc(&data_desc);
      IOTC_SAFE_FREE(test_buffer);

      return;
    err_handling:
    end:
      tt_fail();
    })

IOTC_TT_TESTCASE_WITH_SETUP(
    utest__iotc_make_desc_from_string_copy__valid_data__data_copied,
    iotc_utest_setup_basic, iotc_utest_teardown_basic, NULL, {
      iotc_state_t ret_state = IOTC_STATE_OK;
      char* origin_string = NULL;
      iotc_data_desc_t* data_desc = NULL;

      size_t size = 4;
      for (; size < 256; size *= 2) {
        IOTC_ALLOC_BUFFER_AT(char, origin_string, size, ret_state);
        size_t i = 0;
        for (; i < size - 1; ++i) {
          origin_string[i] = (iotc_bsp_rng_get() % 255) + 1;
        }

        tt_int_op(strlen(origin_string), ==, size - 1);

        data_desc = iotc_make_desc_from_string_copy(origin_string);

        tt_ptr_op(data_desc, !=, NULL);
        tt_int_op(data_desc->capacity, ==, size - 1);
        tt_int_op(data_desc->length, ==, size - 1);
        tt_int_op(IOTC_MEMORY_TYPE_MANAGED, ==, data_desc->memory_type);
        tt_ptr_op(data_desc->data_ptr, !=, origin_string);
        tt_int_op(memcmp(data_desc->data_ptr, origin_string, size - 1), ==, 0);

        iotc_free_desc(&data_desc);
        IOTC_SAFE_FREE(origin_string);
      }

      return;
    err_handling:
    end:
      iotc_free_desc(&data_desc);
      IOTC_SAFE_FREE(origin_string);
      tt_fail();
    })

#ifdef IOTC_MEMORY_LIMITER_ENABLED
IOTC_TT_TESTCASE(
    utest__iotc_make_desc_from_string_copy__valid_data__not_enough_memory_for_buffer,
    {
      const char source_string[] =
          "The quick brown fox jumps over the lazy dog";
      iotc_data_desc_t* data_desc = NULL;

      /* Limited memory space.  It does not include enough space
         to allocate the buffer to copy the source_string into,
         but enough to allocate the tracking data descriptor for the buffer. */
      const size_t dest_footprint_allowance =
          sizeof(iotc_data_desc_t) + sizeof(iotc_memory_limiter_entry_t);

      /* Set the maximum heap size.  Must include the System memory limit, too.
       */
      const size_t memory_limit_size =
          IOTC_MEMORY_LIMITER_SYSTEM_MEMORY_LIMIT + dest_footprint_allowance;

      iotc_state_t memory_set_result =
          iotc_set_maximum_heap_usage(memory_limit_size);
      tt_assert(memory_set_result == IOTC_STATE_OK);

      /* test behavior */
      data_desc = iotc_make_desc_from_string_copy(source_string);
      tt_ptr_op(data_desc, ==, NULL);

    end:
      iotc_free_desc(&data_desc);
      iotc_memory_limiter_set_limit(
          IOTC_MEMORY_LIMITER_SYSTEM_MEMORY_LIMIT +
          IOTC_MEMORY_LIMITER_APPLICATION_MEMORY_LIMIT);
    })

IOTC_TT_TESTCASE(
    utest__iotc_make_desc_from_string_copy__valid_data__not_enough_memory_for_descriptor,
    {
      const char source_string[] =
          "The quick brown fox jumps over the lazy dog";
      iotc_data_desc_t* data_desc = NULL;

      /* Limited memory space.  It does not include enough space
         to allocate the buffer to copy the source_string into,
         nor the descriptor to track the allocation. */
      const size_t dest_footprint_allowance =
          sizeof(iotc_memory_limiter_entry_t);

      /* Set the maximum heap size.  Must include the System memory limit, too
       */
      const size_t memory_limit_size =
          IOTC_MEMORY_LIMITER_SYSTEM_MEMORY_LIMIT + dest_footprint_allowance;

      iotc_state_t memory_set_result =
          iotc_set_maximum_heap_usage(memory_limit_size);
      tt_assert(memory_set_result == IOTC_STATE_OK);

      /* test behavior */
      data_desc = iotc_make_desc_from_string_copy(source_string);
      tt_ptr_op(data_desc, ==, NULL);

    end:
      iotc_free_desc(&data_desc);
      iotc_memory_limiter_set_limit(
          IOTC_MEMORY_LIMITER_SYSTEM_MEMORY_LIMIT +
          IOTC_MEMORY_LIMITER_APPLICATION_MEMORY_LIMIT);
    })

IOTC_TT_TESTCASE(
    utest__iotc_make_desc_from_buffer_copy__valid_data__not_enough_memory_for_buffer,
    {
      const unsigned char source_buffer[] =
          "The quick brown fox jumps over the lazy dog";
      const size_t buffer_size_in_bytes = sizeof(source_buffer);
      iotc_data_desc_t* data_desc = NULL;

      /* Limited memory space.  It does not include enough space
         to allocate a buffer to copy the source_buffer into,
         but enough to allocate the tracking data descriptor for that buffer. */
      const size_t dest_footprint_allowance =
          sizeof(iotc_data_desc_t) + sizeof(iotc_memory_limiter_entry_t);

      /* Set the maximum heap size.  Must include the System memory limit, too.
       */
      const size_t memory_limit_size =
          IOTC_MEMORY_LIMITER_SYSTEM_MEMORY_LIMIT + dest_footprint_allowance;

      iotc_state_t memory_set_result =
          iotc_set_maximum_heap_usage(memory_limit_size);
      tt_assert(memory_set_result == IOTC_STATE_OK);

      /* test behavior */
      data_desc =
          iotc_make_desc_from_buffer_copy(source_buffer, buffer_size_in_bytes);
      tt_ptr_op(data_desc, ==, NULL);

    end:
      iotc_free_desc(&data_desc);
      iotc_memory_limiter_set_limit(
          IOTC_MEMORY_LIMITER_SYSTEM_MEMORY_LIMIT +
          IOTC_MEMORY_LIMITER_APPLICATION_MEMORY_LIMIT);
    })

IOTC_TT_TESTCASE(
    utest__iotc_make_desc_from_buffer_copy__valid_data__not_enough_memory_for_descriptor,
    {
      const unsigned char source_buffer[] =
          "The quick brown fox jumps over the lazy dog";
      const size_t buffer_size_in_bytes = sizeof(source_buffer);
      iotc_data_desc_t* data_desc = NULL;

      /* Limited memory space.  It does not include enough space
         to allocate a buffer to copy the source_buffer into,
         nor enough space to allocate the tracking data descriptorfor that
         buffer.*/
      const size_t dest_footprint_allowance =
          sizeof(iotc_memory_limiter_entry_t);

      /* Set the maximum heap size.  Must include the System memory limit, too.
       */
      const size_t memory_limit_size =
          IOTC_MEMORY_LIMITER_SYSTEM_MEMORY_LIMIT + dest_footprint_allowance;

      iotc_state_t memory_set_result =
          iotc_set_maximum_heap_usage(memory_limit_size);
      tt_assert(memory_set_result == IOTC_STATE_OK);

      /* test behavior */
      data_desc =
          iotc_make_desc_from_buffer_copy(source_buffer, buffer_size_in_bytes);
      tt_ptr_op(data_desc, ==, NULL);

    end:
      iotc_free_desc(&data_desc);
      iotc_memory_limiter_set_limit(
          IOTC_MEMORY_LIMITER_SYSTEM_MEMORY_LIMIT +
          IOTC_MEMORY_LIMITER_APPLICATION_MEMORY_LIMIT);
    })

IOTC_TT_TESTCASE(
    utest__iotc_make_empty_desc_alloc__valid_data__not_enough_memory_for_descriptor,
    {
      /* size values */
      size_t buffer_size = 700;
      iotc_data_desc_t* data_desc = NULL;

      /* Limited memory space.  It does not include enough space
         to allocate a buffer of the requested size (buffer_size)
         nor enough space to allocate the tracking data descriptor for
         that buffer.*/
      const size_t dest_footprint_allowance =
          sizeof(iotc_memory_limiter_entry_t);

      /* Set the maximum heap size.  Must include the System memory limit, too.
       */
      const size_t memory_limit_size =
          IOTC_MEMORY_LIMITER_SYSTEM_MEMORY_LIMIT + dest_footprint_allowance;

      iotc_state_t memory_set_result =
          iotc_set_maximum_heap_usage(memory_limit_size);
      tt_assert(memory_set_result == IOTC_STATE_OK);

      /* test behavior */
      data_desc = iotc_make_empty_desc_alloc(buffer_size);
      tt_ptr_op(data_desc, ==, NULL);

    end:
      iotc_free_desc(&data_desc);
      iotc_memory_limiter_set_limit(
          IOTC_MEMORY_LIMITER_SYSTEM_MEMORY_LIMIT +
          IOTC_MEMORY_LIMITER_APPLICATION_MEMORY_LIMIT);
    })

IOTC_TT_TESTCASE(
    utest__iotc_make_empty_desc_alloc__valid_data__not_enough_memory_for_buffer,
    {
      /* size values */
      size_t buffer_size = 700;
      iotc_data_desc_t* data_desc = NULL;

      /* Limited memory space.  It does not include enough space
         to allocate a buffer of the requested size (buffer_size)
         bu enough space to allocate the tracking data descriptor for
         that buffer.*/
      const size_t dest_footprint_allowance =
          sizeof(iotc_data_desc_t) + sizeof(iotc_memory_limiter_entry_t);

      /* Set the maximum heap size.  Must include the System memory limit, too.
       */
      const size_t memory_limit_size =
          IOTC_MEMORY_LIMITER_SYSTEM_MEMORY_LIMIT + dest_footprint_allowance;

      iotc_state_t memory_set_result =
          iotc_set_maximum_heap_usage(memory_limit_size);
      tt_assert(memory_set_result == IOTC_STATE_OK);

      /* test behavior */
      data_desc = iotc_make_empty_desc_alloc(buffer_size);
      tt_ptr_op(data_desc, ==, NULL);

    end:
      iotc_free_desc(&data_desc);
      iotc_memory_limiter_set_limit(
          IOTC_MEMORY_LIMITER_SYSTEM_MEMORY_LIMIT +
          IOTC_MEMORY_LIMITER_APPLICATION_MEMORY_LIMIT);
    })
#endif /* IOTC_MEMORY_LIMITER_ENABLED */

IOTC_TT_TESTCASE(
    utest__iotc_make_desc_from_string_copy__null_data__null_returned, {
      char* origin_string = NULL;
      iotc_data_desc_t* data_desc = NULL;

      data_desc = iotc_make_desc_from_string_copy(origin_string);

      tt_ptr_op(data_desc, ==, NULL);

      tt_want_int_op(iotc_is_whole_memory_deallocated(), >, 0);
      return;
    end:
      tt_fail();
    })

IOTC_TT_TESTCASE(
    utest__iotc_make_desc_from_string_share__null_data__null_returned, {
      char* origin_string = NULL;
      iotc_data_desc_t* data_desc = NULL;

      data_desc = iotc_make_desc_from_string_share(origin_string);

      tt_ptr_op(data_desc, ==, NULL);

      tt_want_int_op(iotc_is_whole_memory_deallocated(), >, 0);
      return;
    end:
      tt_fail();
    })

IOTC_TT_TESTCASE_WITH_SETUP(
    utest__iotc_make_desc_from_string_share__valid_data__data_shared,
    iotc_utest_setup_basic, iotc_utest_teardown_basic, NULL, {
      iotc_state_t ret_state = IOTC_STATE_OK;

      char* origin_string = NULL;
      iotc_data_desc_t* data_desc = NULL;

      size_t size = 4;
      for (; size < 256; size *= 2) {
        IOTC_ALLOC_BUFFER_AT(char, origin_string, size, ret_state);
        size_t i = 0;
        for (; i < size - 1; ++i) {
          origin_string[i] = (iotc_bsp_rng_get() % 255) + 1;
        }

        tt_int_op(strlen(origin_string), ==, size - 1);

        data_desc = iotc_make_desc_from_string_share(origin_string);

        tt_ptr_op(data_desc, !=, NULL);
        tt_int_op(data_desc->capacity, ==, size - 1);
        tt_int_op(data_desc->length, ==, size - 1);
        tt_int_op(IOTC_MEMORY_TYPE_UNMANAGED, ==, data_desc->memory_type);
        tt_ptr_op(data_desc->data_ptr, ==, origin_string);
        tt_int_op(memcmp(data_desc->data_ptr, origin_string, size - 1), ==, 0);

        origin_string[0] += 1;
        tt_int_op(memcmp(data_desc->data_ptr, origin_string, size - 1), ==, 0);

        iotc_free_desc(&data_desc);
        IOTC_SAFE_FREE(origin_string);
      }

      return;
    err_handling:
    end:
      iotc_free_desc(&data_desc);
      IOTC_SAFE_FREE(origin_string);
      tt_fail();
    })

IOTC_TT_TESTCASE(utest__iotc_data_desc_realloc__valid_data__data_copied, {
  const char test_string[] = "this is a test string 32434 /asdf/";
  iotc_data_desc_t* desc = iotc_make_desc_from_string_copy(test_string);

  iotc_state_t ret_state =
      iotc_data_desc_realloc(desc, 77, &iotc_data_desc_pow2_realloc_strategy);

  tt_want_int_op(ret_state, ==, IOTC_STATE_OK);
  tt_want_int_op(memcmp(desc->data_ptr, test_string, sizeof(test_string) - 1),
                 ==, 0);

  iotc_free_desc(&desc);

  tt_want_int_op(iotc_is_whole_memory_deallocated(), >, 0);
})

IOTC_TT_TESTCASE(utest__iotc_data_desc_realloc__valid_data__size_not_changed, {
  iotc_data_desc_t* desc = iotc_make_empty_desc_alloc(32);

  iotc_state_t ret_state =
      iotc_data_desc_realloc(desc, 32, &iotc_data_desc_pow2_realloc_strategy);

  tt_want_int_op(ret_state, ==, IOTC_STATE_OK);
  tt_want_int_op(desc->capacity, ==, 32);

  iotc_free_desc(&desc);

  tt_want_int_op(iotc_is_whole_memory_deallocated(), >, 0);
})

IOTC_TT_TESTCASE(
    utest__iotc_data_desc_append_data_resize__valid_data__write_string_and_realloc,
    {
      const char test_string[] =
          "this is a test string which will test this function";

      iotc_data_desc_t* desc = iotc_make_empty_desc_alloc(2);

      iotc_state_t state = iotc_data_desc_append_data_resize(
          desc, test_string, sizeof(test_string) - 1);

      tt_want_int_op(state, ==, IOTC_STATE_OK);
      tt_want_int_op(desc->capacity, ==, 64);
      tt_want_int_op(desc->length, ==, sizeof(test_string) - 1);
      tt_want_int_op(
          memcmp(desc->data_ptr, test_string, sizeof(test_string) - 1), ==, 0);

      iotc_free_desc(&desc);

      tt_want_int_op(iotc_is_whole_memory_deallocated(), >, 0);
    })

IOTC_TT_TESTCASE(
    utest__iotc_data_desc_append_data_resize__valid_data__write_string_twice_and_realloc,
    {
      const char test_string[] =
          "this is a test string which will test this function";
      const size_t test_string_size = sizeof(test_string) - 1;

      iotc_data_desc_t* desc = iotc_make_empty_desc_alloc(2);

      iotc_state_t state = iotc_data_desc_append_data_resize(desc, test_string,
                                                             test_string_size);

      tt_want_int_op(state, ==, IOTC_STATE_OK);

      state = iotc_data_desc_append_data_resize(desc, test_string,
                                                test_string_size);

      tt_want_int_op(state, ==, IOTC_STATE_OK);
      tt_want_int_op(desc->capacity, ==, 128);
      tt_want_int_op(desc->length, ==, test_string_size * 2);
      tt_want_int_op(memcmp(desc->data_ptr, test_string, test_string_size), ==,
                     0);
      tt_want_int_op(memcmp(desc->data_ptr + test_string_size, test_string,
                            test_string_size),
                     ==, 0);

      iotc_free_desc(&desc);

      tt_want_int_op(iotc_is_whole_memory_deallocated(), >, 0);
    })

IOTC_TT_TESTCASE(
    utest__iotc_data_desc_append_byte__valid_data__write_byte_and_check_if_the_size_is_correct,
    {
      iotc_data_desc_t* desc = iotc_make_empty_desc_alloc(2);
      iotc_state_t local_state = IOTC_STATE_OK;

      tt_want_int_op(desc->length, ==, 0);

      local_state = iotc_data_desc_append_byte(desc, 'c');

      tt_want_int_op(local_state, ==, IOTC_STATE_OK);
      tt_want_int_op(desc->length, ==, 1);
      tt_want_int_op(desc->data_ptr[0], ==, 'c');

      local_state = iotc_data_desc_append_byte(desc, 'a');

      tt_want_int_op(local_state, ==, IOTC_STATE_OK);
      tt_want_int_op(desc->length, ==, 2);
      tt_want_int_op(desc->data_ptr[0], ==, 'c');
      tt_want_int_op(desc->data_ptr[1], ==, 'a');

      local_state = iotc_data_desc_append_byte(desc, 'z');

      tt_want_int_op(local_state, ==, IOTC_BUFFER_OVERFLOW);
      tt_want_int_op(desc->length, ==, 2);
      tt_want_int_op(desc->data_ptr[0], ==, 'c');
      tt_want_int_op(desc->data_ptr[1], ==, 'a');

      iotc_free_desc(&desc);

      tt_want_int_op(iotc_is_whole_memory_deallocated(), >, 0);
    })

IOTC_TT_TESTCASE(
    utest__iotc_data_desc_append_bytes__valid_data__write_bytes_and_check_if_the_size_is_correct,
    {
      const char data[] = "123456789";
      iotc_data_desc_t* desc = iotc_make_empty_desc_alloc(18);
      iotc_state_t local_state = IOTC_STATE_OK;

      tt_want_int_op(desc->length, ==, 0);

      local_state =
          iotc_data_desc_append_bytes(desc, (const uint8_t*)data, strlen(data));

      tt_want_int_op(local_state, ==, IOTC_STATE_OK);
      tt_want_int_op(desc->length, ==, strlen(data));
      tt_want_int_op(memcmp(desc->data_ptr, data, strlen(data)), ==, 0);

      local_state =
          iotc_data_desc_append_bytes(desc, (const uint8_t*)data, strlen(data));

      tt_want_int_op(local_state, ==, IOTC_STATE_OK);
      tt_want_int_op(desc->length, ==, strlen(data) * 2);
      tt_want_int_op(memcmp(desc->data_ptr, data, strlen(data)), ==, 0);
      tt_want_int_op(memcmp(desc->data_ptr + strlen(data), data, strlen(data)),
                     ==, 0);

      local_state =
          iotc_data_desc_append_bytes(desc, (const uint8_t*)data, strlen(data));

      tt_want_int_op(local_state, ==, IOTC_BUFFER_OVERFLOW);
      tt_want_int_op(desc->length, ==, strlen(data) * 2);
      tt_want_int_op(memcmp(desc->data_ptr, data, strlen(data)), ==, 0);
      tt_want_int_op(memcmp(desc->data_ptr + strlen(data), data, strlen(data)),
                     ==, 0);

      iotc_free_desc(&desc);

      tt_want_int_op(iotc_is_whole_memory_deallocated(), >, 0);
    })

IOTC_TT_TESTCASE(
    utest__iotc_data_desc_append_data_desc__valid_data__write_bytes_and_check_if_the_size_is_correct,
    {
      iotc_data_desc_t* desc = iotc_make_empty_desc_alloc(18);
      iotc_data_desc_t* src = iotc_make_desc_from_string_copy("123456789");
      iotc_state_t local_state = IOTC_STATE_OK;

      tt_want_int_op(desc->length, ==, 0);

      local_state = iotc_data_desc_append_data(desc, src);

      tt_want_int_op(local_state, ==, IOTC_STATE_OK);
      tt_want_int_op(desc->length, ==, src->length);
      tt_want_int_op(memcmp(desc->data_ptr, src->data_ptr, src->length), ==, 0);

      local_state = iotc_data_desc_append_data(desc, src);

      tt_want_int_op(local_state, ==, IOTC_STATE_OK);
      tt_want_int_op(desc->length, ==, src->length * 2);
      tt_want_int_op(memcmp(desc->data_ptr, src->data_ptr, src->length), ==, 0);
      tt_want_int_op(
          memcmp(desc->data_ptr + src->length, src->data_ptr, src->length), ==,
          0);

      local_state = iotc_data_desc_append_data(desc, src);

      tt_want_int_op(local_state, ==, IOTC_BUFFER_OVERFLOW);
      tt_want_int_op(desc->length, ==, src->length * 2);
      tt_want_int_op(memcmp(desc->data_ptr, src->data_ptr, src->length), ==, 0);
      tt_want_int_op(
          memcmp(desc->data_ptr + src->length, src->data_ptr, src->length), ==,
          0);

      iotc_free_desc(&src);
      iotc_free_desc(&desc);

      tt_want_int_op(iotc_is_whole_memory_deallocated(), >, 0);
    })

IOTC_TT_TESTGROUP_END

#ifndef IOTC_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#define IOTC_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#include __FILE__
#undef IOTC_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#endif
