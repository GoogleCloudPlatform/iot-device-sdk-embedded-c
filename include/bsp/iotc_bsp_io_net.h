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

#ifndef __IOTC_BSP_IO_NET_H__
#define __IOTC_BSP_IO_NET_H__

/**
 * \mainpage IoTC Board Support Package (BSP)
 *
 * # Welcome
 * This doxygen catalogs the Board Support Package (BSP), an abstracted
 * framework for hosting all of the platform-specific code used by the
 * Google Cloud IoT Edge Embedded C Client (IoTC).
 *
 * Porting engineers should focus most of their work to a custom
 * implementation of these collection of files. The rest of the
 * IoTC client sources, such as the event system, mqtt serializer, and
 * callback system, use platform generic C code that should not need
 * tailoring to specific device SDKs.
 *
 * # Out of the Box
 * The IoTC Client includes a POSIX implementation of the BSP
 * which it uses by default on Linux desktops and devices.
 * For non POSIX platforms your will need to customize the reference
 * implementation, or begin one from scratch.
 *
 * More information on the porting process can be found
 * in the IoTC Porting Guide which resides in the main /doc directory
 * of this project.
 *
 * # Browsing the Sources
 * The BSP is segmented into several distinct files, each focused
 * around a particular platform library requirement:
 *  - Crypto (IoT Core JWT Signing)
 *  - Networking
 *  - Memory Allocators / Dealloactors
 *  - Random Number Generator
 *  - Time
 *  - Transport Layer Security (TLS)
 *  - File System (For Cert Storage, Optional)

 *
 * The best place to start would be the NET BSP to couple the IoTC Client to
 * your device's networking SDK and Crypto to use your TLS or secure chip
 * library to sign a JWT with a private key.
 *
 * Implementations for Time, Rng and Memory should be highly
 * portable and might not need any customization at all.
 *
 * # TLS BSPs
 * The IoTC Client also ships with support for two TLS implementations
 * out of the box:
 *
 * ## &nbsp;mbedTLS <small>(https://tls.mbed.org)</small>
 * The default make target will download a tagged mbedTLS release from
 * their repository, and build it to link against. Additionally the
 * IoTC Client Sources will be configured to build the its TLS BSP
 * for mbedTLS which resides in: <code>/src/bsp/mbedtls</code>.
 *
 * ## &nbsp;wolfSSL <small>(https://www.wolfssl.com/)</small>
 * The client also has a reference TLS BSP implementation for wolfSSL,
 * another TLS library designed for embedded devices.
 *
 * Please see our User Guide and Porting Guide for more information on
 * how to configure the client to use these different TLS implementations.
 * Both documents reside in the base <code>/doc</code> directory.
 *
 * This should get you up and running quickly but their sources must be
 * licensed for distribution.
 *
 * # Further Reading
 * ### IoTC Client
 * Information on how to use the Google IoT Edge Embedded C Client from the
 * applications perspective can be found in:
 * <ul><li>
 * <a href="../../api/html/index.html">The IoTC Client doxygen</a></li>
 * <li>The Google Cloud IoT Edge Embedded C User Guide in:
 *   <code>/doc/user_guide.md</code></li>
 * </ul>
 *
 * ### Porting Process
 * Documentation on the porting process and more information about
 * the IoTC Client BSP can be found in the Google Cloud IoT Edge Embedded C
 * Client Porting Guide located in: <code>/doc/porting_guide.md</code>.
 *
 */

/**
 * @file iotc_bsp_io_net.h
 * @brief IoTC Client's Board Support Platform (BSP) for Asynchronous Networking
 *
 * This file defines the API of an asynchronous platform specific networking
 * implementation. These are all the functions one should implement to couple
 * the IoTC client to the device's networking SDK.
 *
 * These functions should be implemented in a non-blocking fashion.
 * This means that these function should fit into a coopeartive-multitasking
 * environment of the IoTC client, which is designed not to block on any
 * one operation.
 *
 * A device connecting to the Could IoT Core service would have a standard
 * flow looks like the following:
 *   1. create_socket
 *   2. connect
 *   3. connection_check
 *   4. Iterations of read-write operations
 *   5. close_socket when the application shuts down the connection.
 */

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @typedef iotc_bsp_io_net_state_e
 * @brief Return value of the BSP NET API functions.
 *
 * The implementation reports internal status to IoTC Client through these
 * values.
 */
typedef enum iotc_bsp_io_net_state_e {
  /** operation finished successfully. */
  IOTC_BSP_IO_NET_STATE_OK = 0,
  /** operation failed. */
  IOTC_BSP_IO_NET_STATE_ERROR = 1,
  /** resource is busy, means: please invoke this function again later. */
  IOTC_BSP_IO_NET_STATE_BUSY = 2,
  /** connection lost during read or write operation. */
  IOTC_BSP_IO_NET_STATE_CONNECTION_RESET = 3,
  /** timeout has occurred during the operation. */
  IOTC_BSP_IO_NET_STATE_TIMEOUT = 4,

} iotc_bsp_io_net_state_t;

