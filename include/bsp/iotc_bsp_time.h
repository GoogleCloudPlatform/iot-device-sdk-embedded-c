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

#ifndef __IOTC_BSP_TIME_H__
#define __IOTC_BSP_TIME_H__

/**
 * @file iotc_bsp_time.h
 * @brief IoTC Client's Board Support Platform (BSP) for Time Functions
 *
 * This file defines the Time Functions API used by the IoTC Client.
 *
 * The IoTC Client determines time using these functions. These time functions
 * drive the exact execution time of scheduled tasks and the delayed
 * reconnection logic (aka Backoff) which prevents clients from overloading
 * the Google Cloud IoT Core Server. The implementation should use hardware
 * or NTP servers in order to maintain an accurate clock.
 */

#include <iotc_time.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @function
 * @brief Initialization of BSP TIME module.
 *
 * This function should contain initialization code to fire up time
 * functionality on the target platform. For POSIX systems this usually
 * remains an empty function as a time system is already running, and has
 * accurate time. For microcontrollers, connecting to an NTP service or
 * initializing time-step functionality are examples of the code that
 * could reside here.
 */
void iotc_bsp_time_init();

/**
 * @function
 * @brief Returns elapsed seconds since Epoch.
 */
iotc_time_t iotc_bsp_time_getcurrenttime_seconds();

/**
 * @function
 * @brief Returns elapsed milliseconds since Epoch.
 */
iotc_time_t iotc_bsp_time_getcurrenttime_milliseconds();

/**
 * @function
 * @brief Returns current monotonic time in microseconds.
 */
iotc_time_t iotc_bsp_time_getmonotonictime_microseconds();

#ifdef __cplusplus
}
#endif

#endif /* __IOTC_BSP_TIME_H__ */
