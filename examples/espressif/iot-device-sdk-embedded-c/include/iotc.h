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
 * @brief The main API for compiling your Application against the
 * the Google Cloud IoT Device SDK for Embedded C (IoTC).
 *
 * \mainpage Google Cloud IoT Device SDK for Embedded C API
 *
 * # Welcome
 * The functions of the main IoTC Client API are used to build a client
 * connection to the Google Cloud IoT Core Service over a secure socket, and
 * publish / subscribe to messages over MQTT.
 * This documention, in conjuction with the IoTC Client User Guide
 * (<code>/doc/user_guide.md</code>) and source examples in
 * <code>/examples/</code>, should provide
 * you with enough information to create your custom
 * IoTC Client-enabled applications.
 *
 * If you're looking for information on how to port the IoTC Client
 * to your custom embedded device, then please direct your attention to the
 * Board Support Package (BSP) documentation.  This exists in two parts, the
 * IoTC Client Porting Guide (<code>/doc/porting_guide.md</code>) and the
 * <a href="../../bsp/html/index.html">BSP doxygen</a>.
 *
 * # Where to Start
 * All of the standard IoTC Client API functions are contained within
 * the <code>/include</code> directory of the project's base path, and have the
 * 'iotc' prefix. We suggest clicking on iotc.h in the doxygen
 * File tab to get started.
 *
 * For most POSIX systems this might be all of the documentation that
 * you need. However, if you're building the library to be exeucted on your
 * own custom platform with non POSIX library support, then you may also
 * need to browse the Board Support Package (BSP) functions as well.
 * These functions are documented in the BSP doxygen documentation
 * contained in a sibling directory as this doxygen.
 *
 * # Further Reading
 * <ul><li><a href="../../bsp/html/index.html">
 * IoTC Board Support Package doxygen</a></li>
 * <li>
 * <a href="http://docs.oasis-open.org/mqtt/mqtt/v3.1.1/os/mqtt-v3.1.1-os.html">
 * MQTT v3.1.1 Specification</a></li>
 * <li> IoTC Client Users Guide (<code>/doc/user_guide.md</code>)</li>
 * <li> IoTC Client Porting Guide (<code>/doc/porting_guide.md</code>)</il>

 * </ul>
 */

/* -----------------------------------------------------------------------
 * MAIN LIBRARY FUNCTIONS
 * ----------------------------------------------------------------------- */

/**
 * @brief     Required before use of the IoTC Client library.
 * @detailed  This should be the first function that you call on a new runtime.
 *
 * @retval IOTC_STATE_OK              Status OK
 */
extern iotc_state_t iotc_initialize();

/**
 * @brief     Signals the IoTC Client library to cleanup any internal memory.
 * @detailed  This should by the last function called while shutting down your
 * application. Any resources that were created during initialization will be
 * cleaned up and freed.
 *
 * Note that you should always clean up IoTC contexts individually with
 * iotc_delete_context
 *
 * @see iotc_initialize
 * @see iotc_create_context
 * @see iotc_delete_context
 * @see iotc_shutdown_connection
 *
 * @retval IOTC_STATE_OK              Status OK
 * @retval IOTC_FAILED_INITIALIZATION An urecoverable error occured
 */
extern iotc_state_t iotc_shutdown();

/**
 * @brief     Creates a connection context for subscriptions and publications
 * @detailed  This should by invoked following a successful libiotc
 * initialization.  This  creates a specific context that can be passed to
 * connection, subscription, and publish functions.
 *
 * @see iotc_initialize
 * @see iotc_delete_context
 *
 * @retval nonnegative number         A valid context handle.
 * @retval negative number            A negated error code, defined in
 * iotc_error.h.
 */
extern iotc_context_handle_t iotc_create_context();

