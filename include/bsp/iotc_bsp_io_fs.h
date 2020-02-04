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

#ifndef __IOTC_BSP_IO_FS_H__
#define __IOTC_BSP_IO_FS_H__

/**
 * @file iotc_bsp_io_fs.h
 * @brief Manages files.
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
 * @brief The file management function states.
 *
 * @see #iotc_bsp_io_fs_state_e
 */
typedef enum iotc_bsp_io_fs_state_e {
  /** The file management function succeeded. */
  IOTC_BSP_IO_FS_STATE_OK = 0,
  /** Something went wrong. */
  IOTC_BSP_IO_FS_ERROR = 1,
  /** A parameter is invalid. */
  IOTC_BSP_IO_FS_INVALID_PARAMETER = 2,
  /** The file isn't available. */
  IOTC_BSP_IO_FS_RESOURCE_NOT_AVAILABLE = 3,
  /** The device is out of memory. */
  IOTC_BSP_IO_FS_OUT_OF_MEMORY = 4,
  /** The function isn't implmented on your platform. */
  IOTC_BSP_IO_FS_NOT_IMPLEMENTED = 5,
  /** Can't open file. **/
  IOTC_BSP_IO_FS_OPEN_ERROR = 6,
  /** The file is read-only so the SDK can't open it. */
  IOTC_BSP_IO_FS_OPEN_READ_ONLY = 7,
  /** The file can't be removed. */
  IOTC_BSP_IO_FS_REMOVE_ERROR = 8,
  /** Can't write data to file. */
  IOTC_BSP_IO_FS_WRITE_ERROR = 9,
  /** Can't be read file. */
  IOTC_BSP_IO_FS_READ_ERROR = 10,
  /** Can't close file. */
  IOTC_BSP_IO_FS_CLOSE_ERROR = 11,
} iotc_bsp_io_fs_state_t;

/**
 * @typedef iotc_bsp_io_fs_resource_type_t
 * @brief The resource type of TLS certificates.
 *
 * @see #iotc_bsp_io_fs_resource_type_e
 */
typedef enum iotc_bsp_io_fs_resource_type_e {
  /** A TLS certificate resource. */
  IOTC_BSP_IO_FS_CERTIFICATE = 0,
} iotc_bsp_io_fs_resource_type_t;

/**
 * @typedef iotc_bsp_io_fs_stat_t
 * @brief The size of TLS server authentication certificates.
 * @see #iotc_bsp_io_fs_stat_s
 *
 * @struct iotc_bsp_io_fs_stat_s
 * @brief The size of TLS server authentication certificates.
 */
typedef struct iotc_bsp_io_fs_stat_s {
  /** The size, in bytes, of the resource. */
  size_t resource_size;
} iotc_bsp_io_fs_stat_t;

/**
 * @typedef iotc_bsp_io_fs_open_flags_t
 * @brief The file operations.
 *
 * @see #iotc_bsp_io_fs_open_flags
 */
typedef enum iotc_bsp_io_fs_open_flags {
  /** Open and read the file. */
  IOTC_BSP_IO_FS_OPEN_READ = 1 << 0,
  /** Open and write to the file. */
  IOTC_BSP_IO_FS_OPEN_WRITE = 1 << 1,
  /** Open and append to the file. */
  IOTC_BSP_IO_FS_OPEN_APPEND = 1 << 2,
} iotc_bsp_io_fs_open_flags_t;

/**
 * @brief Gets the size of a file.
 *
 * @param [in] resource_name The file name.
 * @param [out] resource_stat The size, in bytes, of the file.
 */
iotc_bsp_io_fs_state_t iotc_bsp_io_fs_stat(
    const char* const resource_name, iotc_bsp_io_fs_stat_t* resource_stat);

/**
 * @details Opens a file.
 *
 * @param [in] resource_name The filename.
 * @param [in] size The size, in bytes, of the file.
 * @param [in] open_flags A {@link ::iotc_bsp_io_fs_open_flags_t file operation}
 *     bitmask.
 * @param [out] resource_handle_out A
 *     {@link ::iotc_bsp_io_fs_resource_handle_t handle to an open file}.
 */
iotc_bsp_io_fs_state_t iotc_bsp_io_fs_open(
    const char* const resource_name, const size_t size,
    const iotc_bsp_io_fs_open_flags_t open_flags,
    iotc_bsp_io_fs_resource_handle_t* resource_handle_out);

/**
 * @brief Reads a file.
 *
 * @details The function must fill the buffer at offset 0. The function can
 * allocate buffers by:
 *     - Allocating the buffer once, reusing it at each function call,
 *       and freeing it when the file closes.
 *     - Creating a new buffer each time the function is called and
 *       freeing the old buffer before the function returns.
 *
 * For example, see the <a href="../../../src/bsp/platform/posix/iotc_bsp_io_fs_posix.c#L193">POSIX implementation of this function</a>.
 *
 * @param [in] resource_handle A
 *     {@link ::iotc_bsp_io_fs_resource_handle_t handle to an open file}.
 * @param [in] offset The position within the resource, in bytes, from which
 *     to start read operations.
 * @param [out] buffer A pointer to a buffer with the bytes read from the file.
 *     The buffer is already allocated by the SDK.
 * @param [out] buffer_size The number of bytes read from the file and stored
 *     in the buffer.
 */
iotc_bsp_io_fs_state_t iotc_bsp_io_fs_read(
    const iotc_bsp_io_fs_resource_handle_t resource_handle, const size_t offset,
    const uint8_t** buffer, size_t* const buffer_size);

/**
 * @details Writes to a file.
 *
 * @param [in] resource_handle A
 *     {@link ::iotc_bsp_io_fs_resource_handle_t handle to an open file}.
 * @param [in] buffer A pointer to a byte array with the data to write to the
 *     file.
 * @param [in] buffer_size The size, in bytes of the buffer.
 * @param [in] offset The position within the resource, in bytes, from which to
 *     start to the write operation.
 * @param [out] bytes_written The number of bytes written to the file.
 */
iotc_bsp_io_fs_state_t iotc_bsp_io_fs_write(
    const iotc_bsp_io_fs_resource_handle_t resource_handle,
    const uint8_t* const buffer, const size_t buffer_size, const size_t offset,
    size_t* const bytes_written);

/**
 * @details Closes a file and frees all of the resources from reading or writing
 * to the file.
 *
 * @param [in] resource_handle A
 *     {@link ::iotc_bsp_io_fs_resource_handle_t handle to an open file}.
 */
iotc_bsp_io_fs_state_t iotc_bsp_io_fs_close(
    const iotc_bsp_io_fs_resource_handle_t resource_handle);

/**
 * @brief Deletes a file.
 *
 * @param [in] resource_name The filename.
 */
iotc_bsp_io_fs_state_t iotc_bsp_io_fs_remove(const char* const resource_name);

#ifdef __cplusplus
}
#endif

#endif /* __IOTC_BSP_IO_FS_H__ */
