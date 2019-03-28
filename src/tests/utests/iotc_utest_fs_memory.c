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

#include "iotc_tt_testcase_management.h"
#include "tinytest.h"
#include "tinytest_macros.h"

#include "iotc_fs_api.h"
#include "iotc_fs_filenames.h"
#include "iotc_fs_header.h"
#include "iotc_macros.h"

#ifndef IOTC_NO_TLS_LAYER
#include "iotc_RootCA_list.h"
#endif

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef IOTC_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#endif

IOTC_TT_TESTGROUP_BEGIN(utest_fs_memory)

#ifndef IOTC_NO_TLS_LAYER
IOTC_TT_TESTCASE(utest__iotc_fs_memory_stat__valid_data__stat_returned, {
  iotc_fs_stat_t stat = {.resource_size = 0};
  iotc_state_t ret = iotc_fs_stat(NULL, IOTC_FS_CERTIFICATE,
                                  IOTC_GLOBAL_CERTIFICATE_FILE_NAME, &stat);

  tt_int_op(ret, ==, IOTC_STATE_OK);
  tt_int_op(stat.resource_size, ==, sizeof(iotc_RootCA_list));

end:;
})
#endif

#ifndef IOTC_NO_TLS_LAYER
IOTC_TT_TESTCASE(
    utest__iotc_fs_memory_stat__invalid_stat__invalid_parameter_returned, {
      iotc_state_t ret = iotc_fs_stat(NULL, IOTC_FS_CERTIFICATE,
                                      IOTC_GLOBAL_CERTIFICATE_FILE_NAME, NULL);

      tt_int_op(ret, ==, IOTC_INVALID_PARAMETER);
    end:;
    })
#endif

#ifndef IOTC_NO_TLS_LAYER
IOTC_TT_TESTCASE(
    utest__iotc_fs_memory_open__valid_parameters__valid_resource_handle_returned,
    {
      iotc_fs_resource_handle_t resource_handle =
          iotc_fs_init_resource_handle();

      iotc_state_t ret = iotc_fs_open(NULL, IOTC_FS_CONFIG_DATA,
                                      IOTC_GLOBAL_CERTIFICATE_FILE_NAME,
                                      IOTC_FS_OPEN_READ, &resource_handle);

      tt_int_op(ret, ==, IOTC_STATE_OK);
      tt_int_op(resource_handle, !=, IOTC_FS_INVALID_RESOURCE_HANDLE);
    end:
      iotc_fs_close(NULL, resource_handle);
    })
#endif

#ifndef IOTC_NO_TLS_LAYER
IOTC_TT_TESTCASE(
    utest__iotc_fs_memory_read__valid_existing_resource__pointer_to_the_memory_block_returned,
    {
      iotc_fs_resource_handle_t resource_handle =
          iotc_fs_init_resource_handle();

      const uint8_t* buffer = NULL;
      size_t buffer_size = 0;

      iotc_state_t ret = iotc_fs_open(NULL, IOTC_FS_CERTIFICATE,
                                      IOTC_GLOBAL_CERTIFICATE_FILE_NAME,
                                      IOTC_FS_OPEN_READ, &resource_handle);

      tt_int_op(ret, ==, IOTC_STATE_OK);

      ret = iotc_fs_read(NULL, resource_handle, 0, &buffer, &buffer_size);

      tt_int_op(ret, ==, IOTC_STATE_OK);
      tt_ptr_op(iotc_RootCA_list, ==, buffer);
      tt_int_op(IOTC_MIN(sizeof(iotc_RootCA_list), iotc_fs_buffer_size), ==,
                buffer_size);
      tt_int_op(memcmp(iotc_RootCA_list, buffer,
                       IOTC_MIN(sizeof(iotc_RootCA_list), iotc_fs_buffer_size)),
                ==, 0);

    end:
      iotc_fs_close(NULL, resource_handle);
    })
#endif

