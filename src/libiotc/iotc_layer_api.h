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

#ifndef __IOTC_LAYER_API_H__
#define __IOTC_LAYER_API_H__

#include "iotc_config.h"
#include "iotc_event_dispatcher_api.h"
#include "iotc_layer.h"

#ifdef __cplusplus
extern "C" {
#endif

#define IOTC_THIS_LAYER(context) ((iotc_layer_connectivity_t*)context)->self
#define IOTC_THIS_LAYER_STATE(context) IOTC_THIS_LAYER(context)->layer_state
#define IOTC_THIS_LAYER_NOT_OPERATIONAL(context)                 \
  (IOTC_THIS_LAYER_STATE(context) == IOTC_LAYER_STATE_CLOSING || \
   IOTC_THIS_LAYER_STATE(context) == IOTC_LAYER_STATE_CLOSED)
#define IOTC_CONTEXT_DATA(context) IOTC_THIS_LAYER(context)->context_data
#define IOTC_THIS_LAYER_STATE_UPDATE(context, state) \
  IOTC_THIS_LAYER_STATE(context) = state

#if IOTC_DEBUG_EXTRA_INFO
#define IOTC_LAYERS_CONNECT(lp_i, ln_i)           \
  ln_i->layer_connection.prev = lp_i;             \
  lp_i->layer_connection.next = ln_i;             \
  lp_i->debug_info.debug_line_connect = __LINE__; \
  lp_i->debug_info.debug_file_connect = __FILE__; \
  ln_i->debug_info.debug_line_connect = __LINE__; \
  ln_i->debug_info.debug_file_connect = __FILE__;
#else
#define IOTC_LAYERS_CONNECT(lp_i, ln_i) \
  ln_i->layer_connection.prev = lp_i;   \
  lp_i->layer_connection.next = ln_i
#endif

#if IOTC_DEBUG_EXTRA_INFO
#define IOTC_LAYERS_DISCONNECT(lp_i, ln_i)        \
  ln_i->layer_connection.prev = 0;                \
  lp_i->layer_connection.next = 0;                \
  lp_i->debug_info.debug_line_connect = __LINE__; \
  lp_i->debug_info.debug_file_connect = __FILE__; \
  ln_i->debug_info.debug_line_connect = __LINE__; \
  ln_i->debug_info.debug_file_connect = __FILE__;
#else
#define IOTC_LAYERS_DISCONNECT(lp_i, ln_i) \
  ln_i->layer_connection.prev = 0;         \
  lp_i->layer_connection.next = 0
#endif

#if IOTC_DEBUG_EXTRA_INFO
extern iotc_state_t iotc_layer_continue_with_impl(
    iotc_layer_func_t* f, iotc_layer_connectivity_t* from_context,
    iotc_layer_connectivity_t* context, void* data, iotc_state_t state,
    const char* file_name, const int line_no);

#define IOTC_LAYER_CONTINUE_WITH(layer, target, context, data, state) \
  iotc_layer_continue_with_impl(                                      \
      ((NULL == context->layer_connection.layer)                      \
           ? (NULL)                                                   \
           : (context->layer_connection.layer->layer_funcs->target)), \
      &context->layer_connection,                                     \
      ((NULL == context->layer_connection.layer)                      \
           ? (NULL)                                                   \
           : (&context->layer_connection.layer->layer_connection)),   \
      data, state, __FILE__, __LINE__)
#else /* IOTC_DEBUG_EXTRA_INFO */
extern iotc_state_t iotc_layer_continue_with_impl(
    iotc_layer_func_t* f, iotc_layer_connectivity_t* from_context,
    iotc_layer_connectivity_t* context, void* data, iotc_state_t state);

#define IOTC_LAYER_CONTINUE_WITH(layer, target, context, data, state) \
  iotc_layer_continue_with_impl(                                      \
      ((NULL == context->layer_connection.layer)                      \
           ? (NULL)                                                   \
           : (context->layer_connection.layer->layer_funcs->target)), \
      &context->layer_connection,                                     \
      ((NULL == context->layer_connection.layer)                      \
           ? (NULL)                                                   \
           : (&context->layer_connection.layer->layer_connection)),   \
      data, state)

#endif /* IOTC_DEBUG_EXTRA_INFO */

/* ON_DEMAND */
#define IOTC_PROCESS_PUSH_ON_THIS_LAYER(context, data, state) \
  IOTC_LAYER_CONTINUE_WITH(self, push, IOTC_THIS_LAYER(context), data, state)

#define IOTC_PROCESS_PUSH_ON_NEXT_LAYER(context, data, state) \
  IOTC_LAYER_CONTINUE_WITH(next, push, IOTC_THIS_LAYER(context), data, state)

#define IOTC_PROCESS_PUSH_ON_PREV_LAYER(context, data, state) \
  IOTC_LAYER_CONTINUE_WITH(prev, push, IOTC_THIS_LAYER(context), data, state)

/* ON_PUSHING */
#define IOTC_PROCESS_PULL_ON_THIS_LAYER(context, data, state) \
  IOTC_LAYER_CONTINUE_WITH(self, pull, IOTC_THIS_LAYER(context), data, state)

#define IOTC_PROCESS_PULL_ON_NEXT_LAYER(context, data, state) \
  IOTC_LAYER_CONTINUE_WITH(next, pull, IOTC_THIS_LAYER(context), data, state)

#define IOTC_PROCESS_PULL_ON_PREV_LAYER(context, data, state) \
  IOTC_LAYER_CONTINUE_WITH(prev, pull, IOTC_THIS_LAYER(context), data, state)

/* CLOSING */
#define IOTC_PROCESS_CLOSE_ON_THIS_LAYER(context, data, state) \
  IOTC_LAYER_CONTINUE_WITH(self, close, IOTC_THIS_LAYER(context), data, state)

#define IOTC_PROCESS_CLOSE_ON_NEXT_LAYER(context, data, state) \
  IOTC_LAYER_CONTINUE_WITH(next, close, IOTC_THIS_LAYER(context), data, state)

#define IOTC_PROCESS_CLOSE_ON_PREV_LAYER(context, data, state) \
  IOTC_LAYER_CONTINUE_WITH(prev, close, IOTC_THIS_LAYER(context), data, state)

/* CLOSING_EXTERNALLY */
#define IOTC_PROCESS_CLOSE_EXTERNALLY_ON_THIS_LAYER(context, data, state)    \
  IOTC_LAYER_CONTINUE_WITH(self, close_externally, IOTC_THIS_LAYER(context), \
                           data, state)

#define IOTC_PROCESS_CLOSE_EXTERNALLY_ON_NEXT_LAYER(context, data, state)    \
  IOTC_LAYER_CONTINUE_WITH(next, close_externally, IOTC_THIS_LAYER(context), \
                           data, state)

#define IOTC_PROCESS_CLOSE_EXTERNALLY_ON_PREV_LAYER(context, data, state)    \
  IOTC_LAYER_CONTINUE_WITH(prev, close_externally, IOTC_THIS_LAYER(context), \
                           data, state)

/* INIT */
#define IOTC_PROCESS_INIT_ON_THIS_LAYER(context, data, state) \
  IOTC_LAYER_CONTINUE_WITH(self, init, IOTC_THIS_LAYER(context), data, state)

#define IOTC_PROCESS_INIT_ON_NEXT_LAYER(context, data, state) \
  IOTC_LAYER_CONTINUE_WITH(next, init, IOTC_THIS_LAYER(context), data, state)

#define IOTC_PROCESS_INIT_ON_PREV_LAYER(context, data, state) \
  IOTC_LAYER_CONTINUE_WITH(prev, init, IOTC_THIS_LAYER(context), data, state)

/* CONNECT */
#define IOTC_PROCESS_CONNECT_ON_THIS_LAYER(context, data, state)          \
  IOTC_LAYER_CONTINUE_WITH(self, connect, IOTC_THIS_LAYER(context), data, \
                           state);

#define IOTC_PROCESS_CONNECT_ON_NEXT_LAYER(context, data, state) \
  IOTC_LAYER_CONTINUE_WITH(next, connect, IOTC_THIS_LAYER(context), data, state)

#define IOTC_PROCESS_CONNECT_ON_PREV_LAYER(context, data, state) \
  IOTC_LAYER_CONTINUE_WITH(prev, connect, IOTC_THIS_LAYER(context), data, state)

/* POST-CONNECT */
#define IOTC_PROCESS_POST_CONNECT_ON_THIS_LAYER(context, data, state)          \
  IOTC_LAYER_CONTINUE_WITH(self, post_connect, IOTC_THIS_LAYER(context), data, \
                           state);

#define IOTC_PROCESS_POST_CONNECT_ON_NEXT_LAYER(context, data, state)          \
  IOTC_LAYER_CONTINUE_WITH(next, post_connect, IOTC_THIS_LAYER(context), data, \
                           state)

#define IOTC_PROCESS_POST_CONNECT_ON_PREV_LAYER(context, data, state)          \
  IOTC_LAYER_CONTINUE_WITH(prev, post_connect, IOTC_THIS_LAYER(context), data, \
                           state)

#ifdef __cplusplus
}
#endif

#endif /* __IOTC_LAYER_API_H__ */
