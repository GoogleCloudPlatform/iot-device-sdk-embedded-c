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

#ifndef __IOTC_BSP_IO_FS_H__
#define __IOTC_BSP_IO_FS_H__

/**
 * @file iotc_bsp_io.fs.h
 * @brief IoTC Client's Board Support Package (BSP) for File Access.
 *
 * This file defines the File Management API used by the IoTC Client.
 *
 * The IoTC Client uses this BSP to facilitate non-volatile data
 * storage of certificates.
 *
 * NOTE: the use of this BSP to store certificates used during the TLS
 * handshake process is currently an ongoing project.
 *
 * All files are referenced by resource_name strings.
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
 * @brief Return value of the BSP NET API functions.
 *
 * The implementation reports internal status to IoTC Client through these
 * values.
 */
typedef enum iotc_bsp_io_fs_state_e {
  /** operation finished successfully */
  IOTC_BSP_IO_FS_STATE_OK = 0,
  /** operation failed on generic error */
  IOTC_BSP_IO_FS_ERROR = 1,
  /** invalid parameter passed to function */
  IOTC_BSP_IO_FS_INVALID_PARAMETER = 2,
  /** resource/file is not available */
  IOTC_BSP_IO_FS_RESOURCE_NOT_AVAILABLE = 3,
  /** out of memory error */
  IOTC_BSP_IO_FS_OUT_OF_MEMORY = 4,
  /** function not implemented on target platform */
  IOTC_BSP_IO_FS_NOT_IMPLEMENTED = 5,
  /** error opening file resource **/
  IOTC_BSP_IO_FS_OPEN_ERROR = 6,
  /** error that file open could only open read only */
  IOTC_BSP_IO_FS_OPEN_READ_ONLY = 7,
  /** error reported when file cannot be removed */
  IOTC_BSP_IO_FS_REMOVE_ERROR = 8,
  /** error when attempting to write file data */
  IOTC_BSP_IO_FS_WRITE_ERROR = 9,
  /** error reported when file cannot be read */
  IOTC_BSP_IO_FS_READ_ERROR = 10,
  /** error reported when file cannot be closed */
  IOTC_BSP_IO_FS_CLOSE_ERROR = 11,
} iotc_bsp_io_fs_state_t;

/**
 * @enum iotc_bsp_io_fs_resource_type_t
 * @brief describes types of resources that are available through this API.
 * These types were created in order to differentiate types based on their
 * security class.
 */
typedef enum iotc_bsp_io_fs_resource_type_e {
  IOTC_BSP_IO_FS_CERTIFICATE = 0, /**< 0 **/
  IOTC_BSP_IO_FS_CREDENTIALS,     /**< 1 **/
  IOTC_BSP_IO_FS_CONFIG_DATA      /**< 2 **/
} iotc_bsp_io_fs_resource_type_t;

/*
 * @name iotc_bsp_io_fs_stat_s
 * @brief Information that the IoTC Client needs returned
 * when iotc_bsp_io_fs_stat() is called.
 */
typedef struct iotc_bsp_io_fs_stat_s {
  size_t resource_size;
} iotc_bsp_io_fs_stat_t;

/**
 * @enum iotc_bsp_io_fs_open_flags_t
 * @brief Abstracted values that represent the various types of file operations
 * that should be passed to the underlying system when opening a file resource
 * via iotc_bsp_io_fs_open().

 * As a bitmask there could be more than one of these flags set on a given
 * open request.
 */
typedef enum iotc_bsp_io_fs_open_flags {
  IOTC_BSP_IO_FS_OPEN_READ = 1 << 0,
  IOTC_BSP_IO_FS_OPEN_WRITE = 1 << 1,
  IOTC_BSP_IO_FS_OPEN_APPEND = 1 << 2,
} iotc_bsp_io_fs_open_flags_t;

/**
 * @function
 * @brief Used by the IoTC Client to determine the existence and size of a
 * file.
 *
 * @param [in] resource_name the name of the file to check
 * @param [out] resource_stat a structure to be populated based on the file
 * size data.
 *
 * @return iotc_bsp_io_fs_state_t IOTC_STATE_OK in case of a success and other
 * in case of an error.
 */
iotc_bsp_io_fs_state_t iotc_bsp_io_fs_stat(
    const char* const resource_name, iotc_bsp_io_fs_stat_t* resource_stat);