#ifndef IOTC_NO_TLS_LAYER
IOTC_TT_TESTCASE(
    utest__iotc_fs_memory_read__valid_existing_resource_with_valid_offset__pointer_to_the_memory_block_with_given_offset_returned,
    {
      iotc_fs_resource_handle_t resource_handle =
          iotc_fs_init_resource_handle();

      const uint8_t* buffer = NULL;
      const size_t offset = sizeof(iotc_RootCA_list) / 2;
      const size_t size_left =
          IOTC_MIN(sizeof(iotc_RootCA_list) - offset, iotc_fs_buffer_size);
      size_t buffer_size = 0;

      iotc_state_t ret = iotc_fs_open(NULL, IOTC_FS_CERTIFICATE,
                                      IOTC_GLOBAL_CERTIFICATE_FILE_NAME,
                                      IOTC_FS_OPEN_READ, &resource_handle);

      tt_int_op(ret, ==, IOTC_STATE_OK);

      ret = iotc_fs_read(NULL, resource_handle, offset, &buffer, &buffer_size);

      tt_int_op(ret, ==, IOTC_STATE_OK);
      tt_ptr_op(iotc_RootCA_list + offset, ==, buffer);
      tt_int_op(size_left, ==, buffer_size);
      tt_int_op(memcmp(iotc_RootCA_list + offset, buffer, size_left), ==, 0);

    end:;
      iotc_fs_close(NULL, resource_handle);
    })
#endif

#ifndef IOTC_NO_TLS_LAYER
IOTC_TT_TESTCASE(
    utest__iotc_fs_memory_read__valid_existing_resource_with_invalid_offset__pointer_to_the_memory_block_with_clamped_offset_returned,
    {
      iotc_fs_resource_handle_t resource_handle =
          iotc_fs_init_resource_handle();

      const uint8_t* buffer = NULL;
      const size_t offset = sizeof(iotc_RootCA_list) + 1024;
      const size_t size_left = 0;
      size_t buffer_size = 0;

      iotc_state_t ret = iotc_fs_open(NULL, IOTC_FS_CERTIFICATE,
                                      IOTC_GLOBAL_CERTIFICATE_FILE_NAME,
                                      IOTC_FS_OPEN_READ, &resource_handle);

      tt_int_op(ret, ==, IOTC_STATE_OK);

      ret = iotc_fs_read(NULL, resource_handle, offset, &buffer, &buffer_size);

      tt_int_op(ret, ==, IOTC_STATE_OK);
      tt_ptr_op(iotc_RootCA_list + sizeof(iotc_RootCA_list), ==, buffer);
      tt_int_op(size_left, ==, buffer_size);
    end:
      iotc_fs_close(NULL, resource_handle);
    })
#endif

#ifndef IOTC_NO_TLS_LAYER
IOTC_TT_TESTCASE(
    utest__iotc_fs_memory_read__invalid_buffer__invalid_parameter_returned, {
      iotc_fs_resource_handle_t resource_handle =
          iotc_fs_init_resource_handle();

      size_t buffer_size = 0;

      iotc_state_t ret = iotc_fs_open(NULL, IOTC_FS_CERTIFICATE,
                                      IOTC_GLOBAL_CERTIFICATE_FILE_NAME,
                                      IOTC_FS_OPEN_READ, &resource_handle);

      tt_int_op(ret, ==, IOTC_STATE_OK);

      ret = iotc_fs_read(NULL, resource_handle, 0, NULL, &buffer_size);

      tt_int_op(ret, ==, IOTC_INVALID_PARAMETER);
      tt_int_op(0, ==, buffer_size);

    end:
      iotc_fs_close(NULL, resource_handle);
    })
#endif

#ifndef IOTC_NO_TLS_LAYER
IOTC_TT_TESTCASE(
    utest__iotc_fs_memory_read__invalid_buffer_size__invalid_parameter_returned,
    {
      iotc_fs_resource_handle_t resource_handle =
          iotc_fs_init_resource_handle();

      const uint8_t* buffer = NULL;

      iotc_state_t ret = iotc_fs_open(NULL, IOTC_FS_CERTIFICATE,
                                      IOTC_GLOBAL_CERTIFICATE_FILE_NAME,
                                      IOTC_FS_OPEN_READ, &resource_handle);

      tt_int_op(ret, ==, IOTC_STATE_OK);

      ret = iotc_fs_read(NULL, resource_handle, 0, &buffer, NULL);

      tt_int_op(ret, ==, IOTC_INVALID_PARAMETER);
      tt_int_op(NULL, ==, buffer);

    end:
      iotc_fs_close(NULL, resource_handle);
    })
