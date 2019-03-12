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
 * @brief Securely connect client applications to Google Cloud IoT Core.
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
 * directory and have the 'iotc' prefix. To get started, navigate to the <b>File</b> tab 
 * and open <code>iotc.h</code>.
 *
 * These documents are a complete reference for POSIX systems. Use the <a href="../../bsp/html/index.html">BSP</a> 
 * to build the Device SDK on a custom, non-POSIX platform.
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
 * @brief Initialize libraries. Required operation.
 * 
 * Called first when starting a new runtime.
 *
 * @retval IOTC_STATE_OK Status OK.
 */
extern iotc_state_t iotc_initialize();

/**
 * @brief Free all resources created during initialization.
 * 
 * Called last when shutting down a client application.
 *
 * <b>Note</b>: clean up contexts individually with <code>iotc_delete_context()</code>.
 *
 * @see iotc_initialize
 * @see iotc_create_context
 * @see iotc_delete_context
 * @see iotc_shutdown_connection
 *
 * @retval IOTC_STATE_OK Status OK.
 * @retval IOTC_FAILED_INITIALIZATION An urecoverable error occurred.
 */
extern iotc_state_t iotc_shutdown();

/**
 * @brief Create a connection context.
 * 
 * Invoke after initalizing <code>libiotc</code>.
 *
 * @see iotc_initialize
 * @see iotc_delete_context
 *
 * @retval nonnegative_number Valid context handle is created.
 * @retval negative_number If the function breaks, it returns the 
 *     <a href="~/include/iotc_error.h">error code</a> multiplied by -1.
 */
extern iotc_context_handle_t iotc_create_context();

/**
 * @brief Frees the provided context.
 * 
 * Invoked only to free memory after the client application
 * disconnects from Cloud IoT Core.
 *
 * You may reuse disconnected contexts to reconnect to Cloud IoT Core; 
 * you don't need to destroy and recreate contexts.
 *
 * <b>Note</b>: This function deletes contexts only after the client application is
 * disconnected. On POSIX systems, this function deletes the context after <code>iotc_events_stop()</code>
 * returns <code>iotc_process_blocking()</code>. On non-POSIX systems, this function deletes
 * the context on the event loop tick Device SDK disconnect callback is invoked.
 *
 * @param [in] context Context handle to free.
 *
 * @see iotc_create_context
 *
 * @retval IOTC_STATE_OK Status OK.
 * @retval IOTC_INVALID_PARAMETER The provided context handle is invalid.
 */
extern iotc_state_t iotc_delete_context(iotc_context_handle_t context_handle);

/**
 * @brief Determine if a context is connected to Cloud IoT Core.
 *
 * @param [in] context Handle of which to determine the connection.
 *
 * @see iotc_create_context
 * @see iotc_connect
 * @see iotc_connect_to
 *
 * @retval 1 The context is currently connected
 * @retval 0 The context is invalid or the connection is currently any one
 *     of the other following states: unitialized, connecting, closing, or closed.
 */
extern uint8_t iotc_is_context_connected(iotc_context_handle_t context_handle);

/**
 * @brief Invoke the event processing loop.
 * 
 * This function executes the Device SDK as the main application process.
 *
 * To execute on a platform that cannot block indefinitely, use the
 * <code>iotc_events_process_tick()</code> function.
 *
 * <b>Note</b>: The event engine won't process events when the client application
 * is in the IOTC_EVENT_PROCESS_STOPPED state. Destroy and reinitialize the 
 * Device SDK to process events again.
 *
 * @see iotc_events_process_tick
 * @see iotc_events_stop
 */
extern void iotc_events_process_blocking();

/**
 * @brief Invoke the event processing loop.
 * 
 * This function proccesses pending tasks and returns control to the client
 * application.
 *
 * Recommended for RTOS or non-OS devices that must yield standard tick
 * operations. The event engine won't process events when the Device SDK
 * is in the IOTC_EVENT_PROCESS_STOPPED state. Destroy and reinitialize the 
 * Device SDK to process events again.
 *
 * <b>Warning</b>: This function blocks on standard UNIX devices like Linux and
 * macOS.
 *
 * @see iotc_events_process_blocking
 * @see iotc_events_stop
 *
 * @retval IOTC_STATE_OK Status OK. The event system is ongoing and
 *     can continue to operate.
 * @retval IOTC_EVENT_PROCESS_STOPPED The event processor stopped because the
 *     client application invoked <code>iotc_events_stop()</code> or an
 *     unrecoverable error occurred.
 */
