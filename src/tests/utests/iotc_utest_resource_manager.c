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
#include "tinytest.h"
#include "tinytest_macros.h"

#include "iotc.h"
#include "iotc_fs_filenames.h"
#include "iotc_internals.h"
#include "iotc_resource_manager.h"

#include "iotc_globals.h"
#include "iotc_helpers.h"
#include "iotc_memory_checks.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef IOTC_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN

#include "iotc_fs_header.h"
#include "iotc_macros.h"

typedef enum iotc_utest_rm_fs_function_type_e {
  IOTC_UTEST_RM_FS_FUNCTION_STAT = 0,
  IOTC_UTEST_RM_FS_FUNCTION_OPEN,
  IOTC_UTEST_RM_FS_FUNCTION_READ,
  IOTC_UTEST_RM_FS_FUNCTION_WRITE,
  IOTC_UTEST_RM_FS_FUNCTION_CLOSE,
  IOTC_UTEST_RM_FS_FUNCTION_REMOVE,
  IOTC_UTEST_RM_FS_FUNCITON_COUNT
} iotc_utest_rm_fs_function_type_e;

/* test buffer, contains 20 characters, the test is using 4 chunks each 5
 * characters */
static const uint8_t iotc_utest_resource_manager_test_buffer[] = {
    'c', 'h', 'u', 'n', 'k', 't', 'e', 's', 't', '1',
    't', 'e', 's', 't', '2', 't', 'e', 's', 't', '3'};

static uint8_t
    iotc_utest_resource_manager_fs_counters[IOTC_UTEST_RM_FS_FUNCITON_COUNT] = {
        0, 0, 0, 0, 0, 0};

static uint8_t
    iotc_utest_resource_manager_fs_limits[IOTC_UTEST_RM_FS_FUNCITON_COUNT] = {
        0};

static const uint8_t iotc_utest_resource_manager_fs_limit_test_cases
    [][IOTC_UTEST_RM_FS_FUNCITON_COUNT] = {
        {1 /* STAT */, 1 /* OPEN */, 1 /* READ */, 1 /* WRITE */, 1 /* CLOSE */,
         1 /* REMOVE */},
        {4 /* STAT */, 4 /* OPEN */, 4 /* READ */, 4 /* WRITE */, 4 /* CLOSE */,
         4 /* REMOVE */}};

void iotc_utest_resource_manager_fs_set_limit_test_case(const size_t no) {
  /* PRE-CONDITION */
  assert(no < IOTC_ARRAYSIZE(iotc_utest_resource_manager_fs_limit_test_cases));
  memcpy((void*)iotc_utest_resource_manager_fs_limits,
         (const void*)iotc_utest_resource_manager_fs_limit_test_cases[no],
         sizeof(iotc_utest_resource_manager_fs_limits));
}

void iotc_utest_resource_manager_fs_counter_reset() {
  memset(&iotc_utest_resource_manager_fs_counters, 0,
         sizeof(iotc_utest_resource_manager_fs_counters));
}

#define IOTC_UTEST_RM_FS_FUNCTION_BODY(function_type, ret_state, common,   \
                                       chunk_action, limit_reached_action) \
  common;                                                                  \
                                                                           \
  iotc_utest_resource_manager_fs_counters[function_type] += 1;             \
                                                                           \
  if (iotc_utest_resource_manager_fs_counters[function_type] ==            \
      iotc_utest_resource_manager_fs_limits[function_type]) {              \
    limit_reached_action;                                                  \
    return IOTC_STATE_OK;                                                  \
  }                                                                        \
                                                                           \
  chunk_action;                                                            \
                                                                           \
  return ret_state

/* happy versions of file system operations */

iotc_state_t iotc_utest_resource_manager_fs_stat(
    const void* context, const iotc_fs_resource_type_t resource_type,
    const char* const resource_name, iotc_fs_stat_t* resource_stat) {
  IOTC_UNUSED(context);
  IOTC_UNUSED(resource_type);
  IOTC_UNUSED(resource_name);
  IOTC_UNUSED(resource_stat);

  IOTC_UTEST_RM_FS_FUNCTION_BODY(
      IOTC_UTEST_RM_FS_FUNCTION_STAT, IOTC_STATE_WANT_READ, {}, {}, {
        resource_stat->resource_size =
            sizeof(iotc_utest_resource_manager_test_buffer);
      });
}

