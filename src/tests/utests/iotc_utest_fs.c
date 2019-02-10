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
#include "tinytest.h"
#include "tinytest_macros.h"

#include "iotc.h"
#include "iotc_fs_api.h"
#include "iotc_fs_header.h"
#include "iotc_internals.h"
#include "iotc_macros.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef IOTC_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN

iotc_state_t iotc_fs_utest_stat(const void* context,
                                const iotc_fs_resource_type_t resource_type,
                                const char* const resource_name,
                                iotc_fs_stat_t* resource_stat) {
  IOTC_UNUSED(context);
  IOTC_UNUSED(resource_type);
  IOTC_UNUSED(resource_name);
  IOTC_UNUSED(resource_stat);

  return IOTC_FS_RESOURCE_NOT_AVAILABLE;
}

iotc_state_t iotc_fs_utest_open(const void* context,
                                const iotc_fs_resource_type_t resource_type,
                                const char* const resource_name,
                                const iotc_fs_open_flags_t open_flags,
                                iotc_fs_resource_handle_t* resource_handle) {
  IOTC_UNUSED(context);
  IOTC_UNUSED(resource_type);
  IOTC_UNUSED(resource_name);
  IOTC_UNUSED(open_flags);
  IOTC_UNUSED(resource_handle);

  return IOTC_FS_ERROR;
}

iotc_state_t iotc_fs_utest_read(const void* context,
                                const iotc_fs_resource_handle_t resource_handle,
                                const size_t offset, const uint8_t** buffer,
                                size_t* const buffer_size) {
  IOTC_UNUSED(context);
  IOTC_UNUSED(resource_handle);
  IOTC_UNUSED(offset);
  IOTC_UNUSED(buffer);
  IOTC_UNUSED(buffer_size);

  return IOTC_FS_ERROR;
}

