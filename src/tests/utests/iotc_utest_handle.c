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

#include "iotc_memory_checks.h"
#include "iotc_tt_testcase_management.h"
#include "tinytest.h"
#include "tinytest_macros.h"

#include "iotc.h"
#include "iotc_handle.h"
#include "iotc_types_internal.h"
#include "iotc_vector.h"

#ifndef IOTC_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#endif

IOTC_TT_TESTGROUP_BEGIN(utest_handle)

IOTC_TT_TESTCASE(utest__iotc_object_for_handle__object_found, {
  iotc_vector_t* vector = iotc_vector_create();

  void* object = (void*)(intptr_t)333;
  iotc_register_handle_for_object(vector, 5, object);
  iotc_handle_t handle;
  iotc_find_handle_for_object(vector, object, &handle);

  void* object2 = iotc_object_for_handle(vector, handle);

  tt_want_ptr_op(object, ==, object2);

  iotc_vector_destroy(vector);

  tt_int_op(iotc_is_whole_memory_deallocated(), >, 0);
end:;
})

IOTC_TT_TESTCASE(utest__iotc_object_for_handle__object_not_found, {
  iotc_vector_t* vector = iotc_vector_create();

  void* object = (void*)(intptr_t)333;
  iotc_register_handle_for_object(vector, 5, object);
  iotc_handle_t handle;
  iotc_find_handle_for_object(vector, object, &handle);
  handle += 1;

  iotc_context_t* object2 = iotc_object_for_handle(vector, handle);

  tt_want_ptr_op(object, !=, object2);
  tt_want_ptr_op(object2, ==, NULL);

  iotc_vector_destroy(vector);

  tt_int_op(iotc_is_whole_memory_deallocated(), >, 0);
end:;
})

IOTC_TT_TESTCASE(utest__iotc_find_handle_for_object__handle_found, {
  iotc_vector_t* vector = iotc_vector_create();

  void* object = (void*)(intptr_t)333;
  iotc_register_handle_for_object(vector, 5, object);
  iotc_handle_t handle = IOTC_INVALID_CONTEXT_HANDLE;
  iotc_state_t state = iotc_find_handle_for_object(vector, object, &handle);

  tt_want_int_op(state, ==, IOTC_STATE_OK);
  tt_want_int_op(handle, >, IOTC_INVALID_CONTEXT_HANDLE);

  iotc_vector_destroy(vector);

  tt_int_op(iotc_is_whole_memory_deallocated(), >, 0);
end:;
})

IOTC_TT_TESTCASE(utest__iotc_find_handle_for_object__handle_not_found, {
  iotc_vector_t* vector = iotc_vector_create();

  void* object = (void*)(intptr_t)333;
  iotc_register_handle_for_object(vector, 5, object);
  iotc_context_handle_t handle = IOTC_INVALID_CONTEXT_HANDLE;
  object = (intptr_t*)object + 1;  // cast required to keep IAR happy
  iotc_state_t state = iotc_find_handle_for_object(vector, object, &handle);

  tt_want_int_op(state, ==, IOTC_ELEMENT_NOT_FOUND);
  tt_want_int_op(handle, ==, IOTC_INVALID_CONTEXT_HANDLE);

  iotc_vector_destroy(vector);

  tt_int_op(iotc_is_whole_memory_deallocated(), >, 0);
end:;
})

IOTC_TT_TESTCASE(
    utest__iotc_delete_handle_for_object__delete_success_handle_not_found, {
      iotc_vector_t* vector = iotc_vector_create();

      void* object = (void*)(intptr_t)333;
      iotc_register_handle_for_object(vector, 5, object);
      iotc_state_t state = IOTC_STATE_OK;
      iotc_context_handle_t handle = IOTC_INVALID_CONTEXT_HANDLE;

      state = iotc_find_handle_for_object(vector, object, &handle);
      tt_want_int_op(state, ==, IOTC_STATE_OK);
      tt_want_int_op(handle, >, IOTC_INVALID_CONTEXT_HANDLE);

      iotc_state_t delete_state = iotc_delete_handle_for_object(vector, object);
      tt_want_int_op(delete_state, ==, IOTC_STATE_OK);

      state = iotc_find_handle_for_object(vector, object, &handle);
      tt_want_int_op(state, ==, IOTC_ELEMENT_NOT_FOUND);
      tt_want_int_op(handle, ==, IOTC_INVALID_CONTEXT_HANDLE);

      iotc_vector_destroy(vector);

      tt_int_op(iotc_is_whole_memory_deallocated(), >, 0);
    end:;
    })

IOTC_TT_TESTCASE(utest__iotc_delete_handle_for_object__unsuccessful_delete, {
  iotc_vector_t* vector = iotc_vector_create();

  void* unregistered_object = (void*)(intptr_t)444;
  iotc_state_t delete_state =
      iotc_delete_handle_for_object(vector, unregistered_object);

  tt_want_int_op(delete_state, ==, IOTC_ELEMENT_NOT_FOUND);

  iotc_vector_destroy(vector);

  tt_int_op(iotc_is_whole_memory_deallocated(), >, 0);
end:;
})

IOTC_TT_TESTCASE(
    utest__iotc_register_handle_for_object__successful_registration, {
      iotc_vector_t* vector = iotc_vector_create();

      int32_t max_num_contexts = 10;
      int i;
      for (i = 0; i < max_num_contexts; ++i) {
        void* object = (void*)(intptr_t)222;
        iotc_state_t state =
            iotc_register_handle_for_object(vector, max_num_contexts, object);

        tt_want_int_op(state, ==, IOTC_STATE_OK);
      }

      iotc_vector_destroy(vector);

      tt_int_op(iotc_is_whole_memory_deallocated(), >, 0);
    end:;
    })

IOTC_TT_TESTCASE(
    utest__iotc_register_handle_for_object__unsuccessful_registration, {
      iotc_vector_t* vector = iotc_vector_create();

      int32_t max_num_contexts = 10;
      int i;
      for (i = 0; i < max_num_contexts; ++i) {
        void* object = (void*)(intptr_t)222;
        iotc_register_handle_for_object(vector, max_num_contexts, object);
      }

      void* object = (void*)(intptr_t)333;
      iotc_state_t state =
          iotc_register_handle_for_object(vector, max_num_contexts, object);
      tt_want_int_op(state, ==, IOTC_NO_MORE_RESOURCE_AVAILABLE);

      iotc_vector_destroy(vector);

      tt_int_op(iotc_is_whole_memory_deallocated(), >, 0);
    end:;
    })

IOTC_TT_TESTGROUP_END

#ifndef IOTC_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#define IOTC_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#include __FILE__
#undef IOTC_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#endif
