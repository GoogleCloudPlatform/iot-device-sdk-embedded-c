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

#include "iotc_bsp_io_fs.h"
#include "iotc_fs_bsp_to_iotc_mapping.h"
#include "iotc_fs_header.h"
#include "iotc_macros.h"

const size_t iotc_fs_buffer_size = 1024;

iotc_state_t iotc_fs_stat(const void* context,
                          const iotc_fs_resource_type_t resource_type,
                          const char* const resource_name,
                          iotc_fs_stat_t* resource_stat) {
  IOTC_UNUSED(context);
  IOTC_UNUSED(resource_type);

  if (NULL == resource_stat || NULL == resource_name) {
    return IOTC_INVALID_PARAMETER;
  }

  iotc_bsp_io_fs_stat_t bsp_io_fs_resource_stat;
  iotc_state_t result = iotc_fs_bsp_io_fs_2_iotc_state(
      iotc_bsp_io_fs_stat(resource_name, &bsp_io_fs_resource_stat));
  if (IOTC_STATE_OK == result) {
    resource_stat->resource_size = bsp_io_fs_resource_stat.resource_size;
  }

  return result;
}

iotc_state_t iotc_fs_open(const void* context,
                          const iotc_fs_resource_type_t resource_type,
                          const char* const resource_name,
                          const iotc_fs_open_flags_t open_flags,
                          iotc_fs_resource_handle_t* resource_handle) {
  IOTC_UNUSED(context);
  IOTC_UNUSED(resource_type);

  return iotc_fs_bsp_io_fs_2_iotc_state(iotc_bsp_io_fs_open(
      resource_name, 0 /* not used in POSIX version */,
      (iotc_bsp_io_fs_open_flags_t)open_flags, resource_handle));
}

iotc_state_t iotc_fs_read(const void* context,
                          const iotc_fs_resource_handle_t resource_handle,
                          const size_t offset, const uint8_t** buffer,
                          size_t* const buffer_size) {
  IOTC_UNUSED(context);

  return iotc_fs_bsp_io_fs_2_iotc_state(
      iotc_bsp_io_fs_read(resource_handle, offset, buffer, buffer_size));
}

iotc_state_t iotc_fs_write(const void* context,
                           const iotc_fs_resource_handle_t resource_handle,
                           const uint8_t* const buffer,
                           const size_t buffer_size, const size_t offset,
                           size_t* const bytes_written) {
  IOTC_UNUSED(context);

  return iotc_fs_bsp_io_fs_2_iotc_state(iotc_bsp_io_fs_write(
      resource_handle, buffer, buffer_size, offset, bytes_written));
}

iotc_state_t iotc_fs_close(const void* context,
                           const iotc_fs_resource_handle_t resource_handle) {
  IOTC_UNUSED(context);

  return iotc_fs_bsp_io_fs_2_iotc_state(iotc_bsp_io_fs_close(resource_handle));
}

iotc_state_t iotc_fs_remove(const void* context,
                            const iotc_fs_resource_type_t resource_type,
                            const char* const resource_name) {
  IOTC_UNUSED(context);
  IOTC_UNUSED(resource_type);

  return iotc_fs_bsp_io_fs_2_iotc_state(iotc_bsp_io_fs_remove(resource_name));
}
