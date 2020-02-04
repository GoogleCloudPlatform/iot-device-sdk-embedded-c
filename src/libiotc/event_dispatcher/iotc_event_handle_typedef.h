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

#ifndef __IOTC_HANDLE_TYPEDEF_H__
#define __IOTC_HANDLE_TYPEDEF_H__

#include "iotc_config.h"
#include "iotc_event_dispatcher_macros.h"
#include "iotc_layer.h"

#ifdef __cplusplus
extern "C" {
#endif

IOTC_EVTD_GENERATE_EVENT_TYPE_ENUM(4, IOTC_EVENT_WANT_READ,
                                   IOTC_EVENT_WANT_WRITE, IOTC_EVENT_ERROR,
                                   IOTC_EVENT_WANT_CONNECT);

IOTC_EVTD_GENERATE_EVENT_HANDLE_TYPEDEFS(iotc_state_t, void*, void*,
                                         iotc_state_t, void*, void*, void*);

typedef enum {
  IOTC_EVENT_HANDLE_UNSET = 0,
  IOTC_EVENT_HANDLE_ARGC0,
  IOTC_EVENT_HANDLE_ARGC1,
  IOTC_EVENT_HANDLE_ARGC2,
  IOTC_EVENT_HANDLE_ARGC3,
  IOTC_EVENT_HANDLE_ARGC4,
  IOTC_EVENT_HANDLE_ARGC5,
  IOTC_EVENT_HANDLE_ARGC6,
} iotc_event_handle_argc_t;

typedef struct iotc_event_handle_s {
  /* type of function pointer based on argument count */
  iotc_event_handle_argc_t handle_type;
#if IOTC_DEBUG_EXTRA_INFO
  struct {
    int debug_line_init;
    const char* debug_file_init;
    /* TODO:
     * int                     debug_line_last_call;
     * const char*             debug_file_last_call;
     * int                     debug_time_last_call; */
  } debug_info;
#endif
  union {
    struct {
      /* function pointer with 0 arguments */
      iotc_event_handle_func_argc0_ptr fn_argc0;
    } h0;

    struct {
      /* function pointer with 1 argument */
      iotc_event_handle_func_argc1_ptr fn_argc1;
      iotc_event_handle_arg1_t a1;
    } h1;

    struct {
      /* function pointer with 2 arguments */
      iotc_event_handle_func_argc2_ptr fn_argc2;
      iotc_event_handle_arg1_t a1;
      iotc_event_handle_arg2_t a2;
    } h2;

    struct {
      /* function pointer with 3 arguments */
      iotc_event_handle_func_argc3_ptr fn_argc3;
      iotc_event_handle_arg1_t a1;
      iotc_event_handle_arg2_t a2;
      iotc_event_handle_arg3_t a3;
    } h3;

    struct {
      /* function pointer with 4 arguments */
      iotc_event_handle_func_argc4_ptr fn_argc4;
      iotc_event_handle_arg1_t a1;
      iotc_event_handle_arg2_t a2;
      iotc_event_handle_arg3_t a3;
      iotc_event_handle_arg4_t a4;
    } h4;

    struct {
      /* function pointer with 5 arguments */
      iotc_event_handle_func_argc5_ptr fn_argc5;
      iotc_event_handle_arg1_t a1;
      iotc_event_handle_arg2_t a2;
      iotc_event_handle_arg3_t a3;
      iotc_event_handle_arg4_t a4;
      iotc_event_handle_arg5_t a5;
    } h5;

    struct {
      /* function pointer with 6 arguments */
      iotc_event_handle_func_argc6_ptr fn_argc6;
      iotc_event_handle_arg1_t a1;
      iotc_event_handle_arg2_t a2;
      iotc_event_handle_arg3_t a3;
      iotc_event_handle_arg4_t a4;
      iotc_event_handle_arg5_t a5;
      iotc_event_handle_arg6_t a6;
    } h6;
  } handlers;

  uint8_t target_tid;
} iotc_event_handle_t;

#define iotc_make_empty_event_handle(target_tid) \
  { IOTC_EVENT_HANDLE_UNSET, .handlers.h0 = {0}, target_tid }

#ifdef __cplusplus
}
#endif

#endif /* __IOTC_HANDLE_TYPEDEF_H__ */
