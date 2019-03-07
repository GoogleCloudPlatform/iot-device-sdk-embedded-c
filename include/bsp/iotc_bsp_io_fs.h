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

#ifndef __IOTC_BSP_IO_FS_H__
#define __IOTC_BSP_IO_FS_H__

/**
 * @file iotc_bsp_io_fs.h
 * @brief Access and manage files.
 *
 * These functions facilitate non-volatile certificate storage. resource_name
 * strings reference files.
 */
#include <iotc_error.h>
#include <stdint.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef intptr_t iotc_bsp_io_fs_resource_handle_t;

#define IOTC_BSP_IO_FS_INVALID_RESOURCE_HANDLE -1
#define iotc_bsp_io_fs_init_resource_handle() \
  IOTC_BSP_IO_FS_INVALID_RESOURCE_HANDLE

/**
 * @typedef iotc_bsp_io_fs_state_e
 * @brief File management function status.
 *
 * IOTC_BSP_IO_FS_STATE_O represents success. Others represent errors. 
 */
typedef enum iotc_bsp_io_fs_state_e {
  /** Operation successful. */
  IOTC_BSP_IO_FS_STATE_OK = 0,
  /** Operation failed. Generic error. */
  IOTC_BSP_IO_FS_ERROR = 1,
  /** invalid parameter. */
  IOTC_BSP_IO_FS_INVALID_PARAMETER = 2,
  /** File is not available. */
  IOTC_BSP_IO_FS_RESOURCE_NOT_AVAILABLE = 3,
  /** Out of memory. */
  IOTC_BSP_IO_FS_OUT_OF_MEMORY = 4,
  /** Function not implemented on target platform. */
  IOTC_BSP_IO_FS_NOT_IMPLEMENTED = 5,
  /** Cannot open file. **/
  IOTC_BSP_IO_FS_OPEN_ERROR = 6,
  /** Cannot open file because it's read-only. */
  IOTC_BSP_IO_FS_OPEN_READ_ONLY = 7,
  /** File cannot be removed. */
  IOTC_BSP_IO_FS_REMOVE_ERROR = 8,
  /** Cannot write data to file. */
  IOTC_BSP_IO_FS_WRITE_ERROR = 9,
  /** Cannot be read file. */
  IOTC_BSP_IO_FS_READ_ERROR = 10,
  /** Cannot close file. */
  IOTC_BSP_IO_FS_CLOSE_ERROR = 11,
} iotc_bsp_io_fs_state_t;

/**
 * @enum iotc_bsp_io_fs_resource_type_t
 * @brief Resources by security class.
 */
typedef enum iotc_bsp_io_fs_resource_type_e {
  IOTC_BSP_IO_FS_CERTIFICATE = 0, /**< 0 **/
  IOTC_BSP_IO_FS_CREDENTIALS,     /**< 1 **/
  IOTC_BSP_IO_FS_CONFIG_DATA      /**< 2 **/
} iotc_bsp_io_fs_resource_type_t;

/**
 * @name iotc_bsp_io_fs_stat_s
 * @brief Resource size.
 * 
 * When the <code>iotc_bsp_io_fs_stat()</code> function requests this
 * information.
 *
 * @see iotc_bsp_io_fs_stat
 */
typedef struct iotc_bsp_io_fs_stat_s {
  size_t resource_size;
} iotc_bsp_io_fs_stat_t;

/**
 * @enum iotc_bsp_io_fs_open_flags_t
 * @brief File operations.
 * 
 * Each value represents a file operation. To open a file, pass the
 * corresponding value to <code>iotc_bsp_io_fs_open()</code>.
 *
 * @see iotc_bsp_io_fs_open
 * @see iotc_bsp_io_fs_state_e
 */
typedef enum iotc_bsp_io_fs_open_flags {
  IOTC_BSP_IO_FS_OPEN_READ = 1 << 0,
  IOTC_BSP_IO_FS_OPEN_WRITE = 1 << 1,
  IOTC_BSP_IO_FS_OPEN_APPEND = 1 << 2,
} iotc_bsp_io_fs_open_flags_t;

/**
 * @function
 * @brief Determine file size.
 *
 * @param [in] resource_name file name.
 * @param [out] resource_stat a structure for the file size information.
 *
 * @see iotc_bsp_io_fs_state_e 
 *
 * @return iotc_bsp_io_fs_state_t file management function status.
 */
iotc_bsp_io_fs_state_t iotc_bsp_io_fs_stat(
    const char* const resource_name, iotc_bsp_io_fs_stat_t* resource_stat);

