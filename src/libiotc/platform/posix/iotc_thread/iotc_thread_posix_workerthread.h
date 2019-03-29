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

#ifndef __IOTC_THREAD_POSIX_WORKERTHREAD_H__
#define __IOTC_THREAD_POSIX_WORKERTHREAD_H__

#include <pthread.h>
#include "iotc_thread_workerthread.h"

typedef struct iotc_workerthread_s {
  iotc_evtd_instance_t* thread_evtd;
  iotc_evtd_instance_t* thread_evtd_secondary;

  pthread_t thread;
  uint8_t sync_start_flag;
} iotc_workerthread_t;

#endif /* __IOTC_THREAD_POSIX_WORKERTHREAD_H__ */