/**
 * @brief     Frees the provided connection context
 * @detailed  This should by invoked to free up memory when your applicaiton has
 * disconnected from the Google Cloud IoT Core service through the given
 * context.
 *
 * You may reuse a disconnected context to attempt to reconnect
 * to the service; you do not need to destroy a disconnected context and create
 * a new one.
 *
 * NOTE: Please do not delete the context in the disconnection callback
 * itself. Doing so many cause data corruption or a crash after the connect
 * callback returns.  Instead, delete the context either  on the next tick of
 * the client event loop in iotc_process_events_tick, or on POSIX, after
 * iotc_process_blocking returns due to a call to iotc_events_stop.
 *
 * @param [in] context handle that should be freed.
 *
 * @see iotc_create_context
 *
 * @retval IOTC_STATE_OK           Status OK
 * @retval IOTC_INVALID_PARAMETER  If the provided context handle is invalid.
 */
extern iotc_state_t iotc_delete_context(iotc_context_handle_t context_handle);

/**
 * @brief     Used to determine the state of a IoTC Client context's connection
 * to the remote service.
 *
 * @param [in] context handle to determine the connection status of.
 *
 * @see iotc_create_context
 * @see iotc_connect
 * @see iotc_connect_to
 *
 * @retval 1 if the context is currently connected
 * @retval 0 if the context is invalid, or the connection is currently any one
 * of the other following states: Unitialized, connecting, closing or closed.
 */
extern uint8_t iotc_is_context_connected(iotc_context_handle_t context_handle);

/**
 * @brief     Invokes the IoTC Client Event Processing loop.
 * @detailed  This function is meant to be execute the IoTC Client
 * as the main application process and therefore it does not return
 * until iotc_events_stop is invoked.
 *
 * If you're executing on a platform that cannot block indefinitely, then
 * please use iotc_events_process_tick instead.
 *
 * NOTE: if the client is in a state of IOTC_EVENT_PROCESS_STOPPED,
 * then the client will need to be destroyed and reiniitalized before the event
 * engine will properly process events again.  Therefore, stopping the event
 * system should be used sparringly, most likely in the standard process of
 * a reboot operation, or on POSIX, the standard process of client application
 * shutdown.
 *
 * @see iotc_events_process_tick
 * @see iotc_events_stop
 */
extern void iotc_events_process_blocking();

/**
 * @brief     Invokes the IOTC Client Event Processing loop.
 * @detailed This function will begin to process any pending tasks but then
 * return control to the client application.
 *
 * This function should be used on Embedded devices
 * only. It is meant for RTOS or No OS devices which require that control be
 * yielded to the OS for standard tick operations.
 *
 * NOTE: This function WILL BLOCK on standard UNIX devices like LINUX or MAC
 * OSX.
 *
 * NOTE: if the client is in a state of IOTC_EVENT_PROCESS_STOPPED,
 * then the client will need to be destroyed and reiniitalized before the event
 * engine will properly process events again.  Therefore, stopping the event
 * system should be used sparringly, most likely in the standard process of
 * a reboot operation, or on POSIX, the standard process of client application
 * shutdown.
 *
 * IOTC_STATE_OK will be returned if the event system is ongoing and
 * can continue to operate
 *
 * IOTC_EVENT_PROCESS_STOPPED will be returned if iotc_events_stop
 * has been invoked by the client application or if the event
 * processor has been stopped due to an unrecoverable error.
 *
 * @see iotc_events_process_blocking
 * @see iotc_events_stop
 *
 * @retval IOTC_STATE_OK           Status OK.
 * @retval IOTC_EVENT_PROCESS_STOPPED  If the event processor has been stopped.
 */
extern iotc_state_t iotc_events_process_tick();

/**
 * @brief     Causes the IoTC Client event loop to exit.
 * @detailed  Used to exit the blocking event loop, or to signal
 * that an irrecoverable event has occurred.
 *
 * @see iotc_events_process_blocking
 * @see iotc_events_process_tick
 */
extern void iotc_events_stop();

