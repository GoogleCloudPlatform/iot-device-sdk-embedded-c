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

#ifndef __IOTC_MQTT_LOGIC_LAYER_CONNECT_COMMAND_H__
#define __IOTC_MQTT_LOGIC_LAYER_CONNECT_COMMAND_H__

#include "iotc_backoff_status_api.h"
#include "iotc_coroutine.h"
#include "iotc_event_thread_dispatcher.h"
#include "iotc_globals.h"
#include "iotc_io_timeouts.h"
#include "iotc_jwt.h"
#include "iotc_layer_api.h"
#include "iotc_mqtt_logic_layer_data.h"
#include "iotc_mqtt_logic_layer_data_helpers.h"
#include "iotc_mqtt_logic_layer_task_helpers.h"
#include "iotc_mqtt_message.h"

#ifdef __cplusplus
extern "C" {
#endif

#include "iotc_mqtt_logic_layer_keepalive_handler.h"

static inline iotc_state_t get_error_from_connack(int return_code) {
  switch (return_code) {
    case 0x01:
      return IOTC_MQTT_UNACCEPTABLE_PROTOCOL_VERSION;
    case 0x02:
      return IOTC_MQTT_IDENTIFIER_REJECTED;
    case 0x03:
      return IOTC_MQTT_SERVER_UNAVAILIBLE;
    case 0x04:
      return IOTC_MQTT_BAD_USERNAME_OR_PASSWORD;
    case 0x05:
      return IOTC_MQTT_NOT_AUTHORIZED;
    default:
      return IOTC_MQTT_CONNECT_UNKNOWN_RETURN_CODE;
  }
}

/**
 * @brief do_mqtt_connect_timeout
 *
 * Connection timeout, remove event from timeout event vector, exit.
 */

static inline iotc_event_handle_return_t do_mqtt_connect_timeout(
    iotc_event_handle_arg1_t arg1, iotc_event_handle_arg2_t arg2) {
  iotc_layer_connectivity_t* context = (iotc_layer_connectivity_t*)arg1;
  iotc_mqtt_logic_task_t* task = (iotc_mqtt_logic_task_t*)arg2;

  /* Remove finished timeout event from context's io timeout vector. */
  iotc_io_timeouts_remove(&task->timeout,
                          context->self->context_data->io_timeouts);
  assert(NULL == task->timeout.ptr_to_position);

  IOTC_PROCESS_CONNECT_ON_NEXT_LAYER(context, arg2, IOTC_STATE_TIMEOUT);

  IOTC_CR_EXIT(task->cs, iotc_mqtt_logic_layer_finalize_task(context, task));
}

static inline iotc_state_t do_mqtt_connect(
    void* ctx /* Should be the context of the logic layer. */
    ,
    void* data, iotc_state_t state, void* msg_data) {
  iotc_layer_connectivity_t* context = (iotc_layer_connectivity_t*)ctx;
  iotc_mqtt_logic_task_t* task = (iotc_mqtt_logic_task_t*)data;

  iotc_mqtt_logic_layer_data_t* layer_data =
      (iotc_mqtt_logic_layer_data_t*)IOTC_THIS_LAYER(context)->user_data;

  iotc_mqtt_message_t* msg_memory = (iotc_mqtt_message_t*)msg_data;
  iotc_evtd_instance_t* event_dispatcher =
      IOTC_CONTEXT_DATA(context)->evtd_instance;

  if (IOTC_THIS_LAYER_NOT_OPERATIONAL(context) || layer_data == 0) {
    iotc_mqtt_message_free(&msg_memory);

    cancel_task_timeout(task, context);
    IOTC_CR_EXIT(task->cs, IOTC_STATE_OK);
  }

  IOTC_CR_START(task->cs);

  /* Fill in the fields which are required for serializing the MQTT connect
   * message. */
  IOTC_ALLOC_AT(iotc_mqtt_message_t, msg_memory, state);
  IOTC_CHECK_STATE(
      state = fill_with_connect_data(
          msg_memory,
          IOTC_CONTEXT_DATA(context)->connection_data->username,
          IOTC_CONTEXT_DATA(context)->connection_data->password,
          IOTC_CONTEXT_DATA(context)->connection_data->client_id,
          IOTC_CONTEXT_DATA(context)->connection_data->keepalive_timeout,
          IOTC_CONTEXT_DATA(context)->connection_data->session_type,
          IOTC_CONTEXT_DATA(context)->connection_data->will_topic,
          IOTC_CONTEXT_DATA(context)->connection_data->will_message,
          IOTC_CONTEXT_DATA(context)->connection_data->will_qos,
          IOTC_CONTEXT_DATA(context)->connection_data->will_retain));

  if (IOTC_CONTEXT_DATA(context)->connection_data->connection_timeout > 0) {
    state = iotc_io_timeouts_create(
        iotc_globals.evtd_instance,
        iotc_make_handle(&do_mqtt_connect_timeout, context, task),
        IOTC_CONTEXT_DATA(context)->connection_data->connection_timeout,
        context->self->context_data->io_timeouts, &task->timeout);

    IOTC_CHECK_STATE(state);
  } else {
    assert(NULL == task->timeout.ptr_to_position);
  }

  /* Queue sending and wait till it's sent. */
  IOTC_CR_YIELD(task->cs, IOTC_PROCESS_PUSH_ON_PREV_LAYER(context, msg_memory,
                                                          IOTC_STATE_OK));

  if (state == IOTC_STATE_WRITTEN) {
    iotc_debug_logger("connect message has been sent");
  } else {
    /* Inform the next layer about a state change. */
    IOTC_PROCESS_CONNECT_ON_NEXT_LAYER(context, data, state);

    goto err_handling;
  }

  /* Wait till the answer. */
  IOTC_CR_YIELD(task->cs, IOTC_STATE_OK);

  /* Parse the response. */
  if (msg_memory->common.common_u.common_bits.type == IOTC_MQTT_TYPE_CONNACK) {
    /* Cancel the io timeout. */
    if (NULL != task->timeout.ptr_to_position) {
      iotc_io_timeouts_cancel(iotc_globals.evtd_instance, &task->timeout,
                              IOTC_CONTEXT_DATA(context)->io_timeouts);
      assert(NULL == task->timeout.ptr_to_position);
    }

    if (msg_memory->connack.return_code == 0) {
      iotc_mqtt_message_free(&msg_memory);

      if (IOTC_CONTEXT_DATA(context)->connection_data->keepalive_timeout > 0) {
        state = iotc_evtd_execute_in(
            event_dispatcher,
            iotc_make_handle(&do_mqtt_keepalive_once, context),
            IOTC_CONTEXT_DATA(context)->connection_data->keepalive_timeout,
            &layer_data->keepalive_event);

        IOTC_CHECK_STATE(state);
      }

      /* Now the layer is fully connected to the server. */
      IOTC_CONTEXT_DATA(context)->connection_data->connection_state =
          IOTC_CONNECTION_STATE_OPENED;

      /* Inform the next layer about a state change. */
      IOTC_PROCESS_CONNECT_ON_NEXT_LAYER(context, data, state);

      IOTC_CR_EXIT(task->cs,
                   iotc_mqtt_logic_layer_finalize_task(context, task));
    } else {
      iotc_debug_format("connack.return_code == %d",
                        msg_memory->connack.return_code);

      state = get_error_from_connack(msg_memory->connack.return_code);

      /* Inform the next layer about a state change. */
      IOTC_PROCESS_CONNECT_ON_NEXT_LAYER(context, data, state);

      iotc_mqtt_message_free(&msg_memory);

      /* Cancel io timeout. */
      if (NULL != task->timeout.ptr_to_position) {
        iotc_io_timeouts_cancel(iotc_globals.evtd_instance, &task->timeout,
                                IOTC_CONTEXT_DATA(context)->io_timeouts);
        assert(NULL == task->timeout.ptr_to_position);
      }

      IOTC_CR_EXIT(task->cs,
                   iotc_mqtt_logic_layer_finalize_task(context, task));
    }
  }

  iotc_mqtt_message_free(&msg_memory);

  /* Cancel io timeout. */
  if (NULL != task->timeout.ptr_to_position) {
    iotc_io_timeouts_cancel(iotc_globals.evtd_instance, &task->timeout,
                            IOTC_CONTEXT_DATA(context)->io_timeouts);
    assert(NULL == task->timeout.ptr_to_position);
  }

  IOTC_CR_EXIT(task->cs, iotc_mqtt_logic_layer_finalize_task(context, task));

err_handling:
  iotc_mqtt_message_free(&msg_memory);

  /* Cancel io timeout. */
  if (NULL != task->timeout.ptr_to_position) {
    iotc_io_timeouts_cancel(iotc_globals.evtd_instance, &task->timeout,
                            IOTC_CONTEXT_DATA(context)->io_timeouts);
    assert(NULL == task->timeout.ptr_to_position);
  }

  IOTC_CR_EXIT(task->cs, iotc_mqtt_logic_layer_finalize_task(context, task));

  IOTC_CR_END();
}

#ifdef __cplusplus
}
#endif

#endif /* __IOTC_MQTT_LOGIC_LAYER_CONNECT_COMMAND_H__ */
