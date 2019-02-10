/* Copyright 2018-2019 Google LLC
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

#include "iotc_io_timeouts.h"
#include <inttypes.h>

/**
 * @brief iotc_io_timeouts_create
 *
 * Starts io event execution, and stores it in the given vector.
 * Vector binds to an io thread, so no locking is needed.
 */
iotc_state_t iotc_io_timeouts_create(
    iotc_evtd_instance_t* event_dispatcher, iotc_event_handle_t handle,
    iotc_time_t time_diff, iotc_vector_t* io_timeouts,
    iotc_time_event_handle_t* ret_time_event_handle) {
  assert(NULL != event_dispatcher);
  assert(NULL != io_timeouts);
  assert(NULL != ret_time_event_handle &&
         NULL == ret_time_event_handle->ptr_to_position);

  const iotc_vector_elem_t* elem = NULL;

  iotc_state_t ret_state = iotc_evtd_execute_in(
      event_dispatcher, handle, time_diff, ret_time_event_handle);

  IOTC_CHECK_STATE(ret_state);

  elem = iotc_vector_push(
      io_timeouts,
      IOTC_VEC_CONST_VALUE_PARAM(IOTC_VEC_VALUE_PTR(ret_time_event_handle)));

  IOTC_CHECK_MEMORY(elem, ret_state);

err_handling:
  return ret_state;
}

/**
 * @brief iotc_io_timeouts_cancel
 *
 * Cancels io event and removes event from the io event vector.
 */

void iotc_io_timeouts_cancel(iotc_evtd_instance_t* event_dispatcher,
                             iotc_time_event_handle_t* time_event_handle,
                             iotc_vector_t* io_timeouts) {
  assert(event_dispatcher != NULL);
  assert(io_timeouts != NULL);
  assert(NULL != time_event_handle &&
         NULL != time_event_handle->ptr_to_position);

  iotc_io_timeouts_remove(time_event_handle, io_timeouts);
  const iotc_state_t local_state =
      iotc_evtd_cancel(event_dispatcher, time_event_handle);
  IOTC_UNUSED(local_state);
  assert(IOTC_STATE_OK == local_state);
}

/**
 * @brief iotc_io_timeouts_restart
 *
 * Restarts io events in the timeouts vector.
 */
void iotc_io_timeouts_restart(iotc_evtd_instance_t* event_dispatcher,
                              iotc_time_t new_time,
                              iotc_vector_t* io_timeouts) {
  assert(event_dispatcher != NULL);
  assert(io_timeouts != NULL);

  iotc_vector_index_type_t index = 0;

  for (index = 0; index < io_timeouts->elem_no; ++index) {
    iotc_vector_elem_t timeout_element = io_timeouts->array[index];
    iotc_time_event_handle_t* time_event_handle =
        timeout_element.selector_t.ptr_value;

    assert(NULL != time_event_handle &&
           NULL != time_event_handle->ptr_to_position);

    const iotc_state_t local_state =
        iotc_evtd_restart(event_dispatcher, time_event_handle, new_time);
    IOTC_UNUSED(local_state);
    assert(IOTC_STATE_OK == local_state);
  }
}

/**
 * @brief iotc_io_timeouts_compare_heap_elements
 *
 * Compare function for finding events in the io event vector.
 */
static inline int8_t iotc_io_timeouts_compare_pointers(
    const union iotc_vector_selector_u* e0,
    const union iotc_vector_selector_u* e1) {
  return e0->ptr_value == e1->ptr_value ? 0 : 1;
}

/**
 * @brief iotc_io_timeouts_remove
 *
 * Remove event from timeout vector.
 */
void iotc_io_timeouts_remove(iotc_time_event_handle_t* time_event_handle,
                             iotc_vector_t* io_timeouts) {
  assert(time_event_handle != NULL);
  assert(io_timeouts != NULL);

  iotc_vector_index_type_t event_index = iotc_vector_find(
      io_timeouts,
      IOTC_VEC_CONST_VALUE_PARAM(IOTC_VEC_VALUE_PTR(time_event_handle)),
      iotc_io_timeouts_compare_pointers);

  if (event_index > -1) {
    iotc_vector_del(io_timeouts, event_index);
  }
}
