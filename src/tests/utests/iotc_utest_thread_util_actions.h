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

#ifndef __IOTC_UTEST_THREAD_UTIL_ACTIONS_H__
#define __IOTC_UTEST_THREAD_UTIL_ACTIONS_H__

#include <iotc_err.h>
#include <iotc_event_dispatcher_api.h>
#include <pthread.h>
#include "iotc_critical_section_def.h"

struct iotc_critical_section_s* iotc_uteset_local_action_store_cs = NULL;

iotc_state_t iotc_utest_local_action_store_tid(
    iotc_event_handle_arg1_t function_executed_communication_channel) {
  if (function_executed_communication_channel != NULL) {
    uint32_t* communication_channel =
        (uint32_t*)function_executed_communication_channel;

    *communication_channel = (uint32_t)pthread_self();
  }

  return IOTC_STATE_OK;
}

iotc_state_t iotc_utest_local_action_increase_by_one(
    iotc_event_handle_arg1_t function_executed_communication_channel) {
  if (function_executed_communication_channel != NULL) {
    uint32_t* communication_channel =
        (uint32_t*)function_executed_communication_channel;

    iotc_lock_critical_section(iotc_uteset_local_action_store_cs);

    *communication_channel = *communication_channel + 1;

    iotc_unlock_critical_section(iotc_uteset_local_action_store_cs);
  }

  return IOTC_STATE_OK;
}

iotc_state_t iotc_utest_local_action_decrease_by_11(
    iotc_event_handle_arg1_t function_executed_communication_channel) {
  if (function_executed_communication_channel != NULL) {
    uint32_t* communication_channel =
        (uint32_t*)function_executed_communication_channel;

    iotc_lock_critical_section(iotc_uteset_local_action_store_cs);

    *communication_channel = *communication_channel - 11;

    iotc_unlock_critical_section(iotc_uteset_local_action_store_cs);
  }

  return IOTC_STATE_OK;
}

#endif /* __IOTC_UTEST_THREAD_UTIL_ACTIONS_H__ */
