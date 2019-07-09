/* Copyright 2018-2019 Google LLC
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

#ifndef __IOTC_BSP_RNG_H__
#define __IOTC_BSP_RNG_H__

/**
 * @file iotc_bsp_rng.h
 * @brief Generates random numbers.
 *
 * @details Always implement an entropy-based hardware RNG;
 * don't hardcode the same seed value for all devices.
 * 
 * The Device SDK prevents DDoS attacks by randomly distributing
 * fleet-wide reconnection attempts, so predictable numbers may corrupt the
 * backoff system.
 */

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initializes the platform-specific RNG prerequisites.
 *
 * The Device SDK calls this function before requesting the first random number.
 */
void iotc_bsp_rng_init();

/**
 * @brief Generates a random, 32-bit integer.
 */
uint32_t iotc_bsp_rng_get();

/**
 * @brief Shuts down the platform-specific RNG and frees all allocated
 * resources.
 */
void iotc_bsp_rng_shutdown();

#ifdef __cplusplus
}
#endif

#endif /* __IOTC_BSP_RNG_H__ */
