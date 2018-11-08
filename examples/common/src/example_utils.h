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

#define PRIVATE_KEY_BUFFER_SIZE 256

#include <iotc.h>

int iotc_example_handle_command_line_args(int argc, char* argv[]);

int load_ec_private_key_pem_from_posix_fs(char* buf_ec_private_key_pem, size_t buf_len);

void on_connection_state_changed(iotc_context_handle_t in_context_handle,
                                 void* data, iotc_state_t state);

void publish_function(iotc_context_handle_t context_handle,
                      iotc_timed_task_handle_t timed_task, void* user_data);
