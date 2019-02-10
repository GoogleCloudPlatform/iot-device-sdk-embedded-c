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

#include "iotc_time_event.h"

/**
 * @brief This part of the file implements time event functionality. This
 * implementation assumes that the element type is always the iotc_time_event_t.
 *
 * It uses the vector as a container type. The vector stores pointers to the
 * time events. Time events are always sorted in the vector by the time event
 * execution time.
 */

/*
 * STATIC INTERNAL FUNCTIONS
 */

/**
 * @brief iotc_swap_time_events
 *
 * Swaps the two iotc_time_event_t elements in the provided vector.
 *
 *
 * @param vector
 * @param lhs_index
 * @param rhs_index
 */
static void iotc_swap_time_events(iotc_vector_t* vector,
                                  iotc_vector_index_type_t lhs_index,
                                  iotc_vector_index_type_t rhs_index) {
  /* PRE-CONDITIONS */
  assert(NULL != vector);
  assert(lhs_index < vector->elem_no);
  assert(rhs_index < vector->elem_no);
  assert(lhs_index >= 0);
  assert(rhs_index >= 0);

  iotc_time_event_t* lhs_time_event =
      (iotc_time_event_t*)vector->array[lhs_index].selector_t.ptr_value;
  iotc_time_event_t* rhs_time_event =
      (iotc_time_event_t*)vector->array[rhs_index].selector_t.ptr_value;

  iotc_vector_swap_elems(vector, lhs_index, rhs_index);

  lhs_time_event->position = rhs_index;
  rhs_time_event->position = lhs_index;
}

/**
 * @brief iotc_time_event_bubble_core
 *
 * Generic bubbler. This function eliminates code repeatition for the logic of
 * bubbling an element in both directions in the vector. It is used by
 * iotc_time_event_bubble_and_sort_down and iotc_time_event_bubble_and_sort_up.
 *
 * @param vector - input vector, container of time events
 * @param index - position of the elment to bubble
 * @param dir - direction of bubbling - +1 stands for up -1 means down
 * @return new index of bubbled element
 */
static iotc_vector_index_type_t iotc_time_event_bubble_core(
    iotc_vector_t* vector, iotc_vector_index_type_t index, int8_t dir) {
  /* PRE-CONDITIONS */
  assert(NULL != vector);
  assert(vector->elem_no > 0);
  assert(index >= 0);
  assert(index < vector->elem_no);
  assert(dir == -1 || dir == 1);

  /* guards */
  const iotc_vector_elem_t* end =
      &vector->array[IOTC_MAX(vector->elem_no - 1, 0)];
  const iotc_vector_elem_t* begin = &vector->array[0];

  /* lonely traveler */
  iotc_vector_elem_t* elem_to_bubble = &vector->array[index];

  while ((dir < 0) ? (elem_to_bubble != begin) : (elem_to_bubble != end)) {
    const iotc_time_event_t* time_event_to_cmp =
        (elem_to_bubble + dir)->selector_t.ptr_value;
    const iotc_time_event_t* time_event_to_bubble =
        elem_to_bubble->selector_t.ptr_value;

    if ((dir < 0) ? (time_event_to_bubble->time_of_execution <
                     time_event_to_cmp->time_of_execution)
                  : (time_event_to_bubble->time_of_execution >
                     time_event_to_cmp->time_of_execution)) {
      /* if the elements are not in the right order lets swap them */
      iotc_swap_time_events(vector, time_event_to_cmp->position,
                            time_event_to_bubble->position);

      /* update the ptr */
      elem_to_bubble += dir;
    } else {
      /* thanks to the container invariant on the order of the elemements we can
       * break at this moment */
      break;
    }
  }

  return ((iotc_time_event_t*)(elem_to_bubble->selector_t.ptr_value))->position;
}

/**
 * @brief iotc_time_event_bubble_and_sort_down
 *
 * Helper function used for insertion and re-insertion ( cancel opertaion ). It
 * uses the bubble sort algorithm to put the element on the proper position.
 *
 * @note: invariant of this container - it's elements are sorted using the time
 * of execution so: vector[i].time_of_execution <= vector[i+1].time_of_execution
 *
 * @param vector
 * @param index
 * @return new index of the element after bubbling it
 */
static iotc_vector_index_type_t iotc_time_event_bubble_and_sort_down(
    iotc_vector_t* vector, iotc_vector_index_type_t index) {
  /* PRE-CONDITIONS */
  assert(NULL != vector);
  assert(index >= 0);
  assert(index < vector->elem_no);

  return iotc_time_event_bubble_core(vector, index, -1);
}

