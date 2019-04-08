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

#ifndef __IOTC_HELPERS_H__
#define __IOTC_HELPERS_H__

#include <limits.h>
#include <stdlib.h>

#include "iotc_common.h"
#include "iotc_mqtt_message.h"

#ifdef __cplusplus
extern "C" {
#endif

#define IOTC_MAX8_t (0xFF)
#define IOTC_MAX16_t (0xFFFF)
#define IOTC_MAX32_t (0xFFFFFFFF)

/* Our Message Payloads could be either character arrays or binary arrays.
 * This function will ready the payload as a character array and return
 * a null-terminated copy of that data.  You must free this data when you're
 * done with it.
 *
 * Returns NULL if msg is NULL or string allocation failed. */
char* iotc_parse_message_payload_as_string(const iotc_mqtt_message_t* msg);

/* Avoid using `strdup()` which can cause some problems with `free()`,
 * because of buggy implementations of `realloc()`. */
char* iotc_str_dup(const char* s);

char* iotc_str_cat(const char* s1, const char* s2);

int iotc_str_copy_untiln(char* dst, size_t dst_size, const char* src,
                         char delim);

uint32_t iotc_highest_bit_filter(uint32_t value);

#ifdef __cplusplus
}
#endif

#endif /* __IOTC_HELPERS_H__ */
