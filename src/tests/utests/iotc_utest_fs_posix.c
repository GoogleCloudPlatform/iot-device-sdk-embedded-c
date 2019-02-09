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

#include "iotc_memory_checks.h"
#include "iotc_tt_testcase_management.h"
#include "iotc_utest_basic_testcase_frame.h"
#include "tinytest.h"
#include "tinytest_macros.h"

#include "iotc_bsp_rng.h"
#include "iotc_fs_filenames.h"
#include "iotc_fs_header.h"
#include "iotc_macros.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#ifndef IOTC_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
static const char* iotc_utest_fs_posix_existing_file_name =
    "file_that_does_exist.utest_file";
static const char* iotc_utest_fs_posix_not_existing_file_name =
    "file_that_does_not_exist.utest_file";
static const size_t iotc_utest_fs_posix_file_test_size_small = 1024;
static const size_t iotc_utest_fs_posix_file_test_size_big = 4096;

/* this has to be global at least for now until we gain the ability to pass env
 * variable to tests */
char* iotc_utest_fs_posix_memory_block = NULL;

void* iotc_utest_fs_posix_create_test_random_file(const char* const file_name,
                                                  size_t file_size) {
  IOTC_UNUSED(file_name);
  IOTC_UNUSED(file_size);

  unsigned i = 0u;
  size_t blocks_written = 0u;
  char* memory_buffer = NULL;

  FILE* fd = fopen(file_name, "wb");

  if (NULL == fd) {
    tt_abort();
  }

  memory_buffer = (char*)malloc(file_size);

  if (NULL == memory_buffer) {
    goto err_handling;
  }

  for (; i < file_size; ++i) {
    memory_buffer[i] = (char)(iotc_bsp_rng_get() % 255u);
  }

  blocks_written = fwrite(memory_buffer, 1, file_size, fd);

  if (file_size != blocks_written) {
    tt_abort();
  }

end:
  fclose(fd);
  return memory_buffer;

err_handling:
  free(iotc_utest_fs_posix_memory_block);
  tt_abort();

  return NULL;
}

int iotc_utest_fs_posix_unlink_file_free_memory(const char* const file_name,
                                                char* memory_block) {
  free(memory_block);

  int ret = unlink(file_name);

  if (0 != ret) /* if error return error */
  {
    return 0;
  }

  /* otherwise return sucess */
  return 1;
}

void* utest__iotc_fs_posix__setup_small(const struct testcase_t* t) {
  IOTC_UNUSED(t);

  iotc_utest_setup_basic(t);

  iotc_utest_fs_posix_memory_block =
      (char*)iotc_utest_fs_posix_create_test_random_file(
          iotc_utest_fs_posix_existing_file_name,
          iotc_utest_fs_posix_file_test_size_small);

  tt_ptr_op(NULL, !=, iotc_utest_fs_posix_memory_block);

  return iotc_utest_fs_posix_memory_block;

end:
  return NULL;
}

void* utest__iotc_fs_posix__setup_big(const struct testcase_t* t) {
  IOTC_UNUSED(t);

  iotc_utest_setup_basic(t);

  iotc_utest_fs_posix_memory_block =
      (char*)iotc_utest_fs_posix_create_test_random_file(
          iotc_utest_fs_posix_existing_file_name,
          iotc_utest_fs_posix_file_test_size_big);

  tt_ptr_op(NULL, !=, iotc_utest_fs_posix_memory_block);

  return iotc_utest_fs_posix_memory_block;

end:
  return NULL;
}

int utest__iotc_fs_posix__clean(const struct testcase_t* t, void* data) {
  IOTC_UNUSED(t);

  int ret = iotc_utest_fs_posix_unlink_file_free_memory(
      iotc_utest_fs_posix_existing_file_name, (char*)data);

  tt_int_op(1, ==, ret);

  iotc_utest_teardown_basic(t, data);

end:
  return ret;
}

#endif

IOTC_TT_TESTGROUP_BEGIN(utest_fs_posix)

IOTC_TT_TESTCASE_WITH_SETUP(
    utest__iotc_fs_stat_posix__valid_data__stat_returned,
    utest__iotc_fs_posix__setup_small, utest__iotc_fs_posix__clean, NULL, {
      iotc_fs_stat_t resource_stat = {.resource_size = 0};
      iotc_state_t res =
          iotc_fs_stat(NULL, IOTC_FS_CERTIFICATE,
                       iotc_utest_fs_posix_existing_file_name, &resource_stat);

      tt_int_op(IOTC_STATE_OK, ==, res);
      tt_int_op(resource_stat.resource_size, ==,
                iotc_utest_fs_posix_file_test_size_small);

    end:;
    })

IOTC_TT_TESTCASE(utest__iotc_fs_stat_posix__valid_data__file_does_not_exist, {
  iotc_fs_stat_t resource_stat = {.resource_size = 0};
  iotc_state_t res =
      iotc_fs_stat(NULL, IOTC_FS_CERTIFICATE,
                   iotc_utest_fs_posix_not_existing_file_name, &resource_stat);

  tt_int_op(IOTC_FS_RESOURCE_NOT_AVAILABLE, ==, res);
  tt_int_op(iotc_is_whole_memory_deallocated(), >, 0);
end:;
})

