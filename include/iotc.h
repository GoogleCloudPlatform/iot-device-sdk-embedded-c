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
 * <ul><li><a href="../../bsp/html/index.html">
 * Device SDK BSP reference</a></li>
 * <li>
 * <a href="http://docs.oasis-open.org/mqtt/mqtt/v3.1.1/os/mqtt-v3.1.1-os.html">
 * MQTT v3.1.1 Specification</a></li>
 * <li> Device SDK <a href="../../../user_guide.md">user guide</a></li>
 * <li> Device SDK <a href="../../../porting_guide.md">porting guide</a></li>
 * </ul>
 */

/* -----------------------------------------------------------------------
 * MAIN LIBRARY FUNCTIONS
 * ----------------------------------------------------------------------- */

/**
 * @brief    Start a client application. Required operation.
 * @detailed Call first when starting a new runtime.
 *
 * @retval IOTC_STATE_OK if status OK.
 */
extern iotc_state_t iotc_initialize();

/**
 * @brief    Shut down client application and clean up internal memory.
 * @detailed Call last when shutting down a client application. Frees all
 * resources created during initialization.
 *
 * Note: clean up contexts individually with <code>iotc_delete_context()</code>
 *
 * @see iotc_initialize
 * @see iotc_create_context
 * @see iotc_delete_context
 * @see iotc_shutdown_connection
 *
 * @retval IOTC_STATE_OK if status OK.
 * @retval IOTC_FAILED_INITIALIZATION if an urecoverable error occurs.
 */
extern iotc_state_t iotc_shutdown();

/**
 * @brief    Create a context for subscriptions and publications.
 * @detailed Create a context that can be passed to connect, subscribe, 
 * and publish functions. Invoke after initalizing <code>libiotc</code>.
 *
 * @see iotc_initialize
 * @see iotc_delete_context
 *
 * @retval nonnegative_number if valid context handle is created.
 * @retval negative_number if the function breaks, it returns the 
 * <a href="~/include/iotc_error.h">error code</a> multiplied by -1.
 */
extern iotc_context_handle_t iotc_create_context();

/**
 * @brief    Frees the provided context.
 * @detailed Invoke only to free memory after the client application
 * disconnects from Cloud IoT Core.
 *
 * You may reuse disconnected contexts to reconnect to Cloud IoT Core; 
 * you don't need to destroy and recreate contexts.
 *
 * Note: Delete contexts only after the client application is disconnected.
 * On POSIX, delete the context after <code>iotc_events_stop()</code> returns
 * <code>iotc_process_blocking()</code>. On non-POSIX systems, delete the
 * context on the tick after the client event loop.
 *
 * @param [in] context context handle to free.
 *
 * @see iotc_create_context
 *
 * @retval IOTC_STATE_OK if status OK.
 * @retval IOTC_INVALID_PARAMETER if the provided context handle is invalid.
 */
extern iotc_state_t iotc_delete_context(iotc_context_handle_t context_handle);

/**
 * @brief Determine if a context is connected to Cloud IoT Core.
 *
 * @param [in] context handle of which to determine the connection.
 *
 * @see iotc_create_context
 * @see iotc_connect
 * @see iotc_connect_to
 *
 * @retval 1 the context is currently connected
 * @retval 0 the context is invalid or the connection is currently any one
 * of the other following states: unitialized, connecting, closing, or closed.
 */
extern uint8_t iotc_is_context_connected(iotc_context_handle_t context_handle);

/**
 * @brief    Invoke the event processing loop.
 * @detailed Execute the Device SDK as the main application process. Returns
 * after <code>iotc_events_stop()</code> is invoked.

 *
 * To execute on a platform that cannot block indefinitely, use the
 * <code>iotc_events_process_tick()</code>.
 *
 * Note: The event engine won't process events when the client application
 * is in the IOTC_EVENT_PROCESS_STOPPED state. Destroy and reinitialize the 
 * client application to process events again.
 *
 * @see iotc_events_process_tick
 * @see iotc_events_stop
 */
extern void iotc_events_process_blocking();

