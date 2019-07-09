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

#ifndef __IOTC_BSP_IO_NET_H__
#define __IOTC_BSP_IO_NET_H__

/**
 * \mainpage Device SDK Board Support Package
 *
 * @details The Board Support Package (BSP) is a set of well-defined functions
 * that the Device SDK invokes to interact with hardware-specific drivers and
 * routines.
 *
 * Implement these functions to:
 *     - Perform asychronous networking
 *     - Allocate platform memory
 *     - Generate random numbers
 *     - Keep time
 *     - Communicate with TLS
 *
 * The Device SDK builds natively on POSIX platforms. See the
 * <a href="../../../porting_guide.md">porting guide</a> for more information
 * about customizing the BSP for non-POSIX platforms.
 *
 * # POSIX BSP
 * The POSIX BSP implementation is in the
 * <code><a href="~/src/bsp/platforms/posix">src/bsp/platforms/posix</a></code>
 * directory. It implements 
 * <a href="https://en.wikipedia.org/wiki/Mbed_TLS">mbedTLS</a> by default.
 *
 * # TLS implementations
 * The Device SDK has turn-key
 * <a href="../../../src/bsp/tls/mbedtls">mbedTLS</a>
 * and <a href="~/src/bsp/tls/wolfssl">wolfSSL</a> implementations. The 
 * default <code>make</code> target
 * <a href="../../../src/bsp/tls/mbedtls">downloads and builds</a> mbedTLS. See
 * the <a href="../../../doc/user-guide.md">user guide</a> to configure the
 * Device SDK with wolfTLS.
 *
 * The turn-key TLS implementations consist of customized
 * <a href="dd/d79/iotc__bsp__tls_8h.html">iot_bsp_tls.h</a> and
 * <a href="dd/d79/iotc__bsp__crypto_8h.html">iotc_bsp_crypto.h</a>
 * functions:
 * | mbedTLS | wolfSSL |
 * | --- | --- |
 * | <a href="../../../src/bsp/tls/mbedtls/iotc_bsp_tls_mbedtls.c">src/bsp/tls/mbedtls/iotc_bsp_tls_mbedtls.c</a> | <a href="../../../src/bsp/tls/wolfssl/iotc_bsp_tls_wolfssl.c">src/bsp/tls/wolfssl/iotc_bsp_tls_wolfssl.c</a> |
 * | <a href="../../../src/bsp/crypto/mbedtls/iotc_bsp_crypto.c">src/bsp/crypto/mbedtls/iotc_bsp_crypto.c</a> | <a href="../../../src/bsp/crypto/wolfssl/iotc_bsp_crypto.c">src/bsp/crypto/wolfssl/iotc_bsp_crypto.c</a> |
 */

/**
 * @file  iotc_bsp_io_net.h
 * @brief Creates and manages sockets for asynchronous networking
 *
 * @details A typical networking workflow:
 *    1. Create a socket.
 *    2. Connect the socket to a host.
 *    3. Check the connection status.
 *    4. Send data to the host or read data from the socket.
 *    5. Close the socket.
 *
 * The BSP invokes networking functions in the native socket library.
 */

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @typedef iotc_bsp_io_net_state_t
 * @brief Networking status codes.
 *
 * @see iotc_bsp_io_net_state_e
 */
typedef enum iotc_bsp_io_net_state_e {
  /** The networking function succeeded. */
  IOTC_BSP_IO_NET_STATE_OK = 0,
  /** Something went wrong. */
  IOTC_BSP_IO_NET_STATE_ERROR = 1,
  /** @details Resource is busy. Invoke function again. */
  IOTC_BSP_IO_NET_STATE_BUSY = 2,
  /** Connection lost. */
  IOTC_BSP_IO_NET_STATE_CONNECTION_RESET = 3,
  /** Timeout occurred. */
  IOTC_BSP_IO_NET_STATE_TIMEOUT = 4,

} iotc_bsp_io_net_state_t;

