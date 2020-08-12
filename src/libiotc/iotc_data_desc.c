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

#include "iotc_data_desc.h"
#include "iotc_allocator.h"
#include "iotc_helpers.h"
#include "iotc_macros.h"

iotc_data_desc_t* iotc_make_empty_desc_alloc(size_t capacity) {
  assert(capacity > 0);

  iotc_state_t state = IOTC_STATE_OK;

  IOTC_ALLOC(iotc_data_desc_t, data_desc, state);

  data_desc->memory_type = IOTC_MEMORY_TYPE_MANAGED;

  IOTC_ALLOC_BUFFER_AT(unsigned char, data_desc->data_ptr, capacity, state);
  data_desc->length = 0;
  data_desc->capacity = capacity;

  return data_desc;

err_handling:
  iotc_free_desc(&data_desc);
  return NULL;
}

iotc_data_desc_t* iotc_make_desc_from_buffer_copy(unsigned const char* buffer,
                                                  size_t len) {
  assert(buffer != 0);

  iotc_state_t state = IOTC_STATE_OK;

  IOTC_ALLOC(iotc_data_desc_t, data_desc, state);
  data_desc->memory_type = IOTC_MEMORY_TYPE_MANAGED;

  IOTC_ALLOC_BUFFER_AT(unsigned char, data_desc->data_ptr, len, state);
  memcpy(data_desc->data_ptr, buffer, len);

  data_desc->capacity = len;
  data_desc->length = data_desc->capacity;

  return data_desc;

err_handling:
  iotc_free_desc(&data_desc);
  return NULL;
}

iotc_data_desc_t* iotc_make_desc_from_buffer_share(unsigned char* buffer,
                                                   size_t len) {
  assert(buffer != 0);

  iotc_state_t state = IOTC_STATE_OK;

  IOTC_ALLOC(iotc_data_desc_t, data_desc, state);

  data_desc->capacity = len;
  data_desc->length = data_desc->capacity;
  data_desc->data_ptr = buffer;
  data_desc->memory_type = IOTC_MEMORY_TYPE_UNMANAGED;

  return data_desc;

err_handling:
  iotc_free_desc(&data_desc);
  return NULL;
}

iotc_data_desc_t* iotc_make_desc_from_string_copy(const char* str) {
  if (NULL == str) {
    return NULL;
  }

  iotc_state_t state = IOTC_STATE_OK;
  const size_t len = strlen(str);

  IOTC_ALLOC(iotc_data_desc_t, data_desc, state);
  data_desc->memory_type = IOTC_MEMORY_TYPE_MANAGED;

  IOTC_ALLOC_BUFFER_AT(unsigned char, data_desc->data_ptr, len + 1, state);
  memcpy(data_desc->data_ptr, str, len + 1);

  data_desc->capacity = len + 1;
  data_desc->length = len;

  return data_desc;

err_handling:
  iotc_free_desc(&data_desc);
  return NULL;
}

iotc_data_desc_t* iotc_make_desc_from_string_share(const char* str) {
  if (NULL == str) {
    return NULL;
  }

  iotc_state_t state = IOTC_STATE_OK;

  const size_t len = strlen(str);

  IOTC_ALLOC(iotc_data_desc_t, data_desc, state);

  data_desc->data_ptr = (uint8_t*)str;
  data_desc->capacity = len;
  data_desc->length = data_desc->capacity;
  data_desc->memory_type = IOTC_MEMORY_TYPE_UNMANAGED;

  return data_desc;

err_handling:
  iotc_free_desc(&data_desc);
  return NULL;
}

void iotc_free_desc(iotc_data_desc_t** desc) {
  if (desc != NULL && *desc != NULL) {
    /* PRE-CONDITION */
    assert((*desc)->memory_type != IOTC_MEMORY_TYPE_UNKNOWN);

    if (IOTC_MEMORY_TYPE_MANAGED == (*desc)->memory_type) {
      IOTC_SAFE_FREE((*desc)->data_ptr);
    }

    IOTC_SAFE_FREE((*desc));
  }
}

uint8_t iotc_data_desc_will_it_fit(const iotc_data_desc_t* const desc,
                                   size_t len) {
  assert(desc);

  if (desc->capacity < (desc->length + len)) {
    return 0;
  }

  return 1;
}