/**
 * @brief    Invoke the event processing loop.
 * @detailed Start processing pending tasks and return control to the 
 * client application.
 *
 * Use only on RTOS or non-OS devices that must yield standard tick
 * operations.
 *
 * Warning: This function blocks on standard UNIX devices like Linux and macOS.
 *
 *
 * IOTC_STATE_OK is returned if the event system is ongoing and
 * can continue to operate.
 *
 * IOTC_EVENT_PROCESS_STOPPED is returned if the client application invokes
 * <code>iotc_events_stop()</code> or if an unrecoverable error stops the
 * event processes.
 *
 * Note: The event engine won't process events when the client application
 * is in the IOTC_EVENT_PROCESS_STOPPED state. Destroy and reinitialize the 
 * client application to process events again.
 *
 * @see iotc_events_process_blocking
 * @see iotc_events_stop
 *
 * @retval IOTC_STATE_OK if status OK.
 * @retval IOTC_EVENT_PROCESS_STOPPED  if the event processor stopped.
 */
extern iotc_state_t iotc_events_process_tick();

/**
 * @brief    Exit event loops.
 * @detailed Exit blocking event loops or signal that an unrecoverable
 * event occurred.
 *
 * @see iotc_events_process_blocking
 * @see iotc_events_process_tick
 */
extern void iotc_events_stop();

/**
 * @brief    Connect to Cloud IoT Core with the provided context.
 * @detailed Includes a callback. The callback method is a connection state monitor
 * callback. The callback is invoked after any connection attempt.
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
 *   - data: a multifunctional structure. See the <a href="../../../porting_guide.md">porting guide</a>
 *     or <code>examples/</code> for more information.
 *   - state: IOTC_STATE_OK if the connection succeeds. See <code><a href="~/include/iotc_error.h">iotc_error.h</a></code>
 *     for more error codes.
 *
 * @param [in] iotc_h a context handle created by invoking <code>iotc_create_context()</code>.
 * @param [in] username the MQTT username. Cloud IoT Core ignores this parameter.
 * @param [in] password the MQTT password. Cloud IoT Core requires a JWT. Create a
 * JWT with the <code>iotc_create_iotcore_jwt()</code> function.
 * @param [in] client_id: the MQTT client identifier. Cloud IoT Core requires a
 * <a href="https://cloud.google.com/iot/docs/how-tos/mqtt-bridge#device_authentication">device path</a>.
 * @param [in] connection_timeout number of seconds to keep the socket before
 * CONNACK. If 0, the TCP timeout is used.
 * @param [in] keepalive_timeout number of seconds to keep an unused socket.
 *
 * @see iotc_create_context
 * @see iotc_create_iotcore_jwt
 *
 * @retval IOTC_STATE_OK  If the client application is connected to Cloud IoT Core.
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
 * @param [in] host: address at which the client connects
 * @param [in] port: port on which the client connects to host
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
 * @brief    Publish a message to Cloud IoT Core.
 * @detailed Publish a message to Cloud IoT Core on a given topic. The context
 * provides the Cloud IoT Core credentials. To publish a message, the device must
 * already be connected to Cloud IoT Core. Call <code>iotc_connect()</code> or 
 * <code>iot_connect_to()</code> to connect to Cloud IoT Core.
 *
 * The callback function has the following signature:
 *  void foo( iotc_context_handle_t in_context_handle
 *                , void* user_data
 *                , iotc_state_t state )
 *
 * The callback parameters are:
 *   - in_context_handle: context handle provided to <code>iotc_publish()</code>.
 *   - user_data: the value that the client application passed to the <code>iotc_publish()</code>
 *     user_data parameter. Use this parameter to attach descriptions that identify messages.
 *   - state: IOTC_STATE_OK if the connection succeeded. For information on other error
 *     codes, see the <a href="../../../porting_guide.md">porting guide</a>
 *     or <code>examples/</code> directory.
 *
 *
 * @param [in] iotc_h a context handle created by invoking iotc_create_context.
 * @param [in] topic a string based topic name that you have created for
 * messaging via the IoT Core webservice.
 * @param [in] msg the payload of the message.
 * @param [in] qos the Quality of Service (QoS) level. Can be <code>0</code>, <code>1</code>, or <code>2</code>.
 * Cloud IoT Core doesn't support QoS level 2. For more information, see the MQTT
 * specification or <code>iotc_mqtt_qos_e()</code> in <code>iotc_mqtt_message.h</code>.
 * @param [in] callback Optional the callback function. Invoked after a message is
 * successfully or unsuccessfully delivered.
 * @param [in] user_data Optional abstract data passed to a callback function.
 *
 * @see iotc_create_context
 * @see iotc_publish_data
 *
 * @retval IOTC_STATE_OK the publication request is formatted
 * correctly and queued for publication.
 * @retval IOTC_OUT_OF_MEMORY the platform doesn't have enough memory
 * to fulfull the request.
 * @retval IOTC_INTERNAL_ERROR an unrecoverable error occurred.
 * @retval IOTC_BACKOFF_TERMINAL backoff applied.
 */
