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

#include "iotc_tt_testcase_management.h"
#include "iotc_utest_basic_testcase_frame.h"
#include "tinytest.h"
#include "tinytest_macros.h"

#include "iotc_memory_checks.h"
#include "iotc_vector.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifndef IOTC_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN

int8_t vector_is_odd(union iotc_vector_selector_u* e0) {
  if ((e0->i32_value & 1) > 0) {
    return 1;
  }

  return 0;
}

int8_t utest_datastructures_cmp_vector_i32(
    const union iotc_vector_selector_u* e0,
    const union iotc_vector_selector_u* e1) {
  if (e0->i32_value < e1->i32_value) {
    return -1;
  } else if (e0->i32_value > e1->i32_value) {
    return 1;
  }

  return 0;
}

int8_t utest_datastructures_cmp_vector_ptr(
    const union iotc_vector_selector_u* e0,
    const union iotc_vector_selector_u* e1) {
  if (e0->ptr_value < e1->ptr_value) {
    return -1;
  } else if (e0->ptr_value > e1->ptr_value) {
    return 1;
  }

  return 0;
}

int8_t utest_datastructures_cmp_vector_ui32(
    const union iotc_vector_selector_u* e0,
    const union iotc_vector_selector_u* e1) {
  if (e0->ui32_value < e1->ui32_value) {
    return -1;
  } else if (e0->ui32_value > e1->ui32_value) {
    return 1;
  }

  return 0;
}
#endif

/*-----------------------------------------------------------------------*/
// HEAP TESTS
/*-----------------------------------------------------------------------*/
IOTC_TT_TESTGROUP_BEGIN(utest_datastructures)

IOTC_TT_TESTCASE(test_vector_create, {
  iotc_vector_t* sv = iotc_vector_create();

  tt_assert(sv != 0);
  tt_assert(sv->array != 0);
  tt_assert(sv->capacity == 2);
  tt_assert(sv->elem_no == 0);
  tt_assert(sv->memory_type == IOTC_MEMORY_TYPE_MANAGED);

  iotc_vector_destroy(sv);
  tt_want_int_op(iotc_is_whole_memory_deallocated(), >, 0);
end:;
})

IOTC_TT_TESTCASE(test_vector_reserve_upsize, {
  iotc_vector_t* sv = iotc_vector_create();

  iotc_vector_push(sv,
                   IOTC_VEC_CONST_VALUE_PARAM(IOTC_VEC_VALUE_PTR((void*)15)));

  int8_t result = iotc_vector_reserve(sv, 13);

  tt_assert(result == 1);
  tt_assert(sv->capacity == 13);
  tt_assert(sv->elem_no == 1);
  tt_assert(sv->array[0].selector_t.i32_value == 15);

  iotc_vector_destroy(sv);
  tt_want_int_op(iotc_is_whole_memory_deallocated(), >, 0);
end:;
})

IOTC_TT_TESTCASE(iotc_utest__vector_create_from__valid_data__return_vector, {
  iotc_vector_elem_t v_data[] = {
      IOTC_VEC_ELEM(IOTC_VEC_VALUE_UI32(1)),
      IOTC_VEC_ELEM(IOTC_VEC_VALUE_UI32(3)),
      IOTC_VEC_ELEM(IOTC_VEC_VALUE_UI32(0xFFFFFFFF))};
  iotc_vector_t* sv = iotc_vector_create_from((iotc_vector_elem_t*)v_data, 3,
                                              IOTC_MEMORY_TYPE_UNMANAGED);

  tt_assert(sv != 0);
  tt_assert(sv->array != 0);
  tt_assert(sv->capacity == 3);
  tt_assert(sv->elem_no == 3);
  tt_assert(IOTC_MEMORY_TYPE_UNMANAGED == sv->memory_type);

  size_t i = 0;
  for (; i < 3; ++i) {
    tt_want_uint_op(sv->array[i].selector_t.ui32_value, ==,
                    v_data[i].selector_t.ui32_value);
  }

  iotc_vector_destroy(sv);

  tt_want_int_op(iotc_is_whole_memory_deallocated(), >, 0);
end:;
})

IOTC_TT_TESTCASE(
    iotc_utest__iotc_vector_create_from__valid_dynamic_data__return_vector, {
      iotc_state_t local_state = IOTC_STATE_OK;

      const int data_size = 16;
      const int capacity = sizeof(iotc_vector_elem_t) * data_size;

      IOTC_ALLOC_BUFFER(iotc_vector_elem_t, v_data, capacity, local_state);

      int i = 0;
      for (; i < data_size; ++i) {
        v_data[i].selector_t.i32_value = i;
      }

      iotc_vector_t* sv = iotc_vector_create_from(
          (iotc_vector_elem_t*)v_data, data_size, IOTC_MEMORY_TYPE_MANAGED);

      tt_assert(sv != 0);
      tt_assert(sv->array != 0);
      tt_assert(sv->capacity == data_size);
      tt_assert(sv->elem_no == data_size);
      tt_assert(IOTC_MEMORY_TYPE_MANAGED == sv->memory_type);

      i = 0;
      for (; i < data_size; ++i) {
        tt_want_uint_op(sv->array[i].selector_t.ui32_value, ==,
                        v_data[i].selector_t.ui32_value);
      }

      iotc_vector_destroy(sv);
      tt_want_int_op(iotc_is_whole_memory_deallocated(), >, 0);
    err_handling:
    end:;
    })

