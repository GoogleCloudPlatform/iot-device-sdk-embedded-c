/* Copyright 2018-2020 Google LLC
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

#include <iotc_bsp_io_fs.h>
#include <iotc_bsp_mem.h>

#include <iotc_list.h>

#include <assert.h>
#include <errno.h>
#include <iotc_fs_bsp_to_iotc_mapping.h>
#include <string.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

/* The size of the buffer to be used for reads. */
const size_t iotc_bsp_io_fs_buffer_size = 1024;

#define IOTC_BSP_IO_FS_CHECK_CND(cnd, e, s) \
  if ((cnd)) {                              \
    (s) = (e);                              \
    goto err_handling;                      \
  }

/**
 * @brief iotc_bsp_io_fs_posix_file_handle_container_t
 *
 * Database type of file handles and allocated memory chunks.
 */
typedef struct iotc_bsp_io_fs_posix_file_handle_container_s {
  int posix_fp;
  uint8_t* memory_buffer;
  struct iotc_bsp_io_fs_posix_file_handle_container_s* __next;
} iotc_bsp_io_fs_posix_file_handle_container_t;

/* local list of open files */
static iotc_bsp_io_fs_posix_file_handle_container_t*
    iotc_bsp_io_fs_posix_files_container;

/* translates bsp errno errors to the iotc_bsp_io_fs_state_t values */
iotc_bsp_io_fs_state_t iotc_bsp_io_fs_posix_errno_2_iotc_bsp_io_fs_state(
    int errno_value) {
  iotc_bsp_io_fs_state_t ret = IOTC_BSP_IO_FS_STATE_OK;

  switch (errno_value) {
    case EBADF:
    case EACCES:
    case EFAULT:
    case ENOENT:
    case ENOTDIR:
      ret = IOTC_BSP_IO_FS_RESOURCE_NOT_AVAILABLE;
      break;
    case ELOOP:
      ret = IOTC_BSP_IO_FS_ERROR;
      break;
    case ENAMETOOLONG:
      ret = IOTC_BSP_IO_FS_INVALID_PARAMETER;
      break;
    case ENOMEM:
      ret = IOTC_BSP_IO_FS_OUT_OF_MEMORY;
      break;
    default:
      ret = IOTC_BSP_IO_FS_ERROR;
  }

  return ret;
}

/* helper function that translates posix stat to xi_stat */
static iotc_bsp_io_fs_state_t iotc_bsp_io_fs_posix_stat_2_iotc_bsp_io_fs_stat(
    const struct stat* const posix_stat,
    iotc_bsp_io_fs_stat_t* const iotc_stat) {
  assert(NULL != posix_stat);
  assert(NULL != iotc_stat);

  iotc_stat->resource_size = posix_stat->st_size;

  return IOTC_BSP_IO_FS_STATE_OK;
}

/**
 * @brief iotc_bsp_io_fs_posix_file_list_cnd
 *
 * Functor for list find function.
 *
 * @param list
 * @param fp
 * @return 1 if list element is the one with the matching fp 0 otherwise
 */
static uint8_t iotc_bsp_io_fs_posix_file_list_cnd(
    iotc_bsp_io_fs_posix_file_handle_container_t* list_element, int fp) {
  assert(NULL != list_element);
  assert(NULL != fp);

  return (list_element->posix_fp == fp) ? 1 : 0;
}

iotc_bsp_io_fs_state_t iotc_bsp_io_fs_stat(
    const char* const resource_name, iotc_bsp_io_fs_stat_t* resource_stat) {
  if (NULL == resource_stat || NULL == resource_name) {
    return IOTC_BSP_IO_FS_INVALID_PARAMETER;
  }

  iotc_bsp_io_fs_state_t ret = IOTC_BSP_IO_FS_STATE_OK;

  struct stat stat_struct;
  memset(&stat_struct, 0, sizeof(stat_struct));

  int res = stat(resource_name, &stat_struct);

  /* Verification of the os function result.
   * Jump to err_handling label in case of failure. */
  IOTC_BSP_IO_FS_CHECK_CND(
      0 != res, iotc_bsp_io_fs_posix_errno_2_iotc_bsp_io_fs_state(errno), ret);

  /* here we translate stat posix os stat structure to libiotc version */
  ret = iotc_bsp_io_fs_posix_stat_2_iotc_bsp_io_fs_stat(&stat_struct,
                                                        resource_stat);

err_handling:
  return ret;
}

