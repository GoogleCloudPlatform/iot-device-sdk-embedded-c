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

#ifndef __IOTC_EVENT_DISPATCHER_API_H__
#define __IOTC_EVENT_DISPATCHER_API_H__

#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>

#include "iotc_allocator.h"
#include "iotc_config.h"
#include "iotc_event_handle.h"
#include "iotc_event_handle_queue.h"
#include "iotc_macros.h"
#include "iotc_time.h"
#include "iotc_time_event.h"
#include "iotc_vector.h"

#include "iotc_critical_section.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef intptr_t iotc_fd_t;

typedef enum iotc_evtd_fd_type_e {
  IOTC_EVTD_FD_TYPE_SOCKET = 0,
  IOTC_EVTD_FD_TYPE_FILE
} iotc_evtd_fd_type_t;

typedef struct iotc_evtd_tuple_s {
  iotc_fd_t fd;
  iotc_event_handle_t handle;
  iotc_event_handle_t read_handle;
  iotc_event_type_t event_type;
  iotc_evtd_fd_type_t fd_type;
} iotc_evtd_fd_tuple_t;

typedef struct iotc_evtd_instance_s {
  iotc_time_t current_step;
  iotc_vector_t* time_events_container;
  iotc_event_handle_queue_t* call_queue;
  struct iotc_critical_section_s* cs;
  iotc_vector_t* handles_and_socket_fd;
  iotc_vector_t* handles_and_file_fd;
  iotc_event_handle_t on_empty;
  uint8_t stop;
} iotc_evtd_instance_t;

extern int8_t iotc_evtd_register_file_fd(iotc_evtd_instance_t* instance,
                                         iotc_event_type_t event_type,
                                         iotc_fd_t fd,
                                         iotc_event_handle_t handle);

extern int8_t iotc_evtd_register_socket_fd(iotc_evtd_instance_t* instance,
                                           iotc_fd_t fd,
                                           iotc_event_handle_t read_handle);

extern int8_t iotc_evtd_unregister_file_fd(iotc_evtd_instance_t* instance,
                                           iotc_fd_t fd);

extern int8_t iotc_evtd_unregister_socket_fd(iotc_evtd_instance_t* instance,
                                             iotc_fd_t fd);

extern int8_t iotc_evtd_continue_when_evt_on_socket(
    iotc_evtd_instance_t* instance, iotc_event_type_t event_type,
    iotc_event_handle_t handle, iotc_fd_t fd);

extern void iotc_evtd_continue_when_empty(iotc_evtd_instance_t* instance,
                                          iotc_event_handle_t handle);

extern iotc_event_handle_queue_t* iotc_evtd_execute(
    iotc_evtd_instance_t* instance, iotc_event_handle_t handle);

extern iotc_state_t iotc_evtd_execute_in(
    iotc_evtd_instance_t* instance, iotc_event_handle_t handle,
    iotc_time_t time_diff, iotc_time_event_handle_t* ret_time_event_handle);

extern iotc_state_t iotc_evtd_cancel(
    iotc_evtd_instance_t* instance,
    iotc_time_event_handle_t* time_event_handle);

extern iotc_state_t iotc_evtd_restart(
    iotc_evtd_instance_t* instance, iotc_time_event_handle_t* time_event_handle,
    iotc_time_t new_time);

extern iotc_evtd_instance_t* iotc_evtd_create_instance(void);

extern void iotc_evtd_destroy_instance(iotc_evtd_instance_t* instance);

extern iotc_event_handle_return_t iotc_evtd_execute_handle(
    iotc_event_handle_t* handle);

extern uint8_t iotc_evtd_single_step(iotc_evtd_instance_t* instance,
                                     iotc_time_t new_step);

extern void iotc_evtd_step(iotc_evtd_instance_t* instance,
                           iotc_time_t new_step);

extern uint8_t iotc_evtd_dispatcher_continue(iotc_evtd_instance_t* instance);

extern uint8_t iotc_evtd_all_continue(iotc_evtd_instance_t** event_dispatchers,
                                      uint8_t num_evtds);

extern iotc_state_t iotc_evtd_update_event_on_socket(
    iotc_evtd_instance_t* instance, iotc_fd_t fds);

extern iotc_state_t iotc_evtd_update_event_on_file(
    iotc_evtd_instance_t* instance, iotc_fd_t fds);

extern void iotc_evtd_stop(iotc_evtd_instance_t* instance);

extern iotc_event_handle_t iotc_make_event_handle(void* func,
                                                  iotc_event_handle_argc_t argc,
                                                  ...);

/**
 * @brief iotc_evtd_update_file_events
 *
 * Because this function is now platform independent it doesn't need to be
 * implemented explicitly in platform loop
 *
 * @param instance
 * @return 1 if any file has been updated 0 otherwise
 */
extern uint8_t iotc_evtd_update_file_fd_events(
    iotc_evtd_instance_t* const instance);

/**
 * @brief iotc_evtd_get_time_of_earliest_event
 *
 * Calculates the time of execution of the earliest event registered in event
 * dispatcher.
 *
 * @param instance of an event dispatcher which will be queried for the time
 * @param pointer to the iotc_time_t where the value of time of execution of the
 * earliest event will be stored, if there is no events value under the pointer
 * want be modified
 * @return IOTC_STATE_OK if the earliest event exists, IOTC_ELEMENT_NOT_FOUND if
 * there is no events
 */
extern iotc_state_t iotc_evtd_get_time_of_earliest_event(
    iotc_evtd_instance_t* instance, iotc_time_t* out_timeout);

#ifdef __cplusplus
}
#endif

#endif /* __IOTC_EVENT_DISPATCHER_API_H__ */