/**
 * @brief Opens a connection to the Google Cloud IoT Core service using the
 * provided context.  Includes a callback.
 * @detailed  Using the provided context, this function requests that a
 * connection be made to the Google Cloud IoT Core service given the
 * provided credentials. The callback method is a connection state
 * monitor callback, and will be invoked when this connection attempt
 * completes successfully, has failed, or after disconnection if a
 * connection was established.
 *
 * The callback function is type defined by iotc_user_callback_t, which has
 * the following signature:
 *    void foo( iotc_context_handle_t in_context_handle
 *                  , void* data
 *                  , iotc_state_t state )
 *
 * where the callback parameters are:
 *   - in_context_handle is the context handle that you provided to
 *     iotc_connect
 *   - data is a multifunctional structure. Please see the IoTC User Guide
 *     or examples/ for More Information.
 *   - state should be IOTC_STATE_OK if the connection succeeded. For other
 *     error codes please see the IoTC User Guide or Examples.
 *
 * @param [in] iotc_h a context handle created by invoking
 * iotc_create_context.
 * @param [in] username the MQTT Connect Username. For IoT Core, this
 * parameter is unused.
 * @param [in] password the MQTT Connect Password. For IoT Core, this
 * should be the IoT Core Connecting JWT created by the function
 * iotc_create_iotcore_jwt.
 * @param [in] client_id the MQTT connect client identifier. For IoT Core, this
 * must be the device_path of your device. Please see
 * https://cloud.google.com/iot/docs/how-tos/mqtt-bridge which details
 * how to construct the device path string.
 * @param [in] connection_timeout Number of seconds that the socket will be
 * kept before CONNACK without data coming from the server. In case of 0,
 * the TCP timeout will be used.
 * @param [in] keepalive_timeout Number of seconds that the socket will be
 * kept connected while being unused.
 *
 * @see iotc_create_context
 * @see iotc_create_iotcore_jwt
 *
 * @retval IOTC_STATE_OK  If the connection request was formatted
 * correctly and the client is processing the request with IoT Core.
 */
extern iotc_state_t iotc_connect(iotc_context_handle_t iotc_h,
                                 const char* username, const char* password,
                                 const char* client_id,
                                 uint16_t connection_timeout,
                                 uint16_t keepalive_timeout,
                                 iotc_user_callback_t* client_callback);

/**
 * @brief Opens a MQTT connection to a custom service endpoint using the
 * provided context, host and port.
 *
 * @param [in] host client will connect to this address
 * @param [in] port client will connect to the host on this port
 *
 * This function's behavior and other parameters are identical to
 * function iotc_connect.
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
 * @detailed  Using the provided context, this function requests that a message
 * be delivered to the IoT Core service on the given topic. This requires that a
 * connection has already been established to the IoT Core service via a call
 * to iotc_connect or iot_connect_to.
 *
 * The callback function should have the following signature:
 *  void foo( iotc_context_handle_t in_context_handle
 *                , void* user_data
 *                , iotc_state_t state )
 *
 * where the callback parameters are:
 *   - in_context_handle is context handle that you provided to iotc_publish.
 *   - user_data is the value that the application passed in the user_data
 * parameter of the publish call (below.) This can be used for you attach
 * any sort support data or relevant data that would help identify the message.
 *   - state should be IOTC_STATE_OK if the publication succeded, or a non ok
 * value if the publish failed.
 *
 * please return IOTC_STATE_OK for these callbacks.
 *
 * @param [in] iotc_h a context handle created by invoking iotc_create_context.
 * @param [in] topic a string based topic name that you have created for
 * messaging via the IoT Core webservice.
 * @param [in] msg the payload to send to the IoT Core service.
 * @param [in] qos Quality of Service MQTT level. 0, 1, or 2.  Please see MQTT
 * specification or iotc_mqtt_qos_e in iotc_mqtt_message.h for constant values.
 * Note: Currently Google Cloud IoT Core does not support QoS 2.
 * @param [in] callback Optional callback function that will be called upon
 * successful or unsuccessful msg delivery. This may be NULL.
 * @param [in] user_data Optional abstract data that will be passed to callback
 * when the publication completes. This may be NULL.
 *
 * @see iotc_create_context
 * @see iotc_publish_data
 *
 * @retval IOTC_STATE_OK         If the publication request was formatted
 * correctly and has be queued for publication.
 * @retval IOTC_OUT_OF_MEMORY    If the platform did not have enough free memory
 * to fulfill the request.
 * @retval IOTC_INTERNAL_ERROR   If an unforseen and unrecoverable error has
 * occurred.
 * @retval IOTC_BACKOFF_TERMINAL If backoff has been applied.
 */
