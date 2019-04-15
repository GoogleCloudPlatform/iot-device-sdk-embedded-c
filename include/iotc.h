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

#ifndef __IOTC_H__
#define __IOTC_H__

#include <stdint.h>
#include <stdlib.h>

#include <iotc_connection_data.h>
#include <iotc_mqtt.h>
#include <iotc_time.h>
#include <iotc_types.h>

#ifdef __cplusplus
extern "C" {
#endif

/*! \file
 * @brief Securely connects client applications to Google Cloud IoT Core.
 *
 * \mainpage Google Cloud IoT Device SDK for Embedded C API
 *
 * # Welcome
 * The Device SDK API securely connects client applications to Cloud IoT Core
 * in order to publish and subscribe to messages via MQTT.
 *
 * To port the Device SDK to your device, review the Board Support Package (BSP)
 * <a href="../../bsp/html/index.html">reference</a> and
 * <a href="../../../porting_guide.md">porting guide</a>.
 *
 * # Getting started
 * All of the standard Device SDK API functions are in the <code>/include</code>
 * directory and have the 'iotc' prefix. To get started, navigate to the
 * <b>File</b> tab and open <code>iotc.h</code>.
 *
 * These documents are a complete reference for POSIX systems. Use the <a
 * href="../../bsp/html/index.html">BSP</a> to build the Device SDK on a custom,
 * non-POSIX platform.
 *
 * # Further reading
 * <ul><li>Device SDK <a href="../../bsp/html/index.html">BSP reference</a></li>
 * <li>
 * <a href="http://docs.oasis-open.org/mqtt/mqtt/v3.1.1/os/mqtt-v3.1.1-os.html">
 * MQTT v3.1.1 Specification</a></li>
 * <li>Device SDK <a href="../../../user_guide.md">user guide</a></li>
 * <li>Device SDK <a href="../../../porting_guide.md">porting guide</a></li>
 * </ul>
 */

/* -----------------------------------------------------------------------
 * MAIN LIBRARY FUNCTIONS
 * ----------------------------------------------------------------------- */

/**
 * @brief Initializes the BSP time and random number libraries.
 *
 * Applications should call this first when starting a new runtime.
 *
 * @retval IOTC_STATE_OK The libraries are successfully initialized.
 */
extern iotc_state_t iotc_initialize();

/**
 * @brief Shuts down the Device SDK and frees all resources created
 * during initialization.
 *
 * Before calling this function, clean up each context with
 *     <code>iotc_delete_context()</code>.
 *
 * @see iotc_initialize
 * @see iotc_create_context
 * @see iotc_delete_context
 * @see iotc_shutdown_connection
 *
 * @retval IOTC_STATE_OK The Device SDK is successfully shut down.
 */
extern iotc_state_t iotc_shutdown();

/**
 * @brief Creates a connection context.
 *
 * Before calling this function, invoke <code>libiotc</code> via
 * <code>iotc_initialize()</code>.
 *
 * @see iotc_initialize
 * @see iotc_delete_context
 *
 * @return If the function fails, it returns the
 *     <a href="~/include/iotc_error.h">error code</a> multiplied by -1.
 */
extern iotc_context_handle_t iotc_create_context();

/**
 * @brief Frees the provided context.
 *
 * After the client application disconnects from Cloud IoT Core, invoke
 * this function to free memory. Do not delete a context in the connection
 * callback.
 * 
 * On POSIX systems, the client application must delete the context after
 * <code>iotc_process_blocking()</code> returns.
 *
 * On non-POSIX systems, the application must delete the context on the
 * event loop tick after the disconnection event (not in the disconnection
 * callback itself).
 *
 * Note: you may reuse disconnected contexts to reconnect to Cloud IoT Core;
 * you don't need to destroy and recreate contexts.
 *
 * @param [in] context The context handle to free.
 *
 * @see iotc_create_context
 *
 * @retval IOTC_STATE_OK The context is successfully freed.
 * @retval IOTC_INVALID_PARAMETER The provided context handle is invalid.
 */
extern iotc_state_t iotc_delete_context(iotc_context_handle_t context_handle);

/**
 * @brief Determines if a context is connected to Cloud IoT Core.
 *
 * @param [in] context The handle for which to determine the connection.
 *
 * @see iotc_create_context
 * @see iotc_connect
 * @see iotc_connect_to
 *
 * @retval 1 The context is currently connected.
 * @retval 0 The context is invalid or the connection is currently any one
 *     of the following states: uninitialized, connecting, closing, or
 *     closed.
 */
extern uint8_t iotc_is_context_connected(iotc_context_handle_t context_handle);

/**
 * @brief Invokes the event processing loop.
 *
 * The Device SDK has an event queueing mechanism to faciliate connection,
 * subscription and publication requests in a non blocking, asynchronous manner.
 * This function executes the Device SDKs event engine as the main
 * application process. 
 *
 * This function will not return until <code>iotc_events_stop()</code> is invoked.
 *
 * This function only processes events on platforms with main application loops
 * that can block indefinitely. For other platforms, call
 * <code>iotc_events_process_tick()</code>.
 *
 * Note: The event engine won't process events when the Device SDK is in the
 * IOTC_EVENT_PROCESS_STOPPED state. If the Device SDK is in this state, invoke
 * <code>iotc_shutdown()</code> and then reinitialize the Device SDK to process
 * events again.
 *
 * @see iotc_events_process_tick
 * @see iotc_events_stop
 */
extern void iotc_events_process_blocking();

/**
 * @brief Invokes the event processing loop.
 *
 * This function is for RTOS or non-OS devices that must yield for standard tick
 * operations.
 *
 * Note: The event engine won't process events when the Device SDK is in the
 * IOTC_EVENT_PROCESS_STOPPED state. If the Device SDK is in this state, invoke
 * <code>iotc_shutdown()</code> and then reinitialize the Device SDK to process
 * events again.
 *
 * @see iotc_events_process_blocking
 * @see iotc_events_stop
 *
 * @retval IOTC_STATE_OK The event system is ongoing and can continue to
 *     operate.
 * @retval IOTC_EVENT_PROCESS_STOPPED The event processor stopped because the
 *     client application invoked <code>iotc_events_stop()</code> or an
 *     unrecoverable error occurred.
 */
extern iotc_state_t iotc_events_process_tick();

/**
 * @brief Shuts down the event engine.
 *
 * @see iotc_events_process_blocking
 * @see iotc_events_process_tick
 */
extern void iotc_events_stop();

/**
 * @brief Connects to Cloud IoT Core with the provided context.
 *
 * This parameters include a pointer to connection state monitor callback
 * function.
 *
 * @param [in] iotc_h A context handle created by invoking
 * <code>iotc_create_context()</code>.
 * @param [in] username The MQTT username. Cloud IoT Core ignores this
 * parameter.
 * @param [in] password The MQTT password. Cloud IoT Core requires a JWT. Create
 * a JWT with the <code>iotc_create_iotcore_jwt()</code> function.
 * @param [in] client_id The MQTT client identifier. Cloud IoT Core requires a
 *     <a
 * href="https://cloud.google.com/iot/docs/how-tos/mqtt-bridge#device_authentication">device
 * path</a>.
 * @param [in] connection_timeout The number of seconds to keep the socket
 * before CONNACK. If 0, the TCP timeout is used.
 * @param [in] keepalive_timeout The number of seconds that the MQTT service
 * keeps the socket is be kept open without hearing from the client.
 *
 * @see iotc_create_context
 * @see iotc_create_iotcore_jwt
 *
 * @retval IOTC_STATE_OK The connection request is correctly enqueued.
 */
extern iotc_state_t iotc_connect(iotc_context_handle_t iotc_h,
                                 const char* username, const char* password,
                                 const char* client_id,
                                 uint16_t connection_timeout,
                                 uint16_t keepalive_timeout,
                                 iotc_user_callback_t* client_callback);

/**
 * @brief Connects to a custom service endpoint.
 *
 * @param [in] host The address at which the client connects.
 * @param [in] port The port of the host on which to connect.
 * @param [in] iotc_h A context handle created by invoking
 * <code>iotc_create_context()</code>.
 * @param [in] username The MQTT username. Cloud IoT Core ignores this
 * parameter.
 * @param [in] password The MQTT password. Cloud IoT Core requires a JWT. Create
 * a JWT with the <code>iotc_create_iotcore_jwt()</code> function.
 * @param [in] client_id The MQTT client identifier. Cloud IoT Core requires a
 *     <a
 * href="https://cloud.google.com/iot/docs/how-tos/mqtt-bridge#device_authentication">device
 * path</a>.
 * @param [in] connection_timeout The number of seconds to keep the socket
 * before CONNACK. If 0, the TCP timeout is used.
 * @param [in] keepalive_timeout The number of seconds that the MQTT service
 * keeps the socket is be kept open without hearing from the client.
 *
 * @see iotc_connect
 */
extern iotc_state_t iotc_connect_to(iotc_context_handle_t iotc_h,
                                    const char* host, uint16_t port,
                                    const char* username, const char* password,
                                    const char* client_id,
                                    uint16_t connection_timeout,
                                    uint16_t keepalive_timeout,
                                    iotc_user_callback_t* client_callback);

/**
 * @brief Publishes a message to Cloud IoT Core on a given topic.
 *
 * Before publishing a message, connect the device to Cloud IoT Core or a
 * custom service endpoint.
 *
 * @param [in] iotc_h A context handle created by invoking iotc_create_context.
 * @param [in] topic A string based topic name that you have created for
 *     messaging via the IoT Core webservice.
 * @param [in] msg The payload of the message.
 * @param [in] qos The Quality of Service (QoS) level. Can be <code>0</code>,
 *     <code>1</code>, or <code>2</code>. Cloud IoT Core doesn't support QoS
 *     level 2. For more information, see the MQTT specification or
 *     <code>iotc_mqtt_qos_e()</code> in <code>iotc_mqtt_message.h</code>.
 * @param [in] callback (Optional) The callback function. Invoked after a
 * message is successfully or unsuccessfully delivered.
 * @param [in] user_data (Optional) Abstract data passed to the callback function.
 *
 * @see iotc_connect
 * @see iotc_connect_to
 * @see iotc_create_context
 * @see iotc_publish_data
 *
 * @retval IOTC_STATE_OK The publication request is correctly formatted and
 *     queued for publication.
 * @retval IOTC_OUT_OF_MEMORY The platform doesn't have enough memory
 *     to fulfull the request.
 * @retval IOTC_BACKOFF_TERMINAL Backoff applied.
 * @retval IOTC_INTERNAL_ERROR Something went wrong.
 */
extern iotc_state_t iotc_publish(iotc_context_handle_t iotc_h,
                                 const char* topic, const char* msg,
                                 const iotc_mqtt_qos_t qos,
                                 iotc_user_callback_t* callback,
                                 void* user_data);

/**
 * @brief Publishes binary data to Cloud IoT Core on a given topic.
 *
 * Before publishing a message, connect the device to Cloud IoT Core or a
 * custom service endpoint.
 *
 * <code>iotc_publish_data()</code> accepts a pointer and data length, whereas
 * <code>iotc_publish()</code> accepts a null-terminated string.
 *
 * @param [in] data The message payload.
 * @param [in] data_len The size of the message, in bytes.
 *
 * @see iotc_create_context
 * @see iotc_publish
 * @see iotc_publish_data
 *
 * @retval IOTC_STATE_OK The publication request is correctly formatted.
 * @retval IOTC_OUT_OF_MEMORY The platform doesn't have enough memory
 *     to fulfull the request.
 * @retval IOTC_INTERNAL_ERROR Something went wrong.
 */
extern iotc_state_t iotc_publish_data(iotc_context_handle_t iotc_h,
                                      const char* topic, const uint8_t* data,
                                      size_t data_len,
                                      const iotc_mqtt_qos_t qos,
                                      iotc_user_callback_t* callback,
                                      void* user_data);

/**
 * @brief Subscribes to an MQTT topic via Cloud IoT Core.
 *
 * After subscribing to an MQTT topic, incoming messages are delivered to the
 * callback function. If the client application can't subscribe to the MQTT
 * topic, the callback function will return information about the error.
 *
 * @param [in] iotc_h A context handle created by invoking iotc_create_context.
 * @param [in] topic A string with the name of an MQTT topic.
 * @param [in] qos The Quality of Service (QoS) level. Can be <code>0</code>,
 *     <code>1</code>, or <code>2</code>. Cloud IoT Core doesn't support QoS level 2.
 *     For more information, see the MQTT specification or
 *     <code>iotc_mqtt_qos_e</code> in <code>iotc_mqtt_message.h</code>.
 * @param [in] callback The callback function. Invoked after a message is
 *     successfully or unsuccessfully received.
 * @param [in] user_data (Optional) A pointer which will be passed to the
 *callback function's user_data parameter.
 *
 * @see iotc_create_context
 * @see iotc_connect
 * @see iotc_publish
 * @see iotc_publish_data
 *
 * @retval IOTC_STATE_OK The subscription request is correctly formatted.
 * @retval IOTC_OUT_OF_MEMORY The platform doesn't have enough memory
 *     to fulfull the request.
 * @retval IOTC_INTERNAL_ERROR An unrecoverable error occurred.
 */
extern iotc_state_t iotc_subscribe(iotc_context_handle_t iotc_h,
                                   const char* topic, const iotc_mqtt_qos_t qos,
                                   iotc_user_subscription_callback_t* callback,
                                   void* user_data);

/**
 * @brief Asynchronously disconnects from Cloud IoT Core.
 *
 * After disconnecting, the Device SDK passes the disconnection status code to the
 * <code>iotc_connect()</code>.
 *
 * @param [in] iotc_h A context handle created by invoking iotc_create_context.
 *
 * @retval IOTC_STATE_OK The disconnection request is correctly enqueued.
 * @retval IOTC_SOCKET_NO_ACTIVE_CONNECTION_ERROR The device isn't connected to
 *     Cloud IoT Core.
 *
 * @see iotc_create_context
 * @see iotc_connect
 */
extern iotc_state_t iotc_shutdown_connection(iotc_context_handle_t iotc_h);

/**
 * @brief Executes a function after a number of elapsed seconds.
 *
 * Specify the function to execute in the <a href="iotc__types_8h.html">
 * <code>iotc_user_task_callback_t*</code></a> parameter. The Device SDK event
 * system invokes this function after the specified number of seconds.
 *
 * @param [in] iotc_h A context handle created by invoking
 *     <code>iotc_create_context</code>.
 * @param [in] iotc_user_task_callback_t* A function invoked after a specified
 *     amount of time. This function has the following signature: <code>
 *     typedef void() iotc_user_task_callback_t(const iotc_context_handle_t
 *     context_handle, const iotc_timed_task_handle_t timed_task_handle, void
 *     *user_data)</code>.
 * @param [in] seconds_from_now The number of seconds to wait before invoking
 *     the <code>iotc_user_task_callback_t*</code> function.
 * @param [in] repeats_forever The callback is repeatedly executed at
 *     seconds_from_now intervals. If the repeats_forever parameter is set to
 *     <code>0</code>, the callback is executed only once.
 * @param [in] data (Optional) A pointer which will be passed to the callback
 *     function's user_data parameter.
 *
 * @see iotc_create_context
 * @see iotc_cancel_timed_task
 *
 * @retval iotc_time_task_handle_t A unique identifier for the scheduled task.
 *     If the function encounters an error, <code>iotc_time_task_handle_t
 *     </code> is the <a
 *     href="iotc__error_8h.html#a31af29ceaf88c8f978117b3be5545932">error code
 *     </a> multiplied by -1.
 */
iotc_timed_task_handle_t iotc_schedule_timed_task(
    iotc_context_handle_t iotc_h, iotc_user_task_callback_t* callback,
    const iotc_time_t seconds_from_now, const uint8_t repeats_forever,
    void* data);

/**
 * @brief Cancels a timed task by removing it from the internal event system.
 *
 * @param [in] timed_task_handle The handle
 *     <code>iotc_schedule_timed_task</code> returned.
 *
 * @see iotc_create_context
 * @see iotc_schedule_timed_task
 */

void iotc_cancel_timed_task(iotc_timed_task_handle_t timed_task_handle);

/*-----------------------------------------------------------------------
 * HELPER FUNCTIONS
 * ---------------------------------------------------------------------- */

/**
 * @brief Sets the connection timeout.
 *
 * Only new connections observe this timeout. Note that the Device SDK
 * periodically creates network traffic per MQTT specifications.
 *
 * @param [in] timeout The number of seconds sockets remain open when data
 *     isn't passing through them. This parameter is passed to the 
 *     implementing networking layer during socket initialization to
 *     automatically keep connections open.
 *
 * @see iotc_connect
 * @see iotc_connect_to
 * @see iotc_get_network_timeout
 **/
extern void iotc_set_network_timeout(uint32_t timeout);

/**
 * @brief Queries the current network timeout value.
 *
 * @see iotc_set_network_timeout
 **/
extern uint32_t iotc_get_network_timeout(void);

/**
 * @brief Sets the maximum heap memory the Device SDK can use.
 *
 * This function is part of the Device SDK <a
 * href="../../../user_guide.md#memory-limiter">memory limiter</a>.
 *
 * @param [in] max_bytes The maximum amount of heap memory, in bytes, that
 *     the Device SDK can use during standard execution.
 *
 * @retval IOTC_STATE_OK The new memory limit has been succesfully set.
 * @retval IOTC_NOT_SUPPORTED The memory limiter module isn't compiled into the
 *     Device SDK.
 * @retval IOTC_OUT_OF_MEMORY The new memory limit is too small to support
 *     the current heapspace footprint.
 */
iotc_state_t iotc_set_maximum_heap_usage(const size_t max_bytes);

/**
 * @brief Queries the Device SDK's current heap usage.
 *
 * This function is part of the Device SDK <a
 * href="../../../user_guide.md#memory-limiter">memory limiter</a>.
 *
 * @retval IOTC_STATE_OK The current head usage is successfully queried.
 * @retval IOTC_NOT_SUPPORTED The memory limiter module isn't installed.
 * @retval IOTC_INVALID_PARAMETER The parameter is provided but defined as NULL.
 */
iotc_state_t iotc_get_heap_usage(size_t* const heap_usage);

/**
 * @brief The Device SDK major version number.
 **/
extern const uint16_t iotc_major;

/**
 * @brief The Device SDK minor version number.
 **/
extern const uint16_t iotc_minor;

/**
 * @brief The Device SDK revision number.
 **/
extern const uint16_t iotc_revision;

/**
 * @brief The Device SDK "major.minor.revision" version string.
 **/
extern const char iotc_cilent_version_str[];

#ifdef IOTC_EXPOSE_FS
#include "iotc_fs_api.h"

/**
 * @brief Allows the Device SDK to use custom a filesystem.
 */
iotc_state_t iotc_set_fs_functions(const iotc_fs_functions_t fs_functions);

#endif /* IOTC_EXPOSE_FS */

#ifdef __cplusplus
}
#endif

#endif /* __IOTC_H__ */