IOTC_TT_TESTCASE(test_vector_reserve_downsize, {
  iotc_vector_t* sv = iotc_vector_create();

  const iotc_vector_index_type_t test_no_elements = 13;
  const iotc_vector_index_type_t test_no_elements_plus_5 = test_no_elements + 5;

  iotc_vector_index_type_t i = 0;
  for (; i < test_no_elements_plus_5; ++i) {
    tt_assert(NULL != iotc_vector_push(sv, IOTC_VEC_CONST_VALUE_PARAM(
                                               IOTC_VEC_VALUE_I32(15))));
  }

  int8_t result = iotc_vector_reserve(sv, test_no_elements);

  tt_assert(result == 1);
  tt_assert(sv->capacity == test_no_elements);
  tt_assert(sv->elem_no == test_no_elements);

  iotc_vector_destroy(sv);
  tt_want_int_op(iotc_is_whole_memory_deallocated(), >, 0);
end:;
})

IOTC_TT_TESTCASE(test_vector_push, {
  iotc_vector_t* sv = iotc_vector_create();

  tt_assert(sv != 0);
  tt_assert(sv->array != 0);

  iotc_vector_push(sv,
                   IOTC_VEC_CONST_VALUE_PARAM(IOTC_VEC_VALUE_PTR((void*)15)));

  tt_assert(sv->array[0].selector_t.ptr_value == (void*)15);
  tt_assert(sv->elem_no == 1);

end:;
  iotc_vector_destroy(sv);
  tt_want_int_op(iotc_is_whole_memory_deallocated(), >, 0);
})

IOTC_TT_TESTCASE(test_vector_push_all, {
  iotc_vector_t* sv = iotc_vector_create();

  tt_assert(sv != 0);
  tt_assert(sv->array != 0);

  int32_t i = 0;
  for (; i < 4; ++i) {
    const iotc_vector_elem_t* e =
        iotc_vector_push(sv, IOTC_VEC_CONST_VALUE_PARAM(IOTC_VEC_VALUE_I32(i)));
    tt_assert(&sv->array[i] == e);
  }

  tt_assert(sv->capacity == 4);

  for (i = 0; i < 4; ++i) {
    tt_assert(sv->array[i].selector_t.i32_value == i);
  }

  tt_assert(sv->elem_no == 4);

  tt_assert(0 != iotc_vector_push(sv, IOTC_VEC_CONST_VALUE_PARAM(
                                          IOTC_VEC_VALUE_PTR((void*)123))));

  tt_assert(sv->elem_no == 5);
end:;
  iotc_vector_destroy(sv);
  tt_want_int_op(iotc_is_whole_memory_deallocated(), >, 0);
})

IOTC_TT_TESTCASE(test_vector_del, {
  iotc_vector_t* sv = iotc_vector_create();

  int32_t i = 0;
  for (; i < 16; ++i) {
    iotc_vector_push(sv, IOTC_VEC_CONST_VALUE_PARAM(IOTC_VEC_VALUE_I32(i)));
  }

  iotc_vector_del(sv, 0);

  tt_assert(sv->array[0].selector_t.i32_value == 15);
  tt_assert(sv->array[1].selector_t.i32_value == 1);
  tt_assert(sv->array[2].selector_t.i32_value == 2);
  tt_assert(sv->elem_no == 15);

  iotc_vector_del(sv, 1);

  tt_assert(sv->array[0].selector_t.i32_value == 15);
  tt_assert(sv->array[1].selector_t.i32_value == 14);
  tt_assert(sv->elem_no == 14);

  tt_assert(sv != 0);
  tt_assert(sv->array != 0);

  for (i = 0; i < 14; ++i) {
    iotc_vector_del(sv, 0);
  }

  tt_assert(sv->capacity == 16);

end:;
  iotc_vector_destroy(sv);
  tt_want_int_op(iotc_is_whole_memory_deallocated(), >, 0);
})

IOTC_TT_TESTCASE(test_vector_find_I32, {
  iotc_vector_t* sv = iotc_vector_create();

  int32_t i = 0;
  for (; i < 4; ++i) {
    iotc_vector_push(sv, IOTC_VEC_CONST_VALUE_PARAM(IOTC_VEC_VALUE_I32(i)));
  }

  for (i = 3; i >= 0; --i) {
    tt_assert(i == iotc_vector_find(
                       sv, IOTC_VEC_CONST_VALUE_PARAM(IOTC_VEC_VALUE_I32(i)),
                       &utest_datastructures_cmp_vector_i32));
  }

end:;
  iotc_vector_destroy(sv);
  tt_want_int_op(iotc_is_whole_memory_deallocated(), >, 0);
})