extern iotc_state_t iotc_events_process_tick();

/**
 * @brief Exit blocking event loops.
 *
 * @see iotc_events_process_blocking
 * @see iotc_events_process_tick
 */
extern void iotc_events_stop();

/**
 * @brief Connect to Cloud IoT Core with the provided context.
 *
 * This function includes a callback. The callback method is a connection state
 * monitor callback. The callback is invoked after any connection attempt.
 *
 * <code>iotc_user_callback_t()</code> defines the connection state monitor callback. 
 * <code>iotc_user_callback_t()</code> has the following signature:
 *    void foo( iotc_context_handle_t in_context_handle
 *                  , void* data
 *                  , iotc_state_t state )
 *
 * The following <code>iotc_user_callback_t()</code> parameters determine the connection 
 * state monitor callback:
 *   - in_context_handle: the context handle provided to <code>iotc_connect()</code>.
 *   - data: a multifunctional structure in the client application. See the <a href="../../../porting_guide.md">porting guide</a>
 *     or <code>examples/</code> for more information.
 *   - state: IOTC_STATE_OK if the connection succeeds. See <code><a href="~/include/iotc_error.h">iotc_error.h</a></code>
 *     for more error codes.
 *
 * @param [in] iotc_h A context handle created by invoking <code>iotc_create_context()</code>.
 * @param [in] username The MQTT username. Cloud IoT Core ignores this parameter.
 * @param [in] password The MQTT password. Cloud IoT Core requires a JWT. Create a
 *     JWT with the <code>iotc_create_iotcore_jwt()</code> function.
 * @param [in] client_id The MQTT client identifier. Cloud IoT Core requires a
 *     <a href="https://cloud.google.com/iot/docs/how-tos/mqtt-bridge#device_authentication">device path</a>.
 * @param [in] connection_timeout The number of seconds to keep the socket before
 *     CONNACK. If 0, the TCP timeout is used.
 * @param [in] keepalive_timeout The number of seconds that the MQTT service keeps the socket is be kept open without hearing from the client.
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
 * @brief Connect to a custom service endpoint via MQTT.
 *
 * @param [in] host Address at which the client connects.
 * @param [in] port Port on which the client connects to host.
 *
 * This function's behavior and other parameters are identical to
 * the <code>iotc_connect()</code> function.
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

/*
 * @brief Publish a message to Cloud IoT Core on a given topic.
 *
 * To publish a message, the device must already be connected to Cloud IoT Core. Call <code>iotc_connect()</code>
 * or <code>iot_connect_to()</code> to connect to Cloud IoT Core.
 *
 * The callback function has the following signature:
 *  void foo( iotc_context_handle_t in_context_handle
 *                , void* user_data
 *                , iotc_state_t state )
 *
 * The callback parameters are:
 *   - in_context_handle: context handle provided to <code>iotc_publish()</code>.
 *   - user_data: the value that the client application passed to the <code>iotc_publish()</code>
 *     user_data: (Optional) Use this parameter to attach descriptions that identify messages.
 *   - state: IOTC_STATE_OK if the connection succeeded. For information on other error
 *     codes, see the <a href="../../../porting_guide.md">porting guide</a>
 *     or <code>examples/</code> directory.
 *
 *
 * @param [in] iotc_h A context handle created by invoking iotc_create_context.
 * @param [in] topic A string based topic name that you have created for
 *     messaging via the IoT Core webservice.
 * @param [in] msg The payload of the message.
 * @param [in] qos The Quality of Service (QoS) level. Can be <code>0</code>, <code>1</code>, or <code>2</code>.
 *     Cloud IoT Core doesn't support QoS level 2. For more information, see the
 *     MQTT specification or <code>iotc_mqtt_qos_e()</code> in <code>iotc_mqtt_message.h</code>.
 * @param [in] callback (Optional) The callback function. Invoked after a message is
 *     successfully or unsuccessfully delivered.
 * @param [in] user_data (Optional) Abstract data passed to a callback function.
 *
 * @see iotc_create_context
 * @see iotc_publish_data
 *
 * @retval IOTC_STATE_OK The publication request is correctly formatted and
 *     queued for publication.
 * @retval IOTC_OUT_OF_MEMORY The platform doesn't have enough memory
 *     to fulfull the request.
 * @retval IOTC_INTERNAL_ERROR An unrecoverable error occurred.
 * @retval IOTC_BACKOFF_TERMINAL Backoff applied.
 */
