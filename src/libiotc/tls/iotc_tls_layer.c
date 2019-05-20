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

#include <iotc_bsp_tls.h>
#include <iotc_connection_data.h>
#include <iotc_coroutine.h>
#include <iotc_debug.h>
#include <iotc_list.h>
#include <iotc_macros.h>
#include <iotc_tls_layer.h>
#include <iotc_tls_layer_state.h>
#include "iotc_fs_filenames.h"
#include "iotc_layer_api.h"
#include "iotc_resource_manager.h"

/* Forward declarations. */
static iotc_state_t send_handler(void* context, void* data, iotc_state_t state);
static iotc_state_t recv_handler(void* context, void* data, iotc_state_t state);

iotc_bsp_tls_state_t iotc_bsp_tls_recv_callback(char* buf, int sz,
                                                void* context,
                                                int* bytes_read) {
  assert(NULL != buf);
  assert(NULL != context);
  assert(NULL != bytes_read);

  // iotc_debug_format( "Entering: %s", __FUNCTION__ );

  iotc_tls_layer_state_t* layer_data =
      (iotc_tls_layer_state_t*)IOTC_THIS_LAYER(context)->user_data;

  iotc_data_desc_t* recvd = layer_data->raw_buffer;

  /* if there is no buffer in the queue just leave with WANT_READ state */
  if (NULL != recvd) {
    /* calculate how much data left in the buffer and copy as much as it's
     * possible */
    const int recvd_data_length_available = recvd->length - recvd->curr_pos;
    assert(recvd_data_length_available > 0);

    const int bytes_to_copy = IOTC_MIN(sz, recvd_data_length_available);
    memcpy(buf, (recvd->data_ptr + recvd->curr_pos), bytes_to_copy);
    recvd->curr_pos += bytes_to_copy;

    /* if we'ver emptied the buffer let it go */
    if (recvd->curr_pos == recvd->length) {
      iotc_data_desc_t* tmp = NULL;
      IOTC_LIST_POP(iotc_data_desc_t, layer_data->raw_buffer, tmp);
      iotc_free_desc(&tmp);
    }

    /* set the return argument value */
    *bytes_read = bytes_to_copy;

    /* success */
    return IOTC_BSP_TLS_STATE_OK;
  }

  /* May happen if the buffer is not yet received. In this case the TLS
   * implementation will have to wait until there is data available. */
  return IOTC_BSP_TLS_STATE_WANT_READ;
}

iotc_bsp_tls_state_t iotc_bsp_tls_send_callback(char* buf, int sz,
                                                void* context,
                                                int* bytes_sent) {
  assert(NULL != buf);
  assert(NULL != context);
  assert(NULL != bytes_sent);

  iotc_debug_format("Entering %s", __FUNCTION__);

  iotc_state_t state = IOTC_STATE_OK;

  iotc_layer_connectivity_t* ctx = (iotc_layer_connectivity_t*)context;

  IOTC_UNUSED(ctx);

  iotc_tls_layer_state_t* layer_data =
      (iotc_tls_layer_state_t*)IOTC_THIS_LAYER(context)->user_data;

  iotc_data_desc_t* buffer_desc = NULL;

  /* begin the coroutine scope */
  IOTC_CR_START(layer_data->tls_lib_handler_sending_cs);

  /* sanity checks on the outside state vs the coroutine state */
  assert(layer_data->tls_layer_write_state == IOTC_TLS_LAYER_DATA_NONE);
  layer_data->tls_layer_write_state = IOTC_TLS_LAYER_DATA_WRITING;

  /* create new descriptor, but share the data */
  buffer_desc = iotc_make_desc_from_buffer_share((unsigned char*)buf, sz);
  IOTC_CHECK_MEMORY(buffer_desc, state);

  /* call the previous layer push function in order to send the data */
  IOTC_PROCESS_PUSH_ON_PREV_LAYER(context, buffer_desc, IOTC_STATE_OK);

  /* keep yielding until the data is sent */
  do {
    assert(IOTC_TLS_LAYER_DATA_NONE != layer_data->tls_layer_write_state);

    IOTC_CR_YIELD_UNTIL(
        layer_data->tls_lib_handler_sending_cs,
        layer_data->tls_layer_write_state != IOTC_TLS_LAYER_DATA_WRITTEN,
        IOTC_BSP_TLS_STATE_WANT_WRITE);
  } while (layer_data->tls_layer_write_state != IOTC_TLS_LAYER_DATA_WRITTEN);

  /* sanity check on the outside state */
  assert(layer_data->tls_layer_write_state == IOTC_TLS_LAYER_DATA_WRITTEN);
  layer_data->tls_layer_write_state = IOTC_TLS_LAYER_DATA_NONE;

  /* set the return argument value */
  *bytes_sent = sz;

  /* exit the coroutine scope */
  IOTC_CR_EXIT(layer_data->tls_lib_handler_sending_cs, IOTC_BSP_TLS_STATE_OK);

  IOTC_CR_END();

err_handling:
  /* this line will only deallocate the descriptor not the data - since the
   * descriptor is sharing the data */
  iotc_free_desc(&buffer_desc);
  return IOTC_BSP_TLS_STATE_WRITE_ERROR;
}

