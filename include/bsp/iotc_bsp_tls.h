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

#ifndef __IOTC_BSP_TLS_H__
#define __IOTC_BSP_TLS_H__

/**
 * @file iotc_bsp_tls.h
 * @brief Implements Transport Layer Security (TLS).
 *
 * @details Reference implementations are in the
 * <a href="~/src/bsp/tls"><code></code></a> directory. All TLS functions are
 * non-blocking operations.
 */

#include <stddef.h>
#include <stdint.h>

/**
 * @typedef iotc_bsp_io_net_state_t
 * @brief The TLS function states.
 *
 * @see iotc_bsp_io_net_state_e
 */
typedef enum iotc_bsp_tls_state_e {
  /** The TLS function succeeded. */
  IOTC_BSP_TLS_STATE_OK = 0,
  /** Can't initialize TLS library. */
  IOTC_BSP_TLS_STATE_INIT_ERROR = 1,
  /** Can't validate CA certificate. */
  IOTC_BSP_TLS_STATE_CERT_ERROR = 2,
  /** Can't complete TLS handshake. */
  IOTC_BSP_TLS_STATE_CONNECT_ERROR = 3,
  /** TLS handshake is partially complete. Run the function again to read the
  remaining data. */
  IOTC_BSP_TLS_STATE_WANT_READ = 4,
  /** TLS handshake is partially complete. Run the function again to write the
  remaining data. */
  IOTC_BSP_TLS_STATE_WANT_WRITE = 5,
  /** Can't read data. */
  IOTC_BSP_TLS_STATE_READ_ERROR = 6,
  /** Can't write data. */
  IOTC_BSP_TLS_STATE_WRITE_ERROR = 7,
} iotc_bsp_tls_state_t;

/**
 * @typedef iotc_bsp_tls_init_params_t
 * @struct iotc_bsp_tls_init_params_s
 * @brief The TLS context parameters.
 *
 * @see #iotc_bsp_tls_init_params_s
 */
typedef struct iotc_bsp_tls_init_params_s {
  /** Callback context. */
  void* libiotc_io_callback_context;

  /** A pointer to a buffer with root CA PEM certificates. */
  uint8_t* ca_cert_pem_buf;
  /** The length, in bytes, of ca_cert_pem_buf. */
  size_t ca_cert_pem_buf_length;

  /** A pointer to the client application's memory allocation function. */
  void* (*fp_libiotc_alloc)(size_t);

  /** A pointer to the client application's array allocation function. */
  void* (*fp_libiotc_calloc)(size_t, size_t);

  /** A pointer to the client application's reallocation function. */
  void* (*fp_libiotc_realloc)(void*, size_t);

  /** A pointer to the client application's free memory function. */
  void (*fp_libiotc_free)(void*);

  /** A pointer to the host's domain name. The host's domain name must be a
   * null-terminated string. */
  const char* domain_name;

} iotc_bsp_tls_init_params_t;

/**
 * @typedef iotc_bsp_tls_context_t
 * @brief The TLS context.
 */
typedef void iotc_bsp_tls_context_t;

/**
 * @brief Initializes a TLS library and creates a TLS context.
 *
 * The SDK calls the function and then deletes init_params, so store persistant
 * data outside the function scope.
 *
 * @param [out] tls_context A pointer to
 *     {@link ::iotc_bsp_tls_context_t the TLS context}.
 * @param [in] init_params The
 *     {@link ::iotc_bsp_tls_init_params_t TLS context parameters}.
 */
iotc_bsp_tls_state_t iotc_bsp_tls_init(iotc_bsp_tls_context_t** tls_context,
                                       iotc_bsp_tls_init_params_t* init_params);
/**
 * @brief Frees a TLS context from memory and deletes any associated data.
 *
 * @param [out] tls_context A pointer to
 *     {@link ::iotc_bsp_tls_context_t the TLS context}.
 *
 * @retval IOTC_BSP_TLS_STATE_OK TLS context successfully freed.
 */
iotc_bsp_tls_state_t iotc_bsp_tls_cleanup(iotc_bsp_tls_context_t** tls_context);

/**
 * @brief Starts a TLS handshake.
 *
 * @param [out] tls_context A pointer to
 *     {@link ::iotc_bsp_tls_context_t the TLS context}.
 */
iotc_bsp_tls_state_t iotc_bsp_tls_connect(iotc_bsp_tls_context_t* tls_context);

/**
 * @brief Reads data on a socket.
 *
 * @param [out] tls_context A pointer to
 *     {@link ::iotc_bsp_tls_context_t the TLS context}.
 * @param [in] data_ptr A pointer to a buffer to store data that is read.
 * @param [in] data_size The size, in bytes, of the buffer to which data_ptr
 *     points.
 * @param [out] bytes_read The number of bytes read.
 */
iotc_bsp_tls_state_t iotc_bsp_tls_read(iotc_bsp_tls_context_t* tls_context,
                                       uint8_t* data_ptr, size_t data_size,
                                       int* bytes_read);

/**
 * @brief Gets the pending readable bytes.
 *
 * @param [out] tls_context A pointer to
 *     {@link ::iotc_bsp_tls_context_t the TLS context}.
 */
int iotc_bsp_tls_pending(iotc_bsp_tls_context_t* tls_context);

/**
 * @brief Write data to a socket.
 *
 * @param [out] tls_context A pointer to
 *     {@link ::iotc_bsp_tls_context_t the TLS context}.
 * @param [in] data_ptr A pointer to a buffer with the data to be sent.
 * @param [in] data_size The size, in bytes, of the buffer to which
 *     {@code data_ptr} points.
 * @param [out] bytes_written The number of bytes written.
 */
iotc_bsp_tls_state_t iotc_bsp_tls_write(iotc_bsp_tls_context_t* tls_context,
                                        uint8_t* data_ptr, size_t data_size,
                                        int* bytes_written);

/** @details Notifies the client application to read data on a socket. Don't
 * modify this function. */
iotc_bsp_tls_state_t iotc_bsp_tls_recv_callback(char* buf, int sz,
                                                void* context, int* bytes_sent);

/** @details Notifies the client application to write data to a socket. Don't
 * modify this function. */
iotc_bsp_tls_state_t iotc_bsp_tls_send_callback(char* buf, int sz,
                                                void* context, int* bytes_sent);

#endif /* __IOTC_BSP_TLS_H__ */