/**
 * @typedef iotc_bsp_protocol_e
 * @brief Represent protocol types.
 *
 * The implementation is used by create_socket and connect function
 * to act properly with protocols.
 */
typedef enum iotc_bsp_protocol_e {
  /** Transmission Control Protocol. */
  IOTC_BSP_PROTOCOL_TCP = 0,
  /** User Datagram Protocol. */
  IOTC_BSP_PROTOCOL_UDP = 1,
} iotc_bsp_protocol_t;

/**
 * @typedef iotc_bsp_socket_t
 * @brief IoTC Client BSP NET's socket representation type.
 *
 * The IoTC Client BSP NET solution may store platform specific socket
 * representations (handles, descriptors, etc) in a variable of this type in
 * create function. This typed variable will be passed to the other NET BSP's
 * functions for any and all socket operations.
 */
typedef intptr_t iotc_bsp_socket_t;

/**
 * @typedef iotc_bsp_socket_event_s
 * @brief Ties socket with its in/out state required for bsp select call
 *
 * This structure is used by the IoTC Client's internal system to track
 * socket state. The BSP implementation should query native socket states using
 * a native socket call like select() and map those states to the flags in this
 * structure.
 */
typedef struct iotc_bsp_socket_events_s {
  /** platform specific value of socket. */
  iotc_bsp_socket_t iotc_socket;
  /** 1 if socket wants to read, 0 otherwise. */
  uint8_t in_socket_want_read : 1;
  /** 1 if socket wants to write, 0 otherwise. */
  uint8_t in_socket_want_write : 1;
  /** 1 if socket wants to know about error, 0 otherwise. */
  uint8_t in_socket_want_error : 1;
  /** 1 if socket waits to get connected, 0 othwerwise. */
  uint8_t in_socket_want_connect : 1;
  /** set to 1 if socket can read, 0 otherwise. */
  uint8_t out_socket_can_read : 1;
  /** set to 1 if socket can write, 0 otherwise. */
  uint8_t out_socket_can_write : 1;
  /** set to 1 if there was an error on the socket, 0 otherwise */
  uint8_t out_socket_error : 1;
  /** set to 1 if the connection process is finished, 0 otherwise */
  uint8_t out_socket_connect_finished : 1;
} iotc_bsp_socket_events_t;

/**
 * @function
 * @brief Provides a method for the IoTC library to query socket states. These
 * states will be used by the IoTC Client Library to schedule various read and
 * write operations as the library communicates with the Google Cloud IoT Core
 * service
 *
 * The function is passed an array of socket descriptors as were returned by the
 * iotc_bsp_io_net_create_socket call. Each element in the array corresponds to
 * a specific socket, and contains an initialized iotc_bsp_socket_state_t
 * structure to be filled out by this BSP implementation.
 *
 * The BSP function should invoke functions in the native socket library to
 * query the state of the sockets and fill in the corresponding fields in
 * iotc_bsp_socket_state_t for that corresponding descriptor.
 *
 * @param [in] socket_events_array an array of sockets and sockets' events
 * @param [in] socket_events_array_size size of the socket_events_array
 * @param [in] timeout used for passive waiting function must not wait longer
 * than the given timeout ( in seconds )
 *
 * @return
 * - IOTC_BSP_IO_NET_STATE_OK - if select call updated any socket event.
 * - IOTC_BSP_IO_NET_STATE_TIMEOUT - if select call has encountered timeout.
 * - IOTC_BSP_IO_NET_STATE_ERROR - if select call finished with error.
 */
iotc_bsp_io_net_state_t iotc_bsp_io_net_select(
    iotc_bsp_socket_events_t* socket_events_array,
    size_t socket_events_array_size, long timeout_sec /* in seconds */);

/**
 * @function
 * @brief Creates the non-blocking socket.
 *
 * Creates the platform specific non-blocking socket and stores in it in the
 * function parameter iotc_socket_nonblocking.
 *
 * @param [out] iotc_socket_nonblocking the platform specific socket
 * representation should be stored in this variable. This value will be passed
 * along further BSP function calls.
 * @return
 * - IOTC_BSP_IO_NET_STATE_OK - if the socket was created successfully.
 * - IOTC_BSP_IO_NET_STATE_ERROR - otherwise.
 */
iotc_bsp_io_net_state_t iotc_bsp_io_net_create_socket(
    iotc_bsp_socket_t* iotc_socket_nonblocking, iotc_bsp_protocol_t iotc_protocol);

/**
 * @function
 * @brief Connects the socket to an endpoint defined by the host and port
 * parameters.
 *
 * @param [in] iotc_socket_nonblocking the socket which needs to be connected
 *                                   (generated by iotc_bsp_io_net_create_socket
 * function.)
 * @param [in] host Null terminated IP or Fully Qualified Domain Name (FQDN)
 * of the host to connect to.
 * @param [in] port the port number of the endpoint.
 * @return
 * - IOTC_BSP_IO_NET_STATE_OK - if successfully connected.
 * - IOTC_BSP_IO_NET_STATE_ERROR - otherwise.
 */
