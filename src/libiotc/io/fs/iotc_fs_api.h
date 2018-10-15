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

#ifndef __IOTC_FS_API_H__
#define __IOTC_FS_API_H__

#include <stdint.h>
#include <stdlib.h>

#include <iotc_bsp_io_fs.h>
#include <iotc_err.h>

#define IOTC_FS_INVALID_RESOURCE_HANDLE -1
#define iotc_fs_init_resource_handle() IOTC_FS_INVALID_RESOURCE_HANDLE

typedef intptr_t iotc_fs_resource_handle_t;

typedef enum iotc_fs_resource_type_e {
  IOTC_FS_CERTIFICATE = 0, /**< 0 **/
  IOTC_FS_CREDENTIALS,     /**< 1 **/
  IOTC_FS_CONFIG_DATA      /**< 2 **/
} iotc_fs_resource_type_t;

typedef struct iotc_fs_stat_s {
  size_t resource_size;
} iotc_fs_stat_t;

typedef enum iotc_fs_open_flags_e {
  IOTC_FS_OPEN_READ = 1 << 0,
  IOTC_FS_OPEN_WRITE = 1 << 1,
  IOTC_FS_OPEN_APPEND = 1 << 2,
} iotc_fs_open_flags_t;

/* The size of the buffer to be used for reads */
extern const size_t iotc_fs_buffer_size;

/**
 * @name iotc_fs_stat_resource_t
 * @brief returns stats of a resource described via resource type and name
 *
 * @return iotc_state_t IOTC_STATE_OK in case of a success and other in case of
 * an error
 */
typedef iotc_state_t(iotc_fs_stat_resource_t)(
    const void* context, const iotc_fs_resource_type_t resource_type,
    const char* const resource_name, iotc_fs_stat_t* resource_stat);

/**
 * @name iotc_fs_open_resource_t
 * @brief opens the resource described via resource type and name with a given
 * flags, returns a handle to that resource
 *
 * @return iotc_state_t IOTC_STATE_OK in case of a success and other values in
 * case of an error
 */
typedef iotc_state_t(iotc_fs_open_resource_t)(
    const void* context, const iotc_fs_resource_type_t resource_type,
    const char* const resource_name, const iotc_fs_open_flags_t open_flags,
    iotc_fs_resource_handle_t* resource_handle);

/**
 * @name iotc_fs_read_resource_t
 * @brief reads data using a given offset from the beginnig of the resource from
 * the previously opened resource described via resource handle, returns a
 * buffer of data and a size of that buffer
 *
 *
 * @brief it is up to the implementation of this API to free the buffer, if
 * needed, either on the next call to read or when close is called on the file
 * descriptor. This allows the use of ReadOnly File Systems on devices that
 * cannot facilitate a full file system implementation.
 *
 * @note if the storage data is compiled in than this function should return
 * pure pointer to the memory where the data is located, this is perfect
 * solution for ReadOnly File Systems
 *
 * @return iotc_state_t IOTC_STATE_OK in case of a success and other values in
 * case of an error
 */
typedef iotc_state_t(iotc_fs_read_resource_t)(
    const void* context, const iotc_fs_resource_handle_t resource_handle,
    const size_t offset, const uint8_t** buffer, size_t* const buffer_size);

/**
 * @name iotc_fs_write_resource_t
 * @brief writes the data from the provided buffer to the previously opened
 * resource described by resource_handle. The offset is the offset into the
 * destination file. bytes_written is a return parameter describing the number
 * of bytes that were committed to the file in this write operation.
 *
 * @return iotc_state_t IOTC_STATE_OK in case of a success and other values in
 * case of an error
 */
typedef iotc_state_t(iotc_fs_write_resource_t)(
    const void* context, const iotc_fs_resource_handle_t resource_handle,
    const uint8_t* const buffer, const size_t buffer_size, const size_t offset,
    size_t* const bytes_written);

/**
 * @name iotc_fs_close_resource_t
 * @brief closes the previously opened resource represented via resource handle
 *
 * @note close function should also release all previously allocated memory on
 * behalf of the given resource
 *
 * @return iotc_state_t IOTC_STATE_OK in case of a success and other values in
 * case of an error
 */
typedef iotc_state_t(iotc_fs_close_resource_t)(
    const void* context, const iotc_fs_resource_handle_t resource_handle);

/**
 * @name iotc_fs_remove_resource_t
 * @brief removes the resource described via resource type and name
 *
 * @return iotc_state_t IOTC_STATE_OK in case of a success and other values in
 * case of an error
 */
typedef iotc_state_t(iotc_fs_remove_resource_t)(
    const void* context, const iotc_fs_resource_type_t resource_type,
    const char* const resource_name);

/**
 * @name iotc_fs_functions_t
 * @brief a function pointer table that serves as a runtime link between the
 * IoTC Client and the implementing Resource Library API
 */
typedef struct iotc_fs_functions_s {
  size_t fs_functions_size;
  iotc_fs_stat_resource_t* stat_resource;
  iotc_fs_open_resource_t* open_resource;
  iotc_fs_read_resource_t* read_resource;
  iotc_fs_write_resource_t* write_resource;
  iotc_fs_close_resource_t* close_resource;
  iotc_fs_remove_resource_t* remove_resource;
} iotc_fs_functions_t;

#endif /* __IOTC_FS_API_H__ */