iotc_state_t iotc_fs_utest_write(
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

iotc_state_t iotc_fs_utest_close(
    const void* context, const iotc_fs_resource_handle_t resource_handle) {
  IOTC_UNUSED(context);
  IOTC_UNUSED(resource_handle);

  return IOTC_FS_ERROR;
}

iotc_state_t iotc_fs_utest_remove(const void* context,
                                  const iotc_fs_resource_type_t resource_type,
                                  const char* const resource_name) {
  IOTC_UNUSED(context);
  IOTC_UNUSED(resource_type);
  IOTC_UNUSED(resource_name);

  return IOTC_FS_RESOURCE_NOT_AVAILABLE;
}

#endif

IOTC_TT_TESTGROUP_BEGIN(utest_fs)

#ifdef IOTC_EXPOSE_FS
IOTC_TT_TESTCASE(utest__iotc_set_fs_functions__correct_values, {
  const iotc_fs_functions_t fs_functions = {
      sizeof(iotc_fs_functions_t), &iotc_fs_utest_stat,  &iotc_fs_utest_open,
      &iotc_fs_utest_read,         &iotc_fs_utest_write, &iotc_fs_utest_close,
      &iotc_fs_utest_remove};

  iotc_state_t ret = iotc_set_fs_functions(fs_functions);

  tt_int_op(ret, ==, IOTC_STATE_OK);

  tt_ptr_op(iotc_internals.fs_functions.stat_resource, ==, &iotc_fs_utest_stat);
  tt_ptr_op(iotc_internals.fs_functions.open_resource, ==, &iotc_fs_utest_open);
  tt_ptr_op(iotc_internals.fs_functions.read_resource, ==, &iotc_fs_utest_read);
  tt_ptr_op(iotc_internals.fs_functions.write_resource, ==,
            &iotc_fs_utest_write);
  tt_ptr_op(iotc_internals.fs_functions.close_resource, ==,
            &iotc_fs_utest_close);
  tt_ptr_op(iotc_internals.fs_functions.remove_resource, ==,
            &iotc_fs_utest_remove);
end:;
})

IOTC_TT_TESTCASE(utest__iotc_set_fs_functions__incorrect_size, {
  const iotc_fs_functions_t fs_functions = {sizeof(iotc_fs_functions_t) - 2,
                                            &iotc_fs_utest_stat,
                                            &iotc_fs_utest_open,
                                            &iotc_fs_utest_read,
                                            &iotc_fs_utest_write,
                                            &iotc_fs_utest_close,
                                            &iotc_fs_utest_remove};

  iotc_state_t ret = iotc_set_fs_functions(fs_functions);

  tt_int_op(ret, ==, IOTC_INTERNAL_ERROR);
end:;
})
#endif

/* This is the shared tests part, this behaviour should be common for all fs
 * implementations except dummy */
#if defined(IOTC_FS_MEMORY) || defined(IOTC_FS_POSIX)
IOTC_TT_TESTCASE(
    utest__iotc_fs_stat__invalid_resource_name__invalid_parameter_returned, {
      iotc_fs_stat_t stat = {.resource_size = 0};
      iotc_state_t ret = iotc_fs_stat(NULL, IOTC_FS_CERTIFICATE, NULL, &stat);

      tt_int_op(ret, ==, IOTC_INVALID_PARAMETER);
    end:;
    })

IOTC_TT_TESTCASE(utest__iotc_fs_stat__invalid_stat__invalid_parameter_returned,
                 {
                   iotc_state_t ret =
                       iotc_fs_stat(NULL, IOTC_FS_CERTIFICATE,
                                    "file_name_that_does_not_exist!", NULL);

                   tt_int_op(ret, ==, IOTC_INVALID_PARAMETER);
                 end:;
                 })

IOTC_TT_TESTCASE(
    utest__iotc_fs_stat__invalid_file_name_parameters__resource_not_availible_returned,
    {
      iotc_fs_stat_t stat = {.resource_size = 0};
      iotc_state_t ret = iotc_fs_stat(NULL, IOTC_FS_CERTIFICATE,
                                      "file_name_that_does_not_exist!", &stat);

      tt_int_op(ret, ==, IOTC_FS_RESOURCE_NOT_AVAILABLE);
      tt_int_op(stat.resource_size, ==, 0);
    end:;
    })

IOTC_TT_TESTCASE(
    utest__iotc_fs_open__invalid_file_name__resource_not_availible_returned, {
      iotc_fs_resource_handle_t resource_handle =
          iotc_fs_init_resource_handle();

      iotc_state_t ret = iotc_fs_open(NULL, IOTC_FS_CONFIG_DATA,
                                      "file_name_that_does_not_exist!",
                                      IOTC_FS_OPEN_READ, &resource_handle);

      tt_int_op(ret, ==, IOTC_FS_RESOURCE_NOT_AVAILABLE);
      tt_int_op(resource_handle, ==, IOTC_FS_INVALID_RESOURCE_HANDLE);
    end:;
    })

IOTC_TT_TESTCASE(
    utest__iotc_fs_read__invalid_resource_handle__invalid_parameter_returned, {
      iotc_fs_resource_handle_t resource_handle =
          iotc_fs_init_resource_handle();

      const uint8_t* buffer = NULL;
      size_t buffer_size = 0;

      iotc_state_t ret =
          iotc_fs_read(NULL, resource_handle, 0, &buffer, &buffer_size);

      tt_int_op(ret, ==, IOTC_INVALID_PARAMETER);
      tt_ptr_op(NULL, ==, buffer);
      tt_int_op(0, ==, buffer_size);
    end:;
    })

IOTC_TT_TESTCASE(utest__iotc_fs_memory_close__invalid_resource_handle, {
  iotc_fs_resource_handle_t resource_handle = iotc_fs_init_resource_handle();
  iotc_state_t ret = iotc_fs_close(NULL, resource_handle);

  tt_int_op(ret, ==, IOTC_INVALID_PARAMETER);
end:;
})
#endif

IOTC_TT_TESTGROUP_END

#ifndef IOTC_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#define IOTC_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#include __FILE__
#undef IOTC_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#endif
