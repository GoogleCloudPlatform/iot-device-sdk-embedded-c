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

#include "iotc_vector.h"

#define IOTC_VECTOR_DEFAULT_CAPACITY 2

/**
 * \brief reserve new block of memory, copy data and release the one previously
 * used \return 1 in case of success 0 otherwise
 */
static int8_t iotc_vector_realloc(iotc_vector_t* vector,
                                  iotc_vector_index_type_t new_size) {
  assert(new_size != 0 && new_size != vector->capacity);

  /* do not allow to reallocate unmanaged memory blocks */
  if (IOTC_MEMORY_TYPE_MANAGED != vector->memory_type) {
    return 0;
  }

  iotc_state_t state = IOTC_STATE_OK;
  size_t elems_size = new_size * sizeof(iotc_vector_elem_t);

  IOTC_ALLOC_BUFFER(iotc_vector_elem_t, new_array, elems_size, state);

  if (vector->elem_no > 0) {
    memcpy(new_array, vector->array,
           sizeof(iotc_vector_elem_t) * vector->elem_no);
  }

  IOTC_SAFE_FREE(vector->array);

  vector->array = new_array;
  vector->capacity = new_size;

  return 1;

err_handling:
  return 0;
}

iotc_vector_t* iotc_vector_create() {
  iotc_state_t state = IOTC_STATE_OK;

  IOTC_ALLOC(iotc_vector_t, ret, state);

  ret->memory_type = IOTC_MEMORY_TYPE_MANAGED;

  IOTC_CHECK_MEMORY(iotc_vector_realloc(ret, IOTC_VECTOR_DEFAULT_CAPACITY),
                    state);

  return ret;

err_handling:
  if (ret) {
    IOTC_SAFE_FREE(ret->array);
  }
  IOTC_SAFE_FREE(ret);
  return NULL;
}

iotc_vector_t* iotc_vector_create_from(iotc_vector_elem_t* array, size_t len,
                                       iotc_memory_type_t memory_type) {
  assert(array != 0);
  assert(len > 0);

  iotc_state_t state = IOTC_STATE_OK;

  IOTC_ALLOC(iotc_vector_t, ret, state);

  ret->array = array;
  ret->memory_type = memory_type;
  ret->capacity = len;
  ret->elem_no = len;

  return ret;

err_handling:
  if (ret) {
    iotc_vector_destroy(ret);
  }
  return NULL;
}

int8_t iotc_vector_reserve(iotc_vector_t* vector, iotc_vector_index_type_t n) {
  assert(NULL != vector);
  assert(n > 0);

  /* don't reserve if capacity is already set */
  if (n == vector->capacity) {
    return 1;
  }

  /* trim the number of elements to the new size */
  if (vector->elem_no > n) {
    vector->elem_no = n;
  }

  int8_t result = iotc_vector_realloc(vector, n);

  if (result == 0) {
    return 0;
  }

  return 1;
}

iotc_vector_t* iotc_vector_destroy(iotc_vector_t* vector) {
  /* PRECONDITION */
  assert(NULL != vector);
  assert(NULL != vector->array);

  /* only managed type memory can be released by the vector who owns it */
  if (IOTC_MEMORY_TYPE_MANAGED == vector->memory_type) {
    IOTC_SAFE_FREE(vector->array);
  }

  IOTC_SAFE_FREE(vector);

  return NULL;
}

const iotc_vector_elem_t* iotc_vector_push(
    iotc_vector_t* vector, const union iotc_vector_selector_u value) {
  /* PRECONDITION */
  assert(NULL != vector);

  iotc_state_t state = IOTC_STATE_OK;

  if (vector->elem_no + 1 > vector->capacity) {
    IOTC_CHECK_MEMORY(iotc_vector_realloc(vector, vector->capacity * 2), state);
  }

  vector->array[vector->elem_no].selector_t = value;
  vector->elem_no += 1;

  return &vector->array[vector->elem_no - 1];

err_handling:
  return NULL;
}

void iotc_vector_swap_elems(iotc_vector_t* vector, iotc_vector_index_type_t i0,
                            iotc_vector_index_type_t i1) {
  /* PRECONDITIONS */
  assert(NULL != vector);
  assert(i0 >= 0);
  assert(i1 >= 0);
  assert(i0 <= vector->elem_no - 1);
  assert(i1 <= vector->elem_no - 1);

  union iotc_vector_selector_u tmp_value = vector->array[i0].selector_t;
  vector->array[i0].selector_t = vector->array[i1].selector_t;
  vector->array[i1].selector_t = tmp_value;
}

void iotc_vector_del(iotc_vector_t* vector, iotc_vector_index_type_t index) {
  /* PRECONDITIONS */
  assert(NULL != vector);
  assert(index >= 0);
  assert(vector->elem_no > 0 && index < vector->elem_no);

  if (vector->elem_no > 0 && index < vector->elem_no) {
    if (index != vector->elem_no - 1) {
      iotc_vector_swap_elems(vector, index, vector->elem_no - 1);
    }

    vector->elem_no -= 1;
    memset(&vector->array[vector->elem_no].selector_t, 0,
           sizeof(vector->array[vector->elem_no].selector_t));
  }
}

iotc_vector_index_type_t iotc_vector_find(
    iotc_vector_t* vector, const union iotc_vector_selector_u value,
    iotc_vector_cmp_t* fun_cmp) {
  /* PRECONDITIONS */
  assert(NULL != vector);

  iotc_vector_index_type_t i = 0;

  for (i = 0; i < vector->elem_no; ++i) {
    if ((*fun_cmp)(&vector->array[i].selector_t, &value) == 0) {
      return i;
    }
  }

  return -1;
}

void iotc_vector_for_each(iotc_vector_t* vector, iotc_vector_for_t* fun_for,
                          void* arg, iotc_vector_index_type_t offset) {
  assert(NULL != vector);
  assert(offset >= 0);
  assert(offset <= vector->elem_no);

  iotc_vector_index_type_t i = offset;

  for (; i < vector->elem_no; ++i) {
    (*fun_for)(&vector->array[i].selector_t, arg);
  }
}

void* iotc_vector_get(iotc_vector_t* vector, iotc_vector_index_type_t index) {
  if (vector->elem_no <= index || index < 0) {
    return NULL;
  }
  return vector->array[index].selector_t.ptr_value;
}

void iotc_vector_remove_if(iotc_vector_t* vector,
                           iotc_vector_pred_t* fun_pred) {
  /* PRECONDITIONS */
  assert(NULL != vector);

  iotc_vector_index_type_t i = 0;
  iotc_vector_index_type_t i_end = vector->elem_no;

  for (i = i_end - 1; i >= 0; --i) {
    if ((*fun_pred)(&vector->array[i].selector_t) == 1) {
      iotc_vector_del(vector, i);
    }
  }
}
