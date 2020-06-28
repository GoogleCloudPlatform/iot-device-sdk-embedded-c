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

#ifndef __IOTC_EVENT_HANDLE_H__
#define __IOTC_EVENT_HANDLE_H__

#include "iotc_config.h"
#include "iotc_event_handle_typedef.h"
#include "iotc_thread_ids.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Idea for that macro trick has been taken from the other macro
 * that counts the number of arguments of variadic macro. The rule
 * and generic idea is the same. The iotc_make_handle_impl takes always
 * Nth argument and uses it for macro substitution. The N has to be const
 * and it's 6 in our case since macro 'iotc_make_handle' can be called
 * at most with 6 arguments: threadid, fnptr + 4args.
 */

#define iotc_make_handle(...) \
  iotc_make_threaded_handle(IOTC_THREADID_MAINTHREAD, __VA_ARGS__)
#define iotc_make_handle_impl(_0, _1, _2, _3, _4, _5, _6, _7, N, ...) N

#if IOTC_DEBUG_EXTRA_INFO

#define iotc_make_threaded_handle(...)                                      \
  iotc_make_handle_impl(                                                    \
      __VA_ARGS__, iotc_make_handle_argc6(__VA_ARGS__, __FILE__, __LINE__), \
      iotc_make_handle_argc5(__VA_ARGS__, __FILE__, __LINE__),              \
      iotc_make_handle_argc4(__VA_ARGS__, __FILE__, __LINE__),              \
      iotc_make_handle_argc3(__VA_ARGS__, __FILE__, __LINE__),              \
      iotc_make_handle_argc2(__VA_ARGS__, __FILE__, __LINE__),              \
      iotc_make_handle_argc1(__VA_ARGS__, __FILE__, __LINE__),              \
      iotc_make_handle_argc0(__VA_ARGS__, __FILE__, __LINE__),              \
      0) /* the last 0 to not alow the impl macro to not to have last       \
         parameter in case of 1 arg passed */

#define iotc_make_empty_handle(...) \
  iotc_make_empty_handle_impl(IOTC_THREADID_MAINTHREAD, __FILE__, __LINE__)

extern iotc_event_handle_t iotc_make_empty_handle_impl(const uint8_t target_tid,
                                                       const char* file_name,
                                                       const int line_no);

extern iotc_event_handle_t iotc_make_handle_argc0(
    const uint8_t target_tid, iotc_event_handle_func_argc0_ptr fn_ptr,
    const char* file_name, const int line_no);

extern iotc_event_handle_t iotc_make_handle_argc1(
    const uint8_t target_tid, iotc_event_handle_func_argc1_ptr fn_ptr,
    iotc_event_handle_arg1_t a1, const char* file_name, const int line_no);

extern iotc_event_handle_t iotc_make_handle_argc2(
    const uint8_t target_tid, iotc_event_handle_func_argc2_ptr fn_ptr,
    iotc_event_handle_arg1_t a1, iotc_event_handle_arg2_t a2,
    const char* file_name, const int line_no);

extern iotc_event_handle_t iotc_make_handle_argc3(
    const uint8_t target_tid, iotc_event_handle_func_argc3_ptr fn_ptr,
    iotc_event_handle_arg1_t a1, iotc_event_handle_arg2_t a2,
    iotc_event_handle_arg3_t a3, const char* file_name, const int line_no);

extern iotc_event_handle_t iotc_make_handle_argc4(
    const uint8_t target_tid, iotc_event_handle_func_argc4_ptr fn_ptr,
    iotc_event_handle_arg1_t a1, iotc_event_handle_arg2_t a2,
    iotc_event_handle_arg3_t a3, iotc_event_handle_arg4_t a4,
    const char* file_name, const int line_no);

extern iotc_event_handle_t iotc_make_handle_argc5(
    const uint8_t target_tid, iotc_event_handle_func_argc5_ptr fn_ptr,
    iotc_event_handle_arg1_t a1, iotc_event_handle_arg2_t a2,
    iotc_event_handle_arg3_t a3, iotc_event_handle_arg4_t a4,
    iotc_event_handle_arg5_t a5, const char* file_name, const int line_no);

