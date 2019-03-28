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

#include <stdint.h>

#include "iotc_critical_section.h"
#include "iotc_critical_section_def.h"
#include "iotc_debug.h"
#include "iotc_helpers.h"
#include "iotc_macros.h"
#include "iotc_memory_limiter.h"

#ifdef IOTC_PLATFORM_BASE_POSIX
#include <execinfo.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define get_ptr_from_entry(e) \
  (void*)((intptr_t)e + sizeof(iotc_memory_limiter_entry_t))

#define get_entry_from_ptr(p)                  \
  (iotc_memory_limiter_entry_t*)((intptr_t)p - \
                                 sizeof(iotc_memory_limiter_entry_t))

#define get_ptr_from_entry_const(e) (intptr_t) get_ptr_from_entry(e)

#define get_entry_from_ptr_const(p) \
  (iotc_memory_limiter_entry_t*)get_entry_from_ptr(p)

#if IOTC_DEBUG_EXTRA_INFO
static iotc_memory_limiter_entry_t* iotc_memory_limiter_entry_list_head;

typedef const iotc_memory_limiter_entry_t*(
    iotc_memory_limiter_entry_list_on_elem_t)(
    const iotc_memory_limiter_entry_t* entry, const void* user_data);

static const iotc_memory_limiter_entry_t*
iotc_memory_limiter_entry_array_same_predicate(
    const iotc_memory_limiter_entry_t* entry, const void* user_data) {
  return entry == user_data ? entry : NULL;
}

static iotc_memory_limiter_entry_t* iotc_memory_limiter_entry_list_visitor(
    iotc_memory_limiter_entry_list_on_elem_t* predicate,
    const void* user_data) {
  iotc_memory_limiter_entry_t* tmp = iotc_memory_limiter_entry_list_head;

  while (NULL != tmp) {
    if ((*predicate)(tmp, user_data) != NULL) {
      return tmp;
    }

    tmp = tmp->next;
  }

  return NULL;
}
#endif

/* static initialisation of the critical section */
static struct iotc_critical_section_s iotc_memory_limiter_cs = {0};

static volatile size_t iotc_memory_application_limit =
    IOTC_MEMORY_LIMITER_APPLICATION_MEMORY_LIMIT;
static volatile size_t iotc_memory_total_limit =
    IOTC_MEMORY_LIMITER_APPLICATION_MEMORY_LIMIT +
    IOTC_MEMORY_LIMITER_SYSTEM_MEMORY_LIMIT;
static volatile size_t iotc_memory_allocated = 0;

static iotc_state_t iotc_memory_limiter_will_allocation_fit(
    iotc_memory_limiter_allocation_type_t memory_type, size_t size_to_alloc) {
  if (iotc_memory_allocated + size_to_alloc >
      (memory_type == IOTC_MEMORY_LIMITER_ALLOCATION_TYPE_APPLICATION
           ? iotc_memory_application_limit
           : iotc_memory_total_limit)) {
    return IOTC_OUT_OF_MEMORY;
  }

  return IOTC_STATE_OK;
}

iotc_state_t iotc_memory_limiter_set_limit(const size_t new_memory_limit) {
  const size_t current_required_capacity =
      iotc_memory_limiter_get_allocated_space() +
      IOTC_MEMORY_LIMITER_SYSTEM_MEMORY_LIMIT;

  if (new_memory_limit < current_required_capacity) {
    return IOTC_OUT_OF_MEMORY;
  }

  iotc_memory_total_limit = new_memory_limit;
  iotc_memory_application_limit =
      iotc_memory_total_limit - IOTC_MEMORY_LIMITER_SYSTEM_MEMORY_LIMIT;

  return IOTC_STATE_OK;
}

size_t iotc_memory_limiter_get_capacity(
    iotc_memory_limiter_allocation_type_t limit_type) {
  assert(limit_type <= IOTC_MEMORY_LIMITER_ALLOCATION_TYPE_COUNT);

  return limit_type == IOTC_MEMORY_LIMITER_ALLOCATION_TYPE_APPLICATION
             ? iotc_memory_application_limit
             : iotc_memory_total_limit;
}

