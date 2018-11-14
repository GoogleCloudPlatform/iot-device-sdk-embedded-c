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

#include <stdio.h>

#include <getopt.h>
#include <iotc_bsp_time.h>
#include <iotc_event_loop.h>
#include "iotc.h"
#include "iotc_globals.h"
#include "iotc_handle.h"
#include "iotc_libiotc_driver_impl.h"
#include "iotc_macros.h"
#include "iotc_memory_checks.h"

#ifdef IOTC_PLATFORM_BASE_POSIX
#include <execinfo.h>
#endif

#ifdef IOTC_MEMORY_LIMITER_ENABLED

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
  fprintf(stderr,
          "\x1b[33m \t [MLD] --- %zu bytes lost, allocated in %s:%zu\x1b[0m\n",
          entry->size,
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
#endif

  fflush(stderr);
}
#endif /* IOTC_DEBUG_EXTRA_INFO */

uint8_t _iotc_memory_limiter_teardown() {
  /* check for memory leaks */
  if (!iotc_is_whole_memory_deallocated()) {
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

  return 0;
}

#endif

#ifdef IOTC_MEMORY_LIMITER_ENABLED
// clang-format off
#define iotc_is_whole_memory_deallocated() \
  ( iotc_memory_limiter_get_allocated_space() == 0 )
//clang-format on

uint8_t _iotc_memory_limiter_teardown();
#define iotc_memory_limiter_teardown _iotc_memory_limiter_teardown

#else
#define iotc_is_whole_memory_deallocated() 1
#define iotc_memory_limiter_teardown() 1
#endif

void driver_on_connected() {
  iotc_debug_printf("[ driver     ] %s\n", __func__);
}

extern char* optarg;
extern int optind, optopt, opterr;

void iotc_driver_init(int argc, char const* argv[],
                      char** control_channel_host_out,
                      int* control_channel_port_out) {
  int result_getopt = 0;

  /* this is platform specific code */
  opterr = 0;
  while ((result_getopt = getopt(argc, (char* const*)argv, "h:p:")) != -1) {
    switch (result_getopt) {
      case 'h':
        *control_channel_host_out = optarg;
        break;
      case 'p':
        *control_channel_port_out = atoi(optarg);
        break;
    }
  }
}

int main(int argc, char const* argv[]) {
  IOTC_UNUSED(DEFAULT_PROJECT_ID);
  IOTC_UNUSED(DEFAULT_DEVICE_PATH);
  IOTC_UNUSED(DEFAULT_PRIVATE_KEY);

  /*************************************
   * libiotc initialization **********
   *************************************/
  iotc_initialize();

  iotc_context_handle_t iotc_app_context_handle = iotc_create_context();
  if (IOTC_INVALID_CONTEXT_HANDLE >= iotc_app_context_handle) {
    iotc_debug_printf(
        "[ driver     ] "
        "Could not create IoTC application context!\n");
    return -1;
  }

  iotc_globals.default_context_handle = iotc_app_context_handle;

  /*************************************
   * driver initialization *************
   *************************************/
  char* control_channel_host = "localhost";
  int control_channel_port = 12345;

  iotc_driver_init(argc, argv, &control_channel_host, &control_channel_port);

  iotc_debug_printf(
      "[ driver     ] Connecting to control channel: [ %s : %d ]\n",
      control_channel_host, control_channel_port);

  libiotc_driver = iotc_libiotc_driver_create_instance();

  iotc_libiotc_driver_connect_with_callback(
      libiotc_driver, control_channel_host, control_channel_port,
      driver_on_connected);

  // do a single step on driver's evtd to start control
  // channel connect before doing first empty select 1sec blocking
  iotc_evtd_step(libiotc_driver->context->context_data.evtd_instance,
                 iotc_bsp_time_getcurrenttime_seconds());

  iotc_evtd_instance_t* evtd_all[2] = {
      iotc_globals.evtd_instance,
      libiotc_driver->context->context_data.evtd_instance};

  /***********************************************************
   * time-sliced actuation of both: libiotc and driver *****
   ***********************************************************/
  while (iotc_evtd_dispatcher_continue(libiotc_driver->evtd_instance) == 1) {
    iotc_event_loop_with_evtds(1, evtd_all, 2);

    // artificial processing of events in libiotc's and driver's dispatcher
    // this is for speeding up
    // driver->libiotc and
    // driver->libiotc->driver requests (avoiding select timeout between)
    iotc_evtd_step(iotc_globals.evtd_instance,
                   iotc_bsp_time_getcurrenttime_seconds());
    iotc_evtd_step(libiotc_driver->context->context_data.evtd_instance,
                   iotc_bsp_time_getcurrenttime_seconds());
  }

  iotc_libiotc_driver_destroy_instance(&libiotc_driver);

  iotc_delete_context(iotc_app_context_handle);
  iotc_shutdown();

  return iotc_memory_limiter_teardown() ? 0 : 1;
}

void vAssertCalled( unsigned long ulLine, const char * const pcFileName )
{
      printf("ASSERT: %s : %d\n", pcFileName, (int)ulLine);fflush(stdout);
          while(1);
}

void vApplicationMallocFailedHook(void) {
  while (1)
    ;
}