extern iotc_state_t iotc_publish(iotc_context_handle_t iotc_h,
                                 const char* topic, const char* msg,
                                 const iotc_mqtt_qos_t qos,
                                 iotc_user_callback_t* callback,
                                 void* user_data);

/**
 * @brief    Publish binary data to Cloud IoT Core.
 * @detailed Publish binary data to Cloud IoT Core on a given topic. The context
 * provides the Cloud IoT Core credentials. To publish a message, the device must
 * already be connected to Cloud IoT Core. Call <code>iotc_connect</code> or 
 * <code>iot_connect_to()</code> to connect to Cloud IoT Core.
 *
 * Unlike <code>iotc_publish()</code>, <code>iotc_publish_data()</code> accepts a
 * pointer and data length instead of a null-terminated string.
 *
 * @param [in] data the message payload.
 * @param [in] data_len the size of the message, in bytes.
 *
 * @see iotc_create_context
 * @see iotc_publish
 * @see iotc_publish_data
 *
 * @retval IOTC_STATE_OK the publication request is correctly formatted.
 * @retval IOTC_OUT_OF_MEMORY the platform doesn't have enough memory
 * to fulfull the request.
 * @retval IOTC_INTERNAL_ERROR an unrecoverable error occurred.
 */
extern iotc_state_t iotc_publish_data(iotc_context_handle_t iotc_h,
                                      const char* topic, const uint8_t* data,
                                      size_t data_len,
                                      const iotc_mqtt_qos_t qos,
                                      iotc_user_callback_t* callback,
                                      void* user_data);

/**
 * @brief    Subscribe to an MQTT topic via Cloud IoT Core.
 * @detailed Format and submit subscription requests. After subscribing to
 * an MQTT topic, incoming messages are delivered to the callback function. If
 * the client application can't subscribe to the MQTT topic, the callback function
 * returns error information.
 *
 * <code>iotc_user_callback_t</code> defines the subscription call backback function.
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
 * @param [in] iotc_h a context handle created by invoking iotc_create_context.
 * @param [in] topic a string with the name of an MQTT topic.
 * @param [in] qos the Quality of Service (QoS) level. Can be <code>0</code>, <code>1</code>, or <code>2</code>.
 * Cloud IoT Core doesn't support QoS level 2. For more information, see the MQTT
 * specification or <code>iotc_mqtt_qos_e</code> in <code>iotc_mqtt_message.h</code>.
 * @param [in] callback the callback function. Invoked after a message is
 * successfully or unsuccessfully delivered.
 * @param [in] user a pointer returned by the callback function.
 *
 * @see iotc_create_context
 * @see iotc_connect
 * @see iotc_publish
 * @see iotc_publish_data
 *
 * @retval IOTC_STATE_OK the subscription request is correctly formatted.
 * @retval IOTC_OUT_OF_MEMORY the platform doesn't have enough memory
 * to fulfull the request.
 * @retval IOTC_INTERNAL_ERROR an unrecoverable error occurred.
 */
extern iotc_state_t iotc_subscribe(iotc_context_handle_t iotc_h,
                                   const char* topic, const iotc_mqtt_qos_t qos,
                                   iotc_user_subscription_callback_t* callback,
                                   void* user_data);

/**
 * @brief    Disconnect from Cloud IoT Core.
 * @detailed Asynchronously disconnect from Cloud IoT Core. After disconnecting,
 * the callback defined by <code>iotc_connect</code> is invoked.
 *
 * @param [in] iotc_h a context handle created by invoking iotc_create_context.
 *
 * @retval IOTC_SOCKET_NO_ACTIVE_CONNECTION_ERROR if there is no connection for
 * this context.
 * @retval IOTC_STATE_OK if the disconnection request is enqueued.
 *
 * @see iotc_create_context
 * @see iotc_connect
 */
