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

#include "iotc_debug.h"
#include "iotc_fs_filename_defs.h"
#include "iotc_fs_header.h"
#include "iotc_macros.h"

#ifndef IOTC_NO_TLS_LAYER
#include "iotc_RootCA_list.h"
#endif

/* The size of the buffer to be used for reads. */
#ifndef IOTC_NO_TLS_LAYER
const size_t iotc_fs_buffer_size = IOTC_ROOTCA_LIST_BYTE_LENGTH;
#else
const size_t iotc_fs_buffer_size = 512;
#endif /* IOTC_NO_TLS_LAYER */

/* local stat handler function type defined per resource id */
typedef iotc_state_t(iotc_fs_memory_stat_t)(
    const iotc_fs_resource_handle_t resource_id, iotc_fs_stat_t* resource_stat);

/* local open handler function type defined per resource id */
typedef iotc_state_t(iotc_fs_memory_open_t)(
    const iotc_fs_resource_handle_t resource_id);

typedef enum iotc_fs_memory_resource_state_e {
  IOTC_FS_MEMORY_RESOURCE_STATE_CLOSED = 0,
  IOTC_FS_MEMORY_RESOURCE_STATE_OPEN
} iotc_fs_memory_resource_state_t;

/* declarations of all memory functions */
#ifndef IOTC_NO_TLS_LAYER
iotc_state_t iotc_fs_memory_stat_builtin_cert(
    const iotc_fs_resource_handle_t resource_id, iotc_fs_stat_t* resource_stat);
iotc_state_t iotc_fs_memory_open_builtin_cert(
    const iotc_fs_resource_handle_t resource_id);
#endif

/*
 * @struct iotc_fs_memory_database_s
 * @brief describes a single entry in a memory of fs database
 */
typedef struct iotc_fs_memory_database_s {
  const char* const file_name; /* file name of the entry */
  iotc_fs_memory_stat_t*
      stat_handler_function; /* associated stat handler function */
  iotc_fs_memory_open_t*
      open_handler_function; /* associated open handler function */
  const void* memory_ptr;    /* resource memory pointer */
  size_t open_counter;       /* the counter for open counts */
  iotc_fs_memory_resource_state_t resource_state; /* state associated to that
                                                   resource */
} iotc_fs_memory_database_t;

iotc_fs_memory_database_t IOTC_FS_MEMORY_DATABASE[] = {
#ifndef IOTC_NO_TLS_LAYER
    {.file_name = IOTC_GLOBAL_CERTIFICATE_FILE_NAME_DEF,
     .stat_handler_function = &iotc_fs_memory_stat_builtin_cert,
     .open_handler_function = &iotc_fs_memory_open_builtin_cert,
     .resource_state = IOTC_FS_MEMORY_RESOURCE_STATE_CLOSED,
     .open_counter = 0,
     .memory_ptr = NULL}
#endif
};

#ifndef IOTC_NO_TLS_LAYER
iotc_state_t iotc_fs_memory_stat_builtin_cert(
    const iotc_fs_resource_handle_t resource_id,
    iotc_fs_stat_t* resource_stat) {
  IOTC_UNUSED(resource_id);

  if (NULL == resource_stat) {
    return IOTC_INVALID_PARAMETER;
  }

  /* update the resource size by taking it from the compiled in array */
  resource_stat->resource_size = sizeof(iotc_RootCA_list);

  return IOTC_STATE_OK;
}

iotc_state_t iotc_fs_memory_open_builtin_cert(
    const iotc_fs_resource_handle_t resource_id) {
  assert(resource_id >= 0);
  assert(resource_id < (signed)IOTC_ARRAYSIZE(IOTC_FS_MEMORY_DATABASE));

  iotc_fs_memory_database_t* const entry =
      &IOTC_FS_MEMORY_DATABASE[resource_id];

  entry->memory_ptr = iotc_RootCA_list;

  return IOTC_STATE_OK;
}
#endif

