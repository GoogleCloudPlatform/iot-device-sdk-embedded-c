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

#include <iotc_bsp_io_fs.h>

iotc_bsp_io_fs_state_t iotc_bsp_io_fs_open(
    const char* const resource_name, const size_t size,
    const iotc_bsp_io_fs_open_flags_t open_flags,
    iotc_bsp_io_fs_resource_handle_t* resource_handle_out) {
  (void)resource_name;
  (void)size;
  (void)open_flags;
  (void)resource_handle_out;

  return IOTC_BSP_IO_FS_NOT_IMPLEMENTED;
}

iotc_bsp_io_fs_state_t iotc_bsp_io_fs_read(
    const iotc_bsp_io_fs_resource_handle_t resource_handle, const size_t offset,
    const uint8_t** buffer, size_t* const buffer_size) {
  (void)resource_handle;
  (void)offset;
  (void)buffer;
  (void)buffer_size;

  return IOTC_BSP_IO_FS_NOT_IMPLEMENTED;
}

iotc_bsp_io_fs_state_t iotc_bsp_io_fs_write(
    const iotc_bsp_io_fs_resource_handle_t resource_handle,
    const uint8_t* const buffer, const size_t buffer_size, const size_t offset,
    size_t* const bytes_written) {
  (void)resource_handle;
  (void)buffer;
  (void)buffer_size;
  (void)offset;
  (void)bytes_written;

  return IOTC_BSP_IO_FS_NOT_IMPLEMENTED;
}

iotc_bsp_io_fs_state_t iotc_bsp_io_fs_close(
    const iotc_bsp_io_fs_resource_handle_t resource_handle) {
  (void)resource_handle;

  return IOTC_BSP_IO_FS_NOT_IMPLEMENTED;
}

iotc_bsp_io_fs_state_t iotc_bsp_io_fs_remove(const char* const resource_name) {
  (void)resource_name;

  return IOTC_BSP_IO_FS_NOT_IMPLEMENTED;
}