extern iotc_state_t iotc_publish(iotc_context_handle_t iotc_h,
                                 const char* topic, const char* msg,
                                 const iotc_mqtt_qos_t qos,
                                 iotc_user_callback_t* callback,
                                 void* user_data);

/**
 * @brief Publish binary data to Cloud IoT Core on a given topic.
 *
 * To publish a message, the device must already be connected to Cloud IoT Core. Call <code>iotc_connect</code> 
 * or <code>iot_connect_to()</code> to connect to Cloud IoT Core.
 *
 * Unlike <code>iotc_publish()</code>, <code>iotc_publish_data()</code> accepts a
 * pointer and data length instead of a null-terminated string.
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
 * @retval IOTC_INTERNAL_ERROR An unrecoverable error occurred.
 */
extern iotc_state_t iotc_publish_data(iotc_context_handle_t iotc_h,
                                      const char* topic, const uint8_t* data,
                                      size_t data_len,
                                      const iotc_mqtt_qos_t qos,
                                      iotc_user_callback_t* callback,
                                      void* user_data);

/**
 * @brief Subscribe to an MQTT topic via Cloud IoT Core.
 * 
 * This function formats and submits subscription requests. After subscribing to
 * an MQTT topic, incoming messages are delivered to the callback function. If
 * the client application can't subscribe to the MQTT topic, the callback function
 * returns error information.
 *
 * <code>iotc_user_callback_t</code> defines the subscription callback function.
 * <code>iotc_user_callback_t</code> has the following signature.
 *    void foo( iotc_context_handle_t in_context_handle,
 *          iotc_sub_call_type_t call_type,
 *          const iotc_sub_call_params_t* const params,
 *          iotc_state_t state,
 *          void* user_data )
 *
 * The <code>iotc_user_callback_t</code> parameters are:
 *   - context_handle: the context handle provided to <code>iotc_subscribe</code>.
 *   - call_type: the type of callback invocation. Each callback invocation may be
 *   related to a subscription confirmation or a new message.
 *   - params: the details about subscription confirmations or new messages. The data
 *   in this structure varies by call_type value.
 *   - state: IOTC_STATE_OK if the connection succeeded. For information on other error
 *     codes, see the <a href="../../../porting_guide.md">porting guide</a>
 *     or <code>examples/</code> directory.
 *   - user_data: the pointer from the corresponding <code>iotc_subscribe</code> user parameter.
 *
 * @param [in] iotc_h A context handle created by invoking iotc_create_context.
 * @param [in] topic A string with the name of an MQTT topic.
 * @param [in] qos The Quality of Service (QoS) level. Can be <code>0</code>, <code>1</code>, or <code>2</code>.
 *     Cloud IoT Core doesn't support QoS level 2. For more information, see the
 *     MQTT specification or <code>iotc_mqtt_qos_e</code> in <code>iotc_mqtt_message.h</code>.
 * @param [in] callback The callback function. Invoked after a message is
 *     successfully or unsuccessfully received.
 * @param [in] user A pointer returned by the callback function.
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
 * @brief Asynchronously disconnect from Cloud IoT Core.
 *
 * After disconnecting, the callback defined by <code>iotc_connect</code> is
 * invoked.
 *
 * @param [in] iotc_h A context handle created by invoking iotc_create_context.
 *
 * @retval IOTC_SOCKET_NO_ACTIVE_CONNECTION_ERROR If there is no connection for
 * this context.
 * @retval IOTC_STATE_OK If the disconnection request is enqueued.
 *
 * @see iotc_create_context
 * @see iotc_connect
 */
extern iotc_state_t iotc_shutdown_connection(iotc_context_handle_t iotc_h);

/**
 * @brief Execute a function after a number of elapsed seconds.
 *
 * This function adds a task to the internal event system. The callback method executes
 * after a specified number of seconds.
 *
 * The callback function has the following siguature:
 *  typedef void ( iotc_user_task_callback_t ) ( const iotc_context_handle_t
 *    context_handle, const iotc_timed_task_handle_t, timed_task_handle, void*
 *    user_data );
 *
 * The callback parameters are:
 *   - in_context_handle the context handle provided to
 * <code>iotc_schedule_timed_task</code>.
 *   - timed_task_handle the handle <code>iotc_schedule_timed_task</code> returns.
 *   - user_data the data provided to <code>iotc_schedule_timed_task</code>.
 *
 * @param [in] iotc_h A context handle created by invoking <code>iotc_create_context</code>.
 * @param [in] iotc_user_task_callback_t* A function invoked after a specified
 *    amount of time.
 * @param [in] seconds_from_now The number of seconds to wait before invoking
 *     the <code>iotc_user_task_callback_t*</code> function.
 * @param [in] repeats_forever The callback is repeatedly executed at
 *     seconds_from_now intervals. If the repeats_forever parameter is set to 
 *     <code>0</code>, the callback is executed only once.
 *
 * @see iotc_create_context
 * @see iotc_cancel_timed_task
 *
 * @retval iotc_time_task_handle_t The task is successfully scheduled. 
 * iotc_time_task_handle_t A unique identifier for the scheduled task. If the
 *     function encounters an error, iotc_time_task_handle_t is the <a href="~/include/iotc_error.h">error code</a> 
 *     multiplied by -1.
 */