extern iotc_state_t iotc_publish(iotc_context_handle_t iotc_h,
                                 const char* topic, const char* msg,
                                 const iotc_mqtt_qos_t qos,
                                 iotc_user_callback_t* callback,
                                 void* user_data);

/**
 * @brief     Publishes binary data to the Google Cloud IoT service on the given
 * topic.
 * @detailed Using the provided context, this function requests that binary data
 * be delivered to the IoT Core service on the given topic. This requires that a
 * connection already be made to the IoT Core service via a iotc_connect call.
 *
 * Note: This function is idential to iotc_publish except that it
 * doesn't assume a null terminated string. Instead a pointer and data
 * length are provided.
 *
 * @param [in] data the payload to send to the IoT Core service.
 * @param [in] data_len the size in bytes of the data buf to send.
 *
 * @see iotc_create_context
 * @see iotc_publish
 *
 * @retval IOTC_STATE_OK If the publication request was formatted correctly.
 * @retval IOTC_OUT_OF_MEMORY   If the platform did not have enough free memory
 * to fulfill the request.
 * @retval IOTC_INTERNAL_ERROR  If an unforseen and unrecoverable error has
 * occurred.
 */
extern iotc_state_t iotc_publish_data(iotc_context_handle_t iotc_h,
                                      const char* topic, const uint8_t* data,
                                      size_t data_len,
                                      const iotc_mqtt_qos_t qos,
                                      iotc_user_callback_t* callback,
                                      void* user_data);

/**
 * @brief     Subscribes to notifications if a message from the Google
 * Cloud IoT Core service is posted to the given topic.
 * @detailed  Using the provided context, this function formats a request to
 * subscribe to a topic in the IoT Core service.  If the subscription is
 * successful then incoming messages will be provided to the callback function.
 * Subscription falures will also invoke the callback function with parameters
 * that denote the error.
 *
 * The callback function is type defined by iotc_user_callback_t, which has the
 * following siguature:
 *    void foo( iotc_context_handle_t in_context_handle,
 *          iotc_sub_call_type_t call_type,
 *          const iotc_sub_call_params_t* const params,
 *          iotc_state_t state,
 *          void* user_data )
 *
 * where:
 *   - context_handle is the context handle you provided to iotc_subscribe.
 *   - call_type each callback invocation may be related to subscription
 * confirmation or a new message, this enum points to the type of the
 * invocation.
 *   - params is a structure that holds the details about the subscription
 * confirmation or new message.  The data in this structure will vary depending
 * on the call_type value
 *   - state should be IOTC_STATE_OK if the message reception succeeded or
 * IOTC_MQTT_SUBSCRIPTION_SUCCESSFULL/IOTC_MQTT_SUBSCRIPTION_FAILED in case of
 * successfull / failed subscription, values different than described should be
 * treated as errors.
 *   - user_data is the pointer you provided to user parameter in iotc_subscribe
 *
 * @param [in] iotc_h a context handle created by invoking iotc_create_context
 * @param [in] topic a string based topic name that you have created for
 * messaging via the IoT Core webservice.
 * @param [in] qos Quality of Service MQTT level. 0, 1, or 2.  Please see MQTT
 * specification or iotc_mqtt_qos_e in iotc_mqtt_message.h for constant values.
 * @param [in] callback a function pointer to be invoked when a
 * message arrives, as described above
 * @param [in] user a pointer that will be returned back during the callback
 * invocation
 *
 * @see iotc_create_context
 * @see iotc_connect
 * @see iotc_publish
 * @see iotc_publish_data
 *
 * @retval IOTC_STATE_OK If the publication request was formatted correctly.
 * @retval IOTC_OUT_OF_MEMORY   If the platform did not have enough free memory
 * to fulfill the request.
 * @retval IOTC_INTERNAL_ERROR  If an unforseen and unrecoverable error has
 * occurred.
 */