IOTC_TT_TESTCASE_WITH_SETUP(utest__iotc_fs_posix__valid_data__open_success,
                            utest__iotc_fs_posix__setup_small,
                            utest__iotc_fs_posix__clean, NULL, {
                              iotc_fs_resource_handle_t resource_handle =
                                  iotc_fs_init_resource_handle();
                              iotc_state_t res = iotc_fs_open(
                                  NULL, IOTC_FS_CERTIFICATE,
                                  iotc_utest_fs_posix_existing_file_name,
                                  IOTC_FS_OPEN_READ, &resource_handle);

                              tt_int_op(IOTC_STATE_OK, ==, res);
                              tt_int_op(resource_handle, !=,
                                        IOTC_FS_INVALID_RESOURCE_HANDLE);

                              iotc_fs_close(NULL, resource_handle);

                              return;

                            end:
                              iotc_fs_close(NULL, resource_handle);
                            })

IOTC_TT_TESTCASE_WITH_SETUP(
    utest__iotc_fs_posix__valid_data__read_success,
    utest__iotc_fs_posix__setup_small, utest__iotc_fs_posix__clean, NULL, {
      iotc_fs_resource_handle_t resource_handle =
          iotc_fs_init_resource_handle();
      iotc_state_t res = IOTC_STATE_OK;

      res = iotc_fs_open(NULL, IOTC_FS_CERTIFICATE,
                         iotc_utest_fs_posix_existing_file_name,
                         IOTC_FS_OPEN_READ, &resource_handle);

      const uint8_t* buffer = NULL;
      size_t buffer_size = 0u;
      int memcmp_res = 0;

      res = iotc_fs_read(NULL, resource_handle, 0, &buffer, &buffer_size);

      tt_int_op(IOTC_STATE_OK, ==, res);
      tt_int_op(buffer_size, ==, iotc_utest_fs_posix_file_test_size_small);

      memcmp_res = memcmp(buffer, iotc_utest_fs_posix_memory_block,
                          iotc_utest_fs_posix_file_test_size_small);
      tt_int_op(memcmp_res, ==, 0);
      iotc_fs_close(NULL, resource_handle);

      return;
    end:
      iotc_fs_close(NULL, resource_handle);
    })

IOTC_TT_TESTCASE_WITH_SETUP(
    utest__iotc_fs_posix__valid_data__read_in_chunks_success,
    utest__iotc_fs_posix__setup_big, utest__iotc_fs_posix__clean, NULL, {
      iotc_fs_resource_handle_t resource_handle =
          iotc_fs_init_resource_handle();
      iotc_state_t res = IOTC_STATE_OK;

      res = iotc_fs_open(NULL, IOTC_FS_CERTIFICATE,
                         iotc_utest_fs_posix_existing_file_name,
                         IOTC_FS_OPEN_READ, &resource_handle);

      uint8_t* accumulator = NULL;
      size_t bytes_read = 0u;
      int memcmp_res = 0;

      IOTC_ALLOC_BUFFER_AT(uint8_t, accumulator,
                           iotc_utest_fs_posix_file_test_size_big, res);

      do {
        const uint8_t* buffer = NULL;
        size_t buffer_size = 0u;

        res = iotc_fs_read(NULL, resource_handle, bytes_read, &buffer,
                           &buffer_size);
        IOTC_CHECK_STATE(res);
        memcpy(accumulator + bytes_read, buffer, buffer_size);
        bytes_read += buffer_size;
      } while (bytes_read != iotc_utest_fs_posix_file_test_size_big);

      tt_int_op(IOTC_STATE_OK, ==, res);
      tt_int_op(bytes_read, ==, iotc_utest_fs_posix_file_test_size_big);

      memcmp_res = memcmp(accumulator, iotc_utest_fs_posix_memory_block,
                          iotc_utest_fs_posix_file_test_size_small);

      tt_int_op(memcmp_res, ==, 0);
      iotc_fs_close(NULL, resource_handle);
      IOTC_SAFE_FREE(accumulator);

      return;

    err_handling:
      tt_abort();
    end:
      IOTC_SAFE_FREE(accumulator);
      iotc_fs_close(NULL, resource_handle);
    })

IOTC_TT_TESTCASE_WITH_SETUP(utest__iotc_fs_posix__valid_data__close_success,
                            utest__iotc_fs_posix__setup_small,
                            utest__iotc_fs_posix__clean, NULL, {
                              iotc_fs_resource_handle_t resource_handle =
                                  iotc_fs_init_resource_handle();
                              iotc_state_t res = IOTC_STATE_OK;

                              res = iotc_fs_open(
                                  NULL, IOTC_FS_CERTIFICATE,
                                  iotc_utest_fs_posix_existing_file_name,
                                  IOTC_FS_OPEN_READ, &resource_handle);

                              res = iotc_fs_close(NULL, resource_handle);
                              tt_int_op(IOTC_STATE_OK, ==, res);
                            end:;
                            })

IOTC_TT_TESTGROUP_END

#ifndef IOTC_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#define IOTC_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#include __FILE__
#undef IOTC_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#endif