size_t iotc_memory_limiter_get_current_limit(
    iotc_memory_limiter_allocation_type_t limit_type) {
  assert(limit_type <= IOTC_MEMORY_LIMITER_ALLOCATION_TYPE_COUNT);

  const size_t capacity = iotc_memory_limiter_get_capacity(limit_type);
  const size_t allocated_space = iotc_memory_limiter_get_allocated_space();

  if (allocated_space >= capacity) {
    return 0;
  }

  return capacity - allocated_space;
}

size_t iotc_memory_limiter_get_allocated_space() {
  return iotc_memory_allocated;
}

void* iotc_memory_limiter_alloc(
    iotc_memory_limiter_allocation_type_t limit_type, size_t size_to_alloc,
    const char* file, size_t line) {
  assert(limit_type < IOTC_MEMORY_LIMITER_ALLOCATION_TYPE_COUNT);
  assert(NULL != file);

  /* just to satisfy the compiler */
  (void)iotc_memory_limiter_cs;

  void* ptr_to_ret = NULL;

  iotc_lock_critical_section(&iotc_memory_limiter_cs);

  iotc_state_t local_state = IOTC_STATE_OK;

  size_t real_size_to_alloc =
      size_to_alloc + sizeof(iotc_memory_limiter_entry_t);

  local_state =
      iotc_memory_limiter_will_allocation_fit(limit_type, real_size_to_alloc);

  if (IOTC_STATE_OK != local_state) {
    goto end;
  }

  /* this is where we are going to use the platform alloc */
  void* entry_ptr = __iotc_alloc(real_size_to_alloc);

  if (NULL == entry_ptr) {
    goto end;
  }

  ptr_to_ret = get_ptr_from_entry(entry_ptr);

  iotc_memory_limiter_entry_t* entry = (iotc_memory_limiter_entry_t*)entry_ptr;
  memset(entry, 0, sizeof(iotc_memory_limiter_entry_t));

#if IOTC_DEBUG_EXTRA_INFO
  entry->allocation_origin_file_name = file;
  entry->allocation_origin_line_number = line;

#ifdef IOTC_PLATFORM_BASE_POSIX
  int no_of_backtraces =
      backtrace(entry->backtrace_symbols_buffer, MAX_BACKTRACE_SYMBOLS);
  entry->backtrace_symbols_buffer_size = no_of_backtraces;
#endif

  if (NULL == iotc_memory_limiter_entry_list_head) {
    iotc_memory_limiter_entry_list_head = entry;
  } else {
    entry->next = iotc_memory_limiter_entry_list_head;
    iotc_memory_limiter_entry_list_head->prev = entry;
    iotc_memory_limiter_entry_list_head = entry;
  }
#else
  IOTC_UNUSED(file);
  IOTC_UNUSED(line);
#endif

  entry->size = real_size_to_alloc;
  iotc_memory_allocated += real_size_to_alloc;

end:
  iotc_unlock_critical_section(&iotc_memory_limiter_cs);
  return ptr_to_ret;
}

void* iotc_memory_limiter_calloc(
    iotc_memory_limiter_allocation_type_t limit_type, size_t num,
    size_t size_to_alloc, const char* file, size_t line) {
  const size_t allocation_size = num * size_to_alloc;
  void* ret =
      iotc_memory_limiter_alloc(limit_type, allocation_size, file, line);

  /* it's unspecified if memset works with NULL pointer */
  if (NULL != ret) {
    memset(ret, 0, allocation_size);
  }

  return ret;
}

/**
 * @brief simulate realloc on limited memory this will return valid pointer or
 * NULL if memory isn't availible
 */
