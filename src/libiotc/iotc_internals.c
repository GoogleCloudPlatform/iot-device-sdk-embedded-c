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

#include "iotc_internals.h"

/* By default set the function pointers to the enabled type/system/storage
 * if none set use empty function pointer table.
 */
#include "iotc_fs_header.h"

#if defined(IOTC_FS_MEMORY) || defined(IOTC_FS_DUMMY) || defined(IOTC_FS_POSIX)
iotc_internals_t iotc_internals = {
    {sizeof(iotc_fs_functions_t), &iotc_fs_stat, &iotc_fs_open, &iotc_fs_read,
     &iotc_fs_write, &iotc_fs_close, &iotc_fs_remove}};
#else
#error No filesystem defined!!!
#endif
