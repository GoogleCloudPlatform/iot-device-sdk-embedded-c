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
 * @brief Manage platform memory.
 *
 * These functions customize how the Device SDK manages memory. For 
 * instance, custom implementations can use static memory instead of heap memory
 * or inject metrics to track memory.
 */

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @function
 * @brief Allocate memory.
 *
 * @param [in] byte_count The number of bytes to allocate.
 *
 * @return A pointer to the beginning of the allocated block.
 */
void* iotc_bsp_mem_alloc(size_t byte_count);

/**
 * @function
 * @brief Change the size of a memory block.
 *
 * If the new memory block size is smaller than the old memory block
 * size, the maximum amount of memory is saved, even if the block moves to a new
 * location. If the new size is larger, the leftover memory is empty.
 *
 * @param [in] ptr A pointer to a memory block to reallocate.
 *
 * @return A pointer to the reallocated memory block.
 */
void* iotc_bsp_mem_realloc(void* ptr, size_t byte_count);

/**
 * @function
 * @brief Free a block of memory.
 
 * Free A block of memory that was previously allocated by a call to <code>iotc_bsp_mem_malloc()</code>
 * or <code>iotc_bsp_mem_realloc()</code>.
 *
 * @see iotc_bsp_mem_malloc
 * @see iotc_bsp_mem_realloc
 *
 * @param [in] ptr A pointer to a memory block to free.
 */
void iotc_bsp_mem_free(void* ptr);

#ifdef __cplusplus
}
#endif

#endif /* __IOTC_BSP_MEM_H__ */