iotc_bsp_io_net_state_t iotc_bsp_io_net_connect(
    iotc_bsp_socket_t* iotc_socket_nonblocking, const char* host,
    uint16_t port, iotc_bsp_protocol_t iotc_protocol);

/**
 * @function
 * @brief Reports to the IoTC Client whether the provided socket is connected.
 *
 * This is called after the 'connect' function. If the socket is connected, the
 * IoTC will start to use read/write to handshake the TLS connection.  If the
 * return value is otherwise, then a failed connection will be reported to the
 * client Application via its IoTC Connection Callback.
 *
 * The two separate functions (connect and connection_check) may be confusing.
 * The asynchronous property of the IoTC Client requires the separation of
 * these functions. The actual TCP/IP socket connection is performed in the
 * time between these two function calls, when select will be called on POSIX
 * platforms, and networking tick operations invoked on No-OS devcies to
 * complete the connection process.
 *
 * @param [in] iotc_socket_nonblocking the socket on which to perform the
 * connection check.
 * @param [in] host Null terminated IP or FQDN of the host to connect to.
 * @param [in] port the port number of the endpoint.
 * @return
 * - IOTC_BSP_IO_NET_STATE_OK - if socket is successfully connected.
 * - IOTC_BSP_IO_NET_STATE_ERROR - otherwise.
 */
iotc_bsp_io_net_state_t iotc_bsp_io_net_connection_check(
    iotc_bsp_socket_t iotc_socket_nonblocking, const char* host, uint16_t port);

/**
 * @function
 * @brief Sends data on the socket.
 *
 * The IoTC Client calls this function if there is data to be sent. Just like
 * the other functions in this BSP, this send operation shall not block.
 * This function should write as many bytes in a chunk as possible.
 *
 * Keep in mind that as long as this function blocks, no other event
 * or request will be fulfilled by the IoTC Client.
 *
 * The out parameter and return value of the function may signify a unfinished
 * state of the send operation. If this occurs, then this same function
 * will be called again later with the remaining data. A delay between these
 * two calls should afford the system time to complete the transmission
 * of the first chunk so that the second send event may be processed.
 *
 * @param [in] iotc_socket_nonblocking data is sent on this socket.
 * @param [out] out_written_count upon return this should contain the number of
 * sent bytes. Negative values will cause the IoTC client to close the
 * connection.
 * @param [in] buf the data to send.
 * @param [in] count number of bytes to send from the buffer. This is the size
 * of the buffer.
 * @return
 * - IOTC_BSP_IO_NET_STATE_OK - if the whole buffer is sent. i.e. #count
 *                            bytes are sent. *out_written_count == count.
 * - IOTC_BSP_IO_NET_STATE_BUSY - if only a partial send of the buffer was
 * completed.  This return value tells the system to call this function again
 * with remaining data.
 * *out_written_count < count. Outgoing parameter number of written bytes. This
 * must be set properly!
 * - IOTC_BSP_IO_NET_STATE_ERROR - an error occurred during the write
 * operation.
 */
iotc_bsp_io_net_state_t iotc_bsp_io_net_write(
    iotc_bsp_socket_t iotc_socket_nonblocking, int* out_written_count,
    const uint8_t* buf, size_t count);

/**
 * @function
 * @brief Reads data from the socket.
 *
 * IoTC Client calls this function if data is expected to arrive on the socket.
 * This function shall not block. It is up to the implementation to determine
 * how many bytes to read at once without blocking the non blocking operation
 * flow for too long.  If the IoTC Client requires more data this function will
 * be called again.
 *
 * @param [in] iotc_socket_nonblocking data is read from this socket.
 * @param [out] out_read_count upon return this shall contain the number of
 * read bytes.
 * @param [out] buf upon return this buffer shall contain the read bytes.
 * @param [in] count available capacity of the buffer. A maximum this number of
 * bytes can be stored in the buffer.
 * @return
 * - IOTC_BSP_IO_NET_STATE_OK - some bytes were read from the socket, number of
 * bytes should be set properly. 0 < *out_read_count.
 * - IOTC_BSP_IO_NET_STATE_BUSY - if no data is available on the socket
 * currently. This will cause the IoTC Client to revisit this function later on.
 * *out_read_count == 0.
 * - IOTC_BSP_IO_NET_STATE_ERROR - error occurred during the read operation.
 */
iotc_bsp_io_net_state_t iotc_bsp_io_net_read(
    iotc_bsp_socket_t iotc_socket_nonblocking, int* out_read_count,
    uint8_t* buf, size_t count);

/**
 * @function
 * @brief Closes the socket.
 *
 * Platform dependent socket close implementation.
 *
 * @param [in] iotc_socket_nonblocking the socket to be closed.
 * @return
 * - IOTC_BSP_IO_NET_STATE_OK - if socket closed successfully.
 * - IOTC_BSP_IO_NET_STATE_ERROR - otherwise.
 */
iotc_bsp_io_net_state_t iotc_bsp_io_net_close_socket(
    iotc_bsp_socket_t* iotc_socket_nonblocking);

#ifdef __cplusplus
}
#endif

#endif /* __IOTC_BSP_IO_NET_H__ */
