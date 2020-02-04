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

#include <iotc_thread_posix_workerthread.h>
#include "iotc_thread_threadpool.h"

iotc_threadpool_t* iotc_threadpool_create_instance(uint8_t num_of_threads) {
  num_of_threads =
      IOTC_MIN(IOTC_MAX(num_of_threads, 1), IOTC_THREADPOOL_MAXNUMOFTHREADS);

  iotc_state_t state = IOTC_STATE_OK;
  IOTC_ALLOC(iotc_threadpool_t, threadpool, state);

  threadpool->threadpool_evtd = iotc_evtd_create_instance();

  IOTC_CHECK_CND_DBGMESSAGE(threadpool->threadpool_evtd == NULL,
                            IOTC_OUT_OF_MEMORY, state,
                            "could not create event dispatcher for threadpool");

  threadpool->workerthreads = iotc_vector_create();

  IOTC_CHECK_CND_DBGMESSAGE(threadpool->workerthreads == NULL,
                            IOTC_OUT_OF_MEMORY, state,
                            "could not instantiate vector for workerthreads");

  int8_t result =
      iotc_vector_reserve(threadpool->workerthreads, num_of_threads);

  IOTC_CHECK_CND_DBGMESSAGE(
      result == 0, IOTC_OUT_OF_MEMORY, state,
      "could not reserve enough space in vector for workerthreads");

  uint8_t counter_workerthread = 0;
  for (; counter_workerthread < num_of_threads; ++counter_workerthread) {
    iotc_workerthread_t* new_workerthread =
        iotc_workerthread_create_instance(threadpool->threadpool_evtd);

    IOTC_CHECK_CND_DBGMESSAGE(new_workerthread == NULL, IOTC_OUT_OF_MEMORY,
                              state, "could not allocate a workerthread");

    iotc_vector_push(
        threadpool->workerthreads,
        IOTC_VEC_CONST_VALUE_PARAM(IOTC_VEC_VALUE_PTR(new_workerthread)));
  }

  return threadpool;

err_handling:
  return NULL;
}

void iotc_threadpool_destroy_instance(iotc_threadpool_t** threadpool) {
  if (threadpool == NULL || *threadpool == NULL) return;

  iotc_threadpool_t* threadpool_ptr = *threadpool;

  if (threadpool_ptr->workerthreads != NULL) {
    /* stop all workerthreads in advance their destroy to avoid summing up join
     * times at destruction with that all thread exits are done parallelly */
    uint8_t counter_workerthread = 0;
    for (; counter_workerthread < threadpool_ptr->workerthreads->elem_no;
         ++counter_workerthread) {
      iotc_evtd_stop(((iotc_workerthread_t*)threadpool_ptr->workerthreads
                          ->array[counter_workerthread]
                          .selector_t.ptr_value)
                         ->thread_evtd);
    }

    counter_workerthread = 0;
    for (; counter_workerthread < threadpool_ptr->workerthreads->elem_no;
         ++counter_workerthread) {
      iotc_workerthread_destroy_instance(
          (iotc_workerthread_t**)&threadpool_ptr->workerthreads
              ->array[counter_workerthread]
              .selector_t.ptr_value);
    }

    if (threadpool_ptr->threadpool_evtd != NULL) {
      /* ensure all any-thread handlers are executed before destroy */
      iotc_evtd_step(threadpool_ptr->threadpool_evtd, time(0));
    }

    iotc_vector_destroy(threadpool_ptr->workerthreads);
  }

  iotc_evtd_destroy_instance(threadpool_ptr->threadpool_evtd);

  IOTC_SAFE_FREE(*threadpool);
}

iotc_event_handle_queue_t* iotc_threadpool_execute(
    iotc_threadpool_t* threadpool, iotc_event_handle_t handle) {
  if (threadpool == NULL || threadpool->threadpool_evtd == NULL) return NULL;

  return iotc_evtd_execute(threadpool->threadpool_evtd, handle);
}

iotc_event_handle_queue_t* iotc_threadpool_execute_on_thread(
    iotc_threadpool_t* threadpool, iotc_event_handle_t handle, uint8_t tid) {
  if (threadpool == NULL || threadpool->workerthreads == NULL ||
      threadpool->workerthreads->array == NULL)
    return NULL;

  if (tid < threadpool->workerthreads->elem_no &&
      threadpool->workerthreads->array[tid].selector_t.ptr_value !=
          NULL) { /* valid tid */
    iotc_workerthread_t* workerthread_target =
        ((iotc_workerthread_t*)threadpool->workerthreads->array[tid]
             .selector_t.ptr_value);

    if (workerthread_target->thread_evtd != NULL) {
      return iotc_evtd_execute(workerthread_target->thread_evtd, handle);
    }
  }

  /* invalid tid, fallback: execute handler on any thread */
  return iotc_threadpool_execute(threadpool, handle);
}
