/* Copyright 2018 - 2019 Google LLC
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

/* - Every function should return a value
 * - There are special values (usually `0` or `-1`) which indicate occurrence of
 *   an error
 * - User can detect and lookup errors using declarations below */

#ifndef __IOTC_ERR_H__
#define __IOTC_ERR_H__

#include <iotc_error.h>

#ifdef __cplusplus
extern "C" {
#endif

#define IOTC_OPT_NO_ERROR_STRINGS 1

#ifndef IOTC_OPT_NO_ERROR_STRINGS
extern const char* iotc_err_string[IOTC_ERROR_COUNT];
#endif

extern const char* iotc_get_state_string(iotc_state_t e);

#ifdef __cplusplus
}
#endif

#endif /* __IOTC_ERR_H__ */