iotc_state_t iotc_utest_resource_manager_fs_open(
    const void* context, const iotc_fs_resource_type_t resource_type,
    const char* const resource_name, const iotc_fs_open_flags_t open_flags,
    iotc_fs_resource_handle_t* resource_handle) {
  IOTC_UNUSED(context);
  IOTC_UNUSED(resource_type);
  IOTC_UNUSED(resource_name);
  IOTC_UNUSED(open_flags);
  IOTC_UNUSED(resource_handle);

  IOTC_UTEST_RM_FS_FUNCTION_BODY(IOTC_UTEST_RM_FS_FUNCTION_OPEN,
                                 IOTC_STATE_WANT_READ, {}, {},
                                 { *resource_handle = 0; });
}

iotc_state_t iotc_utest_resource_manager_fs_read(
    const void* context, const iotc_fs_resource_handle_t resource_handle,
    const size_t offset, const uint8_t** buffer, size_t* const buffer_size) {
  IOTC_UNUSED(context);
  IOTC_UNUSED(resource_handle);
  IOTC_UNUSED(offset);
  IOTC_UNUSED(buffer);
  IOTC_UNUSED(buffer_size);

  IOTC_UTEST_RM_FS_FUNCTION_BODY(
      IOTC_UTEST_RM_FS_FUNCTION_READ, IOTC_STATE_WANT_READ,
      const uint8_t limit_value =
          iotc_utest_resource_manager_fs_limits[IOTC_UTEST_RM_FS_FUNCTION_READ];

      const size_t data_size = sizeof(iotc_utest_resource_manager_test_buffer);
      const size_t chunk_size = data_size / limit_value;
      const size_t chunk_offset = offset;
      ,
      {
        *buffer =
            iotc_utest_resource_manager_test_buffer + (intptr_t)chunk_offset;
        *buffer_size = chunk_size;
      },
      {
        *buffer =
            iotc_utest_resource_manager_test_buffer + (intptr_t)chunk_offset;
        *buffer_size = chunk_size;
      });
}

iotc_state_t iotc_utest_resource_manager_fs_write(
    const void* context, const iotc_fs_resource_handle_t resource_handle,
    const uint8_t* const buffer, const size_t buffer_size, const size_t offset,
    size_t* const bytes_written) {
  IOTC_UNUSED(context);
  IOTC_UNUSED(resource_handle);
  IOTC_UNUSED(buffer);
  IOTC_UNUSED(buffer_size);
  IOTC_UNUSED(offset);
  IOTC_UNUSED(bytes_written);

  return IOTC_FS_ERROR;
}

iotc_state_t iotc_utest_resource_manager_fs_close(
    const void* context, const iotc_fs_resource_handle_t resource_handle) {
  IOTC_UNUSED(context);
  IOTC_UNUSED(resource_handle);

  IOTC_UTEST_RM_FS_FUNCTION_BODY(IOTC_UTEST_RM_FS_FUNCTION_CLOSE,
                                 IOTC_STATE_WANT_READ, {}, {}, {});
}

iotc_state_t iotc_utest_resource_manager_fs_remove(
    const void* context, const iotc_fs_resource_type_t resource_type,
    const char* const resource_name) {
  IOTC_UNUSED(context);
  IOTC_UNUSED(resource_type);
  IOTC_UNUSED(resource_name);

  return IOTC_FS_RESOURCE_NOT_AVAILABLE;
}

/* Broken version of filesystem operations */

iotc_state_t iotc_utest_resource_manager_fs_failing_stat(
    const void* context, const iotc_fs_resource_type_t resource_type,
    const char* const resource_name, iotc_fs_stat_t* resource_stat) {
  IOTC_UNUSED(context);
  IOTC_UNUSED(resource_type);
  IOTC_UNUSED(resource_name);
  IOTC_UNUSED(resource_stat);

  return IOTC_FS_ERROR;
}