/**
 * @function
 * @brief Requests that the file be opened for reading or writing, and that
 * a handle to the file be returned via an out parameter.
 *
 * @param [in] resource_name the name of the file to open
 * @param [in] size the size of file in bytes. Necessary on some
 * flash file system implementations which reserve space when
 * the file is opened for writing.  Not used in POSIX implementations.
 * @param [in] open_flags a read/write/append bitmask of operations as
 * defined by a bitmask type iotc_bsp_io_fs_open_flags_t.
 * @param [out] resource_handle_out a pointer to an abstracted
 * iotc_bsp_io_fs_resource_handle_t data type. This value will be passed to
 * future file operations such as read, write or close.
 *
 * @return iotc_bsp_io_fs_state_t IOTC_STATE_OK in case of a success and other
 * in case of an error.
 */
iotc_bsp_io_fs_state_t iotc_bsp_io_fs_open(
    const char* const resource_name, const size_t size,
    const iotc_bsp_io_fs_open_flags_t open_flags,
    iotc_bsp_io_fs_resource_handle_t* resource_handle_out);

/**
 * @function
 * @brief reads a subset of a file that was previously opened
 * with a call to iotc_bsp_io_fs_open.
 *
 * @param [in] resource_handle the handle created by a previous call
 * to iotc_bsp_io_fs_open.
 * @param [in] offset the position of the resource, in bytes, from which
 * to start read operations.
 * @param [out] buffer an outgoing pointer to a buffer which contains the bytes
 * read from the file. The BSP is responsible for all memory management of the
 * buffer itself. Please fill this buffer at offset zero each time this
 * function is called. You may reuse the buffer for each invocation.  If
 * you've allocated a buffer for each read operation then you may free the
 * previous buffer on a subsequent iotc_bsp_io_fs_read call, or when the file
 * is closed.
 * @param [out] buffer_size the number of bytes read from file and stored
 * in the buffer.
 *
 * @return iotc_bsp_io_fs_state_t IOTC_STATE_OK in case of a success and other
 * in case of an error.
 */
iotc_bsp_io_fs_state_t iotc_bsp_io_fs_read(
    const iotc_bsp_io_fs_resource_handle_t resource_handle, const size_t offset,
    const uint8_t** buffer, size_t* const buffer_size);

/**
 * @function
 * @brief writes to a portion of a file that was previously opened
 * with a call to iotc_bsp_io_fs_open.
 *
 * @param [in] resource_handle created by a previous call
 * to iotc_bsp_io_fs_open.
 * @param [in] buffer a pointer to a byte array. Data in this buffer
 * should be written to the corresponding resource. This buffer
 * should not be freed by this function.
 * @param [in] buffer_size the length of the buffer in bytes.
 * @param [in] offset the position of the resource, in bytes, to start
 * to the write operation.
 * @param [out] bytes_written store the number of bytes that were
 * successfully written to the resource.
 *
 * @return iotc_bsp_io_fs_state_t IOTC_STATE_OK in case of a success and other
 * in case of an error.
 */
iotc_bsp_io_fs_state_t iotc_bsp_io_fs_write(
    const iotc_bsp_io_fs_resource_handle_t resource_handle,
    const uint8_t* const buffer, const size_t buffer_size, const size_t offset,
    size_t* const bytes_written);

/**
 * @function
 * @brief signals that the corresponding resource read/write
 * operations are complete and that the resource should be closed.
 *
 * @param [in] resource_handle created by a previous call
 * to iotc_bsp_io_fs_open.  This is the handle to the resource
 * that should be closed.
 *
 * @return iotc_bsp_io_fs_state_t IOTC_STATE_OK in case of a success and other
 * in case of an error.
 */
iotc_bsp_io_fs_state_t iotc_bsp_io_fs_close(
    const iotc_bsp_io_fs_resource_handle_t resource_handle);

/**
 * @function
 * @brief request to remove the file/resources with the corresponding
 * resource name.
 *
 * @param [in] resource_name the name of the file to remove
 *
 * @return iotc_bsp_io_fs_state_t IOTC_STATE_OK in case of a success and other
 * in case of an error.
 */
iotc_bsp_io_fs_state_t iotc_bsp_io_fs_remove(const char* const resource_name);

#ifdef __cplusplus
}
#endif

#endif /* __IOTC_BSP_IO_FS_H__ */