#endif

IOTC_TT_TESTCASE(utest__iotc_fs_memory_write__correct_parameters, {
  iotc_fs_resource_handle_t resource_handle = iotc_fs_init_resource_handle();
  uint8_t buffer[1024] = {'a'};
  size_t buffer_size = 1024;
  size_t bytes_written = 0;

  iotc_state_t ret = iotc_fs_write(NULL, resource_handle, buffer, buffer_size,
                                   0, &bytes_written);

  tt_int_op(ret, ==, IOTC_FS_ERROR);
end:;
})

#ifndef IOTC_NO_TLS_LAYER
IOTC_TT_TESTCASE(utest__iotc_fs_memory_close__valid_resource_handle, {
  iotc_fs_resource_handle_t resource_handle = iotc_fs_init_resource_handle();

  iotc_state_t ret =
      iotc_fs_open(NULL, IOTC_FS_CONFIG_DATA, IOTC_GLOBAL_CERTIFICATE_FILE_NAME,
                   IOTC_FS_OPEN_READ, &resource_handle);

  tt_int_op(ret, ==, IOTC_STATE_OK);
  tt_int_op(resource_handle, !=, IOTC_FS_INVALID_RESOURCE_HANDLE);

  ret = iotc_fs_close(NULL, resource_handle);

  tt_int_op(ret, ==, IOTC_STATE_OK);
end:;
})
#endif

#ifndef IOTC_NO_TLS_LAYER
IOTC_TT_TESTCASE(
    utest__iotc_fs_memory_open_twice_close_twice__valid_resource_handle, {
      iotc_fs_resource_handle_t resource_handle1 =
          iotc_fs_init_resource_handle();

      iotc_state_t ret = IOTC_STATE_OK;

      ret = iotc_fs_open(NULL, IOTC_FS_CONFIG_DATA,
                         IOTC_GLOBAL_CERTIFICATE_FILE_NAME, IOTC_FS_OPEN_READ,
                         &resource_handle1);

      tt_int_op(ret, ==, IOTC_STATE_OK);
      tt_int_op(resource_handle1, !=, IOTC_FS_INVALID_RESOURCE_HANDLE);

      iotc_fs_resource_handle_t resource_handle2 =
          iotc_fs_init_resource_handle();

      ret = iotc_fs_open(NULL, IOTC_FS_CONFIG_DATA,
                         IOTC_GLOBAL_CERTIFICATE_FILE_NAME, IOTC_FS_OPEN_READ,
                         &resource_handle2);

      tt_int_op(ret, ==, IOTC_STATE_OK);
      tt_int_op(resource_handle2, !=, IOTC_FS_INVALID_RESOURCE_HANDLE);

      ret = iotc_fs_close(NULL, resource_handle1);

      tt_int_op(ret, ==, IOTC_STATE_OK);

      ret = iotc_fs_close(NULL, resource_handle2);

      tt_int_op(ret, ==, IOTC_STATE_OK);
    end:;
    })
#endif

IOTC_TT_TESTCASE(utest__iotc_fs_memory_remove__incorrect_parameters, {
  iotc_state_t ret = iotc_fs_remove(NULL, IOTC_FS_CONFIG_DATA, "test_name");

  tt_int_op(ret, ==, IOTC_FS_RESOURCE_NOT_AVAILABLE);
end:;
})

#ifndef IOTC_NO_TLS_LAYER
IOTC_TT_TESTCASE(utest__iotc_fs_memory_remove__correct_parameters, {
  iotc_state_t ret = iotc_fs_remove(NULL, IOTC_FS_CONFIG_DATA,
                                    IOTC_GLOBAL_CERTIFICATE_FILE_NAME);

  tt_int_op(ret, ==, IOTC_FS_ERROR);
end:;
})
#endif

IOTC_TT_TESTGROUP_END

#ifndef IOTC_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#define IOTC_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#include __FILE__
#undef IOTC_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#endif