iotc_state_t iotc_utest_resource_manager_fs_failing_open(
    const void* context, const iotc_fs_resource_type_t resource_type,
    const char* const resource_name, const iotc_fs_open_flags_t open_flags,
    iotc_fs_resource_handle_t* resource_handle) {
  IOTC_UNUSED(context);
  IOTC_UNUSED(resource_type);
  IOTC_UNUSED(resource_name);
  IOTC_UNUSED(open_flags);
  IOTC_UNUSED(resource_handle);

  return IOTC_FS_ERROR;
}

iotc_state_t iotc_utest_resource_manager_fs_failing_read(
    const void* context, const iotc_fs_resource_handle_t resource_handle,
    const size_t offset, const uint8_t** buffer, size_t* const buffer_size) {
  IOTC_UNUSED(context);
  IOTC_UNUSED(resource_handle);
  IOTC_UNUSED(offset);
  IOTC_UNUSED(buffer);
  IOTC_UNUSED(buffer_size);

  return IOTC_FS_ERROR;
}

iotc_state_t iotc_utest_resource_manager_fs_failing_write(
    const void* context, const iotc_fs_resource_handle_t resource_handle,
    const uint8_t* const buffer, const size_t buffer_size, const size_t offset,
    size_t* const bytes_written) {
  IOTC_UNUSED(context);
  IOTC_UNUSED(resource_handle);
  IOTC_UNUSED(buffer);
  IOTC_UNUSED(buffer_size);
  IOTC_UNUSED(offset);
  IOTC_UNUSED(bytes_written);

  return IOTC_FS_ERROR;
}

iotc_state_t iotc_utest_resource_manager_fs_failing_close(
    const void* context, const iotc_fs_resource_handle_t resource_handle) {
  IOTC_UNUSED(context);
  IOTC_UNUSED(resource_handle);

  return IOTC_FS_ERROR;
}

iotc_state_t iotc_utest_resource_manager_fs_failing_remove(
    const void* context, const iotc_fs_resource_type_t resource_type,
    const char* const resource_name) {
  IOTC_UNUSED(context);
  IOTC_UNUSED(resource_type);
  IOTC_UNUSED(resource_name);

  return IOTC_FS_RESOURCE_NOT_AVAILABLE;
}

/* local filesystem functions that we are going to inject in order to test
    the resource manager */
static iotc_fs_functions_t iotc_resource_manager_fs_functions = {
    sizeof(iotc_fs_functions_t),
    &iotc_utest_resource_manager_fs_stat,
    &iotc_utest_resource_manager_fs_open,
    &iotc_utest_resource_manager_fs_read,
    &iotc_utest_resource_manager_fs_write,
    &iotc_utest_resource_manager_fs_close,
    &iotc_utest_resource_manager_fs_remove};

/* local filesystem functions that fails we are going to inject in order to test
    the resource manager */
static iotc_fs_functions_t iotc_resource_manager_fs_failing_functions = {
    sizeof(iotc_fs_functions_t),
    &iotc_utest_resource_manager_fs_failing_stat,
    &iotc_utest_resource_manager_fs_failing_open,
    &iotc_utest_resource_manager_fs_failing_read,
    &iotc_utest_resource_manager_fs_failing_write,
    &iotc_utest_resource_manager_fs_failing_close,
    &iotc_utest_resource_manager_fs_failing_remove};

/* simulates the layer callback, it increases the int value passed through void*
 * data parameter */
iotc_state_t iotc_utest_resource_manager_open_callback(void* data, void* data1,
                                                       iotc_state_t state) {
  IOTC_UNUSED(data1);
  IOTC_UNUSED(state);

  int* p = (int*)data;

  if (state == IOTC_STATE_OK) {
    *p += 1;
  } else {
    *p += 1000;
  }

  return IOTC_STATE_OK;
}

/* simulates the layer callback, it increases the int value passed through void*
 * data parameter */
iotc_state_t iotc_utest_resource_manager_read_callback(void* data, void* data1,
                                                       iotc_state_t state) {
  IOTC_UNUSED(data1);
  IOTC_UNUSED(state);

  int* p = (int*)data;
  if (state == IOTC_STATE_OK) {
    *p += 1;
  } else {
    *p += 1000;
  }

  return IOTC_STATE_OK;
}

/* simulates the layer callback, it increases the int value passed through void*
 * data parameter */