static iotc_state_t connect_handler(void* context, void* data,
                                    iotc_state_t in_out_state) {
  IOTC_LAYER_FUNCTION_PRINT_FUNCTION_DIGEST();

  iotc_bsp_tls_state_t bsp_tls_state = IOTC_BSP_TLS_STATE_CONNECT_ERROR;

  iotc_tls_layer_state_t* layer_data =
      (iotc_tls_layer_state_t*)IOTC_THIS_LAYER(context)->user_data;

  if (IOTC_THIS_LAYER_NOT_OPERATIONAL(context) || NULL == layer_data) {
    iotc_debug_format("connect handler is not operational, layer_data = %p",
                      layer_data);
    return IOTC_STATE_OK;
  }

  /* begin coroutine scope */
  IOTC_CR_START(layer_data->tls_layer_conn_cs);

  do {
    bsp_tls_state = iotc_bsp_tls_connect(layer_data->tls_context);

    IOTC_CR_YIELD_UNTIL(layer_data->tls_layer_conn_cs,
                        (bsp_tls_state == IOTC_BSP_TLS_STATE_WANT_READ ||
                         bsp_tls_state == IOTC_BSP_TLS_STATE_WANT_WRITE),
                        ((bsp_tls_state == IOTC_BSP_TLS_STATE_WANT_READ)
                             ? IOTC_STATE_WANT_READ
                             : IOTC_STATE_WANT_WRITE));

    if (IOTC_BSP_TLS_STATE_OK != bsp_tls_state) {
      in_out_state = IOTC_BSP_TLS_STATE_CERT_ERROR == bsp_tls_state
                         ? IOTC_TLS_FAILED_CERT_ERROR
                         : IOTC_TLS_CONNECT_ERROR;
      goto err_handling;
    }
  } while (bsp_tls_state != IOTC_BSP_TLS_STATE_OK);

  /* connection done we can restore the logic handlers */
  layer_data->tls_layer_logic_recv_handler = &recv_handler;
  layer_data->tls_layer_logic_send_handler = &send_handler;

  if (NULL != layer_data->raw_buffer ||
      iotc_bsp_tls_pending(layer_data->tls_context) > 0) {
    iotc_debug_logger("IOTC_PROCESS_PULL_ON_THIS_LAYER");
    IOTC_PROCESS_PULL_ON_THIS_LAYER(context, NULL, IOTC_STATE_WANT_READ);
  }

  IOTC_CR_EXIT(
      layer_data->tls_layer_conn_cs,
      IOTC_PROCESS_CONNECT_ON_NEXT_LAYER(context, data, IOTC_STATE_OK));

  /* end of coroutine scope */
  IOTC_CR_END();

err_handling:
  IOTC_CR_RESET(layer_data->tls_layer_conn_cs);
  return IOTC_PROCESS_CLOSE_ON_THIS_LAYER(context, NULL, in_out_state);
}

