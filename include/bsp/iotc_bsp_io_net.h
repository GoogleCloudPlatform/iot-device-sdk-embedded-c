/* Copyright 2018-2020 Google LLC
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
 * @mainpage Overview
 *
 * @details This is a reference for the Board Support Package (BSP) and its
 * components. The BSP is a set of well-defined functions that the SDK
 * invokes to interact with hardware-specific drivers and routines. The
 * functions run on POSIX platforms by default. To customize the BSP for
 * new devices, see the <a href="../../../porting_guide.md">porting guide</a>.
 *
 * # Function summary
 * The following tables list the hardware-specific drivers and routines you
 * can define to implement MQTT with TLS.
 *
 * ## Timekeeping
 * | Function | Description |
 * | --- | --- |
 * iotc_bsp_time_init() | Initializes the platform-specific timekeeping requirements. |  
 * iotc_bsp_time_getcurrenttime_milliseconds() | Gets the milliseconds since Epoch. |
 * iotc_bsp_time_getcurrenttime_seconds() | Gets the seconds since Epoch. |
 * iotc_bsp_time_getmonotonictime_milliseconds() | Gets the monotonic time in milliseconds. |
 *
 * ## Generating random numbers
 * | Function | Description |
 * | --- | --- |
 * iotc_bsp_rng_get() | Generates and returns a random, 32-bit integer. |
 * iotc_bsp_rng_init() | Initializes the platform-specific RNG requirements. |
 * iotc_bsp_rng_shutdown() | Shuts down the RNG and frees all of the resources from initializing and generating random numbers. |
 *
 * ## Generating JWTs
 * | Function | Description |
 * | --- | --- |
 * iotc_bsp_base64_encode_urlsafe() | Encodes a string as a URL-safe, base64 string by replacing all URL-unsafe characters with a - (dash) or _ (underscore). |
 * iotc_bsp_ecc() | Generates an Elliptic Curve signature for a private key. |
 * iotc_bsp_sha256() | Generates a SHA256 cryptographic hash. |
 *
 * ## Managing files
 * | Function | Description |
 * | --- | --- |
 * iotc_bsp_io_fs_open() | Opens a file. |
 * iotc_bsp_io_fs_read() | Reads a file. |
 * iotc_bsp_io_fs_stat() | Gets the size of a file. |
 * iotc_bsp_io_fs_write() | Writes to a file. |
 * iotc_bsp_io_fs_close() | Closes a file and frees all of the resources from reading or writing to the file. | 
 * iotc_bsp_io_fs_remove() | Deletes a file. | 
 *
 * ## Managing TLS connections
 * | Function | Description |
 * | --- | --- |
 * iotc_bsp_tls_init() | Initializes a TLS library and creates a TLS context. | 
 * iotc_bsp_tls_connect() | Starts a TLS handshake. |
 * iotc_bsp_tls_pending() | Gets the pending readable bytes. |
 * iotc_bsp_tls_read() | Decrypts MQTT messages. |
 * iotc_bsp_tls_write() | Encrypts MQTT messages. |
 * iotc_bsp_tls_cleanup() | Frees a TLS context from memory and deletes any associated data. | 
 *
 * ## Managing platform memory
 * | Function | Description |
 * | --- | --- |
 * iotc_bsp_mem_alloc() | Allocates memory and returns a pointer to the allocated block. |
 * iotc_bsp_mem_free() | Frees a block of memory. |
 * iotc_bsp_mem_realloc() | Changes the size of a memory block and returns a pointer to the reallocated block. |
 *
 * ## Creating and managing sockets
 * | Function | Description |
 * | --- | --- |
 * iotc_bsp_io_net_socket_connect() | Creates a {@link iotc_bsp_io_net_socket_connect() socket} and connects it to an endpoint. | 
 * iotc_bsp_io_net_connection_check() | Checks a {@link iotc_bsp_io_net_socket_connect() socket} connection status |
 * iotc_bsp_io_net_read() | Reads from a {@link iotc_bsp_io_net_socket_connect() socket}. |
 * iotc_bsp_io_net_select() | Checks a {@link iotc_bsp_io_net_socket_connect() socket} for scheduled read or write operations. |
 * iotc_bsp_io_net_write() | Writes to a {@link iotc_bsp_io_net_socket_connect() socket}. |
 * iotc_bsp_io_net_close_socket() | Closes a {@link iotc_bsp_io_net_socket_connect() socket}. | 
 *
 * # POSIX BSP
 * The POSIX BSP is in the
 * <code><a href="../../../src/bsp/platforms/posix">src/bsp/platforms/posix</a></code>
 * directory. If you're customizing the BSP for a new device, you can refer
 * to this implementation as an example.
 *
 * # TLS implementations
 * The SDK has turn-key
 * <a href="../../../src/bsp/tls/mbedtls">mbedTLS</a>
 * and <a href="../../../src/bsp/tls/wolfssl">wolfSSL</a> implementations. The 
 * default <code>make</code> target
 * <a href="../../../src/bsp/tls/mbedtls">downloads and builds</a> mbedTLS.
 *
 * To use the turn-key wolfTLS implementation, specify 
 * <code>IOTC_BSP_TLS=wolfssl</code> in the
 * <a href="../../../doc/porting_guide.md#build-environment">main makefile</a>.
 *
 * The turn-key TLS libraries consist of customized
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
 * @brief Creates and manages asynchronous sockets in the device's native socket library.
 *
 * @details A typical networking workflow:
 *    1. Create a socket.
 *    2. Connect the socket to a host.
 *    3. Check the connection status.
 *    4. Send data to the host or read data from the socket.
 *    5. Close the socket.
 */

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @typedef iotc_bsp_io_net_state_t
 * @brief The networking function states.
 *
 * @see iotc_bsp_io_net_state_e
 */