iotc_state_t iotc_utest_resource_manager_close_callback(void* data, void* data1,
                                                        iotc_state_t state) {
  IOTC_UNUSED(data1);
  IOTC_UNUSED(state);

  int* p = (int*)data;
  if (state == IOTC_STATE_OK) {
    *p += 1;
  } else {
    *p += 1000;
  }

  return IOTC_STATE_OK;
}

#endif

IOTC_TT_TESTGROUP_BEGIN(utest_resource_manager)

IOTC_TT_TESTCASE(
    utest__iotc_resource_manager_create_context__valid_data_empty_buffer__return_IOTC_STATE_OK,
    {
      iotc_resource_manager_context_t* context = NULL;
      iotc_state_t res = IOTC_STATE_OK;

      res = iotc_resource_manager_make_context(NULL, &context);
      tt_int_op(IOTC_STATE_OK, ==, res);

      tt_int_op(NULL, !=, context);
      tt_ptr_op(NULL, ==, context->data_buffer);
      tt_int_op(0, ==, context->cs);
      tt_int_op(0, >, context->resource_handle);
      tt_int_op(0, ==, context->open_flags);
      tt_int_op(IOTC_MEMORY_TYPE_MANAGED, ==, context->memory_type);

      res = iotc_resource_manager_free_context(&context);
      tt_int_op(IOTC_STATE_OK, ==, res);

      tt_ptr_op(NULL, ==, context);

      tt_int_op(iotc_is_whole_memory_deallocated(), >, 0);

      return;
    end:
      iotc_resource_manager_free_context(&context);
    })

IOTC_TT_TESTCASE(
    utest__iotc_resource_manager_create_context__valid_data_filled_buffer__return_IOTC_STATE_OK,
    {
      iotc_resource_manager_context_t* context = NULL;
      iotc_data_desc_t* data_desc = iotc_make_empty_desc_alloc(128);
      iotc_state_t res = IOTC_STATE_OK;

      tt_ptr_op(NULL, !=, data_desc);

      res = iotc_resource_manager_make_context(data_desc, &context);
      tt_int_op(IOTC_STATE_OK, ==, res);

      tt_int_op(NULL, !=, context);
      tt_ptr_op(context->data_buffer, ==, data_desc);
      tt_int_op(0, ==, context->cs);
      tt_int_op(0, >, context->resource_handle);
      tt_int_op(0, ==, context->open_flags);
      tt_int_op(IOTC_MEMORY_TYPE_UNMANAGED, ==, context->memory_type);

      res = iotc_resource_manager_free_context(&context);
      tt_int_op(IOTC_STATE_OK, ==, res);
      tt_ptr_op(NULL, ==, context);

      iotc_free_desc(&data_desc);

      tt_int_op(iotc_is_whole_memory_deallocated(), >, 0);

      return;
    end:
      iotc_free_desc(&data_desc);
      iotc_resource_manager_free_context(&context);
    })