/**
 * @typedef iotc_bsp_socket_type_t
 * @brief The socket protocol.
 *
 * @see #iotc_bsp_socket_type_e
 */
typedef enum iotc_bsp_socket_type_e {
  /** TCP socket. */
  SOCKET_STREAM = 1,
  /** UDP socket. */
  SOCKET_DGRAM = 2,

} iotc_bsp_socket_type_t;

/**
 * @typedef iotc_bsp_protocol_type_t
 * @brief The version of the socket protocol.
 *
 * @see #iotc_bsp_protocol_type_e
 */
typedef enum iotc_bsp_protocol_type_e {
  /** IPv4. */
  PROTOCOL_IPV4 = 2,
  /** IPv6. */
  PROTOCOL_IPV6 = 10,

} iotc_bsp_protocol_type_t;

/**
 * @typedef iotc_bsp_socket_t
 * @brief The socket representation.
 */
typedef intptr_t iotc_bsp_socket_t;

/**
 * @typedef iotc_bsp_socket_events_t
 * @brief The socket state.
 *
 * @see #iotc_bsp_socket_events_s
 */
typedef struct iotc_bsp_socket_events_s {
  /** Platform-specific socket value. */
  iotc_bsp_socket_t iotc_socket;
  /** <code>1</code> if socket wants to read, <code>0</code> otherwise. */
  uint8_t in_socket_want_read : 1;
  /** <code>1</code> if socket wants to write, <code>0</code> otherwise. */
  uint8_t in_socket_want_write : 1;
  /** <code>1</code> if socket wants to know about an error, <code>0</code>
   * otherwise. */
  uint8_t in_socket_want_error : 1;
  /** <code>1</code> if socket waits for a connected, <code>0</code> otherwise.
   */
  uint8_t in_socket_want_connect : 1;
  /** <code>1</code> if socket can read, <code>0</code> otherwise. */
  uint8_t out_socket_can_read : 1;
  /** <code>1</code> if socket can write, <code>0</code> otherwise. */
  uint8_t out_socket_can_write : 1;
  /** <code>1</code> if an error occurs on the socket, <code>0</code> otherwise
   */
  uint8_t out_socket_error : 1;
  /** <code>1</code> if the connection process is finished, <code>0</code>
   * otherwise */
  uint8_t out_socket_connect_finished : 1;
} iotc_bsp_socket_events_t;

/**
 * @details Creates a socket and connect it to an endpoint.
 *
 * @param [out] iotc_socket The platform-specific socket representation
 *     This value is passed to all further BSP networking calls.
 * @param [in] host The null-terminated IP or fully-qualified domain name of the
 *     host at which to connect.
 * @param [in] port The port number of the endpoint.
 * @param [in] socket_type The {@link #iotc_bsp_socket_type_e socket protocol}.
 *
 * @retval IOTC_BSP_IO_NET_STATE_OK Socket successfully connected to host.
 * @retval IOTC_BSP_IO_NET_STATE_ERROR Can't connect socket.
 */
iotc_bsp_io_net_state_t iotc_bsp_io_net_socket_connect(
    iotc_bsp_socket_t* iotc_socket, const char* host, uint16_t port,
    iotc_bsp_socket_type_t socket_type);

/**
 * @brief Queries socket states to schedule read and write operations.
 *
 * @param [in] socket_events_array An array of socket events. Each element must
 *     be an initialized socket.
 * @param [in] socket_events_array_size The number of elements in
 *     socket_events_array.
 * @param [in] timeout The number of seconds before timing out.
 *
 * @retval IOTC_BSP_IO_NET_STATE_OK Socket event successfully updated.
 * @retval IOTC_BSP_IO_NET_STATE_TIMEOUT Query timed out.
 * @retval IOTC_BSP_IO_NET_STATE_ERROR Can't query socket status.
 */
