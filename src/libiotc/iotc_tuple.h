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

#ifndef __IOTC_TUPLE_H__
#define __IOTC_TUPLE_H__

#include "iotc_allocator.h"
#include "iotc_macros.h"

#define IOTC_DEF_TUPLE_TYPE(type_name, ...)                               \
  IOTC_DEF_TUPLE_TYPE_IMPL(__VA_ARGS__,                                   \
                           IOTC_DEF_TUPLE_TYPE_4(type_name, __VA_ARGS__), \
                           IOTC_DEF_TUPLE_TYPE_3(type_name, __VA_ARGS__), \
                           IOTC_DEF_TUPLE_TYPE_2(type_name, __VA_ARGS__), \
                           IOTC_DEF_TUPLE_TYPE_1(type_name, __VA_ARGS__), 0)

#define IOTC_DEF_TUPLE_TYPE_IMPL(_1, _2, _3, _4, N, ...) N

#ifdef IOTC_TUPLES_C
#define IOTC_DEF_TUPLE_TYPE_1(type_name, a1_t)                  \
  typedef struct {                                              \
    a1_t a1;                                                    \
  } type_name;                                                  \
  type_name iotc_make_tuple_##type_name(a1_t a1) {              \
    type_name ret;                                              \
    ret.a1 = a1;                                                \
    return ret;                                                 \
  }                                                             \
  type_name* iotc_alloc_make_tuple_##type_name(a1_t a1) {       \
    type_name* ret = (type_name*)iotc_alloc(sizeof(type_name)); \
    iotc_state_t state = IOTC_STATE_OK;                         \
    IOTC_CHECK_MEMORY(ret, state);                              \
    memset(ret, 0, sizeof(type_name));                          \
    ret->a1 = a1;                                               \
    return ret;                                                 \
  err_handling:                                                 \
    return 0;                                                   \
  }
#else /* IOTC_TUPLES_C */
#define IOTC_DEF_TUPLE_TYPE_1(type_name, a1_t)           \
  typedef struct {                                       \
    a1_t a1;                                             \
  } type_name;                                           \
  extern type_name iotc_make_tuple_##type_name(a1_t a1); \
  extern type_name* iotc_alloc_make_tuple_##type_name(a1_t a1);
#endif /* IOTC_TUPLES_C */

#ifdef IOTC_TUPLES_C
#define IOTC_DEF_TUPLE_TYPE_2(type_name, a1_t, a2_t)               \
  typedef struct {                                                 \
    a1_t a1;                                                       \
    a2_t a2;                                                       \
  } type_name;                                                     \
  type_name iotc_make_tuple_##type_name(a1_t a1, a2_t a2) {        \
    type_name ret;                                                 \
    ret.a1 = a1;                                                   \
    ret.a2 = a2;                                                   \
    return ret;                                                    \
  }                                                                \
  type_name* iotc_alloc_make_tuple_##type_name(a1_t a1, a2_t a2) { \
    type_name* ret = (type_name*)iotc_alloc(sizeof(type_name));    \
    iotc_state_t state = IOTC_STATE_OK;                            \
    IOTC_CHECK_MEMORY(ret, state);                                 \
    memset(ret, 0, sizeof(type_name));                             \
    ret->a1 = a1;                                                  \
    ret->a2 = a2;                                                  \
    return ret;                                                    \
  err_handling:                                                    \
    return 0;                                                      \
  }
#else /* IOTC_TUPLES_C */
#define IOTC_DEF_TUPLE_TYPE_2(type_name, a1_t, a2_t)              \
  typedef struct {                                                \
    a1_t a1;                                                      \
    a2_t a2;                                                      \
  } type_name;                                                    \
  extern type_name iotc_make_tuple_##type_name(a1_t a1, a2_t a2); \
  extern type_name* iotc_alloc_make_tuple_##type_name(a1_t a1, a2_t a2);
#endif /* IOTC_TUPLES_C */

#ifdef IOTC_TUPLES_C
#define IOTC_DEF_TUPLE_TYPE_3(type_name, a1_t, a2_t, a3_t)                  \
  typedef struct {                                                          \
    a1_t a1;                                                                \
    a2_t a2;                                                                \
    a3_t a3;                                                                \
  } type_name;                                                              \
  type_name iotc_make_tuple_##type_name(a1_t a1, a2_t a2, a3_t a3) {        \
    type_name ret;                                                          \
    ret.a1 = a1;                                                            \
    ret.a2 = a2;                                                            \
    ret.a3 = a3;                                                            \
    return ret;                                                             \
  }                                                                         \
  type_name* iotc_alloc_make_tuple_##type_name(a1_t a1, a2_t a2, a3_t a3) { \
    type_name* ret = (type_name*)iotc_alloc(sizeof(type_name));             \
    iotc_state_t state = IOTC_STATE_OK;                                     \
    IOTC_CHECK_MEMORY(ret, state);                                          \
    memset(ret, 0, sizeof(type_name));                                      \
    ret->a1 = a1;                                                           \
    ret->a2 = a2;                                                           \
    ret->a3 = a3;                                                           \
    return ret;                                                             \
  err_handling:                                                             \
    return 0;                                                               \
  }
