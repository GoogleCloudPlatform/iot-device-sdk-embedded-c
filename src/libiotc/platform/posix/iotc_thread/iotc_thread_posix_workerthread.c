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
#include <errno.h>
#include <unistd.h>

#include <iotc_thread_posix_workerthread.h>

#define IOTC_THREAD_WORKERTHREAD_RESTTIME_IN_NANOSECONDS 10000000;  // 1/100 sec
#define IOTC_THREAD_WORKERTHREAD_WAITFORSYNCTIME_IN_NANOSECONDS \
  100000000;  // 1/10 sec

void* iotc_workerthread_start_routine(void* ctx) {
  iotc_state_t state = IOTC_STATE_OK;

  IOTC_CHECK_CND_DBGMESSAGE(ctx == NULL, IOTC_INVALID_PARAMETER, state,
                            "no context is provided for workerthrad function");

  iotc_workerthread_t* corresponding_workerthread = (iotc_workerthread_t*)ctx;

  iotc_debug_format("[%p] sync point reached", pthread_self());
  corresponding_workerthread->sync_start_flag = 1;
  iotc_debug_format("[%p] sync point passed", pthread_self());

  struct timespec deltatime;
  deltatime.tv_sec = 0;
  deltatime.tv_nsec = IOTC_THREAD_WORKERTHREAD_RESTTIME_IN_NANOSECONDS;

  /* Simple event loop impl, just executes all handlers in event dispatcher
   * in each 1/100 sec. */
  while (
      iotc_evtd_dispatcher_continue(corresponding_workerthread->thread_evtd)) {
    /* Consume all handles of evtd. */
    iotc_evtd_step(corresponding_workerthread->thread_evtd, time(0));
    /* Consume a single handle of secondary evtd. */
    if (iotc_evtd_dispatcher_continue(
            corresponding_workerthread->thread_evtd_secondary)) {
      iotc_evtd_single_step(corresponding_workerthread->thread_evtd_secondary,
                            time(0));
    }
    /* Let the thread rest a little. */
    nanosleep(&deltatime, NULL);
  }

  /* Ensuring execution of handlers added right before turning of event
   * dispatcher. */
  iotc_evtd_step(corresponding_workerthread->thread_evtd, time(0));

err_handling:
  return NULL;
}

iotc_workerthread_t* iotc_workerthread_create_instance(
    iotc_evtd_instance_t* evtd_secondary) {
  iotc_state_t state = IOTC_STATE_OK;
  IOTC_ALLOC(iotc_workerthread_t, new_workerthread_instance, state);

  new_workerthread_instance->thread_evtd = iotc_evtd_create_instance();

  IOTC_CHECK_CND_DBGMESSAGE(
      new_workerthread_instance->thread_evtd == NULL, IOTC_OUT_OF_MEMORY, state,
      "could not create event dispatcher for new iotc_workerthread instance");

  new_workerthread_instance->thread_evtd_secondary = evtd_secondary;

  const int ret_pthread_create = pthread_create(
      &new_workerthread_instance->thread, NULL, iotc_workerthread_start_routine,
      new_workerthread_instance);

  if (ret_pthread_create != 0) {
    iotc_debug_format("creation of pthread instance failed with error: %d",
                      ret_pthread_create);
    goto err_handling;
  }

  return new_workerthread_instance;

err_handling:
  IOTC_SAFE_FREE(new_workerthread_instance);

  return NULL;
}

void iotc_workerthread_destroy_instance(iotc_workerthread_t** workerthread) {
  if (workerthread == NULL || *workerthread == NULL) return;

  if ((*workerthread)->thread_evtd != NULL) {
    iotc_evtd_stop((*workerthread)->thread_evtd);
  }

  const int ret_pthread_join = pthread_join((*workerthread)->thread, NULL);
  IOTC_UNUSED(ret_pthread_join);

  if ((*workerthread)->thread_evtd != NULL) {
    iotc_evtd_destroy_instance((*workerthread)->thread_evtd);
  }

  IOTC_SAFE_FREE(*workerthread);
}

uint8_t iotc_workerthread_wait_sync_point(iotc_workerthread_t* workerthread) {
  iotc_debug_format("[%p] waiting for sync start of thread [%p]...",
                    pthread_self(), workerthread->thread);

  struct timespec deltatime;
  deltatime.tv_sec = 0;
  deltatime.tv_nsec = IOTC_THREAD_WORKERTHREAD_WAITFORSYNCTIME_IN_NANOSECONDS;

  /* Note: pthread_mutex_timedlock_np is not available on osx.
   * Note: pthread_barrier_t is optional for POSIX pthreads, not available on
   * osx. */
  size_t i = 0;
  for (; i < 50; ++i) {
    if (workerthread->sync_start_flag != 0) break;
    nanosleep(&deltatime, NULL);
  }

  iotc_debug_format("[%p] sync start %s for thread [%p]", pthread_self(),
                    workerthread->sync_start_flag ? "OK" : "FAILED",
                    workerthread->thread);

  return (workerthread->sync_start_flag != 0) ? 1 : 0;
}