/**
 * @brief iotc_time_event_bubble_and_sort_up
 *
 * Helper function used to restarting time event. It bubbles the time event
 * element up to the end of the container. It works using the same principle as
 * iotc_time_event_bubble_and_sort_down function.
 *
 * @see iotc_time_event_bubble_and_sort_down function iotc_time_event_restart
 *
 * @param vector
 * @param index
 * @return
 */
static iotc_vector_index_type_t iotc_time_event_bubble_and_sort_up(
    iotc_vector_t* vector, iotc_vector_index_type_t index) {
  /* PRE-CONDITIONS */
  assert(NULL != vector);
  assert(index >= 0);
  assert(index < vector->elem_no);

  return iotc_time_event_bubble_core(vector, index, 1);
}

/**
 * @brief iotc_time_event_move_to_the_end
 *
 * Helper function that moves the element form the given position ( index ) to
 * the end of the vector.
 *
 * @param vector
 * @param index
 */
static void iotc_time_event_move_to_the_end(iotc_vector_t* vector,
                                            iotc_vector_index_type_t index) {
  /* PRE-CONDITIONS */
  assert(NULL != vector);
  assert(vector->elem_no > 0);
  assert(index < vector->elem_no);
  assert(index >= 0);

  const iotc_vector_index_type_t last_elem_index = vector->elem_no - 1;
  iotc_vector_index_type_t elem_to_swap_with_index = index + 1;

  while (index < last_elem_index) {
    iotc_swap_time_events(vector, index, elem_to_swap_with_index);

    /* once swapped update the indexes */
    index += 1;
    elem_to_swap_with_index += 1;
  }
}

/**
 * @brief iotc_insert_time_event
 *
 * Helper function that inserts the new time event element to the vector.
 *
 * @param vector
 * @param time_event
 */
static const iotc_vector_elem_t* iotc_insert_time_event(
    iotc_vector_t* vector, iotc_time_event_t* time_event) {
  /* PRE-CONDITIONS */
  assert(NULL != vector);
  assert(NULL != time_event);

  iotc_state_t local_state = IOTC_STATE_OK;
  iotc_vector_index_type_t index = 0;
  const iotc_vector_elem_t* element_added = NULL;

  /* add the element to the end of the vector */
  {
    const iotc_vector_elem_t* inserted_element = iotc_vector_push(
        vector, IOTC_VEC_CONST_VALUE_PARAM(IOTC_VEC_VALUE_PTR(time_event)));

    IOTC_CHECK_MEMORY(inserted_element, local_state);
  }

  /* update the time event new position */
  time_event->position = vector->elem_no - 1;

  index = iotc_time_event_bubble_and_sort_down(vector, vector->elem_no - 1);

  element_added = &vector->array[index];

  return element_added;

err_handling:
  return NULL;
}

/**
 * @brief iotc_time_event_dispose_time_event
 *
 * Helper function used by all operations that requires removal of the time
 * event from container and making it unavailable for future references.
 *
 * @param time_event
 */
static void iotc_time_event_dispose_time_event(iotc_time_event_t* time_event) {
  /* PRE-CONDITIONS */
  assert(NULL != time_event);

  if (NULL != time_event->time_event_handle) {
    time_event->time_event_handle->ptr_to_position = NULL;
  }
}

/**
 * @brief iotc_time_event_destructor
 *
 * Helper function used to release the memory required by time event structure.
 *
 * @param selector
 * @param arg
 */
static void iotc_time_event_destructor(union iotc_vector_selector_u* selector,
                                       void* arg) {
  /* PRE-CONDITIONS */
  assert(NULL != selector);

  IOTC_UNUSED(arg);

  iotc_time_event_t* time_event = (iotc_time_event_t*)selector->ptr_value;
  time_event->position = IOTC_TIME_EVENT_POSITION_INVALID;

  IOTC_SAFE_FREE(time_event);
}

/*
 * PUBLIC FUNCTIONS
 */