#else /* IOTC_TUPLES_C */
#define IOTC_DEF_TUPLE_TYPE_3(type_name, a1_t, a2_t, a3_t)                 \
  typedef struct {                                                         \
    a1_t a1;                                                               \
    a2_t a2;                                                               \
    a3_t a3;                                                               \
  } type_name;                                                             \
  extern type_name iotc_make_tuple_##type_name(a1_t a1, a2_t a2, a3_t a3); \
  extern type_name* iotc_alloc_make_tuple_##type_name(a1_t a1, a2_t a2,    \
                                                      a3_t a3);
#endif /* IOTC_TUPLES_C */

#ifdef IOTC_TUPLES_C
#define IOTC_DEF_TUPLE_TYPE_4(type_name, a1_t, a2_t, a3_t, a4_t)               \
  typedef struct {                                                             \
    a1_t a1;                                                                   \
    a2_t a2;                                                                   \
    a3_t a3;                                                                   \
    a4_t a4;                                                                   \
  } type_name;                                                                 \
  type_name iotc_make_tuple_##type_name(a1_t a1, a2_t a2, a3_t a3, a4_t, a4) { \
    type_name ret;                                                             \
    ret.a1 = a1;                                                               \
    ret.a2 = a2;                                                               \
    ret.a3 = a3;                                                               \
    ret.a4 = a4;                                                               \
    return ret;                                                                \
  }                                                                            \
  type_name* iotc_alloc_make_tuple_##type_name(a1_t a1, a2_t a2, a3_t a3,      \
                                               a4_t, a4) {                     \
    type_name* ret = (type_name*)iotc_alloc(sizeof(type_name));                \
    iotc_state_t state = IOTC_STATE_OK;                                        \
    IOTC_CHECK_MEMORY(ret, state);                                             \
    memset(ret, 0, sizeof(type_name));                                         \
    ret->a1 = a1;                                                              \
    ret->a2 = a2;                                                              \
    ret->a3 = a3;                                                              \
    ret->a4 = a4;                                                              \
    return ret;                                                                \
  err_handling:                                                                \
    return 0;                                                                  \
  }
#else /* IOTC_TUPLES_C */
#define IOTC_DEF_TUPLE_TYPE_4(type_name, a1_t, a2_t, a3_t, a4_t)          \
  typedef struct {                                                        \
    a1_t a1;                                                              \
    a2_t a2;                                                              \
    a3_t a3;                                                              \
    a4_t a4;                                                              \
  } type_name;                                                            \
  extern type_name iotc_make_tuple_##type_name(a1_t a1, a2_t a2, a3_t a3, \
                                               a4_t, a4);                 \
  extern type_name* iotc_alloc_make_tuple_##type_name(a1_t a1, a2_t a2,   \
                                                      a3_t a3, a4_t, a4);
#endif /* IOTC_TUPLES_C */

#define iotc_make_tuple_a1(type_name, a1) iotc_make_tuple_##type_name(a1);

#define iotc_make_tuple_a2(type_name, a1, a2) \
  iotc_make_tuple_##type_name(a1, a2);

#define iotc_make_tuple_a3(type_name, a1, a2, a3) \
  iotc_make_tuple_##type_name(a1, a2, a3);

#define iotc_make_tuple_a4(type_name, a1, a2, a3, a4) \
  iotc_make_tuple_##type_name(a1, a2, a3, a4);

#define iotc_make_tuple(type_name, ...)                            \
  iotc_make_tuple_impl(__VA_ARGS__,                                \
                       iotc_make_tuple_a4(type_name, __VA_ARGS__), \
                       iotc_make_tuple_a3(type_name, __VA_ARGS__), \
                       iotc_make_tuple_a2(type_name, __VA_ARGS__), \
                       iotc_make_tuple_a1(type_name, __VA_ARGS__), 0)

#define iotc_make_tuple_impl(_1, _2, _3, _4, N, ...) N

#define iotc_alloc_make_tuple_a1(type_name, a1) \
  iotc_alloc_make_tuple_##type_name(a1);

#define iotc_alloc_make_tuple_a2(type_name, a1, a2) \
  iotc_alloc_make_tuple_##type_name(a1, a2);

#define iotc_alloc_make_tuple_a3(type_name, a1, a2, a3) \
  iotc_alloc_make_tuple_##type_name(a1, a2, a3);

#define iotc_alloc_make_tuple_a4(type_name, a1, a2, a3, a4) \
  iotc_alloc_make_tuple_##type_name(a1, a2, a3, a4);

#define iotc_alloc_make_tuple(type_name, ...)                        \
  iotc_alloc_make_tuple_impl(                                        \
      __VA_ARGS__, iotc_alloc_make_tuple_a4(type_name, __VA_ARGS__), \
      iotc_alloc_make_tuple_a3(type_name, __VA_ARGS__),              \
      iotc_alloc_make_tuple_a2(type_name, __VA_ARGS__),              \
      iotc_alloc_make_tuple_a1(type_name, __VA_ARGS__), 0)

#define iotc_alloc_make_tuple_impl(_1, _2, _3, _4, N, ...) N

#define IOTC_SAFE_FREE_TUPLE(tuple) IOTC_SAFE_FREE(tuple)

#endif /* __IOTC_TUPLE_H__ */