iotc_bsp_io_fs_state_t iotc_bsp_io_fs_open(
    const char* const resource_name, const size_t size,
    const iotc_bsp_io_fs_open_flags_t open_flags,
    iotc_bsp_io_fs_resource_handle_t* resource_handle_out) {
  (void)size;

  if (NULL == resource_name || NULL == resource_handle_out) {
    return IOTC_BSP_IO_FS_INVALID_PARAMETER;
  }

  /* append not supported */
  if (IOTC_BSP_IO_FS_OPEN_APPEND == (open_flags & IOTC_BSP_IO_FS_OPEN_APPEND)) {
    return IOTC_BSP_IO_FS_INVALID_PARAMETER;
  }

  iotc_bsp_io_fs_posix_file_handle_container_t* new_entry = NULL;
  iotc_bsp_io_fs_state_t ret = IOTC_BSP_IO_FS_STATE_OK;

  int fp = open(resource_name,
                   (open_flags & IOTC_BSP_IO_FS_OPEN_READ) ? O_RDONLY : O_WRONLY);

  /* if error on open check the errno value */
  IOTC_BSP_IO_FS_CHECK_CND(
      fp <= 0, iotc_bsp_io_fs_posix_errno_2_iotc_bsp_io_fs_state(errno),
      ret);

  /* allocate memory for the files database element */
  new_entry =
      iotc_bsp_mem_alloc(sizeof(iotc_bsp_io_fs_posix_file_handle_container_t));
  memset(new_entry, 0, sizeof(iotc_bsp_io_fs_posix_file_handle_container_t));

  /* store the posix file pointer */
  new_entry->posix_fp = fp;

  /* add the entry to the database */
  IOTC_LIST_PUSH_BACK(iotc_bsp_io_fs_posix_file_handle_container_t,
                      iotc_bsp_io_fs_posix_files_container, new_entry);

  /* make sure that the size is as expected. */
  assert(sizeof(fp) == sizeof(iotc_bsp_io_fs_resource_handle_t));

  /* return fp as a resource handle */
  *resource_handle_out = (iotc_bsp_io_fs_resource_handle_t)fp;

  return ret;

err_handling:
  if (0 < fp) {
    close(fp);
  }
  iotc_bsp_mem_free(new_entry);
  *resource_handle_out = iotc_bsp_io_fs_init_resource_handle();
  return ret;
}

iotc_bsp_io_fs_state_t iotc_bsp_io_fs_read(
    const iotc_bsp_io_fs_resource_handle_t resource_handle, const size_t offset,
    const uint8_t** buffer, size_t* const buffer_size) {
  if (NULL == buffer || NULL != *buffer || NULL == buffer_size ||
      IOTC_BSP_IO_FS_INVALID_RESOURCE_HANDLE == resource_handle) {
    return IOTC_BSP_IO_FS_INVALID_PARAMETER;
  }

  iotc_bsp_io_fs_state_t ret = IOTC_BSP_IO_FS_STATE_OK;
  int fp = (int)resource_handle;
  int fop_ret = 0;

  iotc_bsp_io_fs_posix_file_handle_container_t* elem = NULL;
  IOTC_LIST_FIND(iotc_bsp_io_fs_posix_file_handle_container_t,
                 iotc_bsp_io_fs_posix_files_container,
                 iotc_bsp_io_fs_posix_file_list_cnd, fp, elem);

  IOTC_BSP_IO_FS_CHECK_CND(NULL == elem, IOTC_BSP_IO_FS_RESOURCE_NOT_AVAILABLE,
                           ret);

  /* make an allocation for memory block */
  if (NULL == elem->memory_buffer) {
    elem->memory_buffer =
        (uint8_t*)iotc_bsp_mem_alloc(iotc_bsp_io_fs_buffer_size);
    memset(elem->memory_buffer, 0, iotc_bsp_io_fs_buffer_size);
  }

  /* let's set an offset */
  fop_ret = lseek(fp, offset, SEEK_SET);

  /* if error on lseek check errno */
  IOTC_BSP_IO_FS_CHECK_CND(
      fop_ret != 0, iotc_bsp_io_fs_posix_errno_2_iotc_bsp_io_fs_state(errno),
      ret);

  /* use the read to read the file chunk */
  fop_ret =
      read(fp, elem->memory_buffer, (size_t)1);

  /* if error on fread check errno */
  IOTC_BSP_IO_FS_CHECK_CND(
      fop_ret == 0, iotc_bsp_io_fs_posix_errno_2_iotc_bsp_io_fs_state(errno),
      ret);

  /* return buffer, buffer_size */
  *buffer = elem->memory_buffer;
  *buffer_size = fop_ret;

  return IOTC_BSP_IO_FS_STATE_OK;

err_handling:

  *buffer = NULL;
  *buffer_size = 0;
  iotc_bsp_mem_free(elem->memory_buffer);
  elem->memory_buffer = NULL;

  return ret;
}

