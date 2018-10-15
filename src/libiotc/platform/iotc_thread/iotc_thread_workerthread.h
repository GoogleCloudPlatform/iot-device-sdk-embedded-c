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

#ifndef __IOTC_THREAD_WORKERTHREAD_H__
#define __IOTC_THREAD_WORKERTHREAD_H__

#include <iotc_event_dispatcher_api.h>

struct iotc_workerthread_s;

/**
 * @brief Creates a workerthread instance.
 *
 * @param evtd_secondary Secondary event source. Workerthread does not own this
 * evtd neither ensures all event processing upon destruction. Workerthread
 * simply consumes a SINGLE event of this secondary evtd in each loop.
 */
struct iotc_workerthread_s* iotc_workerthread_create_instance(
    iotc_evtd_instance_t* evtd_secondary);

/**
 * @brief Destroys workerthread instance.
 *
 * At return all events are executed contained by the primary evtd.
 */
void iotc_workerthread_destroy_instance(
    struct iotc_workerthread_s** workerthread);

/**
 * @brief Waits for syncpoint: workerthread started to run.
 *
 * At return the workerthread is actually running. After calling this function
 * one can be certain that enqueuing events to evtd of the workerthread will be
 * processed.
 *
 * @retval 1        Sync point is reached, thread is running at return.
 * @retval 0        Timeout occurred, probably the thread has not started yet,
 * see impl. for timeout value (5sec).
 */
uint8_t iotc_workerthread_wait_sync_point(
    struct iotc_workerthread_s* workerthread);

#endif /* __IOTC_THREAD_WORKERTHREAD_H__ */
