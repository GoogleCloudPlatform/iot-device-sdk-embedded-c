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

#ifndef __IOTC_BSP_TLS_H__
#define __IOTC_BSP_TLS_H__

/**
 * @file iotc_bsp_tls.h
 * @brief IoTC Client's Board Support Platform (BSP) for Transport Layer
 * Security (TLS)
 *
 * This file defines the API of a TLS Board Support Package (BSP) that the
 * IoT Device SDK would use to secure its connection to the Google Cloud IoT Core
 * service. In order to build this secure connection you will need to implement
 * these functions to utilize a TLS implementation on your device. Reference
 * implementations of this TLS BSP have been provided in source form for your
 * reference in the src/bsp/tls directory.
 *
 * These functions should be implemented in a non-blocking fashion.
 */

#include <stddef.h>
#include <stdint.h>

/**
 * @typedef iotc_bsp_io_net_state_e
 * @brief Return value of the BSP NET API functions.
 *
 * An implementation of the TLS BSP must return internal status
 * to IoT Device SDK through these state values.
 */
typedef enum iotc_bsp_tls_state_e {
  /** operation finished successfully */
  IOTC_BSP_TLS_STATE_OK = 0,
  /** init operation failed */
  IOTC_BSP_TLS_STATE_INIT_ERROR = 1,
  /** ca certification validation failed */
  IOTC_BSP_TLS_STATE_CERT_ERROR = 2,
  /** error during connecting process */
  IOTC_BSP_TLS_STATE_CONNECT_ERROR = 3,
  /** io is busy wait for data */
  IOTC_BSP_TLS_STATE_WANT_READ = 4,
  /** io buffer is full wait for signal */
  IOTC_BSP_TLS_STATE_WANT_WRITE = 5,
  /** error during reading operation */
  IOTC_BSP_TLS_STATE_READ_ERROR = 6,
  /** error during writing operation */
  IOTC_BSP_TLS_STATE_WRITE_ERROR = 7,
} iotc_bsp_tls_state_t;

/**
 * @typedef iotc_bsp_tls_init_params_t
 * @brief IoT Device SDK  BSP TLS init function parameters.
 *
 * Contains data provided by the IoT Device SDK when it invokes
 * iotc_bsp_tls_init().
 */
typedef struct iotc_bsp_tls_init_params_s {
  /** context variable required by iotc_bsp_tls_send_callback and
   * iotc_bsp_tls_recv_callback functions */
  void* libiotc_io_callback_context;

  /** Pointer to a buffer of Root CA certificate(s) in PEM format
   * which shall be used to authenticate the IoT Core service
   * certificate during TLS handshaking. */
  uint8_t* ca_cert_pem_buf;
  /** length of the buffer containing the above certificate data */
  size_t ca_cert_pem_buf_length;

  /** A set of function pointers to the IoT Device SDK'ss memory management
   * functions. As an option, you may provide these to your TLS implementation
   * if you would like to track the TLS implementation's memory allocations
   * with the optional IoTC Client's Memory Limiter functionality. */
  void* (*fp_libiotc_alloc)(size_t);
  void* (*fp_libiotc_calloc)(size_t, size_t);
  void* (*fp_libiotc_realloc)(void*, size_t);
  void (*fp_libiotc_free)(void*);

  /** A pointer to a NULL terminated string with the domain name that the IoT
   * Device SDK is attempting to connect to.  Provided for use in the TLS
   * implementation's certificate domain name check and for SNI. */
  const char* domain_name;

} iotc_bsp_tls_init_params_t;

/**
 * @typedef iotc_bsp_tls_context_t
 * @brief IoT Device SDK BSP TLS's context representation type
 *
 * It is assumed that you may need to create a TLS library context
 * in the TLS BSP's initialization function.
 *
 * This type can be used to return such a context to the IoT Device SDK,
 * which will then provide the value to your other TLS BSP functions
 * for use in their implementations.
 *
 * The IoT Device SDK is unaware of the actual content and structure
 * of this value, thus it does not read or write this context directly.
 */
typedef void iotc_bsp_tls_context_t;

/**
 * @function
 * @brief Provides a method for the IoT Device SDK to initialize a TLS library.
 *
 * Initializes the TLS library and returns its context through a tls_context
 * parameter.
 *
 * This function will be called before other BSP TLS functions. The
 * implementation should return IOTC_BSP_TLS_STATE_OK in case of a success or
 * IOTC_BSP_TLS_STATE_INIT_ERROR in case of a failure.
 *
 * Content of the init_params will be destroyed after the iotc_bsp_tls_init
 * returns. Any data required from init_params to live outside the scope of
 * this function must be copied and stored by your implementation.
 *
 * The IoT Device SDK I/O implementation requires the regstration
 * custom send and recv functions in the TLS library. These functions,
 * iotc_bsp_tls_recv_callback and iotc_bsp_tls_send_callback, declared
 * below, are implemented by the IoT Device SDK internally to send and
 * received data through the IoT Device SDK's I/O system, respecitvely.
 *
 * For more details please refer to our reference implementations of the
 * TLS BSP in src/bsp/tls.
 *
 * @param [out] tls_context pointer to a pointer to a iotc_bsp_tls_context_t
 * @param [in] init_params data required for TLS library initialization
 * @return
 *  - IOTC_BSP_TLS_STATE_OK in case of success
 *  - IOTC_BSP_TLS_STATE_INIT_ERROR otherwise
 */
