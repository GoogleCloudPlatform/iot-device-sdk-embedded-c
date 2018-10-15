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

#ifndef __IOTC_GLOBALS_H__
#define __IOTC_GLOBALS_H__

#include <stdint.h>

#include "iotc_backoff_status_api.h"
#include "iotc_timed_task.h"
#include "iotc_types_internal.h"

#ifdef __cplusplus
extern "C" {
#endif

#include "iotc_event_dispatcher_api.h"

/* This struct is used for run-time config */
typedef struct {
  uint32_t network_timeout;
  uint8_t globals_ref_count;
  iotc_evtd_instance_t* evtd_instance;
  iotc_context_t* default_context;
  iotc_context_handle_t default_context_handle;
  iotc_vector_t* context_handles_vector;
  iotc_timed_task_container_t* timed_tasks_container;
  struct iotc_threadpool_s* main_threadpool;
  iotc_backoff_status_t backoff_status;
} iotc_globals_t;

extern iotc_globals_t iotc_globals;

#ifdef __cplusplus
}
#endif

#endif /* __IOTC_GLOBALS_H__ */
