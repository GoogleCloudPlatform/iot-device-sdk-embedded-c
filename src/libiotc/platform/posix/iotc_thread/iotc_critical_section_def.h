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

#ifndef __IOTC_POSIX_CRITICAL_SECTION_H__
#define __IOTC_POSIX_CRITICAL_SECTION_H__

#include "iotc_critical_section.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @struct iotc_critical_section_s
 * @brief  Holds the state of the critical section.
 *
 * This implementation is for spin locks so it require
 * the compiler to respect the volatile keyword.
 */
#ifdef IOTC_MODULE_THREAD_ENABLED
typedef struct iotc_critical_section_s {
  volatile int cs_state;
} iotc_critical_section_t;
#endif

#ifdef __cplusplus
}
#endif

#endif /* __IOTC_POSIX_CRITICAL_SECTION_H__ */
