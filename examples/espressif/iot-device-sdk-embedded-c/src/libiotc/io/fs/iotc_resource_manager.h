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

#ifndef __IOTC_RESOURCE_MANAGER_H__
#define __IOTC_RESOURCE_MANAGER_H__

#include "iotc_data_desc.h"
#include "iotc_err.h"
#include "iotc_event_handle.h"
#include "iotc_fs_api.h"

/**
 * @struct iotc_resource_manager_context_t
 */
typedef struct iotc_resource_manager_context_s {
  iotc_data_desc_t* data_buffer; /* the storage for internal data buffer */
  /* pointer to a callback must be disposed at the end of any request processing
   * this is an invariant. If it's NULL it means that the request is finished.
   * While it is set means that one of the operation is pending( open, read,
   * write, close )*/
  iotc_event_handle_t callback;
  iotc_fs_stat_t resource_stat; /* copy of the resource stat passed with open */
  iotc_fs_resource_handle_t resource_handle; /* handle to the opened resource */
  iotc_fs_open_flags_t open_flags; /* copy of open flags passed with open */
  iotc_memory_type_t memory_type;  /* data_buffer's ownership */
  size_t data_offset; /* accumuleted value of bytes read or written */
  uint16_t cs;        /* current operation coroutine state */
} iotc_resource_manager_context_t;

/**
 * @brief iotc_resource_manager_make_context creates fresh context
 *
 * Creates and initialises a context that can be later used for other resource
 * manager functions for reading and writing from/to resource. Context has to be
 * destroyed via iotc_resource_manager_free_context function.
 *
 * Behaves like a memory malloc function.
 *
 * @param data_buffer optional parameter if an external memory suppose to be
 * used
 * @param context return parameter, function will create a context at given
 * pointer
 * @return IOTC_STATE_OK if operation succeded, one of error code otherwise
 */
iotc_state_t iotc_resource_manager_make_context(
    iotc_data_desc_t* data_buffer,
    iotc_resource_manager_context_t** const context);

/**
 * @brief iotc_resource_manager_free_context disposes the previously created
 * context
 *
 * Behaves like a memory free function.
 *
 * @param context valid context or NULL
 * @return IOTC_STATE_OK if operation succeded, one of error code otherwise
 */
iotc_state_t iotc_resource_manager_free_context(
    iotc_resource_manager_context_t** const context);

/**
 * @brief iotc_resource_manager_open begins the open sequence
 *
 * Starts the open coroutine. Open coroutine will keep calling filesystem's
 * functions in order to make sure that the file is available. In order to
 * achieve that filesystem's stat function will be called as a first function.
 * Result of the stat function will be kept in a iotc_resource_manager_context_t
 * structure. Immediately after stat function open filesystem function will be
 * called. After the resource manager finishes with the processing it will call
 * the callback with the result of an operation passed within the callback's
 * state parameter.
 *
 * @todo would be nice to think about a timeout for each function related to
 * long-term resource reading/writing related tasks.
 *
 * @param callback function that will be called upon a success or an error
 * @param resource_type one of the fs resource type
 * @param resource_name name of the resource
 * @param context previously created context
 * @return IOTC_STATE_OK if operation succeded, one of error code otherwise
 */
iotc_state_t iotc_resource_manager_open(
    iotc_resource_manager_context_t* const context,
    iotc_event_handle_t callback, const iotc_fs_resource_type_t resource_type,
    const char* const resource_name, const iotc_fs_open_flags_t open_flags,
    void* fs_context);

/**
 * @brief iotc_resource_manager_read begins the asynchronous reading process
 *
 * Starts the reading coroutine. Reading coroutine will keep calling read fs
 * function until the whole file is read or an error happen. After the
 * coroutine is done it will call the given callback with a state of an
 * operation. The read buffer can be taken from the
 * iotc_resource_manager_context_t's data_buffer.
 *
 * @param callback function that will be called upon a success or an error
 * @param context previously created context
 * @return IOTC_STATE_OK if operation succeded, one of error code otherwise
 */
iotc_state_t iotc_resource_manager_read(
    iotc_resource_manager_context_t* const context,
    iotc_event_handle_t callback, void* fs_context);

/**
 * @brief iotc_resource_manager_write
 *
 * @warning not yet implemented!
 *
 * @param callback function that will be called upon a success or an error
 * @param context previously created context
 * @return IOTC_STATE_OK if operation succeded, one of error code otherwise
 */
iotc_state_t iotc_resource_manager_write(
    iotc_resource_manager_context_t* const context,
    iotc_event_handle_t callback, iotc_data_desc_t* const data,
    void* fs_context);

/**
 * @brief iotc_resource_manager_close begins the asynchronous close operation
 *
 * Starts the close coroutine which will call the filesystem's close function.
 * After it's done it will invoke the given callback with a status of an
 * operation given through state callback's parameter.
 *
 * @param callback function that will be called upon a success or an error
 * @param context previously created context
 * @return IOTC_STATE_OK if operation succeded, one of error code otherwise
 */
iotc_state_t iotc_resource_manager_close(
    iotc_resource_manager_context_t* const context,
    iotc_event_handle_t callback, void* fs_context);

#endif /* __IOTC_RESOURCE_MANAGER_H */
