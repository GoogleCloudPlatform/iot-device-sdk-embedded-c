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

#include "iotc_mqtt_logic_layer.h"
#include "iotc.h"
#include "iotc_backoff_status_api.h"
#include "iotc_coroutine.h"
#include "iotc_event_handle.h"
#include "iotc_event_thread_dispatcher.h"
#include "iotc_globals.h"
#include "iotc_layer_default_functions.h"
#include "iotc_layer_macros.h"
#include "iotc_list.h"
#include "iotc_mqtt_logic_layer_commands.h"
#include "iotc_mqtt_logic_layer_data.h"
#include "iotc_mqtt_logic_layer_handlers.h"
#include "iotc_mqtt_logic_layer_helpers.h"
#include "iotc_mqtt_message.h"
#include "iotc_mqtt_parser.h"
#include "iotc_mqtt_serialiser.h"
#include "iotc_tuples.h"

#ifdef __cplusplus
extern "C" {
#endif

static void iotc_mqtt_logic_task_queue_shutdown(
    iotc_mqtt_logic_task_t** task_queue);

uint8_t iotc_mqtt_logic_layer_task_should_be_stored_predicate(
    void* context, iotc_mqtt_logic_task_t* task, int i);

static void iotc_mqtt_logic_task_queue_shutdown_wrap(void** task_queue) {
  assert(NULL != task_queue);
  assert(NULL != *task_queue);

  iotc_mqtt_logic_task_queue_shutdown((iotc_mqtt_logic_task_t**)task_queue);
}

uint8_t iotc_mqtt_logic_layer_task_should_be_stored_predicate(
    void* context, iotc_mqtt_logic_task_t* task, int i) {
  IOTC_UNUSED(i);
  IOTC_UNUSED(context);

  assert(NULL != task);

  if (task->session_state == IOTC_MQTT_LOGIC_TASK_SESSION_STORE) {
    return 1;
  }

  return 0;
}

static void iotc_mqtt_logic_layer_task_make_context_null(
    iotc_mqtt_logic_task_t* task) {
  assert(NULL != task);
  task->logic.handlers.h4.a1 = NULL;
}

iotc_state_t iotc_mqtt_logic_layer_push(void* context, void* data,
                                        iotc_state_t in_out_state) {
  IOTC_LAYER_FUNCTION_PRINT_FUNCTION_DIGEST();

  iotc_mqtt_logic_layer_data_t* layer_data =
      (iotc_mqtt_logic_layer_data_t*)IOTC_THIS_LAYER(context)->user_data;

  iotc_mqtt_logic_task_t* task = data;

  if (IOTC_THIS_LAYER_NOT_OPERATIONAL(context) || layer_data == 0) {
    iotc_debug_logger("no layer_data");
    goto err_handling;
  }

  /* If there's a problem then update the backoff penalty. */
  if (iotc_backoff_classify_state(in_out_state) ==
      IOTC_BACKOFF_CLASS_TERMINAL) {
    IOTC_PROCESS_CLOSE_ON_THIS_LAYER(context, NULL, IOTC_BACKOFF_TERMINAL);
    goto err_handling;
  }

  /* when it's just a sent-confirmation */
  if (in_out_state == IOTC_STATE_WRITTEN ||
      in_out_state == IOTC_STATE_FAILED_WRITING) {
    /* means that we have to continue on execution
     * within one of the task, we can find the task
     * by checking the msg id, msg id is encoded
     * via simple casting */
    iotc_mqtt_written_data_t* written_data = (iotc_mqtt_written_data_t*)data;

    uint16_t msg_id = written_data->a1;
    iotc_mqtt_type_t msg_type = written_data->a2;

    IOTC_SAFE_FREE_TUPLE(written_data);

    iotc_mqtt_logic_task_t* task_to_be_called = NULL;

    if (0 == msg_id) {
      /* This must have been a current_q0 task */
      task_to_be_called = layer_data->current_q0_task;
    } else {
      iotc_mqtt_message_class_t msg_class =
          iotc_mqtt_class_msg_type_sending(msg_type);

      iotc_mqtt_logic_task_t* task_queue = NULL;

      switch (msg_class) {
        case IOTC_MQTT_MESSAGE_CLASS_FROM_SERVER:
          task_queue = layer_data->q12_recv_tasks_queue;
          break;
        case IOTC_MQTT_MESSAGE_CLASS_TO_SERVER:
          task_queue = layer_data->q12_tasks_queue;
          break;
        case IOTC_MQTT_MESSAGE_CLASS_UNKNOWN:
          in_out_state = IOTC_MQTT_MESSAGE_CLASS_UNKNOWN_ERROR;
          goto err_handling;
        default:
          assert(1 == 0);
          break;
      }

      /* It is one of the qos12 tasks, find the proper task */
      IOTC_LIST_FIND(iotc_mqtt_logic_task_t, task_queue, CMP_TASK_MSG_ID,
                     msg_id, task_to_be_called);
    }

    /* restart the layer keepalive task timer after every every successful
     * send */
    if (IOTC_STATE_WRITTEN == in_out_state &&
        NULL != layer_data->keepalive_event.ptr_to_position) {
      iotc_state_t local_state = iotc_evtd_restart(
          IOTC_CONTEXT_DATA(context)->evtd_instance,
          &layer_data->keepalive_event,
          IOTC_CONTEXT_DATA(context)->connection_data->keepalive_timeout);

      IOTC_CHECK_STATE(local_state);
    }

    if (task_to_be_called != 0) {
      task_to_be_called->logic.handlers.h4.a3 = in_out_state;
      return iotc_evtd_execute_handle(&task_to_be_called->logic);
    }

    iotc_debug_logger(
        "task to be called not be found during processing of "
        "send-confirmation!");
    return IOTC_MQTT_LOGIC_UNKNOWN_TASK_ID;
  }

  assert(task->data.mqtt_settings.scenario <= IOTC_MQTT_SHUTDOWN);
  assert(task->priority <= IOTC_MQTT_LOGIC_TASK_IMMEDIATE);

  /* Assign the proper handler here.
   * We are passing the msg pointer as 0. It can be modified by injecting the
   * proper parameter later on. */
  switch (task->data.mqtt_settings.scenario) {
    case IOTC_MQTT_CONNECT: {
      task->logic =
          iotc_make_handle(&do_mqtt_connect, context, task, IOTC_STATE_OK, 0);
    } break;
    case IOTC_MQTT_PUBLISH: {
      switch (task->data.mqtt_settings.qos) {
        case IOTC_MQTT_QOS_AT_MOST_ONCE: {
          task->logic = iotc_make_handle(&do_mqtt_publish_q0, context, task,
                                         IOTC_STATE_OK, 0);
          break;
        }

        case IOTC_MQTT_QOS_AT_LEAST_ONCE: {
          task->logic = iotc_make_handle(&do_mqtt_publish_q1, context, task,
                                         IOTC_STATE_OK, 0);
          break;
        }

        case IOTC_MQTT_QOS_EXACTLY_ONCE: {
          iotc_debug_logger(
              "Sending publish with qos = "
              "IOTC_MQTT_QOS_EXACTLY_ONCE not implemented "
              "yet!");

          if (task->data.data_u != 0) {
            iotc_mqtt_logic_free_task_data(task);
          }

          break;
        }
      }
    } break;
    case IOTC_MQTT_SUBSCRIBE: {
      task->logic =
          iotc_make_handle(&do_mqtt_subscribe, context, task, IOTC_STATE_OK, 0);
    } break;
    case IOTC_MQTT_KEEPALIVE: {
      task->logic = iotc_make_handle(&do_mqtt_keepalive_task, context, task,
                                     IOTC_STATE_OK, 0);
    } break;
    case IOTC_MQTT_SHUTDOWN: {
      /* This case occurs if there's a shutdown already in progress.
         We should suppress a duplicate shutdown event. */
      if (layer_data->current_q0_task != NULL &&
          layer_data->current_q0_task->data.mqtt_settings.scenario ==
              IOTC_MQTT_SHUTDOWN) {
        iotc_mqtt_logic_free_task(&task);
        return IOTC_STATE_OK;
      }

      task->logic =
          iotc_make_handle(&do_shutdown, context, task, IOTC_STATE_OK, 0);
    } break;
    default:
      return IOTC_MQTT_LOGIC_WRONG_SCENARIO_TYPE;
  }

  run_task(context, task);

  return IOTC_STATE_OK;

err_handling:
  if (in_out_state == IOTC_STATE_WRITTEN ||
      in_out_state == IOTC_STATE_FAILED_WRITING) {
    IOTC_SAFE_FREE_TUPLE(data);
  } else {
    iotc_mqtt_logic_free_task(&task);
  }

  return in_out_state;
}

iotc_state_t iotc_mqtt_logic_layer_pull(void* context, void* data,
                                        iotc_state_t in_out_state) {
  IOTC_LAYER_FUNCTION_PRINT_FUNCTION_DIGEST();

  IOTC_UNUSED(context);

  iotc_mqtt_message_t* recvd_msg = (iotc_mqtt_message_t*)data;

  iotc_mqtt_logic_layer_data_t* layer_data =
      IOTC_THIS_LAYER(context)->user_data;

  if (IOTC_THIS_LAYER_NOT_OPERATIONAL(context) || layer_data == 0) {
    iotc_debug_logger("no layer_data");
    goto err_handling;
  }

  /* If there's a problem then update the backoff penalty. */
  if (iotc_update_backoff_penalty(in_out_state) ==
      IOTC_BACKOFF_CLASS_TERMINAL) {
    IOTC_PROCESS_CLOSE_ON_THIS_LAYER(context, NULL, IOTC_BACKOFF_TERMINAL);
    goto err_handling;
  }

  if (in_out_state == IOTC_STATE_OK) {
    assert(recvd_msg != 0); /* sanity check */

    uint16_t msg_id = iotc_mqtt_get_message_id(recvd_msg);

    /* First we have to make sure that it's not a publish.
     * A Publish could have been sent to the client from something
     * that was not started by our program.  Therefore the state and tasks
     * may not have been initialized yet, so we have to check it
     * explicitly. */
    iotc_debug_format("[m.id[%d] m.type[%d]] received msg", msg_id,
                      recvd_msg->common.common_u.common_bits.type);

    if (recvd_msg->common.common_u.common_bits.type == IOTC_MQTT_TYPE_PUBLISH) {
      return on_publish_recieved(context, recvd_msg, IOTC_STATE_OK);
    }

    /* qos > 0 than the id of the message is going to help the handler
     * otherway we are going to use the current qos_0 task */
    if (msg_id > 0) {
      iotc_mqtt_logic_task_t* task = 0;
      iotc_mqtt_logic_task_t* task_queue = 0;

      /** store the msg class */
      iotc_mqtt_message_class_t msg_class = iotc_mqtt_class_msg_type_receiving(
          (iotc_mqtt_type_t)recvd_msg->common.common_u.common_bits.type);

      /** pick proper msg queue */
      switch (msg_class) {
        case IOTC_MQTT_MESSAGE_CLASS_FROM_SERVER:
          task_queue = layer_data->q12_recv_tasks_queue;
          break;
        case IOTC_MQTT_MESSAGE_CLASS_TO_SERVER:
          task_queue = layer_data->q12_tasks_queue;
          break;
        case IOTC_MQTT_MESSAGE_CLASS_UNKNOWN:
        default:
          in_out_state = IOTC_MQTT_MESSAGE_CLASS_UNKNOWN_ERROR;
          goto err_handling;
      }

      IOTC_LIST_FIND(iotc_mqtt_logic_task_t, task_queue, CMP_TASK_MSG_ID,
                     msg_id, task);

      if (task != 0) /* got the task let's call the proper handler */
      {
        /* assign the message as a fourth parameter */
        task->logic.handlers.h4.a3 = in_out_state;
        task->logic.handlers.h4.a4 = recvd_msg;
        return iotc_evtd_execute_handle(&task->logic);
      } else {
        /* terrible error if we end up here it means that the message of
         * received id is broken or has been made very serious mistake
         * in logic of one of a handler function */
        iotc_debug_format("Error, can't find message with: id = %d", msg_id);

        iotc_debug_mqtt_message_dump(recvd_msg);
        iotc_mqtt_message_free(&recvd_msg);

        return IOTC_MQTT_UNKNOWN_MESSAGE_ID;
      }
    } else {
      /* sanity checks */
      assert(layer_data->current_q0_task != 0);
      assert(layer_data->current_q0_task->logic.handle_type !=
             IOTC_EVENT_HANDLE_UNSET);

      layer_data->current_q0_task->logic.handlers.h4.a4 = recvd_msg;
      layer_data->current_q0_task->logic.handlers.h4.a3 = in_out_state;

      return iotc_evtd_execute_handle(&layer_data->current_q0_task->logic);
    }
  }

err_handling:
  iotc_mqtt_message_free(&recvd_msg);
  return in_out_state;
}

iotc_state_t iotc_mqtt_logic_layer_init(void* context, void* data,
                                        iotc_state_t in_out_state) {
  IOTC_LAYER_FUNCTION_PRINT_FUNCTION_DIGEST();

  iotc_mqtt_logic_layer_data_t* layer_data =
      IOTC_THIS_LAYER(context)->user_data;
  iotc_context_data_t* context_data = IOTC_THIS_LAYER(context)->context_data;

  /* only if the layer_data has been wiped out that means that we are connecting
   * for the first time */
  if (layer_data == NULL) {
    IOTC_ALLOC_AT(iotc_mqtt_logic_layer_data_t,
                  IOTC_THIS_LAYER(context)->user_data, in_out_state);

    /* initialization of the unacked message dstr */
    context_data->copy_of_q12_unacked_messages_queue_dtor_ptr =
        &iotc_mqtt_logic_task_queue_shutdown_wrap;

    layer_data = IOTC_THIS_LAYER(context)->user_data;
  }

  assert(layer_data != NULL);

  if (IOTC_SESSION_CONTINUE ==
      IOTC_CONTEXT_DATA(context)->connection_data->session_type) {
    /* let's swap them with values so we are going to re-use the
     * handlers for topics from last session */
    layer_data->handlers_for_topics = context_data->copy_of_handlers_for_topics;
    context_data->copy_of_handlers_for_topics = NULL;

    /* same story goes with the qos1&2 unacked messages what we have to do is to
     * re-plug them into the queue and connect task will restart the tasks */
    layer_data->q12_tasks_queue =
        context_data->copy_of_q12_unacked_messages_queue;
    context_data->copy_of_q12_unacked_messages_queue = NULL;

    /* restoring the last_msg_id */
    layer_data->last_msg_id = context_data->copy_of_last_msg_id;
    context_data->copy_of_last_msg_id = 0;
  } else {
    if (NULL != context_data->copy_of_handlers_for_topics) {
      iotc_vector_for_each(context_data->copy_of_handlers_for_topics,
                           &iotc_mqtt_task_spec_data_free_subscribe_data_vec,
                           NULL, 0);

      context_data->copy_of_handlers_for_topics =
          iotc_vector_destroy(context_data->copy_of_handlers_for_topics);
    }

    /* clean the unsent QoS12 unacked tasks */
    iotc_mqtt_logic_task_t* saved_unacked_qos_12_queue =
        (iotc_mqtt_logic_task_t*)
            context_data->copy_of_q12_unacked_messages_queue;

    iotc_mqtt_logic_task_queue_shutdown(&saved_unacked_qos_12_queue);
    context_data->copy_of_q12_unacked_messages_queue = NULL;
  }

  /* if there was no copy or this is the fresh (re)start */
  if (NULL == layer_data->handlers_for_topics) {
    /* let's create fresh one */
    layer_data->handlers_for_topics = iotc_vector_create();
    IOTC_CHECK_MEMORY(layer_data->handlers_for_topics, in_out_state);
  }

  IOTC_CONTEXT_DATA(context)->connection_data->connection_state =
      IOTC_CONNECTION_STATE_OPENING;

  return IOTC_PROCESS_INIT_ON_PREV_LAYER(context, data, in_out_state);

err_handling:
  IOTC_SAFE_FREE(IOTC_THIS_LAYER(context)->user_data);
  return in_out_state;
}

iotc_state_t iotc_mqtt_logic_layer_connect(void* context, void* data,
                                           iotc_state_t in_out_state) {
  IOTC_LAYER_FUNCTION_PRINT_FUNCTION_DIGEST();

  /* if the state is not ok, let's just pass it to the next layer */
  if (in_out_state != IOTC_STATE_OK) {
    return IOTC_PROCESS_CONNECT_ON_NEXT_LAYER(context, data, in_out_state);
  }

  IOTC_ALLOC(iotc_mqtt_logic_task_t, task, in_out_state);

  task->data.mqtt_settings.scenario = IOTC_MQTT_CONNECT;
  task->data.mqtt_settings.qos = IOTC_MQTT_QOS_AT_MOST_ONCE;
  task->priority = IOTC_MQTT_LOGIC_TASK_IMMEDIATE;

  /* push the task directly into the queue */
  return iotc_mqtt_logic_layer_push(context, task, IOTC_STATE_OK);

err_handling:
  return in_out_state;
}

iotc_state_t iotc_mqtt_logic_layer_post_connect(void* context, void* data,
                                                iotc_state_t in_out_state) {
  IOTC_LAYER_FUNCTION_PRINT_FUNCTION_DIGEST();

  const iotc_mqtt_logic_layer_data_t* layer_data =
      (iotc_mqtt_logic_layer_data_t*)IOTC_THIS_LAYER(context)->user_data;

  if (NULL == layer_data) {
    return in_out_state;
  }

  /* set new context and send timeout which will make the qos12 tasks to
   * continue they work just where they were stopped */
  IOTC_LIST_FOREACH_WITH_ARG(iotc_mqtt_logic_task_t,
                             layer_data->q12_tasks_queue,
                             set_new_context_and_call_resend, context);

  return iotc_layer_default_post_connect(context, data, in_out_state);
}

iotc_state_t iotc_mqtt_logic_layer_close(void* context, void* data,
                                         iotc_state_t in_out_state) {
  IOTC_LAYER_FUNCTION_PRINT_FUNCTION_DIGEST();

  const iotc_mqtt_logic_layer_data_t* layer_data =
      (iotc_mqtt_logic_layer_data_t*)IOTC_THIS_LAYER(context)->user_data;

  return (NULL == layer_data)
             ? IOTC_PROCESS_CLOSE_EXTERNALLY_ON_THIS_LAYER(context, data,
                                                           in_out_state)
             : IOTC_PROCESS_CLOSE_ON_PREV_LAYER(context, data, in_out_state);
}

static void iotc_mqtt_logic_task_queue_shutdown(
    iotc_mqtt_logic_task_t** task_queue) {
  /* PRE-CONDITION */
  assert(NULL != task_queue);

  /* each q12 task shall delete it's internal data on any try to continue */
  while (*task_queue) {
    iotc_mqtt_logic_task_t* tmp_task = NULL;

    IOTC_LIST_POP(iotc_mqtt_logic_task_t, *task_queue, tmp_task);

    /* sanity check */
    assert(NULL != tmp_task);
    assert(NULL == tmp_task->timeout.ptr_to_position);

    iotc_mqtt_logic_free_task(&tmp_task);
  }

  /* POST-CONDITION */
  assert(NULL == *task_queue);
}

iotc_state_t iotc_mqtt_logic_layer_close_externally(void* context, void* data,
                                                    iotc_state_t in_out_state) {
  IOTC_LAYER_FUNCTION_PRINT_FUNCTION_DIGEST();
  IOTC_UNUSED(in_out_state);

  iotc_context_data_t* context_data = IOTC_CONTEXT_DATA(context);
  iotc_evtd_instance_t* event_dispatcher = context_data->evtd_instance;

  iotc_layer_state_t layer_state = IOTC_THIS_LAYER_STATE(context);

  if (IOTC_STATE_OK != in_out_state &&
      IOTC_LAYER_STATE_CONNECTING == layer_state) {
    /* error handling */
    return IOTC_PROCESS_CONNECT_ON_THIS_LAYER(context, data, in_out_state);
  }

  iotc_mqtt_logic_layer_data_t* layer_data =
      IOTC_THIS_LAYER(context)->user_data;

  if (NULL == layer_data) {
    return IOTC_PROCESS_CLOSE_EXTERNALLY_ON_NEXT_LAYER(context, data,
                                                       in_out_state);
  }

  iotc_update_backoff_penalty(in_out_state);

  /* disable timeouts of all tasks */
  IOTC_LIST_FOREACH_WITH_ARG(iotc_mqtt_logic_task_t,
                             layer_data->q12_tasks_queue, cancel_task_timeout,
                             context);

  IOTC_LIST_FOREACH_WITH_ARG(iotc_mqtt_logic_task_t,
                             layer_data->q12_recv_tasks_queue,
                             cancel_task_timeout, context);

  /* if clean session not set check if we have anything to copy */
  if (IOTC_SESSION_CONTINUE == context_data->connection_data->session_type) {
    iotc_context_data_t* context_data = IOTC_THIS_LAYER(context)->context_data;
    /* this sets the copy of last_msg_id */
    context_data->copy_of_last_msg_id = layer_data->last_msg_id;

    /* this will copy the handlers for topics */
    if (layer_data->handlers_for_topics != NULL &&
        layer_data->handlers_for_topics->elem_no > 0) {
      /* sanity check, let's make sure that it is empty */
      assert(
          IOTC_THIS_LAYER(context)->context_data->copy_of_handlers_for_topics ==
          NULL);

      context_data->copy_of_handlers_for_topics =
          layer_data->handlers_for_topics;

      layer_data->handlers_for_topics = NULL;
    }

    /* here we are going to prereserve the qos12 send queue */
    /* we have to filter it in order to see which one has already been started
     */
    /* we will construct new list out of them */
    iotc_mqtt_logic_task_t* unacked_list = NULL;

    IOTC_LIST_SPLIT_I(iotc_mqtt_logic_task_t, layer_data->q12_tasks_queue,
                      iotc_mqtt_logic_layer_task_should_be_stored_predicate,
                      context, unacked_list);

    context_data->copy_of_q12_unacked_messages_queue = unacked_list;

    IOTC_LIST_FOREACH(iotc_mqtt_logic_task_t, unacked_list,
                      iotc_mqtt_logic_layer_task_make_context_null);
  }

  /* if the handlers for topics are left alone than it means
   * that it has to be freed */
  if (layer_data->handlers_for_topics != NULL) {
    iotc_vector_for_each(layer_data->handlers_for_topics,
                         &iotc_mqtt_task_spec_data_free_subscribe_data_vec,
                         NULL, 0);
    iotc_vector_destroy(layer_data->handlers_for_topics);
  }

  /* let's stop the current task */
  if (layer_data->current_q0_task != 0) {
    if (NULL != layer_data->current_q0_task->timeout.ptr_to_position) {
      iotc_evtd_cancel(event_dispatcher, &layer_data->current_q0_task->timeout);
    }

    layer_data->current_q0_task->logic.handlers.h4.a3 = IOTC_STATE_TIMEOUT;
    iotc_evtd_execute_handle(&layer_data->current_q0_task->logic);
    iotc_mqtt_logic_free_task(&layer_data->current_q0_task);
  }

  /* unregister keepalive */
  if (NULL != layer_data->keepalive_event.ptr_to_position) {
    iotc_evtd_cancel(event_dispatcher, &layer_data->keepalive_event);
  }

  /* sanity check */
  assert(layer_data != 0);

  /* save queues */
  iotc_mqtt_logic_task_t* current_q0 = layer_data->current_q0_task;
  iotc_mqtt_logic_task_t* q12_queue = layer_data->q12_tasks_queue;
  iotc_mqtt_logic_task_t* q12_recv_queue = layer_data->q12_recv_tasks_queue;
  iotc_mqtt_logic_task_t* q0_queue = layer_data->q0_tasks_queue;

  /* destroy user's data */
  IOTC_SAFE_FREE(IOTC_THIS_LAYER(context)->user_data);

  iotc_mqtt_logic_task_queue_shutdown(&q12_queue);
  iotc_mqtt_logic_task_queue_shutdown(&q12_recv_queue);

  /* special case for q0 task */
  if (current_q0) {
    abort_task(current_q0);
    iotc_mqtt_logic_free_task(&current_q0);
  }

  /* as for q0, they haven't been started so they must be just deleted */
  while (q0_queue) {
    iotc_mqtt_logic_task_t* tmp_task = 0;

    IOTC_LIST_POP(iotc_mqtt_logic_task_t, q0_queue, tmp_task);

    /* sanity check */
    assert(tmp_task != 0);
    assert(tmp_task->timeout.ptr_to_position == 0);

    iotc_mqtt_logic_free_task(&tmp_task);
  }

  return IOTC_PROCESS_CLOSE_EXTERNALLY_ON_NEXT_LAYER(context, data,
                                                     in_out_state);
}

#ifdef __cplusplus
}
#endif
