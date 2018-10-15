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

#include "iotc_timed_task.h"
#include "iotc_handle.h"

#define IOTC_MAX_TIMED_EVENT 64

typedef enum {
  IOTC_TTS_SCHEDULED,
  IOTC_TTS_RUNNING,
  IOTC_TTS_DELETABLE
} iotc_timed_task_state_e;

typedef struct iotc_timed_task_data_s {
  iotc_user_task_callback_t* callback;
  iotc_context_handle_t context_handle;
  void* data;
  iotc_time_event_handle_t delayed_event;
  iotc_evtd_instance_t* dispatcher;
  iotc_time_t seconds_repeat;
  iotc_timed_task_state_e state;
} iotc_timed_task_data_t;

iotc_state_t iotc_timed_task_callback_wrapper(void* void_task,
                                              void* void_scheduler);

iotc_timed_task_container_t* iotc_make_timed_task_container() {
  iotc_state_t state = IOTC_STATE_OK;

  IOTC_ALLOC(iotc_timed_task_container_t, container, state);

  container->timed_tasks_vector = iotc_vector_create();
  IOTC_CHECK_STATE(state = iotc_init_critical_section(&container->cs));

  return container;

err_handling:
  IOTC_SAFE_FREE(container);
  return NULL;
}

void iotc_destroy_timed_task_container(iotc_timed_task_container_t* container) {
  assert(NULL != container);
  iotc_vector_destroy(container->timed_tasks_vector);
  iotc_destroy_critical_section(&container->cs);
  IOTC_SAFE_FREE(container);
}

iotc_timed_task_handle_t iotc_add_timed_task(
    iotc_timed_task_container_t* container, iotc_evtd_instance_t* dispatcher,
    iotc_context_handle_t context_handle, iotc_user_task_callback_t* callback,
    iotc_time_t seconds_from_now, const uint8_t repeats_forever, void* data) {
  assert(NULL != container);
  assert(NULL != dispatcher);
  assert(IOTC_INVALID_CONTEXT_HANDLE < context_handle);
  assert(NULL != callback);

  iotc_state_t state = IOTC_STATE_OK;
  iotc_timed_task_handle_t task_handle = IOTC_INVALID_CONTEXT_HANDLE;

  IOTC_ALLOC(iotc_timed_task_data_t, task, state);

  task->context_handle = context_handle;
  task->callback = callback;
  task->data = data;
  task->dispatcher = dispatcher;
  task->seconds_repeat = (repeats_forever) ? seconds_from_now : 0;
  task->state = IOTC_TTS_SCHEDULED;

  iotc_lock_critical_section(container->cs);
  IOTC_CHECK_STATE(
      state = iotc_register_handle_for_object(container->timed_tasks_vector,
                                              IOTC_MAX_TIMED_EVENT, task));

  IOTC_CHECK_STATE(state = iotc_find_handle_for_object(
                       container->timed_tasks_vector, task, &task_handle));
  iotc_unlock_critical_section(container->cs);

  state =
      iotc_evtd_execute_in(dispatcher,
                           iotc_make_handle(&iotc_timed_task_callback_wrapper,
                                            (void*)task, (void*)container),
                           seconds_from_now, &task->delayed_event);

  IOTC_CHECK_STATE(state);

  return task_handle;

err_handling:
  IOTC_SAFE_FREE(task);
  return -state;
}

void iotc_remove_timed_task(iotc_timed_task_container_t* container,
                            iotc_timed_task_handle_t timed_task_handle) {
  assert(NULL != container);

  iotc_state_t state = IOTC_STATE_OK;

  iotc_lock_critical_section(container->cs);

  iotc_timed_task_data_t* task =
      (iotc_timed_task_data_t*)iotc_object_for_handle(
          container->timed_tasks_vector, timed_task_handle);

  if (NULL != task) {
    if (IOTC_TTS_SCHEDULED == task->state) {
      state = iotc_evtd_cancel(task->dispatcher, &task->delayed_event);
      assert(IOTC_STATE_OK == state);

      state =
          iotc_delete_handle_for_object(container->timed_tasks_vector, task);
      IOTC_UNUSED(state);

      /* POST-CONDITION */
      assert(IOTC_STATE_OK == state);

      IOTC_SAFE_FREE(task);
    } else if (IOTC_TTS_RUNNING == task->state) {
      task->state = IOTC_TTS_DELETABLE;
    }
  }

  iotc_unlock_critical_section(container->cs);
}

iotc_state_t iotc_timed_task_callback_wrapper(void* void_task,
                                              void* void_scheduler) {
  iotc_timed_task_data_t* task = (iotc_timed_task_data_t*)void_task;
  assert(NULL != task);
  iotc_timed_task_container_t* container =
      (iotc_timed_task_container_t*)void_scheduler;
  assert(NULL != container);

  iotc_state_t state = IOTC_STATE_OK;
  iotc_timed_task_handle_t task_handle = -1;

  iotc_lock_critical_section(container->cs);
  state = iotc_find_handle_for_object(container->timed_tasks_vector, task,
                                      &task_handle);
  if (IOTC_STATE_OK == state) {
    task->state = IOTC_TTS_RUNNING;
  }
  iotc_unlock_critical_section(container->cs);

  if (IOTC_STATE_OK == state) {
    assert(NULL != task->callback);
    assert(task->context_handle > IOTC_INVALID_CONTEXT_HANDLE);

    (task->callback)(task->context_handle, task_handle, task->data);

    iotc_lock_critical_section(container->cs);

    if (0 == task->seconds_repeat || IOTC_TTS_DELETABLE == task->state) {
      iotc_state_t del_state =
          iotc_delete_handle_for_object(container->timed_tasks_vector, task);

      IOTC_UNUSED(del_state);

      /* POST-CONDITION */
      assert(IOTC_STATE_OK == del_state);

      IOTC_SAFE_FREE(task);
    } else {
      state = iotc_evtd_execute_in(
          task->dispatcher,
          iotc_make_handle(&iotc_timed_task_callback_wrapper, (void*)task,
                           (void*)container),
          task->seconds_repeat, &task->delayed_event);
      assert(IOTC_STATE_OK == state);
      task->state = IOTC_TTS_SCHEDULED;
    }

    iotc_unlock_critical_section(container->cs);
  }

  return state;
}
