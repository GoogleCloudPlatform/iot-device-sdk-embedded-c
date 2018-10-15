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

#include "iotc_driver_codec_protobuf_layer.h"
#include "iotc_control_channel_protocol.pb-c.h"
#include "iotc_data_desc.h"
#include "iotc_debug.h"
#include "iotc_driver_codec_protobuf_layer_data.h"
#include "iotc_layer_api.h"
#include "iotc_layer_macros.h"
#include "iotc_macros.h"

void iotc_driver_free_protobuf_callback(
    struct _XiClientFtestFw__XiClientCallback* callback) {
  IOTC_SAFE_FREE(callback->on_connect_finish);
  IOTC_SAFE_FREE(callback->on_disconnect);

  if (NULL != callback->on_subscribe_finish) {
    IOTC_SAFE_FREE(callback->on_subscribe_finish->subscribe_result_list);
  }

  IOTC_SAFE_FREE(callback->on_subscribe_finish);

  if (NULL != callback->on_message_received) {
    if (1 == callback->on_message_received->has_payload) {
      IOTC_SAFE_FREE(callback->on_message_received->payload.data);
    }
  }

  IOTC_SAFE_FREE(callback->on_message_received);
  IOTC_SAFE_FREE(callback->on_publish_finish);

  IOTC_SAFE_FREE(callback);
}

iotc_state_t iotc_driver_codec_protobuf_layer_push(void* context, void* data,
                                                   iotc_state_t in_out_state) {
  IOTC_LAYER_FUNCTION_PRINT_FUNCTION_DIGEST();

  struct _XiClientFtestFw__XiClientCallback* client_callback = NULL;

  if (IOTC_THIS_LAYER_NOT_OPERATIONAL(context)) {
    goto err_handling;
  }

  if (NULL == data) {
    return IOTC_PROCESS_PUSH_ON_NEXT_LAYER(context, data, in_out_state);
  } else {
    client_callback = (struct _XiClientFtestFw__XiClientCallback*)data;

    const size_t packed_size =
        iotc_client_ftest_fw__iotc_client_callback__get_packed_size(
            client_callback);

    iotc_data_desc_t* data_desc = iotc_make_empty_desc_alloc(packed_size);
    IOTC_CHECK_MEMORY(data_desc, in_out_state);

    iotc_client_ftest_fw__iotc_client_callback__pack(client_callback,
                                                     data_desc->data_ptr);
    data_desc->length = packed_size;

    iotc_debug_printf(
        "[ driver cdc ] packed_size = %lu, data_desc->length = %d\n",
        packed_size, data_desc->length);

    iotc_driver_free_protobuf_callback(client_callback);
    return IOTC_PROCESS_PUSH_ON_PREV_LAYER(context, data_desc, IOTC_STATE_OK);
  }

err_handling:
  if (NULL != client_callback) {
    iotc_driver_free_protobuf_callback(client_callback);
  }

  return IOTC_PROCESS_CLOSE_ON_THIS_LAYER(context, data, in_out_state);
}