iotc_bsp_tls_state_t iotc_bsp_tls_init(iotc_bsp_tls_context_t** tls_context,
                                       iotc_bsp_tls_init_params_t* init_params);
/**
 * @function
 * @brief Provides a method for the TLS BSP implementation to free its context.
 *
 * Invoked when the IoT Device SDK is cleaning up a closed or closing connection.
 * The implementation of this function must deallocate the TLS library's
 * previously allocated resources. Your implementation should also release any
 * data associated with the tls_context as no further operations will be
 * requested of it.
 *
 * @param [in|out] tls_context
 * @return IOTC_BSP_TLS_STATE_OK
 */
iotc_bsp_tls_state_t iotc_bsp_tls_cleanup(iotc_bsp_tls_context_t** tls_context);

/**
 * @function
 * @brief Implements the TLS connect functionality.
 *
 * This function may be called several times by IoT Device SDK before the
 * handshake is completed. This function shouldn't block, and may return
 * IOTC_BSP_TLS_STATE_WANT_WRITE or IOTC_BSP_TLS_STATE_WANT_READ instead.
 *
 * This function will send or receive data internally during it's execution so
 * it is expected that there will be subsequent calls to the TLS library's
 * send and recv functions.
 *
 * In case of any error this function must return
 * IOTC_BSP_TLS_STATE_CONNECT_ERROR.
 *
 * @param [in] tls_context
 * @return
 *  - IOTC_BSP_TLS_STATE_OK in case of a successfully finished handshake
 *  - IOTC_BSP_TLS_STATE_WANT_READ | IOTC_BSP_TLS_STATE_WANT_WRITE in case
 * handshake requires to receive or send more data in order to continue
 *  - IOTC_BSP_TLS_STATE_CERT_ERROR
 *  - IOTC_BSP_TLS_STATE_CONNECT_ERROR in case of failure
 */
iotc_bsp_tls_state_t iotc_bsp_tls_connect(iotc_bsp_tls_context_t* tls_context);

/**
 * @function
 * @brief Implements the a function to read data from the TLS implementation.
 *
 * Implementation of this function should call the TLS library's read function.
 * It should also translate the result returned by the TLS read function to one
 * of the following IoT Device SDK BSP state values:
 * - IOTC_BSP_TLS_STATE_OK - whenever the read operation finished successfully.
 * - IOTC_BSP_TLS_STATE_WANT_READ - whenever the read operation requires more
 * data.
 * - IOTC_BSP_TLS_STATE_READ_ERROR - whenever any other error occurred during
 * the read operation.
 *
 * The argument bytes_read should be set to the number of bytes read and
 * processed by the TLS implementation.
 *
 * @param [in] tls_context - context of the TLS library.
 * @param [in] data_ptr - ptr to a buffer to store any read data.
 * @param [in] data_size - size of the buffer.
 * @param [out] bytes_read - number of bytes read via the TLS library read
 * function.
 * @return
 * - IOTC_BSP_TLS_STATE_OK
 * - IOTC_BSP_TLS_STATE_WANT_READ
 * - IOTC_BSP_TLS_STATE_READ_ERROR
 */
iotc_bsp_tls_state_t iotc_bsp_tls_read(iotc_bsp_tls_context_t* tls_context,
                                       uint8_t* data_ptr, size_t data_size,
                                       int* bytes_read);

/**
 * @function
 * @brief Implements a function to determine number of pending readable bytes.
 *
 * Implementation of this function should call the TLS library's pending
 * function. It should return the the number of bytes that are available to be
 * read by the TLS library's read function.
 *
 * @param [in] tls_context - context of the TLS library.
 * @return number of bytes available to read.
 */
int iotc_bsp_tls_pending(iotc_bsp_tls_context_t* tls_context);

/**
 * @function
 * @brief Implements the TLS write to encrypt and send data.
 *
 * The implementation of this function should call TLSs library write function.
 * It should also translate the result returned by the TLS library to one of:
 * - IOTC_BSP_TLS_STATE_OK - whenever the operation finished successfully.
 * - IOTC_BSP_TLS_STATE_WANT_WRITE - whenever the write operation wants to send
 * more data.
 * - IOTC_BSP_TLS_STATE_WRITE_ERROR - whenever any other error occured during
 * the write operation
 *
 * Argument bytes_written should be set to a number of bytes actually written.
 *
 * @param [in] tls_context - context of the TLS library
 * @param [in] data_ptr - ptr to a buffer with data to be sent
 * @param [in] data_size - size of the buffer
 * @param [out] bytes_written - number of bytes written via the TLS library
 * write function
 * @return
 * - IOTC_BSP_TLS_STATE_OK
 * - IOTC_BSP_TLS_STATE_WANT_WRITE
 * - IOTC_BSP_TLS_STATE_WRITE_ERROR
 */
iotc_bsp_tls_state_t iotc_bsp_tls_write(iotc_bsp_tls_context_t* tls_context,
                                        uint8_t* data_ptr, size_t data_size,
                                        int* bytes_written);

/** These are implemented by IoT Device SDK and should not be defined by
 * any BSP TLS solutions.  However, pointers to these functions must be
 * provided to the TLS implementation to allow the IoTC client to read and
 * write data to it. */
iotc_bsp_tls_state_t iotc_bsp_tls_recv_callback(char* buf, int sz,
                                                void* context, int* bytes_sent);
iotc_bsp_tls_state_t iotc_bsp_tls_send_callback(char* buf, int sz,
                                                void* context, int* bytes_sent);

#endif /* __IOTC_BSP_TLS_H__ */
