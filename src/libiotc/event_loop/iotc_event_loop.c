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

#include "iotc_event_loop.h"
#include "iotc_bsp_io_net.h"
#include "iotc_bsp_time.h"
#include "iotc_event_dispatcher_api.h"

/**
 * @brief iotc_bsp_event_loop_count_all_sockets
 * @param event_dispatchers
 * @param num_evtds
 * @return number of sockets that require update
 */
static size_t iotc_bsp_event_loop_count_all_sockets(
    iotc_evtd_instance_t** event_dispatchers, uint8_t num_evtds) {
  if (NULL == event_dispatchers || 0 == num_evtds) {
    return IOTC_INVALID_PARAMETER;
  }

  size_t ret_num_of_sockets = 0;

  uint8_t evtd_id = 0;
  for (; evtd_id < num_evtds; ++evtd_id) {
    const iotc_evtd_instance_t* event_dispatcher = event_dispatchers[evtd_id];
    assert(NULL != event_dispatcher);

    ret_num_of_sockets +=
        (size_t)event_dispatcher->handles_and_socket_fd->elem_no;
  }

  return ret_num_of_sockets;
}

iotc_state_t iotc_bsp_event_loop_transform_to_bsp_select(
    iotc_evtd_instance_t** in_event_dispatchers, uint8_t in_num_evtds,
    iotc_bsp_socket_events_t* in_socket_events_array,
    size_t in_socket_events_array_length, iotc_time_t* out_timeout) {
  if (NULL == out_timeout || NULL == in_event_dispatchers ||
      NULL == in_socket_events_array || 0 == in_num_evtds) {
    return IOTC_INVALID_PARAMETER;
  }

  size_t socket_id = 0;
  uint8_t was_file_updated = 0;
  uint8_t was_timeout_candidate_set = 0;
  iotc_time_t timeout_candidate = 0;

  uint8_t evtd_id = 0;
  for (evtd_id = 0; evtd_id < in_num_evtds; ++evtd_id) {
    iotc_evtd_instance_t* event_dispatcher = in_event_dispatchers[evtd_id];
    assert(NULL != event_dispatcher);

    iotc_vector_index_type_t i = 0;

    /* pick the smallest possible timeout with respect to all dispatchers */
    {
      iotc_time_t tmp_timeout = 0;
      iotc_state_t state =
          iotc_evtd_get_time_of_earliest_event(event_dispatcher, &tmp_timeout);

      /* if the heap wasn't empty */
      if (IOTC_STATE_OK == state) {
        /* if the timeout candidate has been initialised */
        if (1 == was_timeout_candidate_set) {
          timeout_candidate = IOTC_MIN(timeout_candidate, tmp_timeout);
        } else /* if it hasn't been initialised */
        {
          timeout_candidate = tmp_timeout;
        }

        was_timeout_candidate_set = 1;
      }
    }

    for (i = 0; i < event_dispatcher->handles_and_socket_fd->elem_no; ++i) {
      iotc_evtd_fd_tuple_t* tuple =
          (iotc_evtd_fd_tuple_t*)event_dispatcher->handles_and_socket_fd
              ->array[i]
              .selector_t.ptr_value;

      assert(NULL != tuple);
      assert(socket_id < in_socket_events_array_length);

      IOTC_UNUSED(in_socket_events_array_length);

      iotc_bsp_socket_events_t* socket_to_update =
          &in_socket_events_array[socket_id];
      assert(NULL != socket_to_update);

      socket_to_update->iotc_socket = tuple->fd;
      socket_to_update->in_socket_want_read =
          ((tuple->event_type & IOTC_EVENT_WANT_READ) > 0) ? 1 : 0;
      socket_to_update->in_socket_want_write =
          ((tuple->event_type & IOTC_EVENT_WANT_WRITE) > 0) ? 1 : 0;
      socket_to_update->in_socket_want_error =
          ((tuple->event_type & IOTC_EVENT_ERROR) > 0) ? 1 : 0;
      socket_to_update->in_socket_want_connect =
          ((tuple->event_type & IOTC_EVENT_WANT_CONNECT) > 0) ? 1 : 0;

      socket_id += 1;
    }

    was_file_updated |= iotc_evtd_update_file_fd_events(event_dispatcher);
  }

  /* store the current time */
  const iotc_time_t current_time = iotc_bsp_time_getcurrenttime_seconds();

  /* recalculate the timeout */
  if (was_timeout_candidate_set) {
    if (timeout_candidate >= current_time) {
      timeout_candidate = timeout_candidate - current_time;
    } else {
      /* this is possible if the first event to execute is in the past */
      timeout_candidate = 0;
    }
  } else {
    timeout_candidate = IOTC_DEFAULT_IDLE_TIMEOUT;
  }

  /* make it clamped from the top */
  timeout_candidate = IOTC_MIN(timeout_candidate, IOTC_MAX_IDLE_TIMEOUT);

  /* update the return parameter */
  *out_timeout = (was_file_updated != 0) ? (0) : (timeout_candidate);

  return IOTC_STATE_OK;
}