iotc_bsp_io_fs_state_t iotc_bsp_io_fs_write(
    const iotc_bsp_io_fs_resource_handle_t resource_handle,
    const uint8_t* const buffer, const size_t buffer_size, const size_t offset,
    size_t* const bytes_written) {
  if (NULL == buffer || 0 == buffer_size ||
      IOTC_BSP_IO_FS_INVALID_RESOURCE_HANDLE == resource_handle) {
    return IOTC_BSP_IO_FS_INVALID_PARAMETER;
  }

  iotc_bsp_io_fs_state_t ret = IOTC_BSP_IO_FS_STATE_OK;
  int fp = (int)resource_handle;

  iotc_bsp_io_fs_posix_file_handle_container_t* elem = NULL;
  IOTC_LIST_FIND(iotc_bsp_io_fs_posix_file_handle_container_t,
                 iotc_bsp_io_fs_posix_files_container,
                 iotc_bsp_io_fs_posix_file_list_cnd, fp, elem);

  IOTC_BSP_IO_FS_CHECK_CND(NULL == elem, IOTC_BSP_IO_FS_RESOURCE_NOT_AVAILABLE,
                           ret);

  /* let's set the offset */
  const int fop_ret = lseek(fp, offset, SEEK_SET);

  /* if error on lseek check errno */
  IOTC_BSP_IO_FS_CHECK_CND(
      fop_ret != 0, iotc_bsp_io_fs_posix_errno_2_iotc_bsp_io_fs_state(errno),
      ret);

  *bytes_written = write(fp, buffer, (size_t)1);

  /* if error on fwrite check errno */
  IOTC_BSP_IO_FS_CHECK_CND(
      buffer_size != *bytes_written,
      iotc_bsp_io_fs_posix_errno_2_iotc_bsp_io_fs_state(errno), ret);

err_handling:

  return ret;
}

iotc_bsp_io_fs_state_t iotc_bsp_io_fs_close(
    const iotc_bsp_io_fs_resource_handle_t resource_handle) {
  if (IOTC_BSP_IO_FS_INVALID_RESOURCE_HANDLE == resource_handle) {
    return IOTC_BSP_IO_FS_INVALID_PARAMETER;
  }

  iotc_bsp_io_fs_state_t ret = IOTC_BSP_IO_FS_STATE_OK;
  int fp = (int)resource_handle;
  iotc_bsp_io_fs_posix_file_handle_container_t* elem = NULL;
  int fop_ret = 0;

  IOTC_LIST_FIND(iotc_bsp_io_fs_posix_file_handle_container_t,
                 iotc_bsp_io_fs_posix_files_container,
                 iotc_bsp_io_fs_posix_file_list_cnd, fp, elem);

  /* if element not on the list return resource not available error */
  IOTC_BSP_IO_FS_CHECK_CND(NULL == elem, IOTC_BSP_IO_FS_RESOURCE_NOT_AVAILABLE,
                           ret);

  /* remove element from the list */
  IOTC_LIST_DROP(iotc_bsp_io_fs_posix_file_handle_container_t,
                 iotc_bsp_io_fs_posix_files_container, elem);

  fop_ret = close(fp);

  /* if error on close check errno */
  IOTC_BSP_IO_FS_CHECK_CND(
      0 != fop_ret, iotc_bsp_io_fs_posix_errno_2_iotc_bsp_io_fs_state(errno),
      ret);

  iotc_bsp_mem_free(elem->memory_buffer);
  iotc_bsp_mem_free(elem);

  return IOTC_BSP_IO_FS_STATE_OK;

err_handling:

  if (NULL != elem) {
    iotc_bsp_mem_free(elem->memory_buffer);
    iotc_bsp_mem_free(elem);
  }
  return ret;
}

iotc_bsp_io_fs_state_t iotc_bsp_io_fs_remove(const char* const resource_name) {
  return IOTC_BSP_IO_FS_NOT_IMPLEMENTED;
}

