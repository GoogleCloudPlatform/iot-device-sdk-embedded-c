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

#include "iotc_io_net_layer.h"
#include "iotc_bsp_io_net.h"
#include "iotc_io_net_layer_state.h"

#include "iotc_connection_data.h"
#include "iotc_coroutine.h"
#include "iotc_debug.h"
#include "iotc_layer_api.h"
#include "iotc_macros.h"
#include "iotc_types_internal.h"

#include "iotc_globals.h"
#include "iotc_io_timeouts.h"

#include <sys/socket.h>

iotc_state_t iotc_io_net_layer_connect(void* context, void* data,
                                       iotc_state_t in_out_state) {
  IOTC_LAYER_FUNCTION_PRINT_FUNCTION_DIGEST();

  /* check the state before doing anything */
  if (IOTC_STATE_OK != in_out_state) {
    return IOTC_PROCESS_CONNECT_ON_NEXT_LAYER(context, NULL, in_out_state);
  }

  iotc_io_net_layer_state_t* layer_data =
      (iotc_io_net_layer_state_t*)IOTC_THIS_LAYER(context)->user_data;

  /* if layer_data is null on this stage it means internal error */
  if (NULL == layer_data) {
    return IOTC_PROCESS_CONNECT_ON_NEXT_LAYER(context, NULL,
                                              IOTC_INTERNAL_ERROR);
  }

  iotc_bsp_io_net_state_t state = IOTC_BSP_IO_NET_STATE_OK;

  iotc_connection_data_t* connection_data = (iotc_connection_data_t*)data;
  iotc_evtd_instance_t* event_dispatcher =
      IOTC_CONTEXT_DATA(context)->evtd_instance;

  IOTC_CR_START(layer_data->layer_connect_cs)

  iotc_debug_format("Connecting layer [%d] to the endpoint: %s:%hu",
                    IOTC_THIS_LAYER(context)->layer_type_id,
                    connection_data->host, connection_data->port);

  state =
      iotc_bsp_io_net_socket_connect(&layer_data->socket, connection_data->host,
                                     connection_data->port, SOCKET_STREAM);

  IOTC_CHECK_CND_DBGMESSAGE(IOTC_BSP_IO_NET_STATE_OK != state,
                            IOTC_SOCKET_CONNECTION_ERROR, in_out_state,
                            "Connecting to the endpoint [failed]");

  {
    iotc_evtd_register_socket_fd(
        event_dispatcher, layer_data->socket,
        iotc_make_handle(&iotc_io_net_layer_pull, context, 0, IOTC_STATE_OK));

    iotc_evtd_continue_when_evt_on_socket(
        event_dispatcher, IOTC_EVENT_WANT_CONNECT,
        iotc_make_handle(&iotc_io_net_layer_connect, (void*)context, data,
                         IOTC_STATE_OK),
        layer_data->socket);
  }

  // Return here whenever we can write.
  IOTC_CR_YIELD(layer_data->layer_connect_cs, IOTC_STATE_OK);

  state = iotc_bsp_io_net_connection_check(
      layer_data->socket, connection_data->host, connection_data->port);

  IOTC_CHECK_CND_DBGMESSAGE(IOTC_BSP_IO_NET_STATE_OK != state,
                            IOTC_SOCKET_GETSOCKOPT_ERROR, in_out_state,
                            "Error while calling getsockopt.");
  iotc_debug_logger("Connection successful!");

  IOTC_CR_EXIT(layer_data->layer_connect_cs, IOTC_PROCESS_CONNECT_ON_NEXT_LAYER(
                                                 context, NULL, IOTC_STATE_OK));

err_handling:

  IOTC_CR_EXIT(layer_data->layer_connect_cs,
               IOTC_PROCESS_CONNECT_ON_NEXT_LAYER(context, data, in_out_state));

  IOTC_CR_END();
}

iotc_state_t iotc_io_net_layer_init(void* context, void* data,
                                    iotc_state_t in_out_state) {
  IOTC_LAYER_FUNCTION_PRINT_FUNCTION_DIGEST();

  if (NULL == context) {
    return IOTC_INVALID_PARAMETER;
  }

  if (NULL != IOTC_THIS_LAYER(context)->user_data) {
    return IOTC_FAILED_INITIALIZATION;
  }

  iotc_layer_t* layer = (iotc_layer_t*)IOTC_THIS_LAYER(context);

  IOTC_ALLOC(iotc_io_net_layer_state_t, layer_data, in_out_state);

  layer->user_data = (void*)layer_data;

  return IOTC_PROCESS_CONNECT_ON_THIS_LAYER(context, data, IOTC_STATE_OK);

err_handling:
  /* cleanup the memory */
  iotc_bsp_io_net_close_socket(&layer_data->socket);

  if (layer->user_data) {
    IOTC_SAFE_FREE(layer->user_data);
  }

  return IOTC_PROCESS_CONNECT_ON_THIS_LAYER(context, data, in_out_state);
}