IOTC_TT_TESTCASE(
    utest__iotc_resource_manager_open_resource__valid_data__return_IOTC_STATE_OK,
    {
      iotc_fs_functions_t fs_functions_copy = iotc_internals.fs_functions;
      iotc_internals.fs_functions = iotc_resource_manager_fs_functions;
      iotc_state_t res = IOTC_STATE_OK;

      iotc_resource_manager_context_t* context = NULL;

      size_t test_no = 0;
      for (; test_no <
             IOTC_ARRAYSIZE(iotc_utest_resource_manager_fs_limit_test_cases);
           ++test_no) {
        int test_value = 0;
        iotc_utest_resource_manager_fs_set_limit_test_case(test_no);

        iotc_globals.evtd_instance = iotc_evtd_create_instance();
        tt_ptr_op(NULL, !=, iotc_globals.evtd_instance);

        res = iotc_resource_manager_make_context(NULL, &context);
        tt_int_op(IOTC_STATE_OK, ==, res);

        res = iotc_resource_manager_open(
            context,
            iotc_make_handle(&iotc_utest_resource_manager_open_callback,
                             &test_value, NULL, IOTC_STATE_OK),
            IOTC_FS_CERTIFICATE, IOTC_GLOBAL_CERTIFICATE_FILE_NAME,
            IOTC_FS_OPEN_READ, NULL);

        tt_int_op(IOTC_STATE_OK, ==, res);

        /* invocation of coroutine */
        iotc_evtd_single_step(iotc_globals.evtd_instance, 1);

        {
          const uint8_t least_required_num_of_evtd_steps =
              iotc_utest_resource_manager_fs_limits
                  [IOTC_UTEST_RM_FS_FUNCTION_OPEN] +
              iotc_utest_resource_manager_fs_limits
                  [IOTC_UTEST_RM_FS_FUNCTION_STAT];
          uint8_t steps_already_taken = 2; /* because two times the coroutine
                                              get's activated automaticaly */

          for (; steps_already_taken < least_required_num_of_evtd_steps;
               ++steps_already_taken) {
            tt_int_op(
                1, ==,
                iotc_evtd_update_file_fd_events(iotc_globals.evtd_instance));
            tt_int_op(0, ==, test_value);
          }
        }

        tt_int_op(0, ==, test_value);

        /* invocation of callback */
        iotc_evtd_single_step(iotc_globals.evtd_instance, 1);

        tt_int_op(1, ==, test_value);
        tt_int_op(0, <=, context->resource_handle);

        iotc_evtd_destroy_instance(iotc_globals.evtd_instance);

        res = iotc_resource_manager_free_context(&context);
        tt_int_op(IOTC_STATE_OK, ==, res);
        tt_ptr_op(NULL, ==, context);

        tt_int_op(iotc_is_whole_memory_deallocated(), >, 0);

        iotc_utest_resource_manager_fs_counter_reset();
      }

      iotc_internals.fs_functions = fs_functions_copy;

      return;

    end:
      iotc_evtd_destroy_instance(iotc_globals.evtd_instance);
      iotc_resource_manager_free_context(&context);
      iotc_internals.fs_functions = fs_functions_copy;
      iotc_utest_resource_manager_fs_counter_reset();
    })

