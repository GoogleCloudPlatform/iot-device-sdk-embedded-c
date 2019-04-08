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

#ifndef __IOTC_DATA_DESC_H__
#define __IOTC_DATA_DESC_H__

#include <inttypes.h>
#include <stdint.h>
#include <stdlib.h>

#include "iotc_memory_type.h"

#include <iotc_error.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct data_desc_s {
  uint8_t* data_ptr;
  struct data_desc_s* __next;
  uint32_t capacity;
  uint32_t length;
  uint32_t curr_pos;
  iotc_memory_type_t memory_type;
} iotc_data_desc_t;

typedef uint32_t(iotc_data_desc_realloc_strategy_t)(uint32_t, uint32_t);

extern iotc_data_desc_t* iotc_make_empty_desc_alloc(size_t capacity);

extern iotc_data_desc_t* iotc_make_desc_from_buffer_copy(
    unsigned const char* buffer, size_t len);

extern iotc_data_desc_t* iotc_make_desc_from_buffer_share(unsigned char* buffer,
                                                          size_t len);

extern iotc_data_desc_t* iotc_make_desc_from_string_copy(const char* str);

extern iotc_data_desc_t* iotc_make_desc_from_string_share(const char* str);

extern void iotc_free_desc(iotc_data_desc_t** desc);

extern uint8_t iotc_data_desc_will_it_fit(const iotc_data_desc_t* const,
                                          size_t len);

uint32_t iotc_data_desc_pow2_realloc_strategy(uint32_t original,
                                              uint32_t desired);

extern iotc_state_t iotc_data_desc_realloc(
    iotc_data_desc_t* desc, uint32_t new_capacity,
    iotc_data_desc_realloc_strategy_t* strategy);

extern iotc_state_t iotc_data_desc_assure_buf_len(iotc_data_desc_t* out,
                                                  size_t len);

extern iotc_state_t iotc_data_desc_append_data_resize(iotc_data_desc_t* out,
                                                      const char* const data,
                                                      const size_t len);

extern iotc_state_t iotc_data_desc_append_bytes(iotc_data_desc_t* out,
                                                const uint8_t* bytes,
                                                const size_t len);

extern iotc_state_t iotc_data_desc_append_byte(iotc_data_desc_t* out,
                                               const uint8_t byte);

extern iotc_state_t iotc_data_desc_append_data(iotc_data_desc_t* out,
                                               const iotc_data_desc_t* in);

#ifdef __cplusplus
}
#endif

#endif /* __IOTC_DATA_DESC_H__ */