/**
 * @brief iotc_data_desc_find_pow2_mult
 *
 * Little explanation for this function:
 * what it does it calculates the new capacity for original value so that it
 * becomes new required memory limit - which is calculated from the desire
 * parameter.
 *
 * @param original size of already allocated memory
 * @param desire number of bytes that are required
 * @return multiplier for original value
 */
static uint32_t iotc_data_desc_find_pow2_capacity(uint32_t original,
                                                  uint32_t desire) {
  if (desire <= original) return original;

  uint32_t tmp = iotc_highest_bit_filter(desire);
  if (tmp < desire) {
    tmp <<= 1;
  }

  return tmp;
}

/**
 * @brief iotc_data_desc_pow2_realloc_strategy
 *
 * this is some kind of equivalent of std::allocator which can be defined
 * to handle different allocation strategy like pooling.
 *
 * @param original
 * @param desired
 * @return
 */
uint32_t iotc_data_desc_pow2_realloc_strategy(uint32_t original,
                                              uint32_t desired) {
  uint32_t new_cap = iotc_data_desc_find_pow2_capacity(original, desired);

  return new_cap;
}

iotc_state_t iotc_data_desc_realloc(
    iotc_data_desc_t* desc, uint32_t new_capacity,
    iotc_data_desc_realloc_strategy_t* strategy) {
  /* PRECONDITION */
  assert(desc->memory_type != IOTC_MEMORY_TYPE_UNKNOWN);

  if (desc == NULL || strategy == NULL) {
    return IOTC_INVALID_PARAMETER;
  }

  iotc_state_t ret_state = IOTC_STATE_OK;

  unsigned char* old = desc->data_ptr;

  uint32_t calulated_capacity = (*strategy)(desc->capacity, new_capacity);

  IOTC_ALLOC_BUFFER(unsigned char, new_buf, calulated_capacity, ret_state);

  memcpy(new_buf, old, desc->capacity);

  desc->data_ptr = new_buf;
  desc->capacity = calulated_capacity;

  if (IOTC_MEMORY_TYPE_MANAGED == desc->memory_type) {
    IOTC_SAFE_FREE(old);
  }

  desc->memory_type = IOTC_MEMORY_TYPE_MANAGED;

err_handling:
  return ret_state;
}

iotc_state_t iotc_data_desc_assure_buf_len(iotc_data_desc_t* out, size_t len) {
  if (iotc_data_desc_will_it_fit(out, len) == 0) {
    return iotc_data_desc_realloc(out, out->length + len,
                                  &iotc_data_desc_pow2_realloc_strategy);
  }

  return IOTC_STATE_OK;
}

iotc_state_t iotc_data_desc_append_data_resize(iotc_data_desc_t* out,
                                               const char* const data,
                                               const size_t len) {
  if (out == NULL || data == NULL) {
    return IOTC_INVALID_PARAMETER;
  }

  iotc_state_t ret_state = IOTC_STATE_OK;

  IOTC_CHECK_STATE(ret_state = iotc_data_desc_assure_buf_len(out, len));

  memcpy(out->data_ptr + out->length, data, len);
  out->length += len;

err_handling:
  return ret_state;
}

iotc_state_t iotc_data_desc_append_bytes(iotc_data_desc_t* out,
                                         const uint8_t* bytes,
                                         const size_t len) {
  if (out == NULL || bytes == NULL) {
    return IOTC_INVALID_PARAMETER;
  }

  iotc_state_t local_state = IOTC_BUFFER_OVERFLOW;

  if (iotc_data_desc_will_it_fit(out, len) == 1) {
    memcpy(out->data_ptr + out->length, bytes, len);
    out->length += len;
    local_state = IOTC_STATE_OK;
  }

  return local_state;
}

iotc_state_t iotc_data_desc_append_byte(iotc_data_desc_t* out,
                                        const uint8_t byte) {
  if (out == NULL) {
    return IOTC_INVALID_PARAMETER;
  }

  iotc_state_t local_state = IOTC_BUFFER_OVERFLOW;

  if (iotc_data_desc_will_it_fit(out, 1) == 1) {
    out->data_ptr[out->length] = byte;
    out->length += 1;
    local_state = IOTC_STATE_OK;
  }

  return local_state;
}

extern iotc_state_t iotc_data_desc_append_data(iotc_data_desc_t* out,
                                               const iotc_data_desc_t* in) {
  if (out == NULL || in == NULL) {
    return IOTC_INVALID_PARAMETER;
  }

  return iotc_data_desc_append_bytes(out, in->data_ptr, in->length);
}
