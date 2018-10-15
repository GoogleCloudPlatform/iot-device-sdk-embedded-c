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

#ifndef __IOTC_MEMORY_CHECKS_H__
#define __IOTC_MEMORY_CHECKS_H__

#ifdef IOTC_MEMORY_LIMITER_ENABLED
#include <stdint.h>
#include "iotc_memory_limiter.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifdef IOTC_MEMORY_LIMITER_ENABLED
// clang-format off
#define iotc_is_whole_memory_deallocated() ( iotc_memory_limiter_get_allocated_space() == 0 )
// clang-format on

/* real implementation of the tearup and teardown for tests */
void _iotc_memory_limiter_tearup();
uint8_t _iotc_memory_limiter_teardown();

/* this is the macro for  */
#define iotc_memory_limiter_tearup _iotc_memory_limiter_tearup
#define iotc_memory_limiter_teardown _iotc_memory_limiter_teardown

#else
#define iotc_is_whole_memory_deallocated() 1
#define iotc_memory_limiter_tearup()
#define iotc_memory_limiter_teardown() 1
#endif

#ifdef __cplusplus
}
#endif

#endif /* __IOTC_MEMORY_CHECKS_H__ */