IOTC_TT_TESTCASE(
    utest__iotc_resource_manager_read_resource__valid_data__return_IOTC_STATE_OK_and_buffer_filled_with_data,
    {
      iotc_resource_manager_context_t* context = NULL;
      iotc_data_desc_t* local_buffer = NULL;

      iotc_fs_functions_t fs_functions_copy = iotc_internals.fs_functions;
      iotc_internals.fs_functions = iotc_resource_manager_fs_functions;

      iotc_state_t res = IOTC_STATE_OK;

      size_t test_no = 0;
      for (; test_no <
             IOTC_ARRAYSIZE(iotc_utest_resource_manager_fs_limit_test_cases);
           ++test_no) {
        int test_type = 0;
        for (; test_type < 1; ++test_type) {
          int test_value = 0;
          iotc_utest_resource_manager_fs_set_limit_test_case(test_no);

          iotc_globals.evtd_instance = iotc_evtd_create_instance();
          tt_ptr_op(NULL, !=, iotc_globals.evtd_instance);

          if (test_type == 0) {
            local_buffer = iotc_make_empty_desc_alloc(1);
            res = iotc_resource_manager_make_context(local_buffer, &context);
            tt_int_op(IOTC_STATE_OK, ==, res);
          } else {
            res = iotc_resource_manager_make_context(NULL, &context);
            tt_int_op(IOTC_STATE_OK, ==, res);
          }

          iotc_state_t res = iotc_resource_manager_open(
              context,
              iotc_make_handle(&iotc_utest_resource_manager_open_callback,
                               &test_value, NULL, IOTC_STATE_OK),
              IOTC_FS_CERTIFICATE, IOTC_GLOBAL_CERTIFICATE_FILE_NAME,
              IOTC_FS_OPEN_READ, NULL);

          tt_int_op(IOTC_STATE_OK, ==, res);

          /* invocation of open coroutine */
          iotc_evtd_single_step(iotc_globals.evtd_instance, 1);

          /* open partial reading */
          {
            const uint8_t least_required_num_of_evtd_steps =
                iotc_utest_resource_manager_fs_limits
                    [IOTC_UTEST_RM_FS_FUNCTION_OPEN] +
                iotc_utest_resource_manager_fs_limits
                    [IOTC_UTEST_RM_FS_FUNCTION_STAT];
            uint8_t steps_already_taken =
                2; /* two times the coroutine gets activated automaticaly */

            for (; steps_already_taken < least_required_num_of_evtd_steps;
                 ++steps_already_taken) {
              tt_int_op(
                  1, ==,
                  iotc_evtd_update_file_fd_events(iotc_globals.evtd_instance));
              tt_int_op(0, ==, test_value);
            }
          }

          /* invocation of open callback */
          iotc_evtd_single_step(iotc_globals.evtd_instance, 1);

          tt_int_op(1, ==, test_value);

          /* the resource is open and ready to read */
          res = iotc_resource_manager_read(
              context,
              iotc_make_handle(&iotc_utest_resource_manager_read_callback,
                               &test_value, NULL, IOTC_STATE_OK),
              NULL);

          tt_int_op(IOTC_STATE_OK, ==, res);

          /* invocation of read coroutine */
          iotc_evtd_single_step(iotc_globals.evtd_instance, 1);

          /* call each part of read */
          {
            const uint8_t least_required_num_of_evtd_steps =
                iotc_utest_resource_manager_fs_limits
                    [IOTC_UTEST_RM_FS_FUNCTION_READ];
            uint8_t steps_already_taken =
                1; /* one time the coroutine gets activated automaticaly */

            for (; steps_already_taken < least_required_num_of_evtd_steps;
                 ++steps_already_taken) {
              tt_int_op(
                  1, ==,
                  iotc_evtd_update_file_fd_events(iotc_globals.evtd_instance));
              tt_int_op(1, ==, test_value);
            }
          }

          tt_int_op(1, ==, test_value);

          /* invocation of read callback function */
          iotc_evtd_single_step(iotc_globals.evtd_instance, 1);

          /* here the whole buffer should be read */
          tt_int_op(2, ==, test_value);
          tt_int_op(context->data_buffer->length, ==,
                    sizeof(iotc_utest_resource_manager_test_buffer));
          tt_int_op(0, ==,
                    memcmp(context->data_buffer->data_ptr,
                           iotc_utest_resource_manager_test_buffer,
                           sizeof(iotc_utest_resource_manager_test_buffer)));

          /* the resource is ready to be closed */
          res = iotc_resource_manager_close(
              context,
              iotc_make_handle(&iotc_utest_resource_manager_close_callback,
                               &test_value, NULL, IOTC_STATE_OK),
              NULL);

          /* invocation of close coroutine */
          iotc_evtd_single_step(iotc_globals.evtd_instance, 1);

          /* call each part of close */
          {
            const uint8_t least_required_num_of_evtd_steps =
                iotc_utest_resource_manager_fs_limits
                    [IOTC_UTEST_RM_FS_FUNCTION_CLOSE];
            uint8_t steps_already_taken =
                1; /* one time coroutine gets activated automaticaly */

            for (; steps_already_taken < least_required_num_of_evtd_steps;
                 ++steps_already_taken) {
              tt_int_op(
                  1, ==,
                  iotc_evtd_update_file_fd_events(iotc_globals.evtd_instance));
              tt_int_op(2, ==, test_value);
            }
          }

          tt_int_op(2, ==, test_value);

          /* invocation of close callback function */
          iotc_evtd_single_step(iotc_globals.evtd_instance, 1);

          tt_int_op(3, ==, test_value);
          tt_int_op(0, >, context->resource_handle);

          /* the resource is closed */
          iotc_evtd_destroy_instance(iotc_globals.evtd_instance);

          res = iotc_resource_manager_free_context(&context);
          tt_int_op(IOTC_STATE_OK, ==, res);
          tt_ptr_op(NULL, ==, context);

          if (test_type == 0) {
            iotc_free_desc(&local_buffer);
          }

          tt_int_op(iotc_is_whole_memory_deallocated(), >, 0);

          iotc_utest_resource_manager_fs_counter_reset();
        }
      }

      iotc_internals.fs_functions = fs_functions_copy;

      return;

    end:
      iotc_evtd_destroy_instance(iotc_globals.evtd_instance);
      iotc_resource_manager_free_context(&context);
      iotc_internals.fs_functions = fs_functions_copy;
      iotc_utest_resource_manager_fs_counter_reset();
      iotc_free_desc(&local_buffer);
    })