extern iotc_state_t iotc_subscribe(iotc_context_handle_t iotc_h,
                                   const char* topic, const iotc_mqtt_qos_t qos,
                                   iotc_user_subscription_callback_t* callback,
                                   void* user_data);

/**
 * @brief     Closes the connection associated with the provide context.
 * @detailed  Closes the connection to the Google Cloud IoT Core Service.  This
 * will happen asynchronously. The callback is defined by iotc_connect.
 *
 * @param [in] iotc_h a context handle created by invoking iotc_create_context.
 *
 * @retval IOTC_SOCKET_NO_ACTIVE_CONNECTION_ERROR If there is no connection for
 * this context.
 * @retval IOTC_STATE_OK If the disconnection request has been queued for
 * process.
 *
 * @see iotc_create_context
 * @see iotc_connect
 */
extern iotc_state_t iotc_shutdown_connection(iotc_context_handle_t iotc_h);

/**
 * @brief     Schedule a task for timed execution
 * @detailed Using the provided context, this function adds a task to the
 * internal event system. The callback method will be invoked when the given
 * time has passed.
 *
 * The callback function should have the following siguature:
 *  typedef void ( iotc_user_task_callback_t ) ( const iotc_context_handle_t
 *    context_handle, const iotc_timed_task_handle_t, timed_task_handle, void*
 *    user_data );
 *
 * where:
 *   - in_context_handle the context handle you provided to
 * iotc_schedule_timed_task
 *   - timed_task_handle is the handle that iotc_schedule_timed_task call
 * returned, this can be used to cancel the task if the task is no longer needed
 * from within the callback
 *   - user_data is the data you provided to iotc_schedule_timed_task
 *
 * @param [in] iotc_h a context handle created by invoking iotc_create_context.
 * @param [in] iotc_user_task_callback_t* a function pointer to be invoked when
 * given time has passed.
 * @param [in] seconds_from_now number of seconds to wait before task
 * invocation.
 * @param [in] repeats_forever if zero is passed the callback will be called
 * only once, otherwise the callback will be called continuously with the
 * seconds_from_now delay until iotc_cancel_timed_task() called.
 *
 * @see iotc_create_context
 * @see iotc_cancel_timed_task
 *
 * @retval iotc_time_task_handle_t if bigger than 0 it's the unique identifier
 * of the task, if smaller, it's an error code multiplied by -1, the possible
 * error codes are:
 *
 *  - IOTC_STATE_OK If the request was formatted correctly.
 *  - IOTC_OUT_OF_MEMORY If the platform did not have enough free
 * memory to fulfill the request
 *  - IOTC_INTERNAL_ERROR  If an unforseen and unrecoverable error has
 * occurred.
 */
iotc_timed_task_handle_t iotc_schedule_timed_task(
    iotc_context_handle_t iotc_h, iotc_user_task_callback_t* callback,
    const iotc_time_t seconds_from_now, const uint8_t repeats_forever,
    void* data);

/**
 * @brief     Cancel a timed task
 * @detailed  This function cancels the timed execution of the task specified
 * by the given handle.
 *
 * @param [in] timed_task_handle a handle returned by iotc_schedule_timed_task.
 *
 * @see iotc_create_context
 * @see iotc_schedule_timed_task
 */

void iotc_cancel_timed_task(iotc_timed_task_handle_t timed_task_handle);

/*-----------------------------------------------------------------------
 * HELPER FUNCTIONS
 * ---------------------------------------------------------------------- */

