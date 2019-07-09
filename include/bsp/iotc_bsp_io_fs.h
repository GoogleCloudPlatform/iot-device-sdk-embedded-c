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
 * @brief Accesses and manages files.
 *
 * These functions facilitate non-volatile certificate storage.
 */
#include <iotc_error.h>
#include <stdint.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

/** 
 * @typedef iotc_bsp_io_fs_resource_handle_t
 * @brief A pointer to an open file.
 */
typedef intptr_t iotc_bsp_io_fs_resource_handle_t;

/** The handle doesn't point to an open file. */
#define IOTC_BSP_IO_FS_INVALID_RESOURCE_HANDLE -1
/** The handle points to an open file. */
#define iotc_bsp_io_fs_init_resource_handle() \
  IOTC_BSP_IO_FS_INVALID_RESOURCE_HANDLE

/**
 * @typedef iotc_bsp_io_fs_state_t
 * @brief File management function status. File management functions must
 * return a status message to the client application.
 */
typedef enum iotc_bsp_io_fs_state_e {
  /** The file management function succeeded. */
  IOTC_BSP_IO_FS_STATE_OK = 0,
  /** Something went wrong. */
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
 * @typedef iotc_bsp_io_fs_resource_type_t
 * @details Resource types. Must be TLS server authentication certificates.
 */
typedef enum iotc_bsp_io_fs_resource_type_e {
  /** TLS server authentication certificates. */
  IOTC_BSP_IO_FS_CERTIFICATE = 0,
} iotc_bsp_io_fs_resource_type_t;

/**
 * @typedef iotc_bsp_io_fs_stat_t
 * @brief The size of TLS server authentication certificates.
 */
typedef struct iotc_bsp_io_fs_stat_s {
  /** The size of the resource in bytes. */
  size_t resource_size;
} iotc_bsp_io_fs_stat_t;

/**
 * @typedef iotc_bsp_io_fs_open_flags_t
 * @brief File operations.
 *
 * To open files, Device SDK passes these values to iotc_bsp_io_fs_open().
 */
typedef enum iotc_bsp_io_fs_open_flags {
  /** Open the file for read operations. */
  IOTC_BSP_IO_FS_OPEN_READ = 1 << 0,
  /** Open the file for write operations. */
  IOTC_BSP_IO_FS_OPEN_WRITE = 1 << 1,
  /** Open the file to append. */
  IOTC_BSP_IO_FS_OPEN_APPEND = 1 << 2,
} iotc_bsp_io_fs_open_flags_t;

/**
 * @brief Determines file size.
 *
 * @param [in] resource_name The file name.
 * @param [out] resource_stat A structure for the file size information.
 *
 * @return iotc_bsp_io_fs_state_e File management function status.
 *
 * @see iotc_bsp_io_fs_state_e
 */
iotc_bsp_io_fs_state_t iotc_bsp_io_fs_stat(
    const char* const resource_name, iotc_bsp_io_fs_stat_t* resource_stat);

/**
 * @details Opens a file.
 *
 * @param [in] resource_name The file name.
 * @param [in] size The size, in bytes, of the file.
 * @param [in] open_flags A {@link ::iotc_bsp_io_fs_open_flags_t file operation}
 *     bitmask.
 * @param [out] resource_handle_out A
 *     {@link ::iotc_bsp_io_fs_resource_handle_t handle to an open file}. The 
 *     Device SDK passes this handle to iotc_bsp_io_fs_read() or
 *     iotc_bsp_io_fs_write() to perform the corresponding operation.
 *
 * @return File management function status.
 *
 * @see iotc_bsp_io_fs_state_e
 */
iotc_bsp_io_fs_state_t iotc_bsp_io_fs_open(
    const char* const resource_name, const size_t size,
    const iotc_bsp_io_fs_open_flags_t open_flags,
    iotc_bsp_io_fs_resource_handle_t* resource_handle_out);

/**
 * @details Reads a file. Implementations of this function must fill the buffer
 * parameter at offset 0. The Device SDK opens the file before reading it.
 *
 * Implementations of this function can allocate buffers by:
 *     - Allocating the buffer once, resusing it at each function call,
 *       and freeing it when the file closes.
 *     - Creating a new buffer each time the function is called and
 *       and freeing the old buffer before the function returns.
 *
 * @param [in] resource_handle A
 *     {@link ::iotc_bsp_io_fs_resource_handle_t handle to an open file}.
 * @param [in] offset The position within the resource, in bytes, from which
 *     to start read operations.
 * @param [out] buffer A pointer to a buffer with the bytes read from the file.
 *     Function implementations must define how to allocate buffers The buffer
 *     is already allocated by the Device SDK.
 * @param [out] buffer_size The number of bytes read from the file and stored
 *     in the buffer.
 *
 * @return File management function status.
 *
 * @see iotc_bsp_io_fs_state_e
 */
iotc_bsp_io_fs_state_t iotc_bsp_io_fs_read(
    const iotc_bsp_io_fs_resource_handle_t resource_handle, const size_t offset,
    const uint8_t** buffer, size_t* const buffer_size);

/**
 * @details Writes to a file. The Device SDK opens the file before writing to
 * it.
 *
 * Implementations of this function can allocate buffers by:
 *     - Allocating the buffer once, resusing it at each function call,
 *       and freeing it when the file closes.
 *     - Creating a new buffer each time the function is called and
 *       and freeing the old buffer before the function returns.
 *
 * @param [in] resource_handle A
 *     {@link ::iotc_bsp_io_fs_resource_handle_t handle to an open file}.
 * @param [in] buffer A pointer to a byte array with the data to write to the
 *     file.
 * @param [in] buffer_size The size, in bytes of the buffer.
 * @param [in] offset The position within the resource, in bytes, from which to
 *     start to the write operation.
 * @param [out] bytes_written The number of bytes written to the file.
 *
 * @return File management function status.
 *
 * @see iotc_bsp_io_fs_state_e
 */
iotc_bsp_io_fs_state_t iotc_bsp_io_fs_write(
    const iotc_bsp_io_fs_resource_handle_t resource_handle,
    const uint8_t* const buffer, const size_t buffer_size, const size_t offset,
    size_t* const bytes_written);

/**
 * @details Closes a file. Implementations of this function must free all of the
 * resources used to read or write to the file.
 *
 * @param [in] resource_handle A
 *     {@link ::iotc_bsp_io_fs_resource_handle_t handle to an open file}.
 *
 * @return File management function status.
 *
 * @see iotc_bsp_io_fs_state_e
 */
iotc_bsp_io_fs_state_t iotc_bsp_io_fs_close(
    const iotc_bsp_io_fs_resource_handle_t resource_handle);

/**
 * @brief Deletes a file.
 *
 * @param [in] resource_name The name of the file.
 *
 * @return File management function status.
 *
 * @see iotc_bsp_io_fs_state_e
 */
iotc_bsp_io_fs_state_t iotc_bsp_io_fs_remove(const char* const resource_name);

#ifdef __cplusplus
}
#endif

#endif /* __IOTC_BSP_IO_FS_H__ */