void* iotc_memory_limiter_realloc(
    iotc_memory_limiter_allocation_type_t limit_type, void* ptr,
    size_t size_to_alloc, const char* file, size_t line) {
  if (NULL == ptr) {
    return NULL;
  }

  assert(limit_type < IOTC_MEMORY_LIMITER_ALLOCATION_TYPE_COUNT);
  assert(NULL != file);

  void* ptr_to_ret = NULL;

  iotc_lock_critical_section(&iotc_memory_limiter_cs);

  iotc_state_t local_state = IOTC_STATE_OK;

  iotc_memory_limiter_entry_t* entry = get_entry_from_ptr(ptr);

  const size_t real_size_to_alloc =
      size_to_alloc + sizeof(iotc_memory_limiter_entry_t);

  const long long real_diff = real_size_to_alloc - (long long)entry->size;

  local_state = iotc_memory_limiter_will_allocation_fit(limit_type,
                                                        IOTC_MAX(real_diff, 0));

  if (IOTC_STATE_OK != local_state) {
    goto end;
  }

#if IOTC_DEBUG_EXTRA_INFO
  if (NULL != entry->prev) {
    entry->prev->next = entry->next;
  } else {
    iotc_memory_limiter_entry_list_head = entry->next;
  }

  if (NULL != entry->next) {
    entry->next->prev = entry->prev;
  }
#endif

  /* this is where we are going to use the platform alloc */
  void* r_ptr = __iotc_realloc(entry, real_size_to_alloc);

  if (NULL == r_ptr) {
    goto end;
  }

  entry = (iotc_memory_limiter_entry_t*)r_ptr;

#if IOTC_DEBUG_EXTRA_INFO
  entry->allocation_origin_file_name = file;
  entry->allocation_origin_line_number = line;

  if (NULL == iotc_memory_limiter_entry_list_head) {
    iotc_memory_limiter_entry_list_head = entry;
  } else {
    entry->next = iotc_memory_limiter_entry_list_head;
    iotc_memory_limiter_entry_list_head->prev = entry;
    iotc_memory_limiter_entry_list_head = entry;
  }
#else
  IOTC_UNUSED(file);
  IOTC_UNUSED(line);
#endif

  entry->size = real_size_to_alloc;
  iotc_memory_allocated += real_diff;

  ptr_to_ret = get_ptr_from_entry(r_ptr);

end:
  iotc_unlock_critical_section(&iotc_memory_limiter_cs);
  return ptr_to_ret;
}

void iotc_memory_limiter_free(void* ptr) {
  if (NULL == ptr) {
    return;
  }

  iotc_lock_critical_section(&iotc_memory_limiter_cs);

  iotc_memory_limiter_entry_t* entry = get_entry_from_ptr(ptr);

  const size_t size_to_free = entry->size;

  /* this is the simplest check to verify the memory integrity */
  assert(iotc_memory_limiter_get_allocated_space() >= size_to_free);

#if IOTC_DEBUG_EXTRA_INFO
  {
    iotc_memory_limiter_entry_t* leg = iotc_memory_limiter_entry_list_visitor(
        &iotc_memory_limiter_entry_array_same_predicate, entry);

    IOTC_UNUSED(leg);

    assert(NULL != leg);
    assert(entry == leg);
  }

  if (NULL != entry->prev) {
    entry->prev->next = entry->next;
  } else {
    iotc_memory_limiter_entry_list_head = entry->next;
  }

  if (NULL != entry->next) {
    entry->next->prev = entry->prev;
  }
#endif

  /* this is actual free */
  __iotc_free(entry);

  iotc_memory_allocated =
      IOTC_MAX((int)iotc_memory_allocated - (int)size_to_free, 0);

  iotc_unlock_critical_section(&iotc_memory_limiter_cs);
}

void* iotc_memory_limiter_alloc_application(size_t size_to_alloc,
                                            const char* file, size_t line) {
  return iotc_memory_limiter_alloc(
      IOTC_MEMORY_LIMITER_ALLOCATION_TYPE_APPLICATION, size_to_alloc, file,
      line);
}

