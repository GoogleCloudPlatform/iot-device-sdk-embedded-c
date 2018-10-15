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

#ifndef __IOTC_EVENT_DISPATCHER_MACROS_H__
#define __IOTC_EVENT_DISPATCHER_MACROS_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define IOTC_EVTD_DECLARE_1_EV(e1) +1, e1##_ID = 1, e1 = 1 << 1
#define IOTC_EVTD_DECLARE_2_EV(e1, e2) \
  +1 IOTC_EVTD_DECLARE_1_EV(e1), e2##_ID = 2, e2 = 1 << 2
#define IOTC_EVTD_DECLARE_3_EV(e1, e2, e3) \
  +1 IOTC_EVTD_DECLARE_2_EV(e1, e2), e3##_ID = 3, e3 = 1 << 3
#define IOTC_EVTD_DECLARE_4_EV(e1, e2, e3, e4) \
  +1 IOTC_EVTD_DECLARE_3_EV(e1, e2, e3), e4##_ID = 4, e4 = 1 << 4
#define IOTC_EVTD_DECLARE_5_EV(e1, e2, e3, e4, e5) \
  +1 IOTC_EVTD_DECLARE_4_EV(e1, e2, e3, e4), e5##_ID = 5, e5 = 1 << 5
#define IOTC_EVTD_DECLARE_6_EV(e1, e2, e3, e4, e5, e6) \
  +1 IOTC_EVTD_DECLARE_5_EV(e1, e2, e3, e4, e5), e5##_ID = 6, e6 = 1 << 6

#define IOTC_EVTD_GENERATE_EVENT_TYPE_ENUM(count, ...)              \
  typedef enum evts {                                               \
    IOTC_EVTD_NO_EVENT = 1 << 0,                                    \
    IOTC_EVTD_NO_EVENT_ID = 0,                                      \
    IOTC_EVTD_COUNT = 1 IOTC_EVTD_DECLARE_##count##_EV(__VA_ARGS__) \
  } iotc_event_type_t;

#define IOTC_EVTD_GENERATE_EVENT_HANDLE_TYPEDEFS(ret, arg1, arg2, arg3, arg4, \
                                                 arg5, arg6)                  \
  typedef ret iotc_event_handle_return_t;                                     \
  typedef arg1 iotc_event_handle_arg1_t;                                      \
  typedef arg2 iotc_event_handle_arg2_t;                                      \
  typedef arg3 iotc_event_handle_arg3_t;                                      \
  typedef arg4 iotc_event_handle_arg4_t;                                      \
  typedef arg5 iotc_event_handle_arg5_t;                                      \
  typedef arg6 iotc_event_handle_arg6_t;                                      \
                                                                              \
  typedef iotc_event_handle_return_t(iotc_event_handle_func_argc0)(void);     \
  typedef iotc_event_handle_func_argc0* iotc_event_handle_func_argc0_ptr;     \
                                                                              \
  typedef iotc_event_handle_return_t(iotc_event_handle_func_argc1)(           \
      iotc_event_handle_arg1_t);                                              \
  typedef iotc_event_handle_func_argc1* iotc_event_handle_func_argc1_ptr;     \
                                                                              \
  typedef iotc_event_handle_return_t(iotc_event_handle_func_argc2)(           \
      iotc_event_handle_arg1_t, iotc_event_handle_arg2_t);                    \
  typedef iotc_event_handle_func_argc2* iotc_event_handle_func_argc2_ptr;     \
                                                                              \
  typedef iotc_event_handle_return_t(iotc_event_handle_func_argc3)(           \
      iotc_event_handle_arg1_t, iotc_event_handle_arg2_t,                     \
      iotc_event_handle_arg3_t);                                              \
  typedef iotc_event_handle_func_argc3* iotc_event_handle_func_argc3_ptr;     \
                                                                              \
  typedef iotc_event_handle_return_t(iotc_event_handle_func_argc4)(           \
      iotc_event_handle_arg1_t, iotc_event_handle_arg2_t,                     \
      iotc_event_handle_arg3_t, iotc_event_handle_arg4_t);                    \
  typedef iotc_event_handle_func_argc4* iotc_event_handle_func_argc4_ptr;     \
                                                                              \
  typedef iotc_event_handle_return_t(iotc_event_handle_func_argc5)(           \
      iotc_event_handle_arg1_t, iotc_event_handle_arg2_t,                     \
      iotc_event_handle_arg3_t, iotc_event_handle_arg4_t,                     \
      iotc_event_handle_arg5_t);                                              \
  typedef iotc_event_handle_func_argc5* iotc_event_handle_func_argc5_ptr;     \
                                                                              \
  typedef iotc_event_handle_return_t(iotc_event_handle_func_argc6)(           \
      iotc_event_handle_arg1_t, iotc_event_handle_arg2_t,                     \
      iotc_event_handle_arg3_t, iotc_event_handle_arg4_t,                     \
      iotc_event_handle_arg5_t, iotc_event_handle_arg6_t);                    \
  typedef iotc_event_handle_func_argc6* iotc_event_handle_func_argc6_ptr;
#ifdef __cplusplus
}
#endif

#endif /* __IOTC_EVENT_DISPATCHER_MACROS_H__ */