static iotc_state_t send_handler(void* context, void* data,
                                 iotc_state_t in_out_state) {
  IOTC_UNUSED(data);
  IOTC_UNUSED(in_out_state);

  IOTC_LAYER_FUNCTION_PRINT_FUNCTION_DIGEST();

  /* PRECONDITIONS */
  assert(NULL != context);

  iotc_tls_layer_state_t* layer_data =
      (iotc_tls_layer_state_t*)IOTC_THIS_LAYER(context)->user_data;

  if (IOTC_THIS_LAYER_NOT_OPERATIONAL(context) || NULL == layer_data) {
    iotc_debug_logger("IOTC_THIS_LAYER_NOT_OPERATIONAL");
    return IOTC_STATE_OK;
  }

  /* cook temporary data before entering the coroutine scope */
  iotc_bsp_tls_state_t ret = IOTC_BSP_TLS_STATE_WRITE_ERROR;
  int bytes_written = 0;
  const size_t offset = layer_data->to_write_buffer->curr_pos;
  const size_t size_left = layer_data->to_write_buffer->capacity -
                           layer_data->to_write_buffer->curr_pos;

  /* coroutine scope begins */
  IOTC_CR_START(layer_data->tls_layer_send_cs);

  /* this loop can be break only by success, error state is being checked inside
   */
  do {
    /* passes data and a size to bsp tls write function */
    ret = iotc_bsp_tls_write(layer_data->tls_context,
                             layer_data->to_write_buffer->data_ptr + offset,
                             size_left, &bytes_written);

    if (bytes_written > 0) {
      layer_data->to_write_buffer->curr_pos += bytes_written;
    }

    /* while bsp tls is unable to read let's exit the coroutine */
    IOTC_CR_YIELD_UNTIL(layer_data->tls_layer_send_cs,
                        ret == IOTC_BSP_TLS_STATE_WANT_WRITE,
                        IOTC_STATE_WANT_WRITE);

    /* here we check for an error */
    if (IOTC_BSP_TLS_STATE_OK != ret) {
      goto err_handling;
    }

  } while (ret != IOTC_BSP_TLS_STATE_OK);

  /* free the memory */
  iotc_free_desc(&layer_data->to_write_buffer);

  /* exit the coroutine scope */
  IOTC_CR_EXIT(
      layer_data->tls_layer_send_cs,
      IOTC_PROCESS_PUSH_ON_NEXT_LAYER(context, NULL, IOTC_STATE_WRITTEN));

  IOTC_CR_END();

err_handling:
  /* coroutine reset */
  IOTC_CR_RESET(layer_data->tls_layer_send_cs);
  /* free the memory */
  iotc_free_desc(&layer_data->to_write_buffer);
  return IOTC_PROCESS_PUSH_ON_NEXT_LAYER(context, NULL,
                                         IOTC_STATE_FAILED_WRITING);
}