extern iotc_state_t iotc_shutdown_connection(iotc_context_handle_t iotc_h);

/**
 * @brief    Execute a function at a particular time.
 * @detailed Add a task to the internal event system. Cloud IoT Core
 * credentials are provided in the context. The callback method executes
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
 * Cancels tasks if they're no longer needed.
 *   - user_data the data provided to <code>iotc_schedule_timed_task</code>.
 *
 * @param [in] iotc_h a context handle created by invoking <code>iotc_create_context</code>.
 * @param [in] iotc_user_task_callback_t* a function invoked after
 * a specified amount of time.
 * @param [in] seconds_from_now number of seconds to wait before executing the
 * iotc_user_task_callback_t* function.
 * @param [in] repeats_forever by default, the callback is repeatedly executed at
 * seconds_from_now intervals. If the repeats_forever parameter is set to <code>0</code>, 
 * the callback is executed only once.
 *
 * @see iotc_create_context
 * @see iotc_cancel_timed_task
 *
 * @retval iotc_time_task_handle_t the task is scheduled. 
 * iotc_time_task_handle_t is a unique identifier for that task. If the function
 * breaks, iotc_time_task_handle_t is the <a href="~/include/iotc_error.h">error code</a> 
 * multiplied by -1.
 */
iotc_timed_task_handle_t iotc_schedule_timed_task(
    iotc_context_handle_t iotc_h, iotc_user_task_callback_t* callback,
    const iotc_time_t seconds_from_now, const uint8_t repeats_forever,
    void* data);

/**
 * @brief    Cancel a timed task.
 * @detailed Remove a task from the internal event system.
 *
 * @param [in] timed_task_handle the handle <code>iotc_schedule_timed_task</code> returns.
 *
 * @see iotc_create_context
 * @see iotc_schedule_timed_task
 */

void iotc_cancel_timed_task(iotc_timed_task_handle_t timed_task_handle);

/*-----------------------------------------------------------------------
 * HELPER FUNCTIONS
 * ---------------------------------------------------------------------- */

/**
 * @brief    Set connection timeout.
 * @detailed Specify the number of seconds sockets remain open when data
 * isn't passing through them. Note that the Device SDK periodically creates 
 * network traffic per MQTT specifications. 

 * Only new connections observe this timeout.
 *
 * @param [in] timeout the number of seconds before timing out. This parameter
 * is passed to the implementing networking layer during socket initialization.
 *
 * @see iotc_connect
 * @see iotc_connect_to
 * @see iotc_get_network_timeout
 **/
extern void iotc_set_network_timeout(uint32_t timeout);

/**
 * @brief    Query connection timeout.
 * @detailed Determine the current network timeout value.
 *
 * @see iotc_set_network_timeout
 **/
extern uint32_t iotc_get_network_timeout(void);

/**
 * @brief    Set the maximum heap memory the Device SDK can use.
 * @detailed Restrict the heap memory that the Device SDK can use. The 
 * specified value is the maximum heapspace, in bytes, the Device SDK
 * can use during standard execution.
 * 
 * This function is part of the Device SDK <a href="../../../user_guide.md#memory-limiter">memory limiter</a>.
 *
 * @param [in] max_bytes the maximum amount of heap memory, in bytes, that
 * the Device SDK can use.
 *
 * @retval IOTC_NOT_SUPPORTED the memory limiter module isn't installed.
 * @retval IOTC_OUT_OF_MEMORY the new memory limit is too small to support
 * the current heapspace footprint.
 * @retval IOTC_STATE_OK the new memory limit is the same as the current memory
 * limit.
 */
iotc_state_t iotc_set_maximum_heap_usage(const size_t max_bytes);

/**
 * @brief    Query the client application's current heap usage.
 * @detailed Determine the bytes of heap memory that the Device SDK is using.
 *
 * This function is part of the Device SDK <a href="../../../user_guide.md#memory-limiter">memory limiter</a>.
 *
 * @retval IOTC_NOT_SUPPORTED the memory limiter module isn't installed.
 * @retval IOTC_INVALID_PARAMETER the parameter is provided but defined as NULL.
 * @retval IOTC_STATE_OK the parameter is the same as the current heap usage.
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
