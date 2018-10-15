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

#ifndef __IOTC_THREAD_THREADPOOL_H__
#define __IOTC_THREAD_THREADPOOL_H__

#ifdef IOTC_MODULE_THREAD_ENABLED

#include <iotc_event_dispatcher_api.h>
#include <iotc_vector.h>
#include <stddef.h>

#define IOTC_THREADPOOL_MAXNUMOFTHREADS 10

/**
 * @brief threadpool, owns workerthreads, coordinates event executions
 */
typedef struct iotc_threadpool_s {
  iotc_vector_t* workerthreads;
  iotc_evtd_instance_t* threadpool_evtd;
} iotc_threadpool_t;

/**
 * @brief creates a threadpool instance
 *
 * @param num_of_threads The desired number of threads, note: there is a maximum
 * number of threads.
 */
iotc_threadpool_t* iotc_threadpool_create_instance(uint8_t num_of_threads);

/**
 * @brief Destroys a threadpool instance
 *
 * @param threadpool Double pointer on the treadpool.
 *
 * Destroys all worker threads in the pool, ensures all thread events are
 * executed. This matters for the caller of this destroy
 * function (current thread) as this operation may block.
 */
void iotc_threadpool_destroy_instance(iotc_threadpool_t** threadpool);

/**
 * @brief Enqueues event for any-thread execution.
 *
 * @param threadpool Enqueue the event into this threadpool.
 * @param handle This event handle will be enqueued.
 */
iotc_event_handle_queue_t* iotc_threadpool_execute(
    iotc_threadpool_t* threadpool, iotc_event_handle_t handle);

/**
 * @brief Enqueues event for specific thread execution
 *
 * @param threadpool Enqueue the event into one worker thread of this
 * threadpool.
 * @param handle The event handle will be enqueued.
 */
iotc_event_handle_queue_t* iotc_threadpool_execute_on_thread(
    iotc_threadpool_t* threadpool, iotc_event_handle_t handle, uint8_t tid);

#else

struct iotc_threadpool_t;

/**
 * @brief NULL threadpool generator for non-threaded library versions.
 */
#define iotc_threadpool_create_instance(...) NULL

/**
 * @brief NOOPERATION destructor for non-threaded library versions.
 */
#define iotc_threadpool_destroy_instance(...)

#endif

#endif /* __IOTC_THREAD_THREADPOOL_H__ */
