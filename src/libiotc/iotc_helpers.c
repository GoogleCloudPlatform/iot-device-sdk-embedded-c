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

#include <string.h>

#include "iotc_allocator.h"
#include "iotc_debug.h"
#include "iotc_helpers.h"
#include "iotc_macros.h"

char* iotc_parse_message_payload_as_string(const iotc_mqtt_message_t* msg) {
  if (NULL == msg) {
    iotc_debug_logger("message null returning");
    return NULL;
  }

  char* payload = (char*)iotc_alloc(msg->publish.content->length + 1);

  if (NULL == payload) {
    iotc_debug_logger("allocated string null, returning");
    return NULL;
  }

  memcpy(payload, msg->publish.content->data_ptr, msg->publish.content->length);

  payload[msg->publish.content->length] = '\0';

  return payload;
}

char* iotc_str_dup(const char* s) {
  /* PRECONDITIONS */
  assert(s != 0);

  const size_t len = strlen(s);
  char* ret = iotc_alloc(len + 1);
  if (ret == 0) {
    return 0;
  }
  memcpy(ret, s, len + 1);
  return ret;
}

char* iotc_str_cat(const char* s1, const char* s2) {
  assert(0 != s1);
  assert(0 != s2);

  size_t len1 = strlen(s1);
  size_t len2 = strlen(s2);
  char* ret = iotc_alloc(len1 + len2 + 1);
  if (ret == 0) {
    return 0;
  }

  memcpy(ret, s1, len1);
  memcpy(ret + len1, s2, len2 + 1);

  return ret;
}

int iotc_str_copy_untiln(char* dst, size_t dst_size, const char* src,
                         char delim) {
  /* PRECONDITIONS */
  assert(dst != 0);
  assert(dst_size > 1);
  assert(src != 0);

  size_t counter = 0;
  size_t real_size = dst_size - 1;

  while (*src != delim && counter < real_size && *src != '\0') {
    *dst++ = *src++;
    counter++;
  }

  *dst = '\0';
  return counter;
}

/**
 * @brief leave_heighest_bit
 *
 * function leavs the heighest bit set to 1
 *
 * @param value
 * @return value with the heighest bit set
 */
uint32_t iotc_highest_bit_filter(uint32_t value) {
  value |= (value >> 1);
  value |= (value >> 2);
  value |= (value >> 4);
  value |= (value >> 8);
  value |= (value >> 16);

  return value & ~(value >> 1);
}