/**
 * @function
 * @brief Open a file.
 *
 * This function returns a resource_handle_out handle that specifies the
 * the subsequent operation (read, write, or close). After executing this
 * function, pass resource_handle_out to <code>iotc_bsp_io_fs_read()</code>
 * or <code>iotc_bsp_io_fs_write()</code> to perform the corresponding
 * operation.
 *
 * @param [in] resource_name file name.
 * @param [in] size (Optional) the size, in bytes, of the file. Required on
 * non-POSIX implementations.
 * @param [in] open_flags an iotc_bsp_io_fs_open_flags_t bitmask.
 * @param [out] resource_handle_out a pointer to an
 * iotc_bsp_io_fs_resource_handle_thandle.
 *
 * @see iotc_bsp_io_fs_read
 * @see iotc_bsp_io_fs_write
 * @see iotc_bsp_io_fs_state_e 
 * @see iotc_bsp_io_fs_open_flags_t
 * @see iotc_bsp_io_fs_resource_handle_t
 *
 * @return iotc_bsp_io_fs_state_t file management function status.
 */
iotc_bsp_io_fs_state_t iotc_bsp_io_fs_open(
    const char* const resource_name, const size_t size,
    const iotc_bsp_io_fs_open_flags_t open_flags,
    iotc_bsp_io_fs_resource_handle_t* resource_handle_out);

/**
 * @function
 * @brief Read a file. 
 *
 * To read a file, first run <code>iotc_bsp_io_fs_open()</code> and then pass
 * the corresponding iotc_bsp_io_fs_resource_handle_t handle to <code>iotc_bsp_io_fs_read()</code>.
 *
 * Free buffers only on subsequent <code>iotc_bsp_io_fs_read()</code> or 
 * <code>iotc_bsp_io_fs_close()</code> calls.
 *
 * @param [in] resource_handle the iotc_bsp_io_fs_resource_handle_t handle from 
 * <code>iotc_bsp_io_fs_open()</code>.
 * @param [in] offset the position of the resource, in bytes, from which
 * to start read operations. Set to <code>0</code>.
 * @param [out] buffer a pointer to a buffer with the bytes read from the file.
 * The buffer is automatically allocated.
 * @param [out] buffer_size the number of bytes read from the file and stored
 * in the buffer.
 *
 * @see iotc_bsp_io_fs_open
 * @see iotc_bsp_io_fs_close
 * @see iotc_bsp_io_fs_state_e 
 *
 * @return iotc_bsp_io_fs_state_t file management function status.
 */
iotc_bsp_io_fs_state_t iotc_bsp_io_fs_read(
    const iotc_bsp_io_fs_resource_handle_t resource_handle, const size_t offset,
    const uint8_t** buffer, size_t* const buffer_size);

/**
 * @function
 * @brief Write to a file.
 * 
 * To write to a file, first run <code>iotc_bsp_io_fs_open()</code> and then
 * pass the corresponding iotc_bsp_io_fs_resource_handle_t handle to <code>iotc_bsp_io_fs_read()</code>.
 *
 * @param [in] resource_handle the iotc_bsp_io_fs_resource_handle_t handle from 
 * <code>iotc_bsp_io_fs_open()</code>.
 * @param [in] buffer a pointer to a byte array with the data to write to the
 * file.
 * @param [in] buffer_size the size, in bytes of the buffer.
 * @param [in] offset the position of the resource, in bytes, from which to
 * start to the write operation.
 * @param [out] bytes_written the number of bytes written to the file.
 *
 * @see iotc_bsp_io_fs_open
 * @see iotc_bsp_io_fs_state_e 
 *
 * @return iotc_bsp_io_fs_state_t file management function status.
 */
iotc_bsp_io_fs_state_t iotc_bsp_io_fs_write(
    const iotc_bsp_io_fs_resource_handle_t resource_handle,
    const uint8_t* const buffer, const size_t buffer_size, const size_t offset,
    size_t* const bytes_written);

/**
 * @function
 * @brief    Close a file.
 * 
 * To close a file, first run <code>iotc_bsp_io_fs_open()</code> and then pass
 * the corresponding iotc_bsp_io_fs_resource_handle_t handle to <code>iotc_bsp_io_fs_read()</code>.
 *
 * @param [in] resource_handle the iotc_bsp_io_fs_resource_handle_t handle from 
 * <code>iotc_bsp_io_fs_open()</code>.
 *
 * @see iotc_bsp_io_fs_state_e 
 *
 * @return iotc_bsp_io_fs_state_t file management function status.
 */
iotc_bsp_io_fs_state_t iotc_bsp_io_fs_close(
    const iotc_bsp_io_fs_resource_handle_t resource_handle);

/**
 * @function
 * @brief Delete a file.
 *
 * @param [in] resource_name the name of the file.
 *
 * @see iotc_bsp_io_fs_state_e 
 *
 * @return iotc_bsp_io_fs_state_t file management function status.
 */
iotc_bsp_io_fs_state_t iotc_bsp_io_fs_remove(const char* const resource_name);

#ifdef __cplusplus
}
#endif

#endif /* __IOTC_BSP_IO_FS_H__ */
