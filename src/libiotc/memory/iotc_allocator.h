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

#ifndef __IOTC_PLATFORM_ALLOCATOR_H__
#define __IOTC_PLATFORM_ALLOCATOR_H__

#include <stdlib.h>

#include "iotc_config.h"

#ifdef IOTC_MEMORY_LIMITER_ENABLED
#include "iotc_memory_limiter.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Allocates the number of bytes given by the b parameter.
 * @param b The number of bytes to allocate.
 * @return Ptr to allocated memory or NULL if there was not enough memory.
 * @note Newly allocated memory is unspecified (use memset in order to set it to
 * some value.)
 */
extern void* __iotc_alloc(size_t b);

/**
 * @brief Allocates the number of elements given by the num parameter of size
 * described by size argument.
 *
 * It reduces to alloc( num * size );
 *
 * @param num
 * @param size
 */
extern void* __iotc_calloc(size_t num, size_t size);

/**
 * @brief Changes the size of already allocated memory.
 *
 * If there is no more memory left it will return NULL
 * it may return different ptr than ptr
 * min( b, old_size ) is the space that suppose to be remain and the newly
 * allocated memory is unspecified.
 *
 * @param ptr
 * @param b
 */
extern void* __iotc_realloc(void* ptr, size_t b);

/**
 * @brief Fees previously allocated memory.
 * @param Ptr to allocated memory buffer may be equall to NULL.
 */
extern void __iotc_free(void* ptr);

/**
 * @brief Macro to make thin facade for debug and memory limiting.
 */
#ifdef IOTC_MEMORY_LIMITER_ENABLED
#define iotc_alloc(b) \
  iotc_memory_limiter_alloc_application(b, __FILE__, __LINE__)
#else
#define iotc_alloc __iotc_alloc
#endif

#ifdef IOTC_MEMORY_LIMITER_ENABLED
#define iotc_calloc(num, byte_count) \
  iotc_memory_limiter_calloc_application(num, byte_count, __FILE__, __LINE__)
#else
#define iotc_calloc __iotc_calloc
#endif

/**
 * @brief Macro to make thin facade for debug and memory limiting.
 */
#ifdef IOTC_MEMORY_LIMITER_ENABLED
#define iotc_realloc(p, b) \
  iotc_memory_limiter_realloc_application(p, b, __FILE__, __LINE__)
#else
#define iotc_realloc __iotc_realloc
#endif

/**
 * @brief Macro to make thin facade for debug and memory limiting.
 */
#ifdef IOTC_MEMORY_LIMITER_ENABLED
#define iotc_free(p) iotc_memory_limiter_free(p)
#else
#define iotc_free __iotc_free
#endif

/* For system allocations. */
#ifdef IOTC_MEMORY_LIMITER_ENABLED
#define iotc_alloc_system(b) \
  iotc_memory_limiter_alloc_system(b, __FILE__, __LINE__)
#else
#define iotc_alloc_system(b) __iotc_alloc(b)
#endif

#ifdef IOTC_MEMORY_LIMITER_ENABLED
#define iotc_calloc_system(num, byte_count) \
  iotc_memory_limiter_calloc_system(num, byte_count, __FILE__, __LINE__)
#else
#define iotc_calloc_system(num, byte_count) __iotc_calloc(num, byte_count)
#endif

#ifdef IOTC_MEMORY_LIMITER_ENABLED
#define iotc_realloc_system(b) \
  iotc_memory_limiter_realloc_system(b, __FILE__, __LINE__)
#else
#define iotc_realloc_system(b) __iotc_realloc(b)
#endif

/* For exposing ptr's. */
#ifdef IOTC_MEMORY_LIMITER_ENABLED
#define iotc_alloc_ptr &iotc_memory_limiter_alloc_application_export
#else
#define iotc_alloc_ptr &__iotc_alloc
#endif

#ifdef IOTC_MEMORY_LIMITER_ENABLED
#define iotc_calloc_ptr &iotc_memory_limiter_calloc_application_export
#else
#define iotc_calloc_ptr &__iotc_calloc
#endif

#ifdef IOTC_MEMORY_LIMITER_ENABLED
#define iotc_realloc_ptr &iotc_memory_limiter_realloc_application_export
#else
#define iotc_realloc_ptr &__iotc_realloc
#endif

#ifdef IOTC_MEMORY_LIMITER_ENABLED
#define iotc_free_ptr &iotc_memory_limiter_free
#else
#define iotc_free_ptr &__iotc_free
#endif

#ifdef __cplusplus
}
#endif

#endif /* __IOTC_PLATFORM_ALLOCATOR_H__ */
