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

#include "iotc_memory_checks.h"
#include <stdio.h>

#ifdef IOTC_PLATFORM_BASE_POSIX
#include <execinfo.h>
#include <stdlib.h>
#endif

#include <stdint.h>

#ifdef IOTC_MEMORY_LIMITER_ENABLED

#define get_ptr_from_entry(e) \
  (void*)((intptr_t)e + sizeof(iotc_memory_limiter_entry_t))

#if IOTC_DEBUG_EXTRA_INFO
const char* iotc_memory_checks_get_filename(const char* filename_and_path) {
  const char* p = filename_and_path;
  for (; *p != '\0'; ++p)
    ;
  for (; *p != '/' && p > filename_and_path; --p)
    ;
  if (*p == '/') ++p;
  return p;
}

void iotc_memory_checks_log_memory_leak(
    const iotc_memory_limiter_entry_t* entry) {
  fprintf(
      stderr,
      "\x1b[33m \t [MLD] --- %zu bytes lost, %p allocated in %s:%zu\x1b[0m\n",
      entry->size, get_ptr_from_entry(entry),
      iotc_memory_checks_get_filename(entry->allocation_origin_file_name),
      entry->allocation_origin_line_number);

#ifdef IOTC_PLATFORM_BASE_POSIX
  fprintf(stderr, "\x1b[33m\t\tbacktrace:\x1b[0m\n");
  char** human_readable_symbols = backtrace_symbols(
      entry->backtrace_symbols_buffer, entry->backtrace_symbols_buffer_size);
  int i = 0;
  for (; i < entry->backtrace_symbols_buffer_size; ++i) {
    fprintf(stderr, "\t\t\t\x1b[33m%s\x1b[0m\n", human_readable_symbols[i]);
  }
  free(human_readable_symbols);
#endif

  fflush(stderr);
}
#endif /* IOTC_DEBUG_EXTRA_INFO */

void _iotc_memory_limiter_tearup() {
  if (!iotc_is_whole_memory_deallocated()) {
    fprintf(stderr,
            "\x1b[31m [MLD] Warning: Memory level before the tear up %zu "
            "please check previously executed tests!\x1b[0m\n",
            iotc_memory_limiter_get_allocated_space());

#if IOTC_DEBUG_EXTRA_INFO
    /* print information about leaks */
    iotc_memory_limiter_visit_memory_leaks(&iotc_memory_checks_log_memory_leak);

    /* garbage collection */
    iotc_memory_limiter_gc();
    fprintf(stderr, "\x1b[31m [MLD] Memory has been cleaned for you!\x1b[0m\n");
#else /* IOTC_DEBUG_EXTRA_INFO */
    fprintf(stderr,
            "\x1b[31m [MLD] This version has been built with "
            "IOTC_DEBUG_EXTRA_INFO=0 garbage collection and memory leaks "
            "locator doesn't work\x1b[0m\n");
#endif
    fflush(stderr);
  }
}

uint8_t _iotc_memory_limiter_teardown() {
  uint8_t whole_memory_deallocated = 1;

  /* check for memory leaks */
  if (!iotc_is_whole_memory_deallocated()) {
    whole_memory_deallocated = 0;

    fprintf(stderr,
            "\x1b[31m [MLD] WARNING: Memory leak detected - total memory lost "
            "- %ld bytes \x1b[0m\n",
            iotc_memory_limiter_get_allocated_space());

#if IOTC_DEBUG_EXTRA_INFO
    /* print information about leaks */
    iotc_memory_limiter_visit_memory_leaks(&iotc_memory_checks_log_memory_leak);

    /* garbage collection */
    iotc_memory_limiter_gc();
#else /* IOTC_DEBUG_EXTRA_INFO */
    fprintf(stderr,
            "\x1b[31m [MLD] This version has been built with "
            "IOTC_DEBUG_EXTRA_INFO=0 garbage collection and memory leaks "
            "locator doesn't work!!!  \x1b[0m\n");
#endif
  }

  fflush(stderr);

  return whole_memory_deallocated;
}

#endif
