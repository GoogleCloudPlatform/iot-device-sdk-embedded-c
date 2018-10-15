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

#ifndef __IOTC_IO_TIMEOUTS_H__
#define __IOTC_IO_TIMEOUTS_H__

#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>

#include "iotc_event_dispatcher_api.h"
#include "iotc_time.h"
#include "iotc_time_event.h"
#include "iotc_vector.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  iotc_time_event_handle_t timeout;
} iotc_io_timeout_t;

iotc_state_t iotc_io_timeouts_create(
    iotc_evtd_instance_t* instance, iotc_event_handle_t handle,
    iotc_time_t time_diff, iotc_vector_t* io_timeouts,
    iotc_time_event_handle_t* ret_time_event_handle);

void iotc_io_timeouts_cancel(iotc_evtd_instance_t* instance,
                             iotc_time_event_handle_t* time_event_handle,
                             iotc_vector_t* io_timeouts);

void iotc_io_timeouts_remove(iotc_time_event_handle_t* time_event_handle,
                             iotc_vector_t* io_timeouts);

void iotc_io_timeouts_restart(iotc_evtd_instance_t* instance,
                              iotc_time_t new_time, iotc_vector_t* io_timeouts);

#ifdef __cplusplus
}
#endif

#endif /* __IOTC_IO_TIMEOUTS_H__ */
