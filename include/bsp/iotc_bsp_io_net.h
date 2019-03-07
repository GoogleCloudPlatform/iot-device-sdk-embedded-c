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
 * # Welcome
 * The Board Support Package (BSP) hosts the platform-specific code for the
 * Device SDK.
 *
 * Modify the BSP accordingly to port the Device SDK to new platforms. Other
 * functionality, such as the Device SDK source, event system, MQTT serializer,
 * and callback system, isn't part of the BSP.
 *
 * # Getting started
 * The Device SDK includes a POSIX implementation of the BSP. Use this
 * implementation for for Linux desktops and devices. 
 *
 * To customize the BSP for non-POSIX platforms, see the <a href="../../../porting_guide.md">porting guide</a>.
 * <code>iotc_bsp_rng.h</code>, <code>iotc_bsp_time.h</code>, and <code>iotc_bsp_io_fs.h</code>
 * are the most portable and may not need any customization.
 *
 * # BSP source files
 * The BSP consists of the following files. Each file meets each meet a platform
 * library requirement.
 *  - <code>iotc_bsp_crypto.h</code> implements a cryptography library to sign
 * JWTs.
 *  - <code>iotc_bsp_io_net.h</code> implements asychronous networking.
 *  - <code>iotc_bsp_mem.h</code> allocates platform memory.
 *  - <code>iotc_bsp_rng.h</code> implements random number generation.
 *  - <code>iotc_bsp_time.h</code> implements time functions.
 *  - <code>iotc_bsp_tls.h</code> implements Transport Layer Security (TLS).
 *  - (Optional) <code>iotc_bsp_io_fs.h</code> manages the file system to store
 * certificates.
 *
 * # TLS BSPs
 * The Device SDK includes the following out-of-the-box TLS implementations.
 *
 * ## mbedTLS
 * The default <code>make</code> target <a href"~/src/bsp/mbedtls">downloads and builds</a>
 * <a href"https://tls.mbed.org">mbedTLS</a>.
 *
 * ## wolfSSL
 * See the <a href="../../../user_guide.md">user guide</a> and <a href="../../../porting_guide.md">porting guide</a>
 * to configure the Device SDK for wolfSSL.
 *
 * # Further Reading
 * <ul><li>Device SDK <a href="../../api/html/index.html">API reference</a></li>
 * <li>Device SDK <a href="../../../user_guide.md">user guide</a></li>
 * <li>Device SDK <a href="../../../porting_guide.md">porting guide</a></li>
 * </ul>
 */

/**
 * @file  iotc_bsp_io_net.h
 * @brief Perform asynchronous networking.
 *
 * Connect a platform's networking SDK to the Device SDK in order to
 * perform asynchronous, platform-specific networking.
 *
 * To port the Device SDK to a new platform, integrate the platform's networking
 * SDK with the Device SDK's non-blocking, cooprative multitasking environment.
 *
 * A typical networking workflow consists of the following steps.
 *    1. Create a socket.
 *    2. Connect the socket to a host.
 *    3. Check the connection status. Required for asychronous connections.
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
 * @typedef iotc_bsp_io_net_state_e
 * @brief Networking status codes.
 *
 * All networking functions report a status message to the client
 * application. IOTC_BSP_IO_NET_STATE_OK represents success and others
 * represent errors.
 */
typedef enum iotc_bsp_io_net_state_e {
  /** Operation successful. */
  IOTC_BSP_IO_NET_STATE_OK = 0,
  /** Operation failed. Generic error. */
  IOTC_BSP_IO_NET_STATE_ERROR = 1,
  /** Resource is busy. Invoke function again. */
  IOTC_BSP_IO_NET_STATE_BUSY = 2,
  /** Connection lost. */
  IOTC_BSP_IO_NET_STATE_CONNECTION_RESET = 3,
  /** Timeout occurred. */
  IOTC_BSP_IO_NET_STATE_TIMEOUT = 4,

} iotc_bsp_io_net_state_t;

/**
 * @typedef iotc_bsp_socket_t
 * @brief Socket representation.
 * 
 * This data type stores socket representations, such as handles and 
 * descriptors. All socket operations require a socket representation.
 */
typedef intptr_t iotc_bsp_socket_t;

/**
 * @typedef iotc_bsp_socket_event_s
 * @brief Socket state.
 */
typedef struct iotc_bsp_socket_events_s {
  /** Platform-specific socket value. */
  iotc_bsp_socket_t iotc_socket;
  /** <code>1</code> to attempt a read operation, <code>0</code> otherwise. */
  uint8_t in_socket_want_read : 1;
  /** <code>1</code> to attempt a write operation, <code>0</code> otherwise. */
  uint8_t in_socket_want_write : 1;
  /** <code>1</code> to read error messages, <code>0</code> otherwise. */
  uint8_t in_socket_want_error : 1;
  /** <code>1</code> to connect to a host, <code>0</code> othwerwise. */
  uint8_t in_socket_want_connect : 1;
  /** <code>1</code> if socket is read-enabled, <code>0</code> otherwise. */
  uint8_t out_socket_can_read : 1;
  /** <code>1</code> if socket is write-enabled, <code>0</code> otherwise. */
  uint8_t out_socket_can_write : 1;
  /** <code>1</code> if an error occurs, <code>0</code> otherwise */
  uint8_t out_socket_error : 1;
  /** <code>1</code> if the connected, <code>0</code> otherwise */
  uint8_t out_socket_connect_finished : 1;
} iotc_bsp_socket_events_t;

