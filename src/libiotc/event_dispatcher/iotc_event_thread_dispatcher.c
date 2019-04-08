/* Copyright 2018-2019 Google LLC
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

#ifdef IOTC_MODULE_THREAD_ENABLED

#include "iotc_event_thread_dispatcher.h"
#include "iotc_event_dispatcher_api.h"
#include "iotc_globals.h"
#include "iotc_thread_ids.h"
#include "iotc_thread_threadpool.h"

iotc_event_handle_queue_t* iotc_evttd_execute(iotc_evtd_instance_t* evtd,
                                              iotc_event_handle_t handle) {
  if (IOTC_THREADID_MAINTHREAD == handle.target_tid ||
      iotc_globals.main_threadpool == NULL) {
    return iotc_evtd_execute(evtd, handle);
  } else if (handle.target_tid == IOTC_THREADID_ANYTHREAD) {
    return iotc_threadpool_execute(iotc_globals.main_threadpool, handle);
  } else {
    return iotc_threadpool_execute_on_thread(iotc_globals.main_threadpool,
                                             handle, handle.target_tid);
  }

  return NULL;
}
#else
/*
 * Using strict compiler settings requires every translation unit to contain
 * some code so we'll add an empty function here.
 */
void iotc_evttd_execute_dummy() { return; }
#endif