static iotc_state_t recv_handler(void* context, void* data,
                                 iotc_state_t in_out_state) {
  IOTC_LAYER_FUNCTION_PRINT_FUNCTION_DIGEST();

  (void)context;
  (void)data;

  iotc_tls_layer_state_t* layer_data =
      (iotc_tls_layer_state_t*)IOTC_THIS_LAYER(context)->user_data;

  int size_left = 0;
  iotc_bsp_tls_state_t ret = IOTC_BSP_TLS_STATE_READ_ERROR;
  int bytes_read = 0;

  if (IOTC_THIS_LAYER_NOT_OPERATIONAL(context) || NULL == layer_data) {
    iotc_debug_logger("IOTC_THIS_LAYER_NOT_OPERATIONAL");
    return IOTC_STATE_OK;
  }

  /* if recv buffer is empty than create one */
  if (NULL == layer_data->decoded_buffer) {
    layer_data->decoded_buffer =
        iotc_make_empty_desc_alloc(IOTC_IO_BUFFER_SIZE);
    IOTC_CHECK_MEMORY(layer_data->decoded_buffer, in_out_state);
  }

  /* coroutine scope begin */
  IOTC_CR_START(layer_data->tls_layer_recv_cs);

  do {
    size_left = layer_data->decoded_buffer->capacity -
                layer_data->decoded_buffer->length;

    assert(size_left > 0);

    bytes_read = 0;
    ret = iotc_bsp_tls_read(layer_data->tls_context,
                            layer_data->decoded_buffer->data_ptr +
                                layer_data->decoded_buffer->length,
                            size_left, &bytes_read);

    if (bytes_read > 0) {
      layer_data->decoded_buffer->length += bytes_read;
    }

    IOTC_CR_YIELD_UNTIL(layer_data->tls_layer_recv_cs,
                        (ret == IOTC_BSP_TLS_STATE_WANT_READ),
                        IOTC_STATE_WANT_READ);

    if (ret != IOTC_BSP_TLS_STATE_OK) {
      in_out_state = IOTC_TLS_READ_ERROR;
      goto err_handling;
    }

  } while (ret != IOTC_BSP_TLS_STATE_OK);

#if 0 /* leave it for future use */
    iotc_debug_data_logger( "recved", buffer_desc );
#endif

  iotc_data_desc_t* ret_buffer = layer_data->decoded_buffer;
  layer_data->decoded_buffer = NULL;

  if (NULL != layer_data->raw_buffer ||
      iotc_bsp_tls_pending(layer_data->tls_context) > 0) {
    IOTC_PROCESS_PULL_ON_THIS_LAYER(context, NULL, IOTC_STATE_WANT_READ);
  }

  IOTC_CR_EXIT(
      layer_data->tls_layer_recv_cs,
      IOTC_PROCESS_PULL_ON_NEXT_LAYER(context, ret_buffer, IOTC_STATE_OK));

  /* coroutine scope ends */
  IOTC_CR_END();

err_handling:
  IOTC_CR_RESET(layer_data->tls_layer_recv_cs);

  if (layer_data && layer_data->decoded_buffer) {
    iotc_free_desc(&layer_data->decoded_buffer);
  }

  return IOTC_PROCESS_CLOSE_ON_THIS_LAYER(context, NULL, in_out_state);
}

iotc_state_t iotc_tls_layer_push(void* context, void* data,
                                 iotc_state_t in_out_state) {
  IOTC_LAYER_FUNCTION_PRINT_FUNCTION_DIGEST();

  IOTC_UNUSED(context);
  IOTC_UNUSED(data);

  iotc_tls_layer_state_t* layer_data =
      (iotc_tls_layer_state_t*)IOTC_THIS_LAYER(context)->user_data;

  iotc_data_desc_t* buffer = (iotc_data_desc_t*)data;

  if (IOTC_THIS_LAYER_NOT_OPERATIONAL(context) || NULL == layer_data) {
    iotc_debug_logger("IOTC_THIS_LAYER_NOT_OPERATIONAL");

    /* cleaning of not finished requests */
    iotc_free_desc(&buffer);

    return IOTC_STATE_OK;
  }

  if (in_out_state == IOTC_STATE_WRITTEN) {
    iotc_debug_logger("data written");

    assert(IOTC_TLS_LAYER_DATA_WRITING == layer_data->tls_layer_write_state);
    layer_data->tls_layer_write_state = IOTC_TLS_LAYER_DATA_WRITTEN;

    assert(NULL != layer_data->tls_layer_logic_send_handler);
    return layer_data->tls_layer_logic_send_handler(context, NULL,
                                                    IOTC_STATE_OK);
  }

  if (in_out_state == IOTC_STATE_OK || in_out_state == IOTC_STATE_WANT_WRITE) {
    iotc_debug_logger("init writing data");

    iotc_debug_format("writing status %d and coroutine state %d",
                      layer_data->tls_layer_write_state,
                      layer_data->tls_lib_handler_sending_cs);

    /* sanity checks */
    assert(NULL == layer_data->to_write_buffer);
    assert(0 == IOTC_CR_IS_RUNNING(layer_data->tls_layer_send_cs));
    assert(0 == IOTC_CR_IS_RUNNING(layer_data->tls_lib_handler_sending_cs));

    layer_data->to_write_buffer = buffer;

    /* sanity check */
    assert(NULL != layer_data->tls_layer_logic_send_handler);
    return layer_data->tls_layer_logic_send_handler(context, NULL,
                                                    IOTC_STATE_OK);
  }

  return IOTC_PROCESS_PUSH_ON_PREV_LAYER(context, data, in_out_state);
}

