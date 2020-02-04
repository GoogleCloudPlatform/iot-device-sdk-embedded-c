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

#ifndef __IOTC_EVENT_THREAD_DISPATCHER_H__
#define __IOTC_EVENT_THREAD_DISPATCHER_H__

#include "iotc_event_dispatcher_api.h"

#ifdef IOTC_MODULE_THREAD_ENABLED

#include "iotc_event_handle_queue.h"

/**
 * @brief dispatches event to proper evtd for execution on targeted thread
 *
 * Create a threaded event, enqueue event through this function to
 * ensure execution on proper thread.
 */
extern iotc_event_handle_queue_t* iotc_evttd_execute(
    iotc_evtd_instance_t* evtd, iotc_event_handle_t handle);

#else

/**
 * @brief for non-threaded library version: enqueue event to global evtd
 *          for main thread execution
 */
#define iotc_evttd_execute(evtd, handle) iotc_evtd_execute(evtd, handle)

#endif

#endif /* __IOTC_EVENT_THREAD_DISPATCHER_H__ */
