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

#include "iotc_fs_header.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef IOTC_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#endif

IOTC_TT_TESTGROUP_BEGIN(utest_fs_dummy)

IOTC_TT_TESTCASE(utest__iotc_fs_dummy_stat__values_not_important, {
  iotc_fs_stat_t stat;
  iotc_state_t ret =
      iotc_fs_stat(NULL, IOTC_FS_CONFIG_DATA, "test_data", &stat);

  tt_int_op(ret, ==, IOTC_FS_RESOURCE_NOT_AVAILABLE);
end:;
})

IOTC_TT_TESTCASE(utest__iotc_fs_dummy_open__values_not_important, {
  iotc_fs_resource_handle_t resource_handle = iotc_fs_init_resource_handle();
  iotc_state_t ret = iotc_fs_open(NULL, IOTC_FS_CONFIG_DATA, "test_data",
                                  IOTC_FS_OPEN_READ, &resource_handle);

  tt_int_op(ret, ==, IOTC_FS_ERROR);
end:;
})

IOTC_TT_TESTCASE(utest__iotc_fs_dummy_read__values_not_important, {
  iotc_fs_resource_handle_t resource_handle = iotc_fs_init_resource_handle();
  const uint8_t* buffer = NULL;
  size_t buffer_size = 0;

  iotc_state_t ret =
      iotc_fs_read(NULL, resource_handle, 0, &buffer, &buffer_size);

  tt_int_op(ret, ==, IOTC_FS_ERROR);
end:;
})

IOTC_TT_TESTCASE(utest__iotc_fs_dummy_write__values_not_important, {
  iotc_fs_resource_handle_t resource_handle = iotc_fs_init_resource_handle();
  uint8_t buffer[1024] = {'a'};
  size_t buffer_size = 1024;
  size_t bytes_written = 0;

  iotc_state_t ret = iotc_fs_write(NULL, resource_handle, buffer, buffer_size,
                                   0, &bytes_written);

  tt_int_op(ret, ==, IOTC_FS_ERROR);
end:;
})

IOTC_TT_TESTCASE(utest__iotc_fs_dummy_close__values_not_important, {
  iotc_fs_resource_handle_t resource_handle = iotc_fs_init_resource_handle();
  iotc_state_t ret = iotc_fs_close(NULL, resource_handle);

  tt_int_op(ret, ==, IOTC_FS_ERROR);
end:;
})

IOTC_TT_TESTCASE(utest__iotc_fs_dummy_remove__values_not_important, {
  iotc_state_t ret = iotc_fs_remove(NULL, IOTC_FS_CONFIG_DATA, "test_name");

  tt_int_op(ret, ==, IOTC_FS_RESOURCE_NOT_AVAILABLE);
end:;
})

IOTC_TT_TESTGROUP_END

#ifndef IOTC_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#define IOTC_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#include __FILE__
#undef IOTC_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#endif
