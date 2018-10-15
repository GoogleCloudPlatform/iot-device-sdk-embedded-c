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

#ifndef __IOTC_VECTOR_H__
#define __IOTC_VECTOR_H__

#include <stdint.h>

#include "iotc_allocator.h"
#include "iotc_debug.h"
#include "iotc_macros.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ! This type has to be SIGNED ! */
typedef int8_t iotc_vector_index_type_t;

union iotc_vector_selector_u {
  void* ptr_value;
  intptr_t iptr_value;
  int32_t i32_value;
  uint32_t ui32_value;
};

typedef struct {
  union iotc_vector_selector_u selector_t;
} iotc_vector_elem_t;

typedef struct {
  iotc_vector_elem_t* array;
  iotc_vector_index_type_t elem_no;
  iotc_vector_index_type_t capacity;
  iotc_memory_type_t memory_type;
} iotc_vector_t;

typedef void(iotc_vector_for_t)(union iotc_vector_selector_u*, void* arg);

/* helpers for vector data initialization */
#define IOTC_VEC_VALUE_UI32(v) \
  { .ui32_value = v }
#define IOTC_VEC_VALUE_I32(v) \
  { .i32_value = v }
#define IOTC_VEC_VALUE_PTR(v) \
  { .ptr_value = v }
#define IOTC_VEC_VALUE_IPTR(v) \
  { .iptr_value = v }

#define IOTC_VEC_ELEM(v) \
  { v }
#define IOTC_VEC_CONST_VALUE_PARAM(v) (const union iotc_vector_selector_u) v
#define IOTC_VEC_VALUE_PARAM(v) (union iotc_vector_selector_u) v

/* declaration of the comparition function type
 * suppose to return -1 if e0 < e1, 0 if e0 == e1 and 1 if e0 > e1 */
typedef int8_t(iotc_vector_cmp_t)(const union iotc_vector_selector_u* e0,
                                  const union iotc_vector_selector_u* e1);

typedef int8_t(iotc_vector_pred_t)(union iotc_vector_selector_u* e0);

extern iotc_vector_t* iotc_vector_create();

/**
 * @brief iotc_vector_create_from
 *
 * In the vector implementation it is possible to create vector from the chunk
 * of already allocated memory.
 *
 * returns new vector created on given memory or NULL if there is not enough
 * memory to create the vector structure
 *
 * @param array
 * @param len
 * @param memory_type
 * @return iotc_vector_t*
 */
extern iotc_vector_t* iotc_vector_create_from(iotc_vector_elem_t* array,
                                              size_t len,
                                              iotc_memory_type_t memory_type);

/**
 * @brief iotc_vector_reserve
 *
 * Changes the capacity of the vector. If the capacity is lower than the
 * previous one elements at the end will be lost. Function returns 0 on error
 * and 1 on success.
 *
 * @param vector
 * @param n
 * @return int8_t
 */
extern int8_t iotc_vector_reserve(iotc_vector_t* vector,
                                  iotc_vector_index_type_t n);

extern iotc_vector_t* iotc_vector_destroy(iotc_vector_t* vector);

extern const iotc_vector_elem_t* iotc_vector_push(
    iotc_vector_t* vector, const union iotc_vector_selector_u value);

extern void iotc_vector_swap_elems(iotc_vector_t* vector,
                                   iotc_vector_index_type_t i0,
                                   iotc_vector_index_type_t i1);

extern void iotc_vector_del(iotc_vector_t* vector,
                            iotc_vector_index_type_t index);

extern void iotc_vector_remove_if(iotc_vector_t* vector,
                                  iotc_vector_pred_t* fun_pred);

extern iotc_vector_index_type_t iotc_vector_find(
    iotc_vector_t* vector, const union iotc_vector_selector_u value,
    iotc_vector_cmp_t* fun_cmp);

extern void iotc_vector_for_each(iotc_vector_t* vector,
                                 iotc_vector_for_t* fun_for, void* arg,
                                 iotc_vector_index_type_t offset);

extern void* iotc_vector_get(iotc_vector_t* vector,
                             iotc_vector_index_type_t index);

#ifdef __cplusplus
}
#endif

#endif /* __IOTC_VECTOR_H__ */
