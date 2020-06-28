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

#include "iotc_event_handle.h"
#include <string.h>
#include "iotc_allocator.h"
#include "iotc_config.h"
#include "iotc_debug.h"
#include "iotc_macros.h"

#if IOTC_DEBUG_EXTRA_INFO

iotc_event_handle_t iotc_make_empty_handle_impl(const uint8_t target_tid,
                                                const char* file_name,
                                                const int line_no) {
  return (iotc_event_handle_t){IOTC_EVENT_HANDLE_UNSET,
                               {line_no, file_name},
                               .handlers.h0 = {0},
                               target_tid};
}

iotc_event_handle_t iotc_make_handle_argc0(
    const uint8_t target_tid, iotc_event_handle_func_argc0_ptr fn_ptr,
    const char* file_name, const int line_no) {
  return (iotc_event_handle_t){IOTC_EVENT_HANDLE_ARGC0,
                               {line_no, file_name},
                               .handlers.h0 = {fn_ptr},
                               target_tid};
}

iotc_event_handle_t iotc_make_handle_argc1(
    const uint8_t target_tid, iotc_event_handle_func_argc1_ptr fn_ptr,
    iotc_event_handle_arg1_t a1, const char* file_name, const int line_no) {
  return (iotc_event_handle_t){IOTC_EVENT_HANDLE_ARGC1,
                               {line_no, file_name},
                               .handlers.h1 = {fn_ptr, a1},
                               target_tid};
}

iotc_event_handle_t iotc_make_handle_argc2(
    const uint8_t target_tid, iotc_event_handle_func_argc2_ptr fn_ptr,
    iotc_event_handle_arg1_t a1, iotc_event_handle_arg2_t a2,
    const char* file_name, const int line_no) {
  return (iotc_event_handle_t){IOTC_EVENT_HANDLE_ARGC2,
                               {line_no, file_name},
                               .handlers.h2 = {fn_ptr, a1, a2},
                               target_tid};
}

iotc_event_handle_t iotc_make_handle_argc3(
    const uint8_t target_tid, iotc_event_handle_func_argc3_ptr fn_ptr,
    iotc_event_handle_arg1_t a1, iotc_event_handle_arg2_t a2,
    iotc_event_handle_arg3_t a3, const char* file_name, const int line_no) {
  return (iotc_event_handle_t){IOTC_EVENT_HANDLE_ARGC3,
                               {line_no, file_name},
                               .handlers.h3 = {fn_ptr, a1, a2, a3},
                               target_tid};
}

iotc_event_handle_t iotc_make_handle_argc4(
    const uint8_t target_tid, iotc_event_handle_func_argc4_ptr fn_ptr,
    iotc_event_handle_arg1_t a1, iotc_event_handle_arg2_t a2,
    iotc_event_handle_arg3_t a3, iotc_event_handle_arg4_t a4,
    const char* file_name, const int line_no) {
  return (iotc_event_handle_t){IOTC_EVENT_HANDLE_ARGC4,
                               {line_no, file_name},
                               .handlers.h4 = {fn_ptr, a1, a2, a3, a4},
                               target_tid};
}

iotc_event_handle_t iotc_make_handle_argc5(
    const uint8_t target_tid, iotc_event_handle_func_argc5_ptr fn_ptr,
    iotc_event_handle_arg1_t a1, iotc_event_handle_arg2_t a2,
    iotc_event_handle_arg3_t a3, iotc_event_handle_arg4_t a4,
    iotc_event_handle_arg5_t a5, const char* file_name, const int line_no) {
  return (iotc_event_handle_t){IOTC_EVENT_HANDLE_ARGC5,
                               {line_no, file_name},
                               .handlers.h5 = {fn_ptr, a1, a2, a3, a4, a5},
                               target_tid};
}

iotc_event_handle_t iotc_make_handle_argc6(
    const uint8_t target_tid, iotc_event_handle_func_argc6_ptr fn_ptr,
    iotc_event_handle_arg1_t a1, iotc_event_handle_arg2_t a2,
    iotc_event_handle_arg3_t a3, iotc_event_handle_arg4_t a4,
    iotc_event_handle_arg5_t a5, iotc_event_handle_arg6_t a6,
    const char* file_name, const int line_no) {
  return (iotc_event_handle_t){IOTC_EVENT_HANDLE_ARGC6,
                               {line_no, file_name},
                               .handlers.h6 = {fn_ptr, a1, a2, a3, a4, a5, a6},
                               target_tid};
}

#else /* #if IOTC_DEBUG_EXTRA_INFO */

