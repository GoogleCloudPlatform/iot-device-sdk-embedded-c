/* Copyright 2018 - 2019 Google LLC
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

#ifndef __IOTC_MEMORY_LIMITER_H__
#define __IOTC_MEMORY_LIMITER_H__

#include <stddef.h>

#include "iotc_err.h"

#ifdef __cplusplus
extern "C" {
#endif

#if IOTC_PLATFORM_BASE_POSIX
#define MAX_BACKTRACE_SYMBOLS 16
#endif

/**
 * internal structure for keeping the mapping between
 * the address of the allocated memory and the amount of the
 * memory that has been allocated
 */
typedef struct iotc_memory_limiter_entry_s {
#if IOTC_DEBUG_EXTRA_INFO
  struct iotc_memory_limiter_entry_s* next;
  struct iotc_memory_limiter_entry_s* prev;
  const char* allocation_origin_file_name;
#ifdef IOTC_PLATFORM_BASE_POSIX
  void* backtrace_symbols_buffer[MAX_BACKTRACE_SYMBOLS];
  int backtrace_symbols_buffer_size;
#endif
  size_t allocation_origin_line_number;
#endif
  size_t size;
} iotc_memory_limiter_entry_t;

/**
 * types of memory alloations handled by the memory limiter
 *
 * application is the most common across the library. This is memory that is
 * used to faciliate applicaiton requests for (de)serializing and encoding data.
 *
 * system shares allocation space with application, but has a extra amount of
 * reserved space. Only IoTC mechanisms for scheduling and tracking tasks should
 * use system allocations, so that we can ensure that we have enough memory to
 * schedule tasks that unravel any currently running layer stacks when a
 * application memory exhaustion event occurs.
 */
typedef enum {
  IOTC_MEMORY_LIMITER_ALLOCATION_TYPE_APPLICATION = 0,
  IOTC_MEMORY_LIMITER_ALLOCATION_TYPE_SYSTEM,
  IOTC_MEMORY_LIMITER_ALLOCATION_TYPE_COUNT
} iotc_memory_limiter_allocation_type_t;

void* iotc_memory_limiter_alloc(
    iotc_memory_limiter_allocation_type_t limit_type, size_t size_to_alloc,
    const char* file, size_t line);

void* iotc_memory_limiter_calloc(
    iotc_memory_limiter_allocation_type_t limit_type, size_t num,
    size_t size_to_alloc, const char* file, size_t line);

void* iotc_memory_limiter_realloc(
    iotc_memory_limiter_allocation_type_t limit_type, void* ptr,
    size_t size_to_alloc, const char* file, size_t line);

/**
 * @brief sets the limit of the memory limiter
 */
extern iotc_state_t iotc_memory_limiter_set_limit(
    const size_t new_memory_limit);

/**
 * @brief allows to get the limit
 */
extern size_t iotc_memory_limiter_get_capacity(
    iotc_memory_limiter_allocation_type_t limit_type);

/**
 * @brief allows to get the main limit
 */
extern size_t iotc_memory_limiter_get_current_limit(
    iotc_memory_limiter_allocation_type_t limit_type);

/**
 * @brief iotc_memory_limiter_get_allocated_space
 */
extern size_t iotc_memory_limiter_get_allocated_space();

/**
 * @brief simulates free operation on memory block it just re-add the memory to
 * the pool it will
 *          check if the pool hasn't been exceeded
 */
extern void iotc_memory_limiter_free(void* ptr);

/**
 * @brief fasade for allocing in application memory space
 */
extern void* iotc_memory_limiter_alloc_application(size_t size_to_alloc,
                                                   const char* file,
                                                   size_t line);

/*
 * @brief fasade for an (c)allocation in application memory space
 */
extern void* iotc_memory_limiter_calloc_application(size_t num,
                                                    size_t size_to_alloc,
                                                    const char* file,
                                                    size_t line);

/**
 * @brief fasade for realloc in application memory space
 */
extern void* iotc_memory_limiter_realloc_application(void* ptr,
                                                     size_t size_to_alloc,
                                                     const char* file,
                                                     size_t line);

/**
 * @brief fasade for alloc in system memory space
 */
extern void* iotc_memory_limiter_alloc_system(size_t size_to_alloc,
                                              const char* file, size_t line);

/**
 * @brief fasade for alloc in system memory space
 */
extern void* iotc_memory_limiter_calloc_system(size_t num, size_t size_to_alloc,
                                               const char* file, size_t line);

/**
 * @brief fasade for realloc in system memory space
 */
extern void* iotc_memory_limiter_realloc_system(void* ptr, size_t size_to_alloc,
                                                const char* file, size_t line);

/**
 * @brief iotc_memory_limiter_alloc_application_export
 *
 * Fasade for allocating memory in application memory space in a form of wrapper
 * for outside usage.
 *
 * @see iotc_memory_limiter_alloc_application
 *
 * @param size_to_alloc
 * @return
 */
extern void* iotc_memory_limiter_alloc_application_export(size_t size_to_alloc);

/**
 * @brief iotc_memory_limiter_calloc_application_export
 *
 * Fasade for (c)allocating memory in application memory space in a form of
 * wrapper for outside usage.
 *
 * @see iotc_memory_limiter_calloc_application
 *
 * @param num
 * @param size_to_alloc
 * @return
 */
extern void* iotc_memory_limiter_calloc_application_export(
    size_t num, size_t size_to_alloc);

/**
 * @brief iotc_memory_limiter_realloc_application_export
 *
 * Fasade for re-allocating memory in application memory space in a form of
 * wrapper for outside usage.
 *
 * @see iotc_memory_limiter_realloc_application
 *
 * @param ptr
 * @param size_to_alloc
 * @return
 */
extern void* iotc_memory_limiter_realloc_application_export(
    void* ptr, size_t size_to_alloc);

#if IOTC_DEBUG_EXTRA_INFO

/**
 * @brief it does the garbage collection run ( only if the IOTC_DEBUG_EXTRA_INFO
 * flag is turned ON )
 */
extern void iotc_memory_limiter_gc();

/**
 * @brief iotc_debug_log_leaks
 */
extern void iotc_memory_limiter_visit_memory_leaks(
    void (*visitor_fn)(const iotc_memory_limiter_entry_t*));

/**
 * @brief iotc_debug_log_leaks
 */
extern void iotc_memory_limiter_print_memory_leaks();

#endif /* IOTC_DEBUG_EXTRA_INFO */

#ifdef __cplusplus
}
#endif

#endif /* __IOTC_MEMORY_LIMITER_H__ */