extern iotc_event_handle_t iotc_make_handle_argc6(
    const uint8_t target_tid, iotc_event_handle_func_argc6_ptr fn_ptr,
    iotc_event_handle_arg1_t a1, iotc_event_handle_arg2_t a2,
    iotc_event_handle_arg3_t a3, iotc_event_handle_arg4_t a4,
    iotc_event_handle_arg5_t a5, iotc_event_handle_arg6_t a6,
    const char* file_name, const int line_no);

#else /* IOTC_DEBUG_EXTRA_INFO */

#define iotc_make_threaded_handle(...)                                        \
  iotc_make_handle_impl(__VA_ARGS__, iotc_make_handle_argc6(__VA_ARGS__),     \
                        iotc_make_handle_argc5(__VA_ARGS__),                  \
                        iotc_make_handle_argc4(__VA_ARGS__),                  \
                        iotc_make_handle_argc3(__VA_ARGS__),                  \
                        iotc_make_handle_argc2(__VA_ARGS__),                  \
                        iotc_make_handle_argc1(__VA_ARGS__),                  \
                        iotc_make_handle_argc0(__VA_ARGS__),                  \
                        0) /* the last 0 to not alow tha impl macro to not to \
                            * have last parameter in case of 1 arg passed */

#define iotc_make_empty_handle(...) \
  iotc_make_empty_handle_impl(IOTC_THREADID_MAINTHREAD)

extern iotc_event_handle_t iotc_make_empty_handle_impl(
    const uint8_t target_tid);

extern iotc_event_handle_t iotc_make_handle_argc0(
    const uint8_t target_tid, iotc_event_handle_func_argc0_ptr fn_ptr);

extern iotc_event_handle_t iotc_make_handle_argc1(
    const uint8_t target_tid, iotc_event_handle_func_argc1_ptr fn_ptr,
    iotc_event_handle_arg1_t a1);

extern iotc_event_handle_t iotc_make_handle_argc2(
    const uint8_t target_tid, iotc_event_handle_func_argc2_ptr fn_ptr,
    iotc_event_handle_arg1_t a1, iotc_event_handle_arg2_t a2);

extern iotc_event_handle_t iotc_make_handle_argc3(
    const uint8_t target_tid, iotc_event_handle_func_argc3_ptr fn_ptr,
    iotc_event_handle_arg1_t a1, iotc_event_handle_arg2_t a2,
    iotc_event_handle_arg3_t a3);

extern iotc_event_handle_t iotc_make_handle_argc4(
    const uint8_t target_tid, iotc_event_handle_func_argc4_ptr fn_ptr,
    iotc_event_handle_arg1_t a1, iotc_event_handle_arg2_t a2,
    iotc_event_handle_arg3_t a3, iotc_event_handle_arg4_t a4);

extern iotc_event_handle_t iotc_make_handle_argc5(
    const uint8_t target_tid, iotc_event_handle_func_argc5_ptr fn_ptr,
    iotc_event_handle_arg1_t a1, iotc_event_handle_arg2_t a2,
    iotc_event_handle_arg3_t a3, iotc_event_handle_arg4_t a4,
    iotc_event_handle_arg5_t a5);

extern iotc_event_handle_t iotc_make_handle_argc6(
    const uint8_t target_tid, iotc_event_handle_func_argc6_ptr fn_ptr,
    iotc_event_handle_arg1_t a1, iotc_event_handle_arg2_t a2,
    iotc_event_handle_arg3_t a3, iotc_event_handle_arg4_t a4,
    iotc_event_handle_arg5_t a5, iotc_event_handle_arg5_t a6);
#endif /* IOTC_DEBUG_EXTRA_INFO */

iotc_state_t iotc_pointerize_handle(iotc_event_handle_t handle,
                                    iotc_event_handle_t** pointer);

extern void iotc_dispose_handle(iotc_event_handle_t* handle);

extern uint8_t iotc_handle_disposed(iotc_event_handle_t* handle);

#ifdef __cplusplus
}
#endif

#endif /* __IOTC_EVENT_HANDLE_H__ */
