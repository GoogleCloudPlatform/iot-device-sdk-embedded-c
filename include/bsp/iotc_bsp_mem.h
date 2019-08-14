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

#ifndef __IOTC_BSP_MEM_H__
#define __IOTC_BSP_MEM_H__

/**
 * @file iotc_bsp_mem.h
 * @brief Manages platform memory.
 * 
 * @details For instance, custom implementations can use static instead of
 * heap memory.
 */

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Allocates memory and returns a pointer to the allocated block.
 *
 * @param [in] byte_count The number of bytes to allocate.
 */
void* iotc_bsp_mem_alloc(size_t byte_count);

/**
 * @brief Changes the size of a memory block and returns a pointer to the
 * reallocated block.
 *
 * @details If the new memory block size is smaller than the old memory block,
 * the SDK saves the maximum amount of memory, even if the block moves to a new
 * location. If the new size is larger, the Device SDK leaves the leftover
 * memory empty.
 *
 * @param [in] ptr A pointer to a memory block to reallocate.
 */
void* iotc_bsp_mem_realloc(void* ptr, size_t byte_count);

/**
 * @brief Frees a block of memory.
 *
 * @param [in] ptr A pointer to a memory block to free.
 */
void iotc_bsp_mem_free(void* ptr);

#ifdef __cplusplus
}
#endif

#endif /* __IOTC_BSP_MEM_H__ */
