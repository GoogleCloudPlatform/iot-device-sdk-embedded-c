/* Copyright 2018-2019 Google LLC
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

#include <stdio.h>
#include <stdlib.h>

#include "iotc_utest_basic_testcase_frame.h"
#include "tinytest.h"
#include "tinytest_macros.h"

#include "iotc_list.h"
#include "iotc_macros.h"
#include "iotc_tt_testcase_management.h"

#ifndef IOTC_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN

/**
 * @brief structure used for testing purposes
 */
typedef struct iotc_utest_list_s {
  int value;
  struct iotc_utest_list_s* __next;
} iotc_utest_list_t;

/* list utest helpers */
static void iotc_utest_destroy_list(iotc_utest_list_t* list);
static iotc_utest_list_t* iotc_utest_make_test_list(size_t elem_no);

static iotc_utest_list_t* iotc_utest_make_test_list(size_t elem_no) {
  iotc_state_t state = IOTC_STATE_OK;
  iotc_utest_list_t* out = NULL;

  size_t i = 0;
  for (; i < elem_no; ++i) {
    IOTC_ALLOC(iotc_utest_list_t, elem, state);
    elem->value = (int)i;
    IOTC_LIST_PUSH_BACK(iotc_utest_list_t, out, elem);
  }

  return out;

err_handling:
  iotc_utest_destroy_list(out);
  return (iotc_utest_list_t*)NULL;
}

static void iotc_utest_destroy_list(iotc_utest_list_t* list) {
  while (list) {
    iotc_utest_list_t* next = list->__next;

    IOTC_SAFE_FREE(list);

    list = next;
  }
}

static int iotc_utest_list_verificator(iotc_utest_list_t* list,
                                       int (*pred)(void*, iotc_utest_list_t*,
                                                   int)) {
  size_t i = 0;

  while (list) {
    if (pred(NULL, list, i++) != 1) {
      return 0;
    }
    list = list->__next;
  }

  return i;
}

static int iotc_utest_list_number_odd_predicate(void* arg,
                                                iotc_utest_list_t* elem,
                                                int pos) {
  IOTC_UNUSED(arg);
  IOTC_UNUSED(pos);

  return (elem->value % 2) != 0;
}

static int iotc_utest_list_number_even_predicate(void* arg,
                                                 iotc_utest_list_t* elem,
                                                 int pos) {
  IOTC_UNUSED(arg);
  IOTC_UNUSED(pos);

  return !iotc_utest_list_number_odd_predicate(arg, elem, pos);
}

static int iotc_utest_list_number_less_than_10_predicate(
    void* arg, iotc_utest_list_t* elem, int pos) {
  IOTC_UNUSED(arg);
  IOTC_UNUSED(pos);

  return (elem->value < 10);
}

static int iotc_utest_list_number_more_than_10_predicate(
    void* arg, iotc_utest_list_t* elem, int pos) {
  IOTC_UNUSED(arg);
  IOTC_UNUSED(pos);

  return (elem->value >= 10);
}

#endif

IOTC_TT_TESTGROUP_BEGIN(utest_list)

IOTC_TT_TESTCASE_WITH_SETUP(
    utest__iotc_list_split_list_even_and_odd_test, iotc_utest_setup_basic,
    iotc_utest_teardown_basic, NULL, {
      iotc_utest_list_t* test_list = iotc_utest_make_test_list(10);
      iotc_utest_list_t* odd_list = NULL;

      tt_ptr_op(test_list, !=, NULL);

      IOTC_LIST_SPLIT_I(iotc_utest_list_t, test_list,
                        iotc_utest_list_number_odd_predicate, NULL, odd_list);

      tt_ptr_op(test_list, !=, NULL);
      tt_ptr_op(odd_list, !=, NULL);

      tt_int_op(iotc_utest_list_verificator(
                    odd_list, &iotc_utest_list_number_odd_predicate),
                ==, 5);

      tt_int_op(iotc_utest_list_verificator(
                    test_list, &iotc_utest_list_number_even_predicate),
                ==, 5);

    end:
      iotc_utest_destroy_list(test_list);
      iotc_utest_destroy_list(odd_list);
    })

IOTC_TT_TESTCASE_WITH_SETUP(
    utest__iotc_list_split_list_less_than_10_test_on_10_elements,
    iotc_utest_setup_basic, iotc_utest_teardown_basic, NULL, {
      iotc_utest_list_t* test_list = iotc_utest_make_test_list(10);
      iotc_utest_list_t* less_10_list = NULL;

      tt_ptr_op(test_list, !=, NULL);

      IOTC_LIST_SPLIT_I(iotc_utest_list_t, test_list,
                        iotc_utest_list_number_less_than_10_predicate, NULL,
                        less_10_list);

      tt_ptr_op(test_list, ==, NULL);
      tt_ptr_op(less_10_list, !=, NULL);

      tt_int_op(
          iotc_utest_list_verificator(
              less_10_list, &iotc_utest_list_number_less_than_10_predicate),
          ==, 10);

    end:
      iotc_utest_destroy_list(test_list);
      iotc_utest_destroy_list(less_10_list);
    })

IOTC_TT_TESTCASE_WITH_SETUP(
    utest__iotc_list_split_list_less_than_10_test_on_13_elements,
    iotc_utest_setup_basic, iotc_utest_teardown_basic, NULL, {
      iotc_utest_list_t* test_list = iotc_utest_make_test_list(13);
      iotc_utest_list_t* less_10_list = NULL;

      tt_ptr_op(test_list, !=, NULL);

      IOTC_LIST_SPLIT_I(iotc_utest_list_t, test_list,
                        iotc_utest_list_number_less_than_10_predicate, NULL,
                        less_10_list);

      tt_ptr_op(test_list, !=, NULL);
      tt_ptr_op(less_10_list, !=, NULL);

      tt_int_op(
          iotc_utest_list_verificator(
              less_10_list, &iotc_utest_list_number_less_than_10_predicate),
          ==, 10);

      tt_int_op(iotc_utest_list_verificator(
                    test_list, &iotc_utest_list_number_more_than_10_predicate),
                ==, 3);

    end:
      iotc_utest_destroy_list(test_list);
      iotc_utest_destroy_list(less_10_list);
    })

IOTC_TT_TESTCASE_WITH_SETUP(
    utest__iotc_list_split_list_more_than_10_test, iotc_utest_setup_basic,
    iotc_utest_teardown_basic, NULL, {
      iotc_utest_list_t* test_list = iotc_utest_make_test_list(13);
      iotc_utest_list_t* more_10_list = NULL;

      tt_ptr_op(test_list, !=, NULL);

      IOTC_LIST_SPLIT_I(iotc_utest_list_t, test_list,
                        iotc_utest_list_number_more_than_10_predicate, NULL,
                        more_10_list);

      tt_ptr_op(test_list, !=, NULL);
      tt_ptr_op(more_10_list, !=, NULL);

      tt_int_op(iotc_utest_list_verificator(
                    test_list, &iotc_utest_list_number_less_than_10_predicate),
                ==, 10);

      tt_int_op(
          iotc_utest_list_verificator(
              more_10_list, &iotc_utest_list_number_more_than_10_predicate),
          ==, 3);

    end:
      iotc_utest_destroy_list(test_list);
      iotc_utest_destroy_list(more_10_list);
    })

IOTC_TT_TESTGROUP_END

#ifndef IOTC_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#define IOTC_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#include __FILE__
#undef IOTC_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#endif
