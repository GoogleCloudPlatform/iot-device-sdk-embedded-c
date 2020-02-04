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

/**
 * \file    iotc_debug.h
 * \brief   Macros to use for debugging
 */

#ifndef __IOTC_DEBUG_H__
#define __IOTC_DEBUG_H__

#include "iotc_bsp_time.h"
#include "iotc_config.h"
#include "iotc_data_desc.h"
#include <stdio.h>

#ifdef IOTC_PLATFORM_BASE_WMSDK
#include <wm_os.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifndef IOTC_DEBUG_PRINTF
#define __iotc_printf(...) \
  printf(__VA_ARGS__);     \
  fflush(stdout);
#else /* IOTC_DEBUG_PRINTF */
#define __iotc_printf(...) IOTC_DEBUG_PRINTF(__VA_ARGS__);
#endif /* IOTC_DEBUG_PRINTF */

#if IOTC_DEBUG_OUTPUT
const char* iotc_debug_dont_print_the_path(const char* msg);
void iotc_debug_data_logger_impl(const char* msg,
                                 const iotc_data_desc_t* data_desc);

#define iotc_debug_logger(msg)                                               \
  __iotc_printf(                                                             \
      "[%lld][%s:%d (%s)] %s\n", iotc_bsp_time_getcurrenttime_milliseconds(), \
      iotc_debug_dont_print_the_path(__FILE__), __LINE__, __func__, msg)
#define iotc_debug_format(fmt, ...)                                           \
  __iotc_printf("[%lld][%s:%d (%s)] " fmt "\n",                                \
                iotc_bsp_time_getcurrenttime_milliseconds(),                  \
                iotc_debug_dont_print_the_path(__FILE__), __LINE__, __func__, \
                __VA_ARGS__)
#define iotc_debug_printf(...) __iotc_printf(__VA_ARGS__)
#define iotc_debug_function_entered()                        \
  __iotc_printf("[%lld][%s:%d (%s)] -> entered\n",            \
                iotc_bsp_time_getcurrenttime_milliseconds(), \
                iotc_debug_dont_print_the_path(__FILE__), __LINE__, __func__)
#define iotc_debug_data_logger(msg, dsc)                                       \
  __iotc_printf("[%lld][%s:%d (%s)] #\n",                                       \
                iotc_bsp_time_getcurrenttime_milliseconds(),                   \
                iotc_debug_dont_print_the_path(__FILE__), __LINE__, __func__); \
  iotc_debug_data_logger_impl(msg, dsc)
#else /* IOTC_DEBUG_OUTPUT */
#define iotc_debug_logger(...)
#define iotc_debug_format(...)
#define iotc_debug_printf(...)
#define iotc_debug_function_entered()
#define iotc_debug_data_logger(...)
#endif /* IOTC_DEBUG_OUTPUT */

#define IOTC_LAYER_FUNCTION_PRINT_FUNCTION_DIGEST()
#define IOTC_LAYER_FUNCTION_PRINT_FUNCTION_DIGEST_OFF()      \
  __iotc_printf(                                             \
      "[%lld] %-50s, layerchainid = %p, in_out_state = %d, "  \
      "layer_type_id = "                                     \
      "%d, data = %p\n",                                     \
      iotc_bsp_time_getcurrenttime_milliseconds(), __func__, \
      IOTC_THIS_LAYER(context)->context_data, in_out_state,  \
      IOTC_THIS_LAYER(context)->layer_type_id, data)

#if IOTC_DEBUG_ASSERT
#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>
#else /* IOTC_DEBUG_ASSERT */
/* The actual header is missing in some toolchains, so we wrap it here. */
#ifndef assert
#define assert(e) ((void)0)
#endif /* assert */
#endif /* IOTC_DEBUG_ASSERT */

#ifdef __cplusplus
}
#endif

#endif /* __IOTC_DEBUG_H__ */
