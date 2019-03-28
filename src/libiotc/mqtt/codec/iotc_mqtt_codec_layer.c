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

#include "iotc_mqtt_codec_layer.h"
#include "iotc.h"
#include "iotc_coroutine.h"
#include "iotc_layer_api.h"
#include "iotc_layer_macros.h"
#include "iotc_list.h"
#include "iotc_mqtt_message.h"
#include "iotc_mqtt_parser.h"
#include "iotc_mqtt_serialiser.h"
#include "iotc_tuples.h"

#ifdef __cplusplus
extern "C" {
#endif

static void clear_task_queue(void* context) {
  /* PRE-CONDITIONS */
  assert(context != 0);

  iotc_mqtt_codec_layer_data_t* layer_data =
      (iotc_mqtt_codec_layer_data_t*)IOTC_THIS_LAYER(context)->user_data;

  /* clean the queue */
  while (layer_data->task_queue) {
    iotc_mqtt_codec_layer_task_t* tmp_task = 0;

    IOTC_LIST_POP(iotc_mqtt_codec_layer_task_t, layer_data->task_queue,
                  tmp_task);

    iotc_mqtt_written_data_t* written_data =
        (iotc_mqtt_written_data_t*)iotc_alloc_make_tuple(
            iotc_mqtt_written_data_t, tmp_task->msg_id, tmp_task->msg_type);

    IOTC_PROCESS_PUSH_ON_NEXT_LAYER(context, written_data,
                                    IOTC_STATE_FAILED_WRITING);

    iotc_mqtt_codec_layer_free_task(&tmp_task);
  }

  /* POST_CONDITIONS */
  assert(layer_data->task_queue == 0);
}

iotc_state_t iotc_mqtt_codec_layer_push(void* context, void* data,
                                        iotc_state_t in_out_state) {
  IOTC_LAYER_FUNCTION_PRINT_FUNCTION_DIGEST();

  iotc_mqtt_codec_layer_data_t* layer_data =
      (iotc_mqtt_codec_layer_data_t*)IOTC_THIS_LAYER(context)->user_data;

  iotc_mqtt_message_t* msg = (iotc_mqtt_message_t*)data;
  uint8_t* buffer = NULL;
  iotc_data_desc_t* data_desc = NULL;
  size_t msg_contents_size = 0;
  size_t remaining_len = 0;
  size_t publish_payload_len = 0;
  iotc_mqtt_serialiser_rc_t rc = IOTC_MQTT_SERIALISER_RC_ERROR;
  iotc_data_desc_t* payload_desc = NULL;

  iotc_mqtt_serialiser_t serializer;

  if (IOTC_THIS_LAYER_NOT_OPERATIONAL(context) || NULL == layer_data) {
    /* cleaning of unfinished requests */
    iotc_mqtt_message_free(&msg);
    return IOTC_STATE_OK;
  }

  iotc_mqtt_codec_layer_task_t* task =
      (iotc_mqtt_codec_layer_task_t*)layer_data->task_queue;

  /* There can be only one task that is being sent.
   * If msg != 0 means that we have a notification from next layer. */
  if (IOTC_STATE_OK == in_out_state && NULL != msg) {
    iotc_mqtt_codec_layer_task_t* new_task =
        iotc_mqtt_codec_layer_make_task(msg);

    IOTC_CHECK_MEMORY(new_task, in_out_state);

    IOTC_LIST_PUSH_BACK(iotc_mqtt_codec_layer_task_t, layer_data->task_queue,
                        new_task);

    if (IOTC_CR_IS_RUNNING(layer_data->push_cs)) {
      return IOTC_STATE_OK;
    }
  } else if (in_out_state == IOTC_STATE_WANT_WRITE) {
    /* Additional check. */
    assert(layer_data->push_cs <= 2);

    /* Make the task to continue it's processing with re-attached msg. */
    iotc_mqtt_codec_layer_continue_task(task, msg);
  }

  if (NULL != msg) {
    iotc_debug_mqtt_message_dump(msg);
  }

  /*------------------------------ BEGIN COROUTINE ----------------------- */
  IOTC_CR_START(layer_data->push_cs);

  iotc_mqtt_serialiser_init(&serializer);

  IOTC_CHECK_MEMORY(msg, in_out_state);
  layer_data->msg_id = iotc_mqtt_get_message_id(msg);
  layer_data->msg_type =
      (iotc_mqtt_type_t)msg->common.common_u.common_bits.type;

  in_out_state = iotc_mqtt_serialiser_size(&msg_contents_size, &remaining_len,
                                           &publish_payload_len, NULL, msg);

  IOTC_CHECK_STATE(in_out_state);

  iotc_debug_format("[m.id[%d] m.type[%d]] encoding", layer_data->msg_id,
                    msg->common.common_u.common_bits.type);

  msg_contents_size -= publish_payload_len;

  data_desc = iotc_make_empty_desc_alloc(msg_contents_size);

  IOTC_CHECK_MEMORY(data_desc, in_out_state);

  /* If it's publish then the payload is sent separately
   * for more details check serialiser implementation and this function
   * at line 169. */
  rc = iotc_mqtt_serialiser_write(&serializer, msg, data_desc,
                                  msg_contents_size, remaining_len);

  if (rc == IOTC_MQTT_SERIALISER_RC_ERROR) {
    iotc_debug_format(
        "[m.id[%d] m.type[%d]] mqtt_codec_layer serialization error",
        layer_data->msg_id, layer_data->msg_type);

    in_out_state = IOTC_MQTT_SERIALIZER_ERROR;

    goto err_handling;
  }

  iotc_debug_format("[m.id[%d] m.type[%d]] mqtt_codec_layer sending message",
                    layer_data->msg_id, layer_data->msg_type);

  IOTC_CR_YIELD(layer_data->push_cs, IOTC_PROCESS_PUSH_ON_PREV_LAYER(
                                         context, data_desc, in_out_state));

  /* Here the state must be either WRITTEN or FAILED_WRITING
   * sanity check to valid the state. */
  assert(IOTC_STATE_WRITTEN == in_out_state ||
         IOTC_STATE_FAILED_WRITING == in_out_state);

  /* PRE-CONTINUE-CONDITIONS */
  assert(NULL != task);

  /* Restore msg pointer since it is no longer available
   * from the function argument. */
  msg = task->msg;

  /* PRE-CONTINUE-CONDITIONS */
  assert(NULL != msg);

  /* If header failed, then there's no need to. */
  if (IOTC_STATE_WRITTEN != in_out_state) {
    goto finalise;
  }

  /* If publish and not empty payload then send the payload. */
  if (IOTC_MQTT_TYPE_PUBLISH == msg->common.common_u.common_bits.type &&
      msg->publish.content->length > 0) {
    /* make a new desc but keep sharing memory */
    payload_desc = iotc_make_desc_from_buffer_share(
        msg->publish.content->data_ptr, msg->publish.content->length);

    IOTC_CHECK_MEMORY(payload_desc, in_out_state);

    IOTC_CR_YIELD(
        layer_data->push_cs,
        IOTC_PROCESS_PUSH_ON_PREV_LAYER(context, payload_desc, IOTC_STATE_OK));
  }

finalise: /* Common part for all messages. */
  if (IOTC_STATE_WRITTEN == in_out_state) {
    iotc_debug_format("[m.id[%d] m.type[%d]] mqtt_codec_layer message sent",
                      layer_data->msg_id, layer_data->msg_type);
  } else {
    iotc_debug_format("[m.id[%d] m.type[%d]] mqtt_codec_layer message not sent",
                      layer_data->msg_id, layer_data->msg_type);
  }

  /* PRE-CONDITIONS */
  assert(NULL != task);
  assert(NULL != task->msg);

  /* Releases the msg memory as it's no longer needed. */
  iotc_mqtt_message_free(&task->msg);

  /* Here the state must be either WRITTEN or FAILED_WRITING
   * sanity check to valid the state. */
  assert(in_out_state == IOTC_STATE_WRITTEN ||
         in_out_state == IOTC_STATE_FAILED_WRITING);

  iotc_mqtt_written_data_t* written_data = iotc_alloc_make_tuple(
      iotc_mqtt_written_data_t, layer_data->msg_id, layer_data->msg_type);

  IOTC_CHECK_MEMORY(written_data, in_out_state);

  IOTC_PROCESS_PUSH_ON_NEXT_LAYER(context, written_data, in_out_state);

  IOTC_LIST_POP(iotc_mqtt_codec_layer_task_t, layer_data->task_queue, task);

  /* Release the task as it's no longer required. */
  iotc_mqtt_codec_layer_free_task(&task);

  /* Pop the next task and register it's execution. */
  if (NULL != layer_data->task_queue) {
    task = layer_data->task_queue;

    iotc_mqtt_message_t* msg_to_send =
        iotc_mqtt_codec_layer_activate_task(task);

    IOTC_CR_EXIT(layer_data->push_cs,
                 iotc_mqtt_codec_layer_push(context, msg_to_send,
                                            IOTC_STATE_WANT_WRITE));
  }

  IOTC_CR_EXIT(layer_data->push_cs, IOTC_STATE_OK);

  IOTC_CR_END();

err_handling:
  iotc_debug_format("something went wrong during mqtt message encoding: %s",
                    iotc_get_state_string(in_out_state));

  IOTC_SAFE_FREE(buffer);
  IOTC_SAFE_FREE(data_desc);
  clear_task_queue(context);
  IOTC_CR_RESET(layer_data->push_cs);

  return IOTC_PROCESS_PUSH_ON_NEXT_LAYER(context, data,
                                         IOTC_STATE_FAILED_WRITING);
}

iotc_state_t iotc_mqtt_codec_layer_pull(void* context, void* data,
                                        iotc_state_t in_out_state) {
  IOTC_LAYER_FUNCTION_PRINT_FUNCTION_DIGEST();

  IOTC_UNUSED(data);
  IOTC_UNUSED(in_out_state);

  iotc_mqtt_codec_layer_data_t* layer_data =
      (iotc_mqtt_codec_layer_data_t*)IOTC_THIS_LAYER(context)->user_data;

  iotc_data_desc_t* data_desc = (iotc_data_desc_t*)data;

  if (IOTC_THIS_LAYER_NOT_OPERATIONAL(context) || layer_data == 0) {
    iotc_free_desc(&data_desc);
    return IOTC_STATE_OK;
  }

  if (in_out_state != IOTC_STATE_OK) {
    goto err_handling;
  }

  IOTC_CR_START(layer_data->pull_cs);

  assert(layer_data->msg == 0);

  IOTC_ALLOC_AT(iotc_mqtt_message_t, layer_data->msg, in_out_state);

  iotc_mqtt_parser_init(&layer_data->parser);

  do {
    layer_data->local_state = iotc_mqtt_parser_execute(
        &layer_data->parser, layer_data->msg, data_desc);

    if (layer_data->local_state == IOTC_STATE_WANT_READ) {
      iotc_free_desc(&data_desc);
    }

    IOTC_CR_YIELD_UNTIL(layer_data->pull_cs,
                        (layer_data->local_state == IOTC_STATE_WANT_READ),
                        IOTC_STATE_OK);
  } while (in_out_state == IOTC_STATE_OK &&
           layer_data->local_state == IOTC_STATE_WANT_READ);

  if (in_out_state != IOTC_STATE_OK ||
      layer_data->local_state != IOTC_STATE_OK) {
    iotc_debug_logger("error while reading msg!");
    goto err_handling;
  }

  iotc_debug_format("[m.id[%d] m.type[%d]] msg decoded!",
                    iotc_mqtt_get_message_id(layer_data->msg),
                    layer_data->msg->common.common_u.common_bits.type);

  iotc_debug_mqtt_message_dump(layer_data->msg);

  iotc_mqtt_message_t* recvd = layer_data->msg;
  layer_data->msg = NULL;

  /* Register next stage of processing. */
  IOTC_PROCESS_PULL_ON_NEXT_LAYER(context, recvd, layer_data->local_state);

  /* Reset the coroutine since it has to read new msg. */
  IOTC_CR_RESET(layer_data->pull_cs);

  /* Delete the buffer if limit reached or previous call failed
   * reading the message. */
  if (data_desc->curr_pos == data_desc->length ||
      layer_data->local_state != IOTC_STATE_OK) {
    /* delete buffer no longer needed */
    iotc_free_desc(&data_desc);
  } else {
    /* Call this function recursively cause there might be next message. */
    return iotc_mqtt_codec_layer_pull(context, data, in_out_state);
  }

  IOTC_CR_EXIT(layer_data->pull_cs, IOTC_STATE_OK);
  IOTC_CR_END();

err_handling:
  iotc_mqtt_message_free(&layer_data->msg);

  if (data_desc != 0) {
    iotc_free_desc(&data_desc);
  }

  if (layer_data) {
    /* Reset the coroutine state. */
    IOTC_CR_RESET(layer_data->pull_cs);
    clear_task_queue(context);
  }

  return IOTC_PROCESS_CLOSE_ON_PREV_LAYER(
      context, 0, IOTC_MAX(in_out_state, layer_data->local_state));
}

iotc_state_t iotc_mqtt_codec_layer_init(void* context, void* data,
                                        iotc_state_t in_out_state) {
  IOTC_LAYER_FUNCTION_PRINT_FUNCTION_DIGEST();

  assert(IOTC_THIS_LAYER(context)->user_data == 0);

  IOTC_ALLOC_AT(iotc_mqtt_codec_layer_data_t,
                IOTC_THIS_LAYER(context)->user_data, in_out_state);

  return IOTC_PROCESS_INIT_ON_PREV_LAYER(context, data, in_out_state);

err_handling:
  IOTC_SAFE_FREE(IOTC_THIS_LAYER(context)->user_data);

  return IOTC_PROCESS_INIT_ON_PREV_LAYER(context, data, in_out_state);
}

iotc_state_t iotc_mqtt_codec_layer_connect(void* context, void* data,
                                           iotc_state_t in_out_state) {
  IOTC_LAYER_FUNCTION_PRINT_FUNCTION_DIGEST();
  return IOTC_PROCESS_CONNECT_ON_NEXT_LAYER(context, data, in_out_state);
}

iotc_state_t iotc_mqtt_codec_layer_close(void* context, void* data,
                                         iotc_state_t in_out_state) {
  IOTC_LAYER_FUNCTION_PRINT_FUNCTION_DIGEST();

  const iotc_mqtt_codec_layer_data_t* layer_data =
      (iotc_mqtt_codec_layer_data_t*)IOTC_THIS_LAYER(context)->user_data;

  return (NULL == layer_data)
             ? IOTC_PROCESS_CLOSE_EXTERNALLY_ON_THIS_LAYER(context, data,
                                                           in_out_state)
             : IOTC_PROCESS_CLOSE_ON_PREV_LAYER(context, data, in_out_state);
}

iotc_state_t iotc_mqtt_codec_layer_close_externally(void* context, void* data,
                                                    iotc_state_t in_out_state) {
  IOTC_LAYER_FUNCTION_PRINT_FUNCTION_DIGEST();

  iotc_mqtt_codec_layer_data_t* layer_data =
      (iotc_mqtt_codec_layer_data_t*)IOTC_THIS_LAYER(context)->user_data;

  if (layer_data) {
    clear_task_queue(context);
    IOTC_CR_RESET(layer_data->push_cs);

    iotc_mqtt_message_free(&layer_data->msg);

    IOTC_SAFE_FREE(IOTC_THIS_LAYER(context)->user_data);
  }

  return IOTC_PROCESS_CLOSE_EXTERNALLY_ON_NEXT_LAYER(context, data,
                                                     in_out_state);
}

#ifdef __cplusplus
}
#endif
