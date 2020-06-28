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

#ifndef __IOTC_CRITICAL_SECTION_H__
#define __IOTC_CRITICAL_SECTION_H__

#include "iotc_err.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef IOTC_MODULE_THREAD_ENABLED

/* forward declaration of critical section */
struct iotc_critical_section_s;

/**
 * @brief iotc_init_critical_section Creates and initializes the
 * iotc_critical_section_t structure.
 *
 * @param cs Double pointer to the critical section object.
 * @return iotc_state_t
 */
extern iotc_state_t iotc_init_critical_section(
    struct iotc_critical_section_s** cs);

/**
 * @brief iotc_lock_critical_section Locks a critical section.
 * @param cs
 */
extern void iotc_lock_critical_section(struct iotc_critical_section_s* cs);

/**
 * @brief iotc_unlock_critical_section Unlocks a critical section.
 * @param cs
 */
extern void iotc_unlock_critical_section(struct iotc_critical_section_s* cs);

/**
 * @brief iotc_destroy_critical_section Destroys the critical section and frees
 * it's memory.
 * @param cs
 */
extern void iotc_destroy_critical_section(struct iotc_critical_section_s** cs);

#else

struct iotc_critical_section_s {
  char c;
};

#define iotc_init_critical_section(section) IOTC_STATE_OK
#define iotc_lock_critical_section(section)
#define iotc_unlock_critical_section(section)
#define iotc_destroy_critical_section(section)

#endif

#ifdef __cplusplus
}
#endif

#endif /* __IOTC_CRITICAL_SECTION_H__ */