iotc_event_handle_t iotc_make_empty_handle_impl(const uint8_t target_tid) {
  return (iotc_event_handle_t)iotc_make_empty_event_handle(target_tid);
}

iotc_event_handle_t iotc_make_handle_argc0(
    const uint8_t target_tid, iotc_event_handle_func_argc0_ptr fn_ptr) {
  return (iotc_event_handle_t){IOTC_EVENT_HANDLE_ARGC0, .handlers.h0 = {fn_ptr},
                               target_tid};
}

iotc_event_handle_t iotc_make_handle_argc1(
    const uint8_t target_tid, iotc_event_handle_func_argc1_ptr fn_ptr,
    iotc_event_handle_arg1_t a1) {
  return (iotc_event_handle_t){IOTC_EVENT_HANDLE_ARGC1,
                               .handlers.h1 = {fn_ptr, a1}, target_tid};
}

iotc_event_handle_t iotc_make_handle_argc2(
    const uint8_t target_tid, iotc_event_handle_func_argc2_ptr fn_ptr,
    iotc_event_handle_arg1_t a1, iotc_event_handle_arg2_t a2) {
  return (iotc_event_handle_t){IOTC_EVENT_HANDLE_ARGC2,
                               .handlers.h2 = {fn_ptr, a1, a2}, target_tid};
}

iotc_event_handle_t iotc_make_handle_argc3(
    const uint8_t target_tid, iotc_event_handle_func_argc3_ptr fn_ptr,
    iotc_event_handle_arg1_t a1, iotc_event_handle_arg2_t a2,
    iotc_event_handle_arg3_t a3) {
  return (iotc_event_handle_t){IOTC_EVENT_HANDLE_ARGC3,
                               .handlers.h3 = {fn_ptr, a1, a2, a3}, target_tid};
}

iotc_event_handle_t iotc_make_handle_argc4(
    const uint8_t target_tid, iotc_event_handle_func_argc4_ptr fn_ptr,
    iotc_event_handle_arg1_t a1, iotc_event_handle_arg2_t a2,
    iotc_event_handle_arg3_t a3, iotc_event_handle_arg4_t a4) {
  return (iotc_event_handle_t){IOTC_EVENT_HANDLE_ARGC4,
                               .handlers.h4 = {fn_ptr, a1, a2, a3, a4},
                               target_tid};
}

iotc_event_handle_t iotc_make_handle_argc5(
    const uint8_t target_tid, iotc_event_handle_func_argc5_ptr fn_ptr,
    iotc_event_handle_arg1_t a1, iotc_event_handle_arg2_t a2,
    iotc_event_handle_arg3_t a3, iotc_event_handle_arg4_t a4,
    iotc_event_handle_arg5_t a5) {
  return (iotc_event_handle_t){IOTC_EVENT_HANDLE_ARGC5,
                               .handlers.h5 = {fn_ptr, a1, a2, a3, a4, a5},
                               target_tid};
}

iotc_event_handle_t iotc_make_handle_argc6(
    const uint8_t target_tid, iotc_event_handle_func_argc6_ptr fn_ptr,
    iotc_event_handle_arg1_t a1, iotc_event_handle_arg2_t a2,
    iotc_event_handle_arg3_t a3, iotc_event_handle_arg4_t a4,
    iotc_event_handle_arg5_t a5, iotc_event_handle_arg5_t a6) {
  return (iotc_event_handle_t){IOTC_EVENT_HANDLE_ARGC6,
                               .handlers.h6 = {fn_ptr, a1, a2, a3, a4, a5, a6},
                               target_tid};
}

#endif /* #if IOTC_DEBUG_EXTRA_INFO */

iotc_state_t iotc_pointerize_handle(iotc_event_handle_t handle,
                                    iotc_event_handle_t** pointer) {
  /* PRECONDITIONS */
  if (NULL == pointer) {
    return IOTC_INVALID_PARAMETER;
  }

  if (NULL != *pointer) {
    return IOTC_INVALID_PARAMETER;
  }

  iotc_state_t state = IOTC_STATE_OK;

  IOTC_ALLOC_AT(iotc_event_handle_t, *pointer, state);
  memcpy(*pointer, &handle, sizeof(iotc_event_handle_t));

err_handling:
  return state;
}

void iotc_dispose_handle(iotc_event_handle_t* handle) {
  memset(handle, 0, sizeof(iotc_event_handle_t));
  handle->handle_type = IOTC_EVENT_HANDLE_UNSET;
}

uint8_t iotc_handle_disposed(iotc_event_handle_t* handle) {
  return handle->handle_type == IOTC_EVENT_HANDLE_UNSET ? 1 : 0;
}