/**
 * @brief     Timeout value that's passed to the networking implementation
 * @detailed  This function configures how long a socket will live while there
 * is no data sent on the socket.  Note that the MQTT Keep Alive
 * value configured in the iotc_connect will cause the IoTC Client to
 * periodically create network traffic, as part of the MQTT specification.
 *
 * This value is only observed when constructing new connections, so invoking
 * this will not have any affect on on-going connections.
 *
 * @param [in] timeout a timeout value that will be passed to the implementing
 * networking layer during socket initialization.
 *
 * @see iotc_connect
 * @see iotc_connect_to
 * @see iotc_get_network_timeout
 **/
extern void iotc_set_network_timeout(uint32_t timeout);

/**
 * @brief     Returns the timeout value for socket connections.
 * @detailed  Queries the current configuration of network timeout values that
 *will be fed to that native network implementation when sockets are
 *initialized.
 *
 * @see iotc_set_network_timeout
 **/
extern uint32_t iotc_get_network_timeout(void);

/**
 * @brief     Sets Maximum Amount of Heap Allocated Memory the IoTC Client may
 * use.
 * @detailed  This function is part of an optional configuration of the IoTC
 * Client called the Memory Limiter.
 *
 * This system can be used to guarantee that the IoTC Client will use only a
 * certain amount of the heapspace during its standard execution.  On many
 * platforms this can also include the heap usage of the TLS implementation if
 * the TLS implementation can be written to use the IoTC malloc and free
 * functions.
 *
 * @param [in] max_bytes the upper bounds of memory that the IoTC Client will
 * use in its serialization, parsing and encryption of mqtt packets, and for the
 *  facilitation of Client Application callbacks and events.
 *
 * @retval IOTC_NOT_SUPPORTED If the Memory Limiter module has not been compiled
 *  into this version of the IoTC Client Library
 *
 * @retval IOTC_OUT_OF_MEMORY If the new memory limit is too small to support
 *  the current IoTC Client heapspace footprint.
 *
 * @retval IOTC_STATE_OK the new memory limit has been set to the value
 *  specified by max_bytes.
 */
iotc_state_t iotc_set_maximum_heap_usage(const size_t max_bytes);

/**
 * @brief     Fetches the IoTC Client's current amount of heap usage
 * @detailed  This function is part of an optional configuration of the IoTC
 * Client called the Memory Limiter.  If enabled, you can use this function to
 * determine the current heap usage of the IoTC Client.  Depending on the TLS
 * implementation, this might also include the TLS buffers used for encoding /
 * decoding and certificate parsing.
 *
 * @retval IOTC_NOT_SUPPORTED If the Memory Limiter module has not been compiled
 *  into this version of the IoTC Client Library
 *
 * @retval IOTC_INVALID_PARAMETER If the provided parameter is NULL
 *
 * @retval IOTC_STATE_OK if the parameter has been filled-in with the IoTC
 * Client's current heap usage.
 *
 */
iotc_state_t iotc_get_heap_usage(size_t* const heap_usage);

/**
 * @brief Contains the major version number of the IoTC Client
 * library.
 **/
extern const uint16_t iotc_major;

/**
 * @brief Contains the minor version number of the IoTC Client
 * library.
 **/
extern const uint16_t iotc_minor;

/**
 * @brief Contains the revsion number of the IoTC Client library.
 **/
extern const uint16_t iotc_revision;

/**
 * @brief String representation of the Major.Minor.Revision version
 * of the IoTC Client library.
 * library.
 **/
extern const char iotc_cilent_version_str[];

#ifdef IOTC_EXPOSE_FS
#include "iotc_fs_api.h"

/**
 * @brief     Sets filesystem functions so that the libiotc can use custom
 * filesystem
 */
iotc_state_t iotc_set_fs_functions(const iotc_fs_functions_t fs_functions);

#endif /* IOTC_EXPOSE_FS */

#ifdef __cplusplus
}
#endif

#endif /* __IOTC_H__ */
