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

#ifndef __IOTC_TIMED_TASK_H__
#define __IOTC_TIMED_TASK_H__

#include "iotc_critical_section.h"
#include "iotc_macros.h"
#include "iotc_types_internal.h"
#include "iotc_vector.h"

typedef struct iotc_timed_task_container_s {
  struct iotc_critical_section_s* cs;
  iotc_vector_t* timed_tasks_vector;
} iotc_timed_task_container_t;

iotc_timed_task_container_t* iotc_make_timed_task_container();

void iotc_destroy_timed_task_container(iotc_timed_task_container_t* container);

iotc_timed_task_handle_t iotc_add_timed_task(
    iotc_timed_task_container_t* container, iotc_evtd_instance_t* dispatcher,
    iotc_context_handle_t context_handle, iotc_user_task_callback_t* callback,
    iotc_time_t seconds_from_now, const uint8_t repeats_forever, void* data);

void iotc_remove_timed_task(iotc_timed_task_container_t* container,
                            iotc_timed_task_handle_t timed_task_handle);

#endif /* __IOTC_TIMED_TASK_H__ */