iotc_state_t iotc_bsp_event_loop_update_event_dispatcher(
    iotc_evtd_instance_t** in_event_dispatchers, uint8_t in_num_evtds,
    iotc_bsp_socket_events_t* in_socket_events_array,
    size_t in_socket_events_array_length)

{
  if (NULL == in_event_dispatchers || 0 == in_num_evtds ||
      NULL == in_socket_events_array) {
    return IOTC_INVALID_PARAMETER;
  }

  iotc_state_t state = IOTC_STATE_OK;
  size_t socket_id = 0;

  uint8_t evtd_id = 0;
  for (evtd_id = 0; evtd_id < in_num_evtds; ++evtd_id) {
    iotc_evtd_instance_t* event_dispatcher = in_event_dispatchers[evtd_id];

    iotc_vector_index_type_t i = 0;

    for (i = 0; i < event_dispatcher->handles_and_socket_fd->elem_no; ++i) {
      iotc_evtd_fd_tuple_t* tuple =
          (iotc_evtd_fd_tuple_t*)event_dispatcher->handles_and_socket_fd
              ->array[i]
              .selector_t.ptr_value;

      /* mark things unused cause release build won't compile assertions */
      IOTC_UNUSED(tuple);
      IOTC_UNUSED(in_socket_events_array_length);

      /* sanity check */
      assert(NULL != tuple);

      /* make sure that the socket_id is valid */
      assert(socket_id < in_socket_events_array_length);
      iotc_bsp_socket_events_t* socket_to_update =
          &in_socket_events_array[socket_id];

      /* sanity check on retrieved value */
      assert(NULL != socket_to_update);

      /* sanity check on socket id */
      assert(tuple->fd == socket_to_update->iotc_socket);

      if (0 != socket_to_update->out_socket_can_read ||
          0 != socket_to_update->out_socket_can_write ||
          0 != socket_to_update->out_socket_connect_finished ||
          0 != socket_to_update->out_socket_error) {
        state = iotc_evtd_update_event_on_socket(event_dispatcher,
                                                 socket_to_update->iotc_socket);
        IOTC_CHECK_STATE(state);
      }

      socket_id += 1;
    }
  }

err_handling:
  return state;
}

iotc_state_t iotc_event_loop_with_evtds(
    uint32_t num_iterations, iotc_evtd_instance_t** event_dispatchers,
    uint8_t num_evtds) {
  if (NULL == event_dispatchers || 0 == num_evtds) {
    return IOTC_INVALID_PARAMETER;
  }

  iotc_state_t state = IOTC_STATE_OK;
  uint32_t loops_processed = 0;

  while (iotc_evtd_all_continue(event_dispatchers, num_evtds) &&
         (0 == num_iterations || loops_processed < num_iterations)) {
    loops_processed += 1;

    /* count all sockets that are registered */
    const size_t no_of_sockets_to_update =
        iotc_bsp_event_loop_count_all_sockets(event_dispatchers, num_evtds);

    /* allocate and prepare space for the sockets */
    iotc_bsp_socket_events_t
        array_of_sockets_to_update[no_of_sockets_to_update];
    memset(array_of_sockets_to_update, 0,
           sizeof(iotc_bsp_socket_events_t) * no_of_sockets_to_update);

    /* for storing the timeout */
    iotc_time_t timeout = 0;

    /* transpose data from event dispatcher to socketd to update array */
    state = iotc_bsp_event_loop_transform_to_bsp_select(
        event_dispatchers, num_evtds, array_of_sockets_to_update,
        no_of_sockets_to_update, &timeout);
    IOTC_CHECK_STATE(state);

    /* call the bsp select function */
    const iotc_bsp_io_net_state_t select_state = iotc_bsp_io_net_select(
        (iotc_bsp_socket_events_t*)&array_of_sockets_to_update,
        no_of_sockets_to_update, timeout);

    if (IOTC_BSP_IO_NET_STATE_OK == select_state) {
      /* tranform output from bsp select to event dispatcher updates */
      state = iotc_bsp_event_loop_update_event_dispatcher(
          event_dispatchers, num_evtds, array_of_sockets_to_update,
          no_of_sockets_to_update);
      IOTC_CHECK_STATE(state);
    } else if (IOTC_BSP_IO_NET_STATE_ERROR == select_state) {
      state = IOTC_INTERNAL_ERROR;
      goto err_handling;
    }

    /* update time based events */
    uint8_t evtd_id = 0;
    for (evtd_id = 0; evtd_id < num_evtds; ++evtd_id) {
      iotc_evtd_step(event_dispatchers[evtd_id],
                     iotc_bsp_time_getcurrenttime_seconds());
    }
  }

err_handling:
  return state;
}