typedef enum iotc_bsp_io_net_state_e {
  /** The networking function succeeded. */
  IOTC_BSP_IO_NET_STATE_OK = 0,
  /** Something went wrong. */
  IOTC_BSP_IO_NET_STATE_ERROR = 1,
  /** @details The resource is busy. Invoke function again. */
  IOTC_BSP_IO_NET_STATE_BUSY = 2,
  /** The connection was lost. */
  IOTC_BSP_IO_NET_STATE_CONNECTION_RESET = 3,
  /** A timeout occurred. */
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
 * @see #iotc_bsp_socket_events_s
 *
 * @struct iotc_bsp_socket_events_s
 * @brief The socket state.
 */
typedef struct iotc_bsp_socket_events_s {
  /** The platform-specific socket value. */
  iotc_bsp_socket_t iotc_socket;
  /** <code>1</code> if a write request is made to a socket, <code>0</code>
   * otherwise. */
  uint8_t in_socket_want_read : 1;
  /** <code>1</code> if a read request is made to a socket, <code>0</code>
   * otherwise. */
  uint8_t in_socket_want_write : 1;
  /** <code>1</code> if an error request is made to a socket, <code>0</code>
   * otherwise. */
  uint8_t in_socket_want_error : 1;
  /** <code>1</code> if a connection request is made to a socket, <code>0</code>
   * otherwise. */
  uint8_t in_socket_want_connect : 1;
  /** <code>1</code> if data can be read from a socket, <code>0</code>
   * otherwise. */
  uint8_t out_socket_can_read : 1;
  /** <code>1</code> if data can be written to a socket, <code>0</code>
   * otherwise. */
  uint8_t out_socket_can_write : 1;
  /** <code>1</code> if an error occurs on the socket, <code>0</code>
   * otherwise. */
  uint8_t out_socket_error : 1;
  /** <code>1</code> if a connection request succeeded, <code>0</code>
   * otherwise. */
  uint8_t out_socket_connect_finished : 1;
} iotc_bsp_socket_events_t;

/**
 * @details Creates a socket and connects it to an endpoint.
 *
 * @param [out] iotc_socket The platform-specific socket representation
 *     This value is passed to all further BSP networking calls.
 * @param [in] host The null-terminated IP or fully-qualified domain name of the
 *     host at which to connect.
 * @param [in] port The port number of the endpoint.
 * @param [in] socket_type The {@link #iotc_bsp_socket_type_e socket protocol}.
 */
iotc_bsp_io_net_state_t iotc_bsp_io_net_socket_connect(
    iotc_bsp_socket_t* iotc_socket, const char* host, uint16_t port,
    iotc_bsp_socket_type_t socket_type);

/**
 * @brief Checks a {@link iotc_bsp_io_net_socket_connect() socket} for scheduled
 * read or write operations.
 *
 * @param [in] socket_events_array An array of socket events.
 * @param [in] socket_events_array_size The number of elements in
 *     socket_events_array.
 * @param [in] timeout_sec The number of seconds before timing out.
 *
 * @returns A {@link #iotc_bsp_socket_events_s networking function state}.
 */
iotc_bsp_io_net_state_t iotc_bsp_io_net_select(
    iotc_bsp_socket_events_t* socket_events_array,
    size_t socket_events_array_size, long timeout_sec);

/**
 * @details Checks a {@link iotc_bsp_io_net_socket_connect() socket} connection
 * status.
 *
 * The SDK calls the function after iotc_bsp_io_net_connect() to complete the
 * socket connection. If the socket is connected, the SDK initiates a TLS
 * handshake.
 *
 * @param [in] iotc_socket The socket on which check the connection.
 * @param [in] host The null-terminated IP or fully-qualified domain name of the
 *     host at which to connect.
 * @param [in] port The port number of the endpoint.
 */
iotc_bsp_io_net_state_t iotc_bsp_io_net_connection_check(
    iotc_bsp_socket_t iotc_socket, const char* host, uint16_t port);

/**
 * @brief Writes data to a {@link iotc_bsp_io_net_socket_connect() socket}.
 *
 * @details This function writes data in chunks, so the SDK calls it
 * repeatedly until all chunks are written to the buffer. The SDK writes
 * a new chunk to the socket on each event loop tick.
 *
 * @param [in] iotc_socket_nonblocking The socket on which to send data.
 * @param [out] out_written_count The number of bytes written to the socket.
 *     If the value is negative, the connection is closed.
 * @param [in] buf A pointer to a buffer with the data.
 * @param [in] count The size, in bytes, of the buffer to which the buf
 *     parameter points.
 */
iotc_bsp_io_net_state_t iotc_bsp_io_net_write(
    iotc_bsp_socket_t iotc_socket_nonblocking, int* out_written_count,
    const uint8_t* buf, size_t count);

/**
 * @brief Reads data from a {@link iotc_bsp_io_net_socket_connect() socket}.
 *
 * @param [in] iotc_socket_nonblocking The socket from which to read data.
 * @param [out] out_read_count The number of bytes read from the socket.
 * @param [out] buf A pointer to a buffer with the data read from the socket.
 * @param [in] count The size, in bytes, of the buffer to which the buf
 *     parameter points.
 */
iotc_bsp_io_net_state_t iotc_bsp_io_net_read(
    iotc_bsp_socket_t iotc_socket_nonblocking, int* out_read_count,
    uint8_t* buf, size_t count);

/**
 * @brief Closes a {@link iotc_bsp_io_net_socket_connect() socket}.
 *
 * @param [in] iotc_socket_nonblocking The socket to close.
 */
iotc_bsp_io_net_state_t iotc_bsp_io_net_close_socket(
    iotc_bsp_socket_t* iotc_socket_nonblocking);

#ifdef __cplusplus
}
#endif

#endif /* __IOTC_BSP_IO_NET_H__ */