/**
 * @function
 * @brief Query socket states to schedule read and write operations.
 *
 * Each element in the socket_events_array parameter corresponds to a socket and
 * contains an initialized iotc_bsp_socket_state_t structure. 
 *
 * @param [in] socket_events_array An array of socket events.
 * @param [in] socket_events_array_size The size, in bytes of
 * socket_events_array.
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
 * @function
 * @brief Create a non-blocking socket.
 *
 * @param [out] iotc_socket_nonblocking the platform-specific socket
 * representation.
 *
 * @retval IOTC_BSP_IO_NET_STATE_OK Socket successfully created.
 * @retval IOTC_BSP_IO_NET_STATE_ERROR Can't create socket.
 */
iotc_bsp_io_net_state_t iotc_bsp_io_net_create_socket(
    iotc_bsp_socket_t* iotc_socket_nonblocking);

/**
 * @function
 * @brief Connect a socket to an endpoint. 
 *
 * To connect a socket to an endpoint, first run <code>iotc_bsp_io_net_create_socket()</code>
 * to create a socket.
 *
 * @param [in] iotc_socket_nonblocking The socket to connect to the host.
 * @param [in] host The null-terminated IP or fully-qualified domain name of the
 *     host to connect to.
 * @param [in] port The port number of the endpoint.
 *
 * @see iotc_bsp_io_net_create_socket
 *
 * @retval IOTC_BSP_IO_NET_STATE_OK Socket successfully connected to host.
 * @retval IOTC_BSP_IO_NET_STATE_ERROR Socket didn't connect.
 */
iotc_bsp_io_net_state_t iotc_bsp_io_net_connect(
    iotc_bsp_socket_t* iotc_socket_nonblocking, const char* host,
    uint16_t port);

/**
 * @function
 * @brief Check the socket connection status and report whether the socket is
 * connected to a host. 
 *
 * Call this function after <code>iotc_bsp_io_net_connect()</code> to complete
 * the socket the connection.
 *
 * This function is required for asychronous connections. If this function
 * determines that the socket is connected, the Device SDK initiates a TLS
 * handshake.
 *
 * @param [in] iotc_socket_nonblocking The socket on which check the connection.
 * @param [in] host The null-terminated IP or fully-qualified domain name of the
 *     host to connect to.
 * @param [in] port The port number of the endpoint.
 *
 * @see iotc_bsp_io_net_connect
 *
 * @retval IOTC_BSP_IO_NET_STATE_OK Socket successfully connected.
 * @retval IOTC_BSP_IO_NET_STATE_ERROR Socket didn't connected.
 */
iotc_bsp_io_net_state_t iotc_bsp_io_net_connection_check(
    iotc_bsp_socket_t iotc_socket_nonblocking, const char* host, uint16_t port);

/**
 * @function
 * @brief Write data to a socket.
 *
 * This function writes data in chunks, so if it returns
 * IOTC_BSP_IO_NET_STATE_BUSY then call it again to send the remaining data.
 * If the subsequent call also returns IOTC_BSP_IO_NET_STATE_BUSY, wait more
 * time for the first chunk to transmit and then try again.
 *
 * This is a non-blocking operation. 
 *
 * @param [in] iotc_socket_nonblocking The socket on which to send data.
 * @param [out] out_written_count The number of bytes written to the socket.
 *     Negative values indicate a closed connection.
 * @param [in] buf A pointer to a buffer with the data.
 * @param [in] count The size, in bytes, of the buffer to which the buf 
 * parameter points.
 *
 * @see iotc_bsp_io_net_create_socket
 *
 * @retval IOTC_BSP_IO_NET_STATE_OK All data is written to the socket.
 * @retval IOTC_BSP_IO_NET_STATE_BUSY Some of the data is written to the socket.
 *     Call the function again to send the remaining data.
 * @retval IOTC_BSP_IO_NET_STATE_ERROR No data is sent to the host.
 */
iotc_bsp_io_net_state_t iotc_bsp_io_net_write(
    iotc_bsp_socket_t iotc_socket_nonblocking, int* out_written_count,
    const uint8_t* buf, size_t count);

/**
 * @function
 * @brief Read data from a socket. This is a non-blocking operation.
 *
 * @param [in] iotc_socket_nonblocking The socket from which to read data.
 * @param [out] out_read_count The number of bytes read from the socket.
 * @param [out] buf A pointer to a buffer with the data read from the socket.
 * @param [in] count The maximum size, in bytes, of the buf parameter.
 *
 * @see iotc_bsp_io_net_create_socket
 *
 * @retval IOTC_BSP_IO_NET_STATE_OK Data is successfully read from the socket.
 * @retval IOTC_BSP_IO_NET_STATE_BUSY No data is available on the socket.
 * @retval IOTC_BSP_IO_NET_STATE_ERROR No data is read from the socket.
 */
iotc_bsp_io_net_state_t iotc_bsp_io_net_read(
    iotc_bsp_socket_t iotc_socket_nonblocking, int* out_read_count,
    uint8_t* buf, size_t count);

/**
 * @function
 * @brief Close a socket.
 *
 * @param [in] iotc_socket_nonblocking The socket to close.
 *
 * @see iotc_bsp_io_net_create_socket
 *
 * @retval IOTC_BSP_IO_NET_STATE_OK Socket successfully closed.
 * @retval IOTC_BSP_IO_NET_STATE_ERROR Socket remains open.
 */
iotc_bsp_io_net_state_t iotc_bsp_io_net_close_socket(
    iotc_bsp_socket_t* iotc_socket_nonblocking);

#ifdef __cplusplus
}
#endif

#endif /* __IOTC_BSP_IO_NET_H__ */
