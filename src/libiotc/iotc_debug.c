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

#include <string.h>

#include "iotc_allocator.h"
#include "iotc_debug.h"

#if IOTC_DEBUG_OUTPUT
void iotc_debug_data_logger_impl(const char* msg,
                                 const iotc_data_desc_t* data_desc) {
  char* tmp = iotc_alloc(1024);
  memset(tmp, 0, 1024);

  size_t counter = 0;

  unsigned short i = 0;
  for (; i < data_desc->length; ++i) {
    char c = data_desc->data_ptr[i];

    if (c < 33 || c > 126) {
      counter += sprintf(tmp + counter, "0x%02x", c);
    } else {
      counter += sprintf(tmp + counter, "\'%c\'", c);
    }

    counter += sprintf(tmp + counter, " ");
  }

  iotc_debug_printf("%s = [%s]\n", msg, tmp);

  iotc_free(tmp);
}

/** Trims the absolute file path to just the file name */
const char* iotc_debug_dont_print_the_path(const char* msg) {
  const char* tmp = msg;

  /* let's find the end */
  while (*(tmp) != '\0') {
    ++tmp;
  }

  /* let's crawl back to the beginning */
  while (*tmp != '/' && *tmp != '\\' && tmp != msg) {
    --tmp;
  }

  if (*tmp != '\0') {
    ++tmp;
  }

  return tmp;
}
#endif
