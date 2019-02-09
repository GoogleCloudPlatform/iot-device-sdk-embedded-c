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

#ifndef __IOTC_LIST_H__
#define __IOTC_LIST_H__

#define IOTC_LIST_PUSH_BACK(type, list, elem) \
  {                                           \
    type* prev = list;                        \
    type* curr = prev;                        \
    while (NULL != curr) {                    \
      prev = curr;                            \
      curr = curr->__next;                    \
    }                                         \
    if (prev == curr) {                       \
      list = elem;                            \
    } else {                                  \
      prev->__next = elem;                    \
    }                                         \
  }

#define IOTC_LIST_PUSH_FRONT(type, list, elem) \
  {                                            \
    if (NULL != list) {                        \
      elem->__next = list;                     \
      list = elem;                             \
    } else {                                   \
      list = elem;                             \
    }                                          \
  }

#define IOTC_LIST_FIND(type, list, cnd, val, out) \
  {                                               \
    out = list;                                   \
    while ((NULL != out) && (!cnd(out, val))) {   \
      out = out->__next;                          \
    }                                             \
  }

#define IOTC_LIST_DROP(type, list, elem)       \
  {                                            \
    type* prev = list;                         \
    type* curr = prev;                         \
    while ((NULL != curr) && (curr != elem)) { \
      prev = curr;                             \
      curr = curr->__next;                     \
    }                                          \
    if (curr == elem) {                        \
      if (prev == curr) {                      \
        list = curr->__next;                   \
        elem->__next = 0;                      \
      } else {                                 \
        prev->__next = curr->__next;           \
        elem->__next = 0;                      \
      }                                        \
    }                                          \
  }

#define IOTC_LIST_POP(type, list, out) \
  {                                    \
    out = list;                        \
    if (NULL == out->__next) {         \
      list = NULL;                     \
    } else {                           \
      list = out->__next;              \
      out->__next = NULL;              \
    }                                  \
  }

#define IOTC_LIST_EMPTY(type, list) (NULL == list)

#define IOTC_LIST_FOREACH(type, list, fun) \
  {                                        \
    type* curr = list;                     \
    type* tmp = NULL;                      \
    while (NULL != curr) {                 \
      tmp = curr->__next;                  \
      fun(curr);                           \
      curr = tmp;                          \
    }                                      \
  }

#define IOTC_LIST_FOREACH_WITH_ARG(type, list, fun, arg) \
  {                                                      \
    type* curr = list;                                   \
    type* tmp = NULL;                                    \
    while (NULL != curr) {                               \
      tmp = curr->__next;                                \
      fun(curr, arg);                                    \
      curr = tmp;                                        \
    }                                                    \
  }

#define IOTC_LIST_NTH(type, list, n, out) \
  {                                       \
    type* curr = list;                    \
    size_t i = 0;                         \
    while (NULL != curr) {                \
      if (i++ == n) {                     \
        out = curr;                       \
        break;                            \
      }                                   \
      curr = curr->__next;                \
    }                                     \
  }

#define IOTC_LIST_FIND_I(type, list, pred, pred_params, out) \
  {                                                          \
    type* curr = list;                                       \
    size_t i = 0;                                            \
    IOTC_UNUSED(i);                                          \
    while (NULL != curr) {                                   \
      if (pred((pred_params), curr, i++) == 1) {             \
        out = curr;                                          \
        break;                                               \
      }                                                      \
      curr = curr->__next;                                   \
    }                                                        \
  }

#define IOTC_LIST_SPLIT_I(type, list, pred, pred_params, out) \
  {                                                           \
    type* curr = list;                                        \
    type* out_curr = out;                                     \
    type* prev = NULL;                                        \
    type* tmp = NULL;                                         \
    size_t i = 0;                                             \
    IOTC_UNUSED(i);                                           \
    while (NULL != curr) {                                    \
      if (pred((pred_params), curr, i++) == 1) {              \
        if (NULL != prev) {                                   \
          prev->__next = curr->__next;                        \
        } else {                                              \
          list = curr->__next;                                \
        }                                                     \
                                                              \
        if (NULL != out_curr) {                               \
          out_curr->__next = curr;                            \
          out_curr = curr;                                    \
        } else {                                              \
          out_curr = curr;                                    \
          out = out_curr;                                     \
        }                                                     \
                                                              \
        tmp = curr;                                           \
        curr = curr->__next;                                  \
        tmp->__next = NULL;                                   \
      } else {                                                \
        prev = curr;                                          \
        curr = curr->__next;                                  \
      }                                                       \
    }                                                         \
  }

#endif /* __IOTC_LIST_H__ */
