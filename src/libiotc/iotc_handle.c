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

#include "iotc_handle.h"
#include "iotc_types.h"

/* -----------------------------------------------------------------------
 *  INTERNAL FUNCTIONS
 * ----------------------------------------------------------------------- */

static inline int8_t iotc_compare_context_pointers(
    const union iotc_vector_selector_u* e0,
    const union iotc_vector_selector_u* e1) {
  return e0->ptr_value == e1->ptr_value ? 0 : 1;
}

/* -----------------------------------------------------------------------
 *  MAIN LIBRARY FUNCTIONS
 * ----------------------------------------------------------------------- */

void* iotc_object_for_handle(iotc_vector_t* vector, iotc_handle_t handle) {
  assert(vector != NULL);
  return iotc_vector_get(vector, handle);
}

iotc_state_t iotc_find_handle_for_object(iotc_vector_t* vector,
                                         const void* object,
                                         iotc_handle_t* handle) {
  assert(vector != NULL);
  iotc_vector_index_type_t handler_index = iotc_vector_find(
      vector, IOTC_VEC_CONST_VALUE_PARAM(IOTC_VEC_VALUE_PTR((void*)object)),
      iotc_compare_context_pointers);
  if (handler_index < 0) {
    *handle = IOTC_INVALID_CONTEXT_HANDLE;
    return IOTC_ELEMENT_NOT_FOUND;
  }
  *handle = handler_index;
  return IOTC_STATE_OK;
}

iotc_state_t iotc_delete_handle_for_object(iotc_vector_t* vector,
                                           const void* object) {
  assert(vector != NULL);
  iotc_vector_index_type_t handler_index = iotc_vector_find(
      vector, IOTC_VEC_CONST_VALUE_PARAM(IOTC_VEC_VALUE_PTR((void*)object)),
      iotc_compare_context_pointers);

  if (handler_index < 0) {
    return IOTC_ELEMENT_NOT_FOUND;
  }
  vector->array[handler_index].selector_t.ptr_value = NULL;
  return IOTC_STATE_OK;
}

iotc_state_t iotc_register_handle_for_object(iotc_vector_t* vector,
                                             const int32_t max_object_cnt,
                                             const void* object) {
  assert(vector != NULL);
  iotc_vector_index_type_t handler_index = iotc_vector_find(
      vector, IOTC_VEC_CONST_VALUE_PARAM(IOTC_VEC_VALUE_PTR(NULL)),
      iotc_compare_context_pointers);
  if (handler_index < 0) {
    if (vector->elem_no >= max_object_cnt) {
      return IOTC_NO_MORE_RESOURCE_AVAILABLE;
    }
    iotc_vector_push(
        vector, IOTC_VEC_CONST_VALUE_PARAM(IOTC_VEC_VALUE_PTR((void*)object)));
  } else {
    vector->array[handler_index].selector_t.ptr_value = (void*)object;
  }
  return IOTC_STATE_OK;
}