iotc_state_t iotc_tls_layer_pull(void* context, void* data,
                                 iotc_state_t in_out_state) {
  IOTC_LAYER_FUNCTION_PRINT_FUNCTION_DIGEST();

  IOTC_UNUSED(data);

  iotc_tls_layer_state_t* layer_data =
      (iotc_tls_layer_state_t*)IOTC_THIS_LAYER(context)->user_data;

  iotc_data_desc_t* data_desc = (iotc_data_desc_t*)data;

  if (IOTC_THIS_LAYER_NOT_OPERATIONAL(context) || NULL == layer_data) {
    iotc_debug_logger("IOTC_THIS_LAYER_NOT_OPERATIONAL");
    goto err_handling;
  }

  /* there is data to read */
  if (in_out_state == IOTC_STATE_OK && NULL != data_desc) {
    assert(data_desc->length - data_desc->curr_pos > 0);

    /* assign the raw data so that the cyassl is able to read
     * through handler */
    IOTC_LIST_PUSH_BACK(iotc_data_desc_t, layer_data->raw_buffer, data_desc);
  }

  assert(NULL != layer_data->tls_layer_logic_recv_handler);
  return layer_data->tls_layer_logic_recv_handler(context, data_desc,
                                                  in_out_state);

err_handling:
  iotc_free_desc(&data_desc);
  return IOTC_PROCESS_CLOSE_ON_THIS_LAYER(context, NULL, in_out_state);
}

