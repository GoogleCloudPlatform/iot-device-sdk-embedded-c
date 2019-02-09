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

/**
 * @file iotc_time_event.h
 * @brief Implements time based events
 *
 * Time based events can be used to register function handler for later
 * execution. Entities such timeouts can be easily implemented using time based
 * events.
 */

#ifndef __IOTC_TIME_EVENT_H__
#define __IOTC_TIME_EVENT_H__

#include <stdint.h>

#include "iotc_allocator.h"
#include "iotc_debug.h"
#include "iotc_event_handle.h"
#include "iotc_time.h"
#include "iotc_vector.h"

typedef struct iotc_time_event_handle_s {
  iotc_vector_index_type_t* ptr_to_position;
} iotc_time_event_handle_t;

typedef struct iotc_time_event_s {
  iotc_event_handle_t event_handle;
  iotc_time_t time_of_execution;
  iotc_vector_index_type_t position;
  iotc_time_event_handle_t* time_event_handle;
} iotc_time_event_t;

#define IOTC_TIME_EVENT_POSITION_INVALID -1

#define iotc_make_empty_time_event_handle() \
  { NULL }

#define iotc_make_time_event_handle(event_handle) \
  { &event_handle->position }

#define iotc_make_empty_time_event() \
  { iotc_make_empty_event_handle(), 0, IOTC_TIME_EVENT_POSITION_INVALID, NULL }

/* API */
/**
 * @brief iotc_time_event_add
 *
 * Adds new time_event to the given vector. If the operation succeded it have to
 * return the iotc_time_event_handle_t using the ret_time_event_handle return
 * parameter. The iotc_time_event_handle_t is associated with the time_event and
 * it can be used in order to cancel or restart the time_event via calling
 * iotc_time_event_cancel or iotc_time_event_restart.
 *
 * @see iotc_time_event_restart iotc_time_event_cancel
 *
 * @note The user of this API has the ownership of the memory created outside of
 * this API, so if a time_event has been allocated on the heap, it has to be
 * deallocated after it is no longer used.
 *
 * @param vector - the storage for time_events
 * @param time_event - new time event to get registered
 * @param ret_time_event_handle - return parameter, a handle associated with the
 * time_event
 * @return IOTC_STATE_OK in case of success other values in case of failure
 */
iotc_state_t iotc_time_event_add(
    iotc_vector_t* vector, iotc_time_event_t* time_event,
    iotc_time_event_handle_t* ret_time_event_handle);

/**
 * @brief iotc_time_event_get_top
 *
 * Returns the pointer to the first element in the container which is guaranteed
 * by the time event implementation to be the time event with minimum execution
 * time of all time events stored within this container.
 *
 * It removes the returned time event element from the vector. Use
 * iotc_time_event_pee_top in order to minitor for the value of the minimum
 * element without removing it from the container.
 *
 * @param vector
 * @return pointer to the time event with minimum execution time, NULL if the
 * time event container is empty
 */
iotc_time_event_t* iotc_time_event_get_top(iotc_vector_t* vector);

/**
 * @brief iotc_time_event_peek_top
 *
 * Returns the pointer to the first element in the container which is guaranteed
 * by the time event implementation to be the time event with minimum execution
 * time of all time events stored within this container.
 *
 * @param vector
 * @return pointer to the time event with minimum execution time, NULL if the
 * time event container is empty
 */
iotc_time_event_t* iotc_time_event_peek_top(iotc_vector_t* vector);

/**
 * @brief iotc_time_event_restart
 *
 * Changes the execution time of a time event associated with the gven
 * time_event_handle.
 *
 * @param vector
 * @param time_event_handle
 * @return IOTC_STATE_OK in case of the success, IOTC_ELEMENT_NOT_FOUND if the
 * time event does not exist in the container
 */
iotc_state_t iotc_time_event_restart(
    iotc_vector_t* vector, iotc_time_event_handle_t* time_event_handle,
    iotc_time_t new_time);

/**
 * @brief iotc_time_event_cancel
 *
 * Cancels execution of the time event associated by the time_event_handle. It
 * removes the time event from the time events container.
 *
 * @param vector
 * @param time_event_handle
 * @return IOTC_STATE_OK in case of the success, IOTC_ELEMENT_NOT_FOUND if the
 * time event couldn't be found
 */
iotc_state_t iotc_time_event_cancel(iotc_vector_t* vector,
                                    iotc_time_event_handle_t* time_event_handle,
                                    iotc_time_event_t** cancelled_time_event);

/**
 * @brief iotc_time_event_destroy
 *
 * Releases all the memory allocated by time events.
 *
 * @param vector
 */
void iotc_time_event_destroy(iotc_vector_t* vector);

#endif /* __IOTC_TIME_EVENT_H__ */
