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

#include "iotc_fs_header.h"
#include "iotc_macros.h"

iotc_state_t iotc_fs_stat(const void* context,
                          const iotc_fs_resource_type_t resource_type,
                          const char* const resource_name,
                          iotc_fs_stat_t* resource_stat) {
  IOTC_UNUSED(context);
  IOTC_UNUSED(resource_type);
  IOTC_UNUSED(resource_name);
  IOTC_UNUSED(resource_stat);

  return IOTC_FS_RESOURCE_NOT_AVAILABLE;
}

iotc_state_t iotc_fs_open(const void* context,
                          const iotc_fs_resource_type_t resource_type,
                          const char* const resource_name,
                          const iotc_fs_open_flags_t open_flags,
                          iotc_fs_resource_handle_t* resource_handle) {
  IOTC_UNUSED(context);
  IOTC_UNUSED(resource_type);
  IOTC_UNUSED(resource_name);
  IOTC_UNUSED(open_flags);
  IOTC_UNUSED(resource_handle);

  return IOTC_FS_ERROR;
}

iotc_state_t iotc_fs_read(const void* context,
                          const iotc_fs_resource_handle_t resource_handle,
                          const size_t offset, const uint8_t** buffer,
                          size_t* const buffer_size) {
  IOTC_UNUSED(context);
  IOTC_UNUSED(resource_handle);
  IOTC_UNUSED(offset);
  IOTC_UNUSED(buffer);
  IOTC_UNUSED(buffer_size);

  return IOTC_FS_ERROR;
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

  return IOTC_FS_ERROR;
}

iotc_state_t iotc_fs_close(const void* context,
                           const iotc_fs_resource_handle_t resource_handle) {
  IOTC_UNUSED(context);
  IOTC_UNUSED(resource_handle);

  return IOTC_FS_ERROR;
}

iotc_state_t iotc_fs_remove(const void* context,
                            const iotc_fs_resource_type_t resource_type,
                            const char* const resource_name) {
  IOTC_UNUSED(context);
  IOTC_UNUSED(resource_type);
  IOTC_UNUSED(resource_name);

  return IOTC_FS_RESOURCE_NOT_AVAILABLE;
}