void* iotc_memory_limiter_calloc_application(size_t num, size_t size_to_alloc,
                                             const char* file, size_t line) {
  return iotc_memory_limiter_calloc(
      IOTC_MEMORY_LIMITER_ALLOCATION_TYPE_APPLICATION, num, size_to_alloc, file,
      line);
}

void* iotc_memory_limiter_realloc_application(void* ptr, size_t size_to_alloc,
                                              const char* file, size_t line) {
  return iotc_memory_limiter_realloc(
      IOTC_MEMORY_LIMITER_ALLOCATION_TYPE_APPLICATION, ptr, size_to_alloc, file,
      line);
}

void* iotc_memory_limiter_alloc_system(size_t size_to_alloc, const char* file,
                                       size_t line) {
  return iotc_memory_limiter_alloc(IOTC_MEMORY_LIMITER_ALLOCATION_TYPE_SYSTEM,
                                   size_to_alloc, file, line);
}

void* iotc_memory_limiter_calloc_system(size_t num, size_t size_to_alloc,
                                        const char* file, size_t line) {
  return iotc_memory_limiter_calloc(IOTC_MEMORY_LIMITER_ALLOCATION_TYPE_SYSTEM,
                                    num, size_to_alloc, file, line);
}

void* iotc_memory_limiter_realloc_system(void* ptr, size_t size_to_alloc,
                                         const char* file, size_t line) {
  return iotc_memory_limiter_realloc(IOTC_MEMORY_LIMITER_ALLOCATION_TYPE_SYSTEM,
                                     ptr, size_to_alloc, file, line);
}

void* iotc_memory_limiter_alloc_application_export(size_t size_to_alloc) {
  return iotc_memory_limiter_alloc_application(size_to_alloc, "exported alloc",
                                               0);
}

void* iotc_memory_limiter_calloc_application_export(size_t num,
                                                    size_t size_to_alloc) {
  return iotc_memory_limiter_calloc_application(num, size_to_alloc,
                                                "exported calloc", 0);
}

void* iotc_memory_limiter_realloc_application_export(void* ptr,
                                                     size_t size_to_alloc) {
  return iotc_memory_limiter_realloc_application(ptr, size_to_alloc,
                                                 "exported re-alloc", 0);
}

#if IOTC_DEBUG_EXTRA_INFO
void iotc_memory_limiter_gc() {
  iotc_lock_critical_section(&iotc_memory_limiter_cs);

  iotc_memory_limiter_entry_t* tmp = iotc_memory_limiter_entry_list_head;

  /* traverse to the last element of the list */
  if (NULL != tmp) {
    while (NULL != tmp->next) {
      tmp = tmp->next;
    };
  } else /* the list is empty */
  {
    return;
  }

  /* tmp points to the last element of the list */
  /* idea is to peel the list from the bottom to the top ( reverse order of
   * deallocation
   * ) */
  while (NULL != tmp) {
    iotc_memory_limiter_entry_t* prev = tmp->prev;

    iotc_unlock_critical_section(&iotc_memory_limiter_cs);

    /* using memory limiter free let's deallocate the memory */
    iotc_memory_limiter_free(get_ptr_from_entry(tmp));

    iotc_lock_critical_section(&iotc_memory_limiter_cs);

    tmp = prev;
  }

  iotc_unlock_critical_section(&iotc_memory_limiter_cs);
}

void iotc_memory_limiter_visit_memory_leaks(
    void (*visitor_fn)(const iotc_memory_limiter_entry_t*)) {
  iotc_lock_critical_section(&iotc_memory_limiter_cs);

  iotc_memory_limiter_entry_t* tmp = iotc_memory_limiter_entry_list_head;

  /* traverse to the last element of the list calling the visitor function */
  if (NULL != tmp) {
    do {
      visitor_fn(tmp);
      tmp = tmp->next;
    } while (NULL != tmp);
  }

  iotc_unlock_critical_section(&iotc_memory_limiter_cs);
}

#endif /* IOTC_DEBUG_EXTRA_INFO */

#ifdef __cplusplus
}
#endif
