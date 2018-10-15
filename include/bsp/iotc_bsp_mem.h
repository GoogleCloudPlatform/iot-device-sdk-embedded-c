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

#ifndef __IOTC_BSP_MEM_H__
#define __IOTC_BSP_MEM_H__

/**
 * @file iotc_bsp_mem.h
 * @brief IoTC Client's Board Support Platform (BSP) for Memory Management
 *
 * This file defines the Memory Management API used by the IoTC Client.
 *
 * The IoTC Client manages memory through these functions. By implementing these
 * functions the application can customize memory management of the IoTC Client.
 * For instance a custom implementation can use static memory on devices where
 * heap isn't desired. Another use case might be memory tracking by injecting
 * metrics into the implementation.
 */

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @function
 * @brief Allocates a block of byte_count bytes of memory, returns a pointer to
 *        the beginning of the block.
 *
 * There are no requirements against the content of the memory block returned.
 *
 * @param [in] byte_count the number of bytes to allocate
 * @return the beginning of the allocated block
 */
void* iotc_bsp_mem_alloc(size_t byte_count);

/**
 * @function
 * @brief Changes the size of the memory block pointed to by ptr.
 *
 * The content of the memory block is preserved up to the lesser of the new and
 * old sizes, even if the block is moved to a new location. If the new size is
 * larger, the value of the newly allocated portion is indeterminate.
 *
 * @param [in] ptr pointer to a memory block, this memory block will be
 * reallocated
 * @return a pointer to the reallocated memory block
 */
void* iotc_bsp_mem_realloc(void* ptr, size_t byte_count);

/**
 * @function
 * @brief A block of memory previously allocated by a call to
 * iotc_bsp_mem_malloc, iotc_bsp_mem_realloc is deallocated, making it available
 * again for further allocations.
 *
 * @param [in] ptr pointer to a memory block
 */
void iotc_bsp_mem_free(void* ptr);

#ifdef __cplusplus
}
#endif

#endif /* __IOTC_BSP_MEM_H__ */