iotc_state_t iotc_driver_codec_protobuf_layer_pull(void* context, void* data,
                                                   iotc_state_t in_out_state) {
  IOTC_LAYER_FUNCTION_PRINT_FUNCTION_DIGEST();

  iotc_driver_codec_protobuf_layer_data_t* layer_data = NULL;

  if (IOTC_THIS_LAYER_NOT_OPERATIONAL(context)) {
    goto err_handling;
  }

  iotc_data_desc_t* encoded_protobuf_chunk = (iotc_data_desc_t*)data;

  iotc_debug_printf("[ driver cdc ] data length = %d\n",
                    encoded_protobuf_chunk->length);

  layer_data =
      (iotc_driver_codec_protobuf_layer_data_t*)IOTC_THIS_LAYER(context)
          ->user_data;

  IOTC_CHECK_CND_DBGMESSAGE(
      NULL == layer_data, IOTC_NOT_INITIALIZED, in_out_state,
      "driver's codec layer has no layer data allocated. Uninitialized.");

  {  // accumulate chunks
    if (layer_data->encoded_protobuf_accumulated == NULL) {  // first chunk
      layer_data->encoded_protobuf_accumulated = encoded_protobuf_chunk;

      encoded_protobuf_chunk = NULL;
    } else {  // trailing chunks -> accumulate them
      iotc_state_t append_bytes_result = iotc_data_desc_append_data_resize(
          layer_data->encoded_protobuf_accumulated,
          (const char*)encoded_protobuf_chunk->data_ptr,
          encoded_protobuf_chunk->length);

      IOTC_CHECK_CND_DBGMESSAGE(
          IOTC_STATE_OK != append_bytes_result, append_bytes_result,
          in_out_state, "could not accumulate incoming probobuf chunks");

      iotc_free_desc(&encoded_protobuf_chunk);
    }
  }

  iotc_debug_printf("[ driver cdc ] accumulated protobuf length = %d\n",
                    layer_data->encoded_protobuf_accumulated->length);

  // try to unpack
  struct _XiClientFtestFw__XiClientAPI* message_API_call =
      iotc_client_ftest_fw__iotc_client_api__unpack(
          NULL, layer_data->encoded_protobuf_accumulated->length,
          layer_data->encoded_protobuf_accumulated->data_ptr);

  // protobuf does not support unpacking data in chunks:
  // https://code.google.com/p/protobuf-c/issues/detail?id=67
  // so concatenate data until it can be unpacked successfully

  iotc_debug_printf("[ driver cdc ] unpacked message ptr = %p\n",
                    message_API_call);

  if (NULL != message_API_call) {  // successful unpack
    iotc_free_desc(&layer_data->encoded_protobuf_accumulated);

    return IOTC_PROCESS_PULL_ON_NEXT_LAYER(context, message_API_call,
                                           IOTC_STATE_OK);
  } else {
    return IOTC_PROCESS_PULL_ON_PREV_LAYER(context, NULL, IOTC_STATE_WANT_READ);
  }

err_handling:
  if (NULL != layer_data) {
    iotc_free_desc(&layer_data->encoded_protobuf_accumulated);
  }
  // close layer chaing in case of error: not doing recovery
  return IOTC_PROCESS_CLOSE_ON_THIS_LAYER(context, NULL,
                                          IOTC_SERIALIZATION_ERROR);
}

iotc_state_t iotc_driver_codec_protobuf_layer_close(void* context, void* data,
                                                    iotc_state_t in_out_state) {
  IOTC_LAYER_FUNCTION_PRINT_FUNCTION_DIGEST();

  return IOTC_PROCESS_CLOSE_ON_PREV_LAYER(context, data, in_out_state);
}

iotc_state_t iotc_driver_codec_protobuf_layer_close_externally(
    void* context, void* data, iotc_state_t in_out_state) {
  IOTC_LAYER_FUNCTION_PRINT_FUNCTION_DIGEST();

  iotc_driver_codec_protobuf_layer_data_t* layer_data =
      (iotc_driver_codec_protobuf_layer_data_t*)IOTC_THIS_LAYER(context)
          ->user_data;

  if (NULL != layer_data) {
    iotc_free_desc(&layer_data->encoded_protobuf_accumulated);

    IOTC_SAFE_FREE(IOTC_THIS_LAYER(context)->user_data);
  }

  return IOTC_PROCESS_CLOSE_EXTERNALLY_ON_NEXT_LAYER(context, data,
                                                     in_out_state);
}

iotc_state_t iotc_driver_codec_protobuf_layer_init(void* context, void* data,
                                                   iotc_state_t in_out_state) {
  IOTC_LAYER_FUNCTION_PRINT_FUNCTION_DIGEST();

  IOTC_LAYER_FUNCTION_PRINT_FUNCTION_DIGEST();

  assert(IOTC_THIS_LAYER(context)->user_data == 0);

  IOTC_ALLOC_AT(iotc_driver_codec_protobuf_layer_data_t,
                IOTC_THIS_LAYER(context)->user_data, in_out_state);

  return IOTC_PROCESS_INIT_ON_PREV_LAYER(context, data, in_out_state);

err_handling:
  IOTC_SAFE_FREE(IOTC_THIS_LAYER(context)->user_data);

  return IOTC_PROCESS_INIT_ON_NEXT_LAYER(context, data, in_out_state);
}

iotc_state_t iotc_driver_codec_protobuf_layer_connect(
    void* context, void* data, iotc_state_t in_out_state) {
  IOTC_LAYER_FUNCTION_PRINT_FUNCTION_DIGEST();

  return IOTC_PROCESS_CONNECT_ON_NEXT_LAYER(context, data, in_out_state);
}
