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

#ifndef __IOTC_BSP_RNG_H__
#define __IOTC_BSP_RNG_H__

/**
 * @file iotc_bsp_rng.h
 * @brief IoTC Client's Board Support Platform (BSP) for Random Number Generator
 *
 * This file defines the Random Number Generator API needed by the IoTC Client.
 *
 * The IoTC Client calls the init function before requesting any random number.
 * The iotc_bsp_rng_get function's responsibility is to provide a random number
 * each time it is called. The shutdown function will be called in case no
 * further random numbers are required by the library. This happens at library
 * shutdown.
 *
 * Note that the values returned here should be as random as possible -- the
 * BackOff system is dependent on these values to prevent accidental DDoS
 * attacks by randomly distributing fleet-wide reconnection attempts.
 * Please do not hardcode a seed value for all of your devices, and please,
 * whenever possible, use a entropy-based hardware random number generator.
 */

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @function
 * @brief Initializes the platform specific RNG prerequisites.
 *
 * Called before requesting the first random number. Implementations should
 * perform all initialization actions in this function (generate random seed,
 * create any structures, etc.) required by the platform specific random number
 * generation.
 */
void iotc_bsp_rng_init();

/**
 * @function
 * @brief Generates new 32bit random number.
 *
 * Called whenever a random number is required by the IoTC Client.
 *
 * @return 32bit random integer
 */
uint32_t iotc_bsp_rng_get();

/**
 * @function
 * @brief Shuts down the platform specific RNG mechanism.
 *
 * Called when no further random number is required by the IoTC Client. Takes
 * place during the shut down process of the IoTC Client itself.
 */
void iotc_bsp_rng_shutdown();

#ifdef __cplusplus
}
#endif

#endif /* __IOTC_BSP_RNG_H__ */