iotc_timed_task_handle_t iotc_schedule_timed_task(
    iotc_context_handle_t iotc_h, iotc_user_task_callback_t* callback,
    const iotc_time_t seconds_from_now, const uint8_t repeats_forever,
    void* data);

/**
 * @brief Cancel a timed task by removing it from the internal event system.
 *
 * @param [in] timed_task_handle The handle <code>iotc_schedule_timed_task</code>
 *     returns.
 *
 * @see iotc_create_context
 * @see iotc_schedule_timed_task
 */

void iotc_cancel_timed_task(iotc_timed_task_handle_t timed_task_handle);

/*-----------------------------------------------------------------------
 * HELPER FUNCTIONS
 * ---------------------------------------------------------------------- */

/**
 * @brief Set connection timeout.
 * 
 * This function specifies the number of seconds sockets remain open when data
 * isn't passing through them. Note that the Device SDK periodically creates 
 * network traffic per MQTT specifications. 

 * Only new connections observe this timeout.
 *
 * @param [in] timeout The number of seconds before timing out. This parameter
 *     is passed to the implementing networking layer during socket
 *     initialization to automatically keep connections open.
 *
 * @see iotc_connect
 * @see iotc_connect_to
 * @see iotc_get_network_timeout
 **/
extern void iotc_set_network_timeout(uint32_t timeout);

/**
 * @brief Query the current network timeout value.
 *
 * @see iotc_set_network_timeout
 **/
extern uint32_t iotc_get_network_timeout(void);

/**
 * @brief Set the maximum heap memory the Device SDK can use.
 * 
 * This function restricts the heap memory that the Device SDK can use. The 
 * specified value is the maximum heapspace, in bytes, the Device SDK
 * can use during standard execution.
 * 
 * This function is part of the Device SDK <a href="../../../user_guide.md#memory-limiter">memory limiter</a>.
 *
 * @param [in] max_bytes The maximum amount of heap memory, in bytes, that
 *     the Device SDK can use.
 *
 * @retval IOTC_NOT_SUPPORTED The memory limiter module isn't compiled into the Device SDK.
 * @retval IOTC_OUT_OF_MEMORY The new memory limit is too small to support
 *     the current heapspace footprint.
 * @retval IOTC_STATE_OK The new memory limit is the same as the current memory
 *     limit.
 */
iotc_state_t iotc_set_maximum_heap_usage(const size_t max_bytes);

/**
 * @brief  Query the client application's current heap usage.
 * 
 * This function determines the bytes of heap memory that the Device SDK is
 * using. This function is part of the Device SDK <a href="../../../user_guide.md#memory-limiter">memory limiter</a>.
 *
 * @retval IOTC_NOT_SUPPORTED The memory limiter module isn't installed.
 * @retval IOTC_INVALID_PARAMETER The parameter is provided but defined as NULL.
 * @retval IOTC_STATE_OK The parameter is the same as the current heap usage.
 */
iotc_state_t iotc_get_heap_usage(size_t* const heap_usage);

/**
 * @brief Device SDK major version number.
 **/
extern const uint16_t iotc_major;

/**
 * @brief Device SDK minor version number.
 **/
extern const uint16_t iotc_minor;

/**
 * @brief Device SDK revision number.
 **/
extern const uint16_t iotc_revision;

/**
 * @brief Device SDK "major.minor.revision" version string.
 **/
extern const char iotc_cilent_version_str[];

#ifdef IOTC_EXPOSE_FS
#include "iotc_fs_api.h"

/**
 * @brief Permit <code>libiotc</code> to use custom a filesystem.
 */
iotc_state_t iotc_set_fs_functions(const iotc_fs_functions_t fs_functions);

#endif /* IOTC_EXPOSE_FS */

#ifdef __cplusplus
}
#endif

#endif /* __IOTC_H__ */
