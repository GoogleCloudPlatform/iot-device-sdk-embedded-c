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
#ifndef __IOTC_THREAD_IDS_H__
#define __IOTC_THREAD_IDS_H__

/**
 * @brief Global library thread IDs.
 *
 * With one of these IDs can one create event with a target thread.
 */
enum {
  /* Specific thread IDs must start with 0, they specify position of thread
   * in main threadpool. */
  IOTC_THREADID_THREAD_0 = 0,
  IOTC_THREADID_THREAD_1,
  IOTC_THREADID_THREAD_2,

  IOTC_THREADID_ANYTHREAD = 250,
  IOTC_THREADID_MAINTHREAD
};

#endif /* __IOTC_THREAD_IDS_H__ */