inline static iotc_state_t iotc_fs_memory_find_entry(
    const char* resource_name, iotc_fs_resource_handle_t* resource_id) {
  const size_t database_size = IOTC_ARRAYSIZE(IOTC_FS_MEMORY_DATABASE);

  // PRE-CONDITIONS
  assert(NULL != resource_name);
  assert(NULL != resource_id);

  size_t i = 0;

  *resource_id = IOTC_FS_INVALID_RESOURCE_HANDLE;

  for (; i < database_size; ++i) {
    if (0 == strcmp(resource_name, IOTC_FS_MEMORY_DATABASE[i].file_name)) {
      /* update the return resource_id parameter value */
      *resource_id = (iotc_fs_resource_handle_t)i;

      /* bail out as early as it's possible */
      return IOTC_STATE_OK;
    }
  }

  return IOTC_FS_RESOURCE_NOT_AVAILABLE;
}

iotc_state_t iotc_fs_stat(const void* context,
                          const iotc_fs_resource_type_t resource_type,
                          const char* const resource_name,
                          iotc_fs_stat_t* resource_stat) {
  IOTC_UNUSED(context);

  iotc_state_t ret = IOTC_FS_RESOURCE_NOT_AVAILABLE;
  iotc_fs_resource_handle_t resource_id = iotc_fs_init_resource_handle();

  if (NULL == resource_stat || NULL == resource_name) {
    return IOTC_INVALID_PARAMETER;
  }

  switch (resource_type) {
    case IOTC_FS_CERTIFICATE:
    case IOTC_FS_CREDENTIALS:
    case IOTC_FS_CONFIG_DATA:
      ret = iotc_fs_memory_find_entry(resource_name, &resource_id);
      break;
    default:
      assert(0);
      ret = IOTC_INTERNAL_ERROR;
  }

  if (IOTC_STATE_OK == ret) {
    return (*IOTC_FS_MEMORY_DATABASE[resource_id].stat_handler_function)(
        resource_id, resource_stat);
  }

  return IOTC_FS_RESOURCE_NOT_AVAILABLE;
}

iotc_state_t iotc_fs_open(const void* context,
                          const iotc_fs_resource_type_t resource_type,
                          const char* const resource_name,
                          const iotc_fs_open_flags_t open_flags,
                          iotc_fs_resource_handle_t* resource_handle) {
  IOTC_UNUSED(resource_type);
  IOTC_UNUSED(context);

  if (NULL == resource_name || NULL == resource_handle) {
    return IOTC_INVALID_PARAMETER;
  }

  /* it's read only filesystem */
  if (IOTC_FS_OPEN_WRITE == (open_flags & IOTC_FS_OPEN_WRITE) ||
      IOTC_FS_OPEN_APPEND == (open_flags & IOTC_FS_OPEN_APPEND)) {
    return IOTC_FS_ERROR;
  }

  iotc_state_t res = iotc_fs_memory_find_entry(resource_name, resource_handle);

  if (IOTC_STATE_OK == res) {
    /* PRE-CONDITION */
    assert(*resource_handle >= 0);
    assert(*resource_handle < (signed)IOTC_ARRAYSIZE(IOTC_FS_MEMORY_DATABASE));

    iotc_fs_memory_database_t* const entry =
        &IOTC_FS_MEMORY_DATABASE[*resource_handle];

    entry->resource_state = IOTC_FS_MEMORY_RESOURCE_STATE_OPEN;
    entry->open_counter += 1;

    /* PRE-CONDITION */
    assert(NULL != entry->open_handler_function);

    return entry->open_handler_function(*resource_handle);
  }

  return res;
}

