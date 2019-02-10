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

#include "iotc_globals.h"

iotc_globals_t iotc_globals = {
    .network_timeout = 1500,
    .globals_ref_count = 0,
    .evtd_instance = NULL,
    .default_context = NULL,
    .default_context_handle = IOTC_INVALID_CONTEXT_HANDLE,
    .context_handles_vector = NULL,
    .timed_tasks_container = NULL,
    .main_threadpool = NULL,
    .backoff_status = {iotc_make_empty_time_event_handle(), 0, 0,
                       IOTC_BACKOFF_CLASS_NONE, 0}};