iotc_state_t iotc_time_event_add(
    iotc_vector_t* vector, iotc_time_event_t* time_event,
    iotc_time_event_handle_t* ret_time_event_handle) {
  /* PRE-CONDITIONS */
  assert(NULL != vector);
  assert(NULL != time_event);
  assert((NULL != ret_time_event_handle &&
          NULL == ret_time_event_handle->ptr_to_position) ||
         (NULL == ret_time_event_handle));

  iotc_state_t out_state = IOTC_STATE_OK;
  iotc_time_event_t* added_time_event = NULL;

  /* call the insert at function it will place the new element at the proper
   * place
   */
  const iotc_vector_elem_t* elem = iotc_insert_time_event(vector, time_event);

  /* if there is a problem with the memory go to err_handling */
  IOTC_CHECK_MEMORY(elem, out_state);

  /* extract the element */
  added_time_event = (iotc_time_event_t*)elem->selector_t.ptr_value;

  /* sanity checks */
  assert(added_time_event == time_event);

  if (NULL != ret_time_event_handle) {
    /* update the return value with the pointer to the position in the vector */
    ret_time_event_handle->ptr_to_position = &added_time_event->position;

    /* set the time event handle pointer for further sanity checks and cleaning
     */
    added_time_event->time_event_handle = ret_time_event_handle;
  }

  /* exit with out_state value */
  return out_state;

err_handling:
  return out_state;
}

iotc_time_event_t* iotc_time_event_get_top(iotc_vector_t* vector) {
  /* PRE-CONDITIONS */
  assert(NULL != vector);

  if (0 == vector->elem_no) {
    return NULL;
  }

  iotc_time_event_t* top_one =
      (iotc_time_event_t*)vector->array[0].selector_t.ptr_value;

  /* the trick is to bubble the element to the end of the vector and then to
   * delete it*/
  if (vector->elem_no > 1) {
    iotc_time_event_move_to_the_end(vector, 0);
    iotc_vector_del(vector, vector->elem_no - 1);
  } else {
    iotc_vector_del(vector, vector->elem_no - 1);
  }

  iotc_time_event_dispose_time_event(top_one);

  return top_one;
}

iotc_time_event_t* iotc_time_event_peek_top(iotc_vector_t* vector) {
  /* PRE-CONDITIONS */
  assert(NULL != vector);

  if (0 == vector->elem_no) {
    return NULL;
  }

  iotc_time_event_t* top_one =
      (iotc_time_event_t*)vector->array[0].selector_t.ptr_value;

  return top_one;
}

iotc_state_t iotc_time_event_restart(
    iotc_vector_t* vector, iotc_time_event_handle_t* time_event_handle,
    iotc_time_t new_time) {
  /* PRE-CONDITIONS */
  assert(NULL != vector);
  assert(NULL != time_event_handle);

  /* the element can be found with O(1) complexity cause we've been updating
   * each element's position during every operation that could've broken it */

  iotc_vector_index_type_t index = *time_event_handle->ptr_to_position;

  /* check for the correctness of the time_event_handle */
  if (index >= vector->elem_no || index < 0) {
    return IOTC_ELEMENT_NOT_FOUND;
  }

  /* let's update the key of this element */
  iotc_time_event_t* time_event =
      (iotc_time_event_t*)vector->array[index].selector_t.ptr_value;

  /* sanity check on the time handle */
  assert(time_event->time_event_handle == time_event_handle);

  iotc_time_t old_time = time_event->time_of_execution;
  time_event->time_of_execution = new_time;

  if (new_time < old_time) {
    iotc_time_event_bubble_and_sort_down(vector, index);
  } else {
    iotc_time_event_bubble_and_sort_up(vector, index);
  }

  return IOTC_STATE_OK;
}

iotc_state_t iotc_time_event_cancel(iotc_vector_t* vector,
                                    iotc_time_event_handle_t* time_event_handle,
                                    iotc_time_event_t** cancelled_time_event) {
  /* PRE-CONDITIONS */
  assert(NULL != vector);
  assert(NULL != time_event_handle);
  assert(NULL != time_event_handle->ptr_to_position);
  assert(NULL != cancelled_time_event);

  /* the element we would like to remove should be at position described by the
   * time_event_handle */

  iotc_vector_index_type_t index = *time_event_handle->ptr_to_position;

  if (index >= vector->elem_no || index < 0) {
    return IOTC_ELEMENT_NOT_FOUND;
  }

  /* if it's somwhere else than the end, let's swap it with the last element */
  if (index < vector->elem_no - 1) {
    iotc_time_event_move_to_the_end(vector, index);
  }

  /* let's update the return parameter */
  *cancelled_time_event = (iotc_time_event_t*)vector->array[vector->elem_no - 1]
                              .selector_t.ptr_value;

  /* now we can remove that element from the vector */
  iotc_vector_del(vector, vector->elem_no - 1);

  iotc_time_event_dispose_time_event(*cancelled_time_event);

  return IOTC_STATE_OK;
}

void iotc_time_event_destroy(iotc_vector_t* vector) {
  iotc_vector_for_each(vector, &iotc_time_event_destructor, NULL, 0);
}