iotc_state_t iotc_tls_layer_init(void* context, void* data,
                                 iotc_state_t in_out_state) {
  IOTC_LAYER_FUNCTION_PRINT_FUNCTION_DIGEST();

  iotc_connection_data_t* connection_data = (iotc_connection_data_t*)data;
  iotc_tls_layer_state_t* layer_data =
      (iotc_tls_layer_state_t*)IOTC_THIS_LAYER(context)->user_data;

  IOTC_CHECK_CND_DBGMESSAGE(NULL == connection_data,
                            IOTC_TLS_INITALIZATION_ERROR, in_out_state,
                            "no connection data during TLS initialization");

  /* if coroutine returns layer_data will be set so we have to be prepared */
  if (NULL == layer_data) {
    IOTC_ALLOC_AT(iotc_tls_layer_state_t, layer_data, in_out_state);
    IOTC_THIS_LAYER(context)->user_data = layer_data;
  }

  /* let's use the connection coroutine state */
  IOTC_CR_START(layer_data->tls_layer_conn_cs);

  /* make the resource manager context */
  in_out_state =
      iotc_resource_manager_make_context(NULL, &layer_data->rm_context);

  if (IOTC_STATE_OK != in_out_state) {
    iotc_debug_format("failed to create a resource manager context, reason: %d",
                      in_out_state);
    in_out_state = IOTC_TLS_FAILED_LOADING_CERTIFICATE;
    goto err_handling;
  }

  in_out_state = iotc_resource_manager_open(
      layer_data->rm_context,
      iotc_make_handle(&iotc_tls_layer_init, context, data, in_out_state),
      IOTC_FS_CERTIFICATE, IOTC_GLOBAL_CERTIFICATE_FILE_NAME, IOTC_FS_OPEN_READ,
      NULL);

  if (IOTC_STATE_OK != in_out_state) {
    iotc_debug_format(
        "failed to start open on CA certificate using resource manager "
        "context, reason: %d",
        in_out_state);
    in_out_state = IOTC_TLS_FAILED_LOADING_CERTIFICATE;
    goto err_handling;
  }

  IOTC_CR_YIELD(layer_data->tls_layer_conn_cs, IOTC_STATE_OK);

  if (IOTC_STATE_OK != in_out_state) {
    iotc_debug_format(
        "failed to open CA certificate from filesystem, reason: %d",
        in_out_state);
    in_out_state = IOTC_TLS_FAILED_LOADING_CERTIFICATE;
    goto err_handling;
  }

  in_out_state = iotc_resource_manager_read(
      layer_data->rm_context,
      iotc_make_handle(&iotc_tls_layer_init, context, data, in_out_state),
      NULL);

  if (IOTC_STATE_OK != in_out_state) {
    iotc_debug_format(
        "failed to start read on CA certificate using resource manager, "
        "reason: %d",
        in_out_state);
    in_out_state = IOTC_TLS_FAILED_LOADING_CERTIFICATE;
    goto err_handling;
  }

  /* here the resource manager will start reading the resource content from a
   * choosen filesystem */
  IOTC_CR_YIELD(layer_data->tls_layer_conn_cs, IOTC_STATE_OK);
  /* here the resource manager finished reading the resource content from a
   * choosen filesystem */

  if (IOTC_STATE_OK != in_out_state) {
    iotc_debug_format(
        "failed to read CA certificate from filesystem, reason: %d",
        in_out_state);
    in_out_state = IOTC_TLS_FAILED_LOADING_CERTIFICATE;
    goto err_handling;
  }

  /* POST/PRE-CONDITIONS */
  assert(NULL != layer_data->rm_context->data_buffer->data_ptr);
  assert(0 < layer_data->rm_context->data_buffer->length);

  { /* initialisation block for bsp tls init function */
    iotc_bsp_tls_init_params_t init_params;
    memset(&init_params, 0, sizeof(init_params));

    init_params.libiotc_io_callback_context = context;
    init_params.fp_libiotc_alloc = iotc_alloc_ptr;
    init_params.fp_libiotc_calloc = iotc_calloc_ptr;
    init_params.fp_libiotc_free = iotc_free_ptr;
    init_params.fp_libiotc_realloc = iotc_realloc_ptr;
    init_params.domain_name = connection_data->host;
    init_params.ca_cert_pem_buf = layer_data->rm_context->data_buffer->data_ptr;
    init_params.ca_cert_pem_buf_length =
        layer_data->rm_context->data_buffer->length;

    /* bsp init function call */
    const iotc_bsp_tls_state_t bsp_tls_state =
        iotc_bsp_tls_init(&layer_data->tls_context, &init_params);

    /*  */
    if (IOTC_BSP_TLS_STATE_OK != bsp_tls_state) {
      in_out_state = IOTC_BSP_TLS_STATE_CERT_ERROR == bsp_tls_state
                         ? IOTC_TLS_FAILED_LOADING_CERTIFICATE
                         : IOTC_TLS_INITALIZATION_ERROR;
      iotc_debug_logger("ERROR: during BSP TLS initialization");
      goto err_handling;
    }
  }

  iotc_debug_logger("BSP TLS initialization successfull");

  in_out_state = iotc_resource_manager_close(
      layer_data->rm_context,
      iotc_make_handle(&iotc_tls_layer_init, context, data, in_out_state),
      NULL);

  /* here the resource manager will start the close action */
  IOTC_CR_YIELD(layer_data->tls_layer_conn_cs, IOTC_STATE_OK);
  /* here the resource manger finished closing this resource */

  if (IOTC_STATE_OK != in_out_state) {
    iotc_debug_format("failed to close the CA certificate resource, reason: %d",
                      in_out_state);
    in_out_state = IOTC_TLS_FAILED_LOADING_CERTIFICATE;
    goto err_handling;
  }

  in_out_state = iotc_resource_manager_free_context(&layer_data->rm_context);

  if (IOTC_STATE_OK != in_out_state) {
    iotc_debug_format("failed to free the context memory, reason: %d",
                      in_out_state);
    in_out_state = IOTC_TLS_FAILED_LOADING_CERTIFICATE;
    goto err_handling;
  }

  /* setup the logic handlers for connection purposes */
  layer_data->tls_layer_logic_recv_handler = &connect_handler;
  layer_data->tls_layer_logic_send_handler = &connect_handler;

  iotc_debug_logger("successfully initialized BSP TLS module");

  IOTC_CR_EXIT(layer_data->tls_layer_recv_cs,
               IOTC_PROCESS_INIT_ON_PREV_LAYER(context, data, in_out_state));

  /* end of coroutine scope */
  IOTC_CR_END();

err_handling:
  /* whenever error before layer_data created */
  if (NULL != layer_data) {
    IOTC_CR_RESET(layer_data->tls_layer_conn_cs);
    iotc_bsp_tls_cleanup(&layer_data->tls_context);
  }

  return IOTC_PROCESS_CONNECT_ON_THIS_LAYER(context, data, in_out_state);
}