iotc_state_t iotc_io_net_layer_push(void* context, void* data,
                                    iotc_state_t in_out_state) {
  IOTC_LAYER_FUNCTION_PRINT_FUNCTION_DIGEST();

  IOTC_UNUSED(in_out_state);

  iotc_io_net_layer_state_t* layer_data =
      (iotc_io_net_layer_state_t*)IOTC_THIS_LAYER(context)->user_data;

  iotc_data_desc_t* buffer = (iotc_data_desc_t*)data;
  size_t left = 0;
  int len = 0;
  iotc_bsp_io_net_state_t bsp_state = IOTC_BSP_IO_NET_STATE_OK;

  /* check if the layer has been disconnected */
  if (IOTC_THIS_LAYER_NOT_OPERATIONAL(context) || layer_data == NULL) {
    iotc_debug_logger("layer not operational");
    iotc_free_desc(&buffer);

    return IOTC_STATE_OK;
  }

  if (buffer != 0 && buffer->capacity > 0) {
    do {
      /* call bsp write */
      bsp_state = iotc_bsp_io_net_write(layer_data->socket, &len,
                                        buffer->data_ptr + buffer->curr_pos,
                                        buffer->capacity - buffer->curr_pos);

      /* verify the state if it's an error or a need to wait */
      if (IOTC_BSP_IO_NET_STATE_OK != bsp_state || len < 0) {
        if (IOTC_BSP_IO_NET_STATE_BUSY ==
            bsp_state) /* that can happen in asynch environments */
        {
          /* mark the socket for wake-up call */
          if (0 > iotc_evtd_continue_when_evt_on_socket(
                      IOTC_CONTEXT_DATA(context)->evtd_instance,
                      IOTC_EVENT_WANT_WRITE,
                      iotc_make_handle(&iotc_io_net_layer_push, context, data,
                                       IOTC_STATE_WANT_WRITE),
                      layer_data->socket)) {
            iotc_debug_format("given socket is not registered - [%d]",
                              (int)layer_data->socket);
            return IOTC_PROCESS_CLOSE_EXTERNALLY_ON_THIS_LAYER(
                context, 0, IOTC_INTERNAL_ERROR);
          }

          /* this is not an error so we can leave the coroutine within this
           * state */
          iotc_debug_format("yield in write - [%d]", (int)layer_data->socket);
          return IOTC_STATE_OK;
        } else if (IOTC_BSP_IO_NET_STATE_CONNECTION_RESET == bsp_state) {
          iotc_free_desc(&buffer);
          iotc_debug_logger("connection reset");
          return IOTC_PROCESS_CLOSE_EXTERNALLY_ON_THIS_LAYER(
              context, 0, IOTC_CONNECTION_RESET_BY_PEER_ERROR);
        } else {
          /* any other issue */
          iotc_debug_format("error writing: BSP error code = %d, len = %d\n",
                            (int)bsp_state, len);
          iotc_free_desc(&buffer);
          return IOTC_PROCESS_CLOSE_EXTERNALLY_ON_THIS_LAYER(
              context, data, IOTC_SOCKET_WRITE_ERROR);
        }
      }

      buffer->curr_pos += len;
      left = buffer->capacity - buffer->curr_pos;
    } while (left > 0);
  }

  iotc_debug_format("%d bytes written", len);
  iotc_free_desc(&buffer);

  return IOTC_PROCESS_PUSH_ON_NEXT_LAYER(context, 0, IOTC_STATE_WRITTEN);
}

