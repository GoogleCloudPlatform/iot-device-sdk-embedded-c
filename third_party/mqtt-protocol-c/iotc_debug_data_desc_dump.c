/* 2018-2019 Google LLC
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

#include "iotc_debug_data_desc_dump.h"
#include "iotc_debug.h"

#if IOTC_DEBUG_OUTPUT
void iotc_debug_data_desc_dump(const iotc_data_desc_t* buffer) {
  if (buffer == NULL) {
    iotc_debug_printf("NULL");
    return;
  }

  iotc_debug_printf("[%" SCNu32 "] ", buffer->length);

  char hex = 0;
  size_t i = 0;
  for (; i < buffer->length; ++i) {
    if ((uint8_t)buffer->data_ptr[i] < 0x20 ||
        (uint8_t)buffer->data_ptr[i] > 0x7e) {
      hex = 1;
      break;
    }
  }

  if (hex) {
    iotc_debug_data_desc_dump_hex(buffer);
  } else {
    iotc_debug_data_desc_dump_ascii(buffer);
  }
}

void iotc_debug_data_desc_dump_ascii(const iotc_data_desc_t* buffer) {
  if (buffer == NULL) {
    iotc_debug_printf("NULL");
    return;
  }

  size_t i = 0;
  for (; i < buffer->length; ++i) {
    iotc_debug_printf("%c", buffer->data_ptr[i]);
  }
}

void iotc_debug_data_desc_dump_hex(const iotc_data_desc_t* buffer) {
  if (buffer == NULL) {
    iotc_debug_printf("NULL");
    return;
  }

  size_t i = 0;
  for (; i < buffer->length; ++i) {
    iotc_debug_printf("%02x ", buffer->data_ptr[i]);
  }
}
#endif