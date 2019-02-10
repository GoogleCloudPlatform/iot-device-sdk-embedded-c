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

#ifndef __IOTC_BSP_DEBUG__
#define __IOTC_BSP_DEBUG__

#define BSP_DEBUG_LOG 0

#ifndef IOTC_DEBUG_PRINTF
#include <stdio.h>
#define __iotc_printf(...) \
  printf(__VA_ARGS__);     \
  fflush(stdout)
#else /* IOTC_DEBUG_PRINTF */
#define __iotc_printf(...) IOTC_DEBUG_PRINTF(__VA_ARGS__);
#endif /* IOTC_DEBUG_PRINTF */

#if BSP_DEBUG_LOG
#define iotc_bsp_debug_logger(format_string) \
  __iotc_printf("%s@%d[  BSP  ] " format_string "\n", __FILE__, __LINE__)
#define iotc_bsp_debug_format(format_string, ...)                         \
  __iotc_printf("%s@%d[  BSP  ] " format_string "\n", __FILE__, __LINE__, \
                __VA_ARGS__)
#else /* BSP_DEBUG_LOG */
#define iotc_bsp_debug_logger(...)
#define iotc_bsp_debug_format(...)
#endif /* BSP_DEBUG_LOG */

#endif /*  __IOTC_BSP_DEBUG__ */