IOTC_TT_TESTCASE(test_vector_find_UI32, {
  iotc_vector_t* sv = iotc_vector_create();

  uint32_t i = 0;
  for (; i < 4; ++i) {
    iotc_vector_push(sv, IOTC_VEC_CONST_VALUE_PARAM(IOTC_VEC_VALUE_UI32(i)));
  }

  int8_t vi;
  for (vi = 3; vi >= 0; --vi) {
    i = (uint32_t)vi;
    tt_assert(vi == iotc_vector_find(
                        sv, IOTC_VEC_CONST_VALUE_PARAM(IOTC_VEC_VALUE_UI32(i)),
                        &utest_datastructures_cmp_vector_ui32));
  }

end:;
  iotc_vector_destroy(sv);
  tt_want_int_op(iotc_is_whole_memory_deallocated(), >, 0);
})

IOTC_TT_TESTCASE(test_vector_find_PTR, {
  iotc_vector_t* sv = iotc_vector_create();

  unsigned long i = 0;
  for (; i < 4; ++i) {
    iotc_vector_push(sv,
                     IOTC_VEC_CONST_VALUE_PARAM(IOTC_VEC_VALUE_PTR((char*)i)));
  }

  int8_t vi;
  for (vi = 3; vi >= 0; --vi) {
    i = vi;
    tt_assert(vi ==
              iotc_vector_find(
                  sv, IOTC_VEC_CONST_VALUE_PARAM(IOTC_VEC_VALUE_PTR((char*)i)),
                  &utest_datastructures_cmp_vector_ptr));
  }

end:;
  iotc_vector_destroy(sv);
  tt_want_int_op(iotc_is_whole_memory_deallocated(), >, 0);
})

IOTC_TT_TESTCASE(test_vector_get, {
  iotc_vector_t* sv = iotc_vector_create();

  int32_t i = 0;
  for (; i < 32; ++i) {
    iotc_vector_push(sv, IOTC_VEC_CONST_VALUE_PARAM(IOTC_VEC_VALUE_I32(i)));
  }

  // Test with existing elements
  for (; i < 32; ++i) {
    void* elem = iotc_vector_get(sv, i);
    tt_assert(elem == (void*)(intptr_t)i);
  }

  // Test overindexing
  void* elem = iotc_vector_get(sv, 32);
  tt_assert(NULL == elem);

end:;
  iotc_vector_destroy(sv);
  tt_want_int_op(iotc_is_whole_memory_deallocated(), >, 0);
})

IOTC_TT_TESTCASE(test_vector_remove_if_I32, {
  iotc_vector_t* sv = iotc_vector_create();

  int32_t i = 0;
  for (; i < 10; ++i) {
    iotc_vector_push(sv, IOTC_VEC_CONST_VALUE_PARAM(IOTC_VEC_VALUE_I32(i)));
  }

  iotc_vector_remove_if(sv, &vector_is_odd);

  tt_want_int_op(sv->elem_no, ==, 5);

  for (i = 0; i < sv->elem_no; ++i) {
    tt_want_int_op((sv->array[i].selector_t.i32_value & 1), ==, 0);
  }

  iotc_vector_destroy(sv);

  tt_want_int_op(iotc_is_whole_memory_deallocated(), >, 0);
})

IOTC_TT_TESTCASE(test_vector_remove_if_UI32, {
  iotc_vector_t* sv = iotc_vector_create();

  int32_t i = 0;
  for (; i < 10; ++i) {
    iotc_vector_push(
        sv, IOTC_VEC_CONST_VALUE_PARAM(IOTC_VEC_VALUE_UI32((uint32_t)i)));
  }

  iotc_vector_remove_if(sv, &vector_is_odd);

  tt_want_int_op(sv->elem_no, ==, 5);

  for (i = 0; i < sv->elem_no; ++i) {
    tt_want_int_op((sv->array[i].selector_t.ui32_value & 1), ==, 0);
  }

  iotc_vector_destroy(sv);

  tt_want_int_op(iotc_is_whole_memory_deallocated(), >, 0);
})

IOTC_TT_TESTCASE(test_vector_remove_if_PTR, {
  iotc_vector_t* sv = iotc_vector_create();

  unsigned long i = 0;
  for (; i < 10; ++i) {
    iotc_vector_push(sv,
                     IOTC_VEC_CONST_VALUE_PARAM(IOTC_VEC_VALUE_PTR((void*)i)));
  }

  iotc_vector_remove_if(sv, &vector_is_odd);

  tt_want_int_op(sv->elem_no, ==, 5);

  for (i = 0; i < (unsigned long)sv->elem_no; ++i) {
    tt_want_int_op(((unsigned long)sv->array[i].selector_t.ptr_value & 1), ==,
                   0);
  }

  iotc_vector_destroy(sv);

  tt_want_int_op(iotc_is_whole_memory_deallocated(), >, 0);
})

IOTC_TT_TESTGROUP_END

#ifndef IOTC_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#define IOTC_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#include __FILE__
#undef IOTC_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#endif