iotc_state_t iotc_tls_layer_connect(void* context, void* data,
                                    iotc_state_t in_out_state) {
  IOTC_LAYER_FUNCTION_PRINT_FUNCTION_DIGEST();

  iotc_tls_layer_state_t* layer_data =
      (iotc_tls_layer_state_t*)IOTC_THIS_LAYER(context)->user_data;

  if (IOTC_THIS_LAYER_NOT_OPERATIONAL(context) || NULL == layer_data) {
    /* cleaning of not finished requests */
    goto err_handling;
  }

  if (IOTC_STATE_OK != in_out_state) {
    iotc_debug_format("error connecting state = %d", in_out_state);
    goto err_handling;
  }

  return connect_handler(context, NULL, IOTC_STATE_OK);

err_handling:
  return IOTC_PROCESS_CONNECT_ON_NEXT_LAYER(context, data, in_out_state);
}

iotc_state_t iotc_tls_layer_close(void* context, void* data,
                                  iotc_state_t in_out_state) {
  IOTC_LAYER_FUNCTION_PRINT_FUNCTION_DIGEST();

  const iotc_tls_layer_state_t* layer_data =
      (iotc_tls_layer_state_t*)IOTC_THIS_LAYER(context)->user_data;

  return (NULL == layer_data)
             ? IOTC_PROCESS_CLOSE_EXTERNALLY_ON_THIS_LAYER(context, data,
                                                           in_out_state)
             : IOTC_PROCESS_CLOSE_ON_PREV_LAYER(context, data, in_out_state);
}

#define RELEASE_DATADESCRIPTOR(ds) \
  { iotc_free_desc(&ds); }

iotc_state_t iotc_tls_layer_close_externally(void* context, void* data,
                                             iotc_state_t in_out_state) {
  IOTC_LAYER_FUNCTION_PRINT_FUNCTION_DIGEST();

  iotc_tls_layer_state_t* layer_data =
      (iotc_tls_layer_state_t*)IOTC_THIS_LAYER(context)->user_data;

  if (NULL != layer_data) {
    if (layer_data->rm_context) {
      if (layer_data->rm_context->resource_handle >= 0) {
        if (IOTC_STATE_OK ==
            iotc_resource_manager_close(
                layer_data->rm_context,
                iotc_make_handle(&iotc_tls_layer_close_externally, context,
                                 data, in_out_state),
                NULL)) {
          return IOTC_STATE_OK;
        }
      }

      iotc_resource_manager_free_context(&layer_data->rm_context);
    }

    iotc_debug_logger("cleaning TLS library");
    iotc_bsp_tls_cleanup(&layer_data->tls_context);

    if (layer_data->raw_buffer) {
      iotc_debug_logger("cleaning received buffer");

      IOTC_LIST_FOREACH(iotc_data_desc_t, layer_data->raw_buffer,
                        RELEASE_DATADESCRIPTOR);
    }

    if (layer_data->decoded_buffer) {
      iotc_debug_logger("cleaning decoded buffer");
      iotc_free_desc(&layer_data->decoded_buffer);
    }

    if (layer_data->to_write_buffer) {
      iotc_debug_logger("cleaning to write buffer");
      iotc_free_desc(&layer_data->to_write_buffer);
    }

    /* user data removed */
    IOTC_SAFE_FREE(IOTC_THIS_LAYER(context)->user_data);
  }

  if (in_out_state != IOTC_STATE_OK &&
      IOTC_THIS_LAYER_STATE(context) == IOTC_LAYER_STATE_CONNECTING) {
    /* error handling */
    return IOTC_PROCESS_CONNECT_ON_THIS_LAYER(context, data, in_out_state);
  }

  return IOTC_PROCESS_CLOSE_EXTERNALLY_ON_NEXT_LAYER(context, data,
                                                     in_out_state);
}