iotc_state_t iotc_fs_read(const void* context,
                          const iotc_fs_resource_handle_t resource_handle,
                          const size_t offset, const uint8_t** buffer,
                          size_t* const buffer_size) {
  IOTC_UNUSED(context);

  const size_t database_size = IOTC_ARRAYSIZE(IOTC_FS_MEMORY_DATABASE);

  if (IOTC_FS_INVALID_RESOURCE_HANDLE == resource_handle ||
      resource_handle > (iotc_fs_resource_handle_t)database_size) {
    return IOTC_INVALID_PARAMETER;
  }

  if (IOTC_FS_MEMORY_RESOURCE_STATE_OPEN !=
      IOTC_FS_MEMORY_DATABASE[resource_handle].resource_state) {
    return IOTC_FS_ERROR;
  }

  if (NULL == buffer || NULL == buffer_size) {
    return IOTC_INVALID_PARAMETER;
  }

  if (NULL != *buffer) {
    return IOTC_INVALID_PARAMETER;
  }

  iotc_fs_stat_t resource_stat;

  const iotc_fs_memory_database_t* const entry =
      &IOTC_FS_MEMORY_DATABASE[resource_handle];

  // PRE-CONDITION
  assert(NULL != entry->stat_handler_function);

  iotc_state_t res =
      entry->stat_handler_function(resource_handle, &resource_stat);

  if (IOTC_STATE_OK == res) {
    /* calculate the real offset by picking the min of
     * ( offset value, size ) */
    intptr_t real_offset =
        (intptr_t)(IOTC_MIN(offset, resource_stat.resource_size));

    /* update the return pointer with the proper information */
    *buffer = (const uint8_t*)((intptr_t)entry->memory_ptr + real_offset);

    /* update the size of the buffer to be returned taking into account chunk
     * size */
    *buffer_size = IOTC_MIN(resource_stat.resource_size - real_offset,
                            iotc_fs_buffer_size);
  }

  return res;
}

iotc_state_t iotc_fs_write(const void* context,
                           const iotc_fs_resource_handle_t resource_handle,
                           const uint8_t* const buffer,
                           const size_t buffer_size, const size_t offset,
                           size_t* const bytes_written) {
  IOTC_UNUSED(context);
  IOTC_UNUSED(resource_handle);
  IOTC_UNUSED(buffer);
  IOTC_UNUSED(buffer_size);
  IOTC_UNUSED(offset);
  IOTC_UNUSED(bytes_written);

  /* writing is forbidden as this is read only filesystem */
  return IOTC_FS_ERROR;
}

iotc_state_t iotc_fs_close(const void* context,
                           const iotc_fs_resource_handle_t resource_handle) {
  IOTC_UNUSED(context);

  const size_t database_size = IOTC_ARRAYSIZE(IOTC_FS_MEMORY_DATABASE);

  if (IOTC_FS_INVALID_RESOURCE_HANDLE == resource_handle ||
      resource_handle > (iotc_fs_resource_handle_t)database_size) {
    return IOTC_INVALID_PARAMETER;
  }

  iotc_fs_memory_database_t* const entry =
      &IOTC_FS_MEMORY_DATABASE[resource_handle];

  if (IOTC_FS_MEMORY_RESOURCE_STATE_OPEN != entry->resource_state) {
    return IOTC_FS_ERROR;
  }

  entry->open_counter -= 1;

  if (0 == entry->open_counter) {
    entry->memory_ptr = NULL;
    entry->resource_state = IOTC_FS_MEMORY_RESOURCE_STATE_CLOSED;
  }

  return IOTC_STATE_OK;
}

iotc_state_t iotc_fs_remove(const void* context,
                            const iotc_fs_resource_type_t resource_type,
                            const char* const resource_name) {
  IOTC_UNUSED(context);
  IOTC_UNUSED(resource_type);
  IOTC_UNUSED(resource_name);

  iotc_state_t ret = IOTC_FS_ERROR;
  iotc_fs_resource_handle_t resource_id = iotc_fs_init_resource_handle();

  switch (resource_type) {
    case IOTC_FS_CERTIFICATE:
    case IOTC_FS_CREDENTIALS:
    case IOTC_FS_CONFIG_DATA:
      ret = iotc_fs_memory_find_entry(resource_name, &resource_id);
  }

  return (ret == IOTC_STATE_OK) ? IOTC_FS_ERROR
                                : IOTC_FS_RESOURCE_NOT_AVAILABLE;
}
