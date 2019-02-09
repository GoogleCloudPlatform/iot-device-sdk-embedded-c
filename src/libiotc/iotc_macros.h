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

#ifndef __IOTC_MACROS_H__
#define __IOTC_MACROS_H__

#include <string.h>
#include "iotc_allocator.h"
#include "iotc_err.h"

#define IOTC_STR_EXPAND(tok) #tok
#define IOTC_STR(tok) IOTC_STR_EXPAND(tok)
#define IOTC_MIN(a, b) ((a) > (b) ? (b) : (a))
#define IOTC_MAX(a, b) ((a) > (b) ? (a) : (b))
#define IOTC_UNUSED(x) (void)(x)

#define IOTC_GUARD_EOS(s, size) \
  { (s)[(size)-1] = '\0'; }

/* a = input value, mi = minimum, ma = maximum */
#define IOTC_CLAMP(a, mi, ma) IOTC_MIN(IOTC_MAX((a), (mi)), (ma))

#define IOTC_CHECK_CND(cnd, e, s) \
  IOTC_UNUSED((s));               \
  if ((cnd)) {                    \
    (s) = (e);                    \
    goto err_handling;            \
  }

#define IOTC_CHECK_STATE(s)   \
  if ((s) != IOTC_STATE_OK) { \
    goto err_handling;        \
  }

#define IOTC_CHECK_ZERO(a, e, s) IOTC_CHECK_CND((a) == 0, (e), (s))

#define IOTC_CHECK_CND_DBGMESSAGE(cnd, e, s, dbgmessage) \
  IOTC_UNUSED((s));                                      \
  if ((cnd)) {                                           \
    (s) = (e);                                           \
    iotc_debug_logger(dbgmessage);                       \
    goto err_handling;                                   \
  }

#define IOTC_CHECK_NEG(a) \
  if ((a) < 0) )

#define IOTC_CHECK_PTR(a, b) if ((a) == (b))

#define IOTC_SAFE_FREE(a) \
  if ((a)) {              \
    iotc_free(a);         \
    (a) = NULL;           \
  }

#define IOTC_CHECK_MEMORY(a, s) IOTC_CHECK_CND((a) == 0, IOTC_OUT_OF_MEMORY, s)

#define IOTC_CHECK_SIZE(a, b, e, s) \
  IOTC_CHECK_CND(((a) >= (b) || (a) < 0), e, s)

#define IOTC_CLEAR_STATIC_BUFFER(a) memset((a), 0, sizeof(a))

#define IOTC_CHECK_S(s, size, o, e, st) \
  {                                     \
    IOTC_CHECK_SIZE(s, size - o, e, st) \
    else {                              \
      (o) += (s);                       \
    }                                   \
  }

/* Generic allocation macro group. */
#define IOTC_ALLOC_BUFFER_AT(type_t, out, size, state) \
  out = (type_t*)iotc_alloc(size);                     \
  IOTC_CHECK_MEMORY(out, state);                       \
  memset(out, 0, size);

#define IOTC_ALLOC_BUFFER(type_t, out, size, state) \
  type_t* out = NULL;                               \
  IOTC_ALLOC_BUFFER_AT(type_t, out, size, state);

#define IOTC_ALLOC_AT(type_t, out, state) \
  IOTC_ALLOC_BUFFER_AT(type_t, out, sizeof(type_t), state);

#define IOTC_ALLOC(type_t, out, state) \
  type_t* out = NULL;                  \
  IOTC_ALLOC_AT(type_t, out, state)

/* System allocation macro group. */
#define IOTC_ALLOC_SYSTEM_BUFFER_AT(type_t, out, size, state) \
  out = (type_t*)iotc_alloc_system(size);                     \
  IOTC_CHECK_MEMORY(out, state);                              \
  memset(out, 0, size);

#define IOTC_ALLOC_SYSTEM_BUFFER(type_t, out, size, state) \
  type_t* out = NULL;                                      \
  IOTC_ALLOC_SYSTEM_BUFFER_AT(type_t, out, size, state);

#define IOTC_ALLOC_SYSTEM_AT(type_t, out, state) \
  IOTC_ALLOC_SYSTEM_BUFFER_AT(type_t, out, sizeof(type_t), state);

#define IOTC_ALLOC_SYSTEM(type_t, out, state) \
  type_t* out = NULL;                         \
  IOTC_ALLOC_SYSTEM_AT(type_t, out, state)

#define IOTC_ARRAYSIZE(a) (sizeof(a) / sizeof((a)[0]))

#define IOTC_TIME_MILLISLEEP(milliseconds, timespec_localvariablename) \
  struct timespec timespec_localvariablename;                          \
  timespec_localvariablename.tv_sec = 0;                               \
  timespec_localvariablename.tv_nsec = milliseconds * 1000 * 1000;     \
  nanosleep(&timespec_localvariablename, NULL);

#endif /*__IOTC_MACROS_H__ */