iotc_state_t iotc_io_net_layer_pull(void* context, void* data,
                                    iotc_state_t in_out_state) {
  IOTC_LAYER_FUNCTION_PRINT_FUNCTION_DIGEST();

  iotc_io_net_layer_state_t* layer_data =
      (iotc_io_net_layer_state_t*)IOTC_THIS_LAYER(context)->user_data;

  iotc_data_desc_t* buffer_desc = 0;
  int len = 0;
  iotc_bsp_io_net_state_t bsp_state = IOTC_BSP_IO_NET_STATE_OK;

  if (IOTC_THIS_LAYER_NOT_OPERATIONAL(context) || layer_data == NULL) {
    if (data != NULL) // let's clean the memory
    {
      buffer_desc = (iotc_data_desc_t*)data;
      iotc_free_desc(&buffer_desc);
    }

    return IOTC_STATE_OK;
  }

  if (data) /* let's reuse already allocated buffer */
  {
    buffer_desc = (iotc_data_desc_t*)data;

    memset(buffer_desc->data_ptr, 0, IOTC_IO_BUFFER_SIZE);
    assert(buffer_desc->capacity == IOTC_IO_BUFFER_SIZE); // sanity check

    buffer_desc->curr_pos = 0;
    buffer_desc->length = 0;
  } else /* if there was no buffer we have to create new one */
  {
    buffer_desc = iotc_make_empty_desc_alloc(IOTC_IO_BUFFER_SIZE);
    IOTC_CHECK_MEMORY(buffer_desc, in_out_state);
  }

  bsp_state = iotc_bsp_io_net_read(
      layer_data->socket, &len, buffer_desc->data_ptr, buffer_desc->capacity);

  // iotc_debug_format( "read: %d bytes", len );

  if (IOTC_BSP_IO_NET_STATE_OK != bsp_state) {
    if (IOTC_BSP_IO_NET_STATE_BUSY ==
        bsp_state) /* register socket to get call when can read */
    {
      /* note for future bsp of select etc. this is the place in the interface
       * of event dispatcher where the socket is the identification value */
      iotc_evtd_continue_when_evt_on_socket(
          IOTC_CONTEXT_DATA(context)->evtd_instance, IOTC_EVENT_WANT_READ,
          iotc_make_handle(&iotc_io_net_layer_pull, context, buffer_desc,
                           in_out_state),
          layer_data->socket);

      return IOTC_STATE_WANT_READ;
    }

    if (IOTC_BSP_IO_NET_STATE_CONNECTION_RESET == bsp_state) {
      /* connection reset by peer */
      iotc_debug_logger("connection reset by peer");
      in_out_state = IOTC_CONNECTION_RESET_BY_PEER_ERROR;
      goto err_handling;
    }

    iotc_debug_format("error reading on socket %d", (int)layer_data->socket);
    in_out_state = IOTC_SOCKET_READ_ERROR;
    goto err_handling;
  }

  /* restart io timeouts if needed */
  if (IOTC_CONTEXT_DATA(context)->io_timeouts->elem_no > 0 &&
      IOTC_CONTEXT_DATA(context)->connection_data->connection_timeout > 0) {
    iotc_io_timeouts_restart(
        iotc_globals.evtd_instance,
        IOTC_CONTEXT_DATA(context)->connection_data->connection_timeout,
        IOTC_CONTEXT_DATA(context)->io_timeouts);
  }

  buffer_desc->length = len;
  buffer_desc->curr_pos = 0;

  return IOTC_PROCESS_PULL_ON_NEXT_LAYER(context, (void*)buffer_desc,
                                         IOTC_STATE_OK);

err_handling:
  iotc_free_desc(&buffer_desc);

  return IOTC_PROCESS_CLOSE_EXTERNALLY_ON_THIS_LAYER(context, 0, in_out_state);
}

iotc_state_t iotc_io_net_layer_close(void* context, void* data,
                                     iotc_state_t in_out_state) {
  IOTC_LAYER_FUNCTION_PRINT_FUNCTION_DIGEST();

  return IOTC_PROCESS_CLOSE_EXTERNALLY_ON_THIS_LAYER(context, data,
                                                     in_out_state);
}

iotc_state_t iotc_io_net_layer_close_externally(void* context, void* data,
                                                iotc_state_t in_out_state) {
  IOTC_LAYER_FUNCTION_PRINT_FUNCTION_DIGEST();

  iotc_io_net_layer_state_t* layer_data =
      (iotc_io_net_layer_state_t*)IOTC_THIS_LAYER(context)->user_data;

  if (layer_data == 0) {
    return IOTC_PROCESS_CLOSE_EXTERNALLY_ON_NEXT_LAYER(context, data,
                                                       in_out_state);
  }

  /* unregister the fd */
  iotc_evtd_unregister_socket_fd(IOTC_CONTEXT_DATA(context)->evtd_instance,
                                 layer_data->socket);

  iotc_bsp_io_net_close_socket(&layer_data->socket);

  /* cleanup the memory */
  IOTC_SAFE_FREE(IOTC_THIS_LAYER(context)->user_data);

  return IOTC_PROCESS_CLOSE_EXTERNALLY_ON_NEXT_LAYER(context, data,
                                                     in_out_state);
}