IOTC_TT_TESTCASE(
    utest__iotc_resource_manager_read_resource__valid_data__fs_functions_failing,
    {
      iotc_resource_manager_context_t* context = NULL;
      iotc_data_desc_t* local_buffer = NULL;

      iotc_fs_functions_t fs_functions_copy = iotc_internals.fs_functions;
      iotc_internals.fs_functions = iotc_resource_manager_fs_failing_functions;
      iotc_state_t res = IOTC_STATE_OK;

      size_t test_no = 0;
      for (; test_no <
             IOTC_ARRAYSIZE(iotc_utest_resource_manager_fs_limit_test_cases);
           ++test_no) {
        int test_type = 0;
        for (; test_type < 1; ++test_type) {
          int test_value = 0;
          iotc_utest_resource_manager_fs_set_limit_test_case(test_no);

          iotc_globals.evtd_instance = iotc_evtd_create_instance();
          tt_ptr_op(NULL, !=, iotc_globals.evtd_instance);

          if (test_type == 0) {
            local_buffer = iotc_make_empty_desc_alloc(1);
            res = iotc_resource_manager_make_context(local_buffer, &context);
            tt_int_op(IOTC_STATE_OK, ==, res);
          } else {
            res = iotc_resource_manager_make_context(NULL, &context);
            tt_int_op(IOTC_STATE_OK, ==, res);
          }

          iotc_state_t res = iotc_resource_manager_open(
              context,
              iotc_make_handle(&iotc_utest_resource_manager_open_callback,
                               &test_value, NULL, IOTC_STATE_OK),
              IOTC_FS_CERTIFICATE, IOTC_GLOBAL_CERTIFICATE_FILE_NAME,
              IOTC_FS_OPEN_READ, NULL);

          tt_int_op(IOTC_STATE_OK, ==, res);

          /* invocation of open coroutine */
          iotc_evtd_single_step(iotc_globals.evtd_instance, 1);

          tt_int_op(1000, ==, test_value);

          /* let's pretend that we have valid handle */
          context->resource_handle = 1;

          /* the resource is open and ready to read */
          res = iotc_resource_manager_read(
              context,
              iotc_make_handle(&iotc_utest_resource_manager_read_callback,
                               &test_value, NULL, IOTC_STATE_OK),
              NULL);

          tt_int_op(IOTC_STATE_OK, ==, res);

          /* invocation of read coroutine */
          iotc_evtd_single_step(iotc_globals.evtd_instance, 1);

          /* here the whole buffer should be read */
          tt_int_op(2000, ==, test_value);

          if (test_type == 1) {
            tt_ptr_op(NULL, ==, context->data_buffer);
          }

          /* the resource is ready to be closed */
          res = iotc_resource_manager_close(
              context,
              iotc_make_handle(&iotc_utest_resource_manager_close_callback,
                               &test_value, NULL, IOTC_STATE_OK),
              NULL);

          /* invocation of close coroutine */
          iotc_evtd_single_step(iotc_globals.evtd_instance, 1);

          tt_int_op(3000, ==, test_value);

          iotc_evtd_destroy_instance(iotc_globals.evtd_instance);

          res = iotc_resource_manager_free_context(&context);
          tt_int_op(IOTC_STATE_OK, ==, res);
          tt_ptr_op(NULL, ==, context);

          if (test_type == 0) {
            iotc_free_desc(&local_buffer);
          }

          tt_int_op(iotc_is_whole_memory_deallocated(), >, 0);

          iotc_utest_resource_manager_fs_counter_reset();
        }
      }

      iotc_internals.fs_functions = fs_functions_copy;

      return;

    end:
      iotc_evtd_destroy_instance(iotc_globals.evtd_instance);
      iotc_resource_manager_free_context(&context);
      iotc_internals.fs_functions = fs_functions_copy;
      iotc_utest_resource_manager_fs_counter_reset();
      iotc_free_desc(&local_buffer);
    })

IOTC_TT_TESTGROUP_END

#ifndef IOTC_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#define IOTC_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#include __FILE__
#undef IOTC_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#endif