iotc_bsp_io_net_state_t iotc_bsp_io_net_select(
    iotc_bsp_socket_events_t* socket_events_array,
    size_t socket_events_array_size, long timeout_sec);

/**
 * @details Checks the socket connection status.
 *
 * The Device SDK calls this function implementation after
 * iotc_bsp_io_net_connect() to complete the socket connection.
 *
 * This function implemetation serves as a tool for monitoring asychronous
 * connections. If this function impelementation determines that the socket is
 * connected, the Device SDK initiates a TLS handshake.
 *
 * @param [in] iotc_socket_nonblocking The socket on which check the connection.
 * @param [in] host The null-terminated IP or fully-qualified domain name of the
 *     host to connect to.
 * @param [in] port The port number of the endpoint.
 *
 * @retval IOTC_BSP_IO_NET_STATE_OK Socket is connected to host.
 * @retval IOTC_BSP_IO_NET_STATE_ERROR Socket isn't connected.
 */
iotc_bsp_io_net_state_t iotc_bsp_io_net_connection_check(
    iotc_bsp_socket_t iotc_socket, const char* host, uint16_t port);

/**
 * @brief Writes data to a socket. This is a non-blocking operation.
 *
 * This function writes data in chunks, so the Device SDK calls it repeatedly
 * until all chunks are written to the buf parameter. The Device SDK writes a
 * new chunk to the socket on each event loop tick.
 *
 * @param [in] iotc_socket_nonblocking The socket on which to send data.
 * @param [out] out_written_count The number of bytes written to the socket.
 *     If the value is negative, the connection is closed.
 * @param [in] buf A pointer to a buffer with the data.
 * @param [in] count The size, in bytes, of the buffer to which the buf
 *     parameter points.
 *
 * @retval IOTC_BSP_IO_NET_STATE_OK All data is written to the socket.
 * @retval IOTC_BSP_IO_NET_STATE_BUSY None or some of the data is written to the
 *     socket but no error occurred.
 * @retval IOTC_BSP_IO_NET_STATE_ERROR Socket isn't connected.
 *
 * @see iotc_bsp_io_net_create_socket()
 */
iotc_bsp_io_net_state_t iotc_bsp_io_net_write(
    iotc_bsp_socket_t iotc_socket_nonblocking, int* out_written_count,
    const uint8_t* buf, size_t count);

/**
 * @brief Reads data from a socket. This is a non-blocking operation.
 *
 * @param [in] iotc_socket_nonblocking The socket from which to read data.
 * @param [out] out_read_count The number of bytes read from the socket.
 * @param [out] buf A pointer to a buffer with the data read from the socket.
 * @param [in] count The size, in bytes, of the buf parameter.
 *
 * @retval IOTC_BSP_IO_NET_STATE_OK Data is successfully read from the socket.
 * @retval IOTC_BSP_IO_NET_STATE_BUSY No data is available on the socket.
 * @retval IOTC_BSP_IO_NET_STATE_ERROR Socket isn't connected.
 *
 * @see iotc_bsp_io_net_create_socket()
 */
iotc_bsp_io_net_state_t iotc_bsp_io_net_read(
    iotc_bsp_socket_t iotc_socket_nonblocking, int* out_read_count,
    uint8_t* buf, size_t count);

/**
 * @brief Closes a socket. This is a non-blocking operation.
 *
 * @param [in] iotc_socket_nonblocking The socket to close.
 *
 * @retval IOTC_BSP_IO_NET_STATE_OK Socket successfully closed.
 * @retval IOTC_BSP_IO_NET_STATE_ERROR Socket isn't connected.
 */
iotc_bsp_io_net_state_t iotc_bsp_io_net_close_socket(
    iotc_bsp_socket_t* iotc_socket_nonblocking);

#ifdef __cplusplus
}
#endif

#endif /* __IOTC_BSP_IO_NET_H__ */
