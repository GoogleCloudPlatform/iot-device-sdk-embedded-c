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

#include "iotc_allocator.h"
#include "iotc_critical_section_def.h"
#include "iotc_debug.h"
#include "iotc_macros.h"

#include <iotc_error.h>

#ifdef __cplusplus
extern "C" {
#endif

iotc_state_t iotc_init_critical_section(struct iotc_critical_section_s** cs) {
  assert(cs != NULL);

  iotc_state_t ret_state = IOTC_STATE_OK;

  IOTC_ALLOC_AT(struct iotc_critical_section_s, *cs, ret_state);

err_handling:
  return ret_state;
}

void iotc_lock_critical_section(struct iotc_critical_section_s* cs) {
  assert(cs != 0);

  while (!__sync_bool_compare_and_swap(&cs->cs_state, 0, 1))
    ;
}

void iotc_unlock_critical_section(struct iotc_critical_section_s* cs) {
  assert(cs != 0);

  while (!__sync_bool_compare_and_swap(&cs->cs_state, 1, 0))
    ;
}

void iotc_destroy_critical_section(struct iotc_critical_section_s** cs) {
  assert(cs != 0);

  IOTC_SAFE_FREE(*cs);
}

#ifdef __cplusplus
}
#endif
