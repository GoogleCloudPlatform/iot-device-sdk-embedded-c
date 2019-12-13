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
 * @brief Connects to and communicates with Cloud IoT Core
 * @mainpage Overview
 *
 * @details The Cloud IoT device SDK for embedded C is a library of source files
 * for securely connecting to and communicating with
 * <a href="https://cloud.google.com/iot-core/">Cloud IoT Core</a>. The SDK is
 * designed for embedded IoT devices,
 * <a href="../../../user_guide.md#feature-overview">so it</a>:
 *
 * - Communicates over MQTT v3.1.1 with TLS
 * - Publishes and subscribes asychronously
 * - Abstracts hardware-specific drivers and routines in the Board Support Package
 * - Runs on single, non-blocking thread and operates a thread-safe event queue
 *
 * # Installing the SDK
 *
 * To use the SDK:
 *
 * <ol>
 *   <li>Download the latest version:
 * 
 *   @code{.sh}
 *   git clone https://github.com/GoogleCloudPlatform/iot-device-sdk-embedded-c.git --recurse-submodules
 *   @endcode
 *   </li>
 *   <li>Link the main library files to the client application:
 *   
 *   @code{.c}
 *   #include <iotc.h>
 *   #include <iotc_error.h>
 *   #include <iotc_jwt.h>
 *   @endcode
 *   </li>
 * </ol>
 *
 * # Function summary
 * The following tables list the functions you can use to build an MQTT client.
 *
 * ## Client SDK infrastructure functions
 * | Function | Description |
 * | --- | --- |
 * | iotc_create_context() | Creates a connection context. |
 * | iotc_delete_context() | Deletes and frees the provided context. | 
 * | iotc_initialize() | Initializes the BSP time and random number libraries. |
 * | iotc_is_context_connected() | Checks if a context is {@link iotc_connect() connected to an MQTT broker}. | 
 * | iotc_shutdown() | Shuts down the IoT device SDK and frees all resources created during {@link iotc_initialize() initialization}. |
 * | iotc_get_heap_usage() | Gets the amount of heap memory allocated to the IoT device SDK. |
 * | iotc_get_network_timeout() | Gets the {@link iotc_set_network_timeout() connection timeout}.
 * | iotc_get_state_string() | Gets the {@link ::iotc_state_t state message} associated with a numeric code. |
 * | iotc_set_fs_functions() | Sets the file operations to <a href="../../bsp/html/d8/dc3/iotc__bsp__io__fs_8h.html">custom file management functions</a>. |
 * | iotc_set_maximum_heap_usage() | Sets the maximum heap memory that the IoT device SDK can use. |
 * | iotc_set_network_timeout() | Sets the connection timeout. |  
 *
 * ## MQTT functions
 * | Function | Description |
 * | --- | --- | 
 * | iotc_connect() | Connects to Cloud IoT Core. |
 * | iotc_connect_to() | Connects to a custom MQTT broker endpoint. |
 * | iotc_create_iotcore_jwt() | Creates a JSON Web Token, which is required to authenticate to Cloud IoT Core. |
 * | iotc_publish() | Publishes a message to an MQTT topic. |
 * | iotc_publish_data() | Publishes binary data to an MQTT topic. | 
 * | iotc_shutdown_connection() | Disconnects asynchronously from an MQTT broker. |
 * | iotc_subscribe() | Subscribes to an MQTT topic. |
 *
 * ## Scheduling functions
 * | Function | Description |
 * | --- | --- |
 * | iotc_cancel_timed_task() | Removes a scheduled task from the internal event system. |
 * | iotc_events_process_blocking() | Invokes the event processing loop and executes the event engine as the main application process. |
 * | iotc_events_process_tick() | Invokes the event processing loop on RTOS or non-OS devices that must yield for standard tick operations. |
 * | iotc_events_stop() | Shuts down the event engine. |
 * | iotc_schedule_timed_task() | Executes a function after an interval and returns a unique ID for the scheduled task. |
 *
 * # Example MQTT clients
 * The repository has example client applications for connecting:
 *
 * - <a href="../../../../examples/iot_core_mqtt_client">Native Linux</a>
 * devices to Cloud IoT Core
 * - <a href="../../../../examples/zephyr_native_posix">Zephyr <code>native_posix</code></a>
 * boards to Cloud IoT Core
 * - <a href="../../../../examples/freertos_linux/Linux_gcc_gcp_iot">FreeRTOS</a>
 * platforms to Cloud IoT Core 
 *
 * # Board Support Package 
 * The SDK depends on hardware-specific drivers and routines to implement
 * MQTT with TLS. Embedded systems rely on unique hardware, so the SDK abstracts
 * these dependencies in the <a href="../../bsp/html/index.html">Board Support
 * Package</a> (BSP).
 *
 * The SDK has a turn-key POSIX BSP, so it builds natively on POSIX platforms.
 * You can customize the BSP to port the SDK to non-POSIX platforms.
 */

/* -----------------------------------------------------------------------
 * MAIN LIBRARY FUNCTIONS
 * ----------------------------------------------------------------------- */

/**
 * @details Initializes the BSP time and random number libraries. Applications
 * must call the function first when starting a new runtime.
 */
extern iotc_state_t iotc_initialize();

/**
 * @details Shuts down the IoT device SDK and frees all resources created during
 * {@link iotc_initialize() initialization}. {@link iotc_delete_context() Free}
 * all contexts before calling the function.
 */
extern iotc_state_t iotc_shutdown();

/**
 * @details Creates a connection context. If the function fails, it returns the
 * opposite of the numeric {@link ::iotc_state_t error code}.
 */
extern iotc_context_handle_t iotc_create_context();

/**
 * @brief Frees the provided context.
 *
 * @details If iotc_events_process_blocking() invokes the event engine, the
 * client application must free the context after iotc_events_process_blocking()
 * returns. The application must free the context on the event loop tick after
 * the disconnection event (not in the disconnection callback itself).
 *
 * @param [in] context The {@link iotc_create_context() context handle} to free.
 */
extern iotc_state_t iotc_delete_context(iotc_context_handle_t context_handle);

/**
 * @brief Checks if a context is
 * {@link iotc_connect() connected to Cloud IoT Core}.
 *
 * @param [in] context The handle for which to determine the connection.
 *
 * @retval 1 The context is connected to Cloud IoT Core.
 * @retval 0 The context is invalid or the connection is either uninitialized,
 *     connecting, closing, or closed.
 */
extern uint8_t iotc_is_context_connected(iotc_context_handle_t context_handle);

/**
 * @details Invokes the event processing loop and executes event engine
 * as the main application process. The function processes events on platforms
 * with main application loops that can block indefinitely. For other platforms,
 * call iotc_events_process_tick().
 *
 * You can call the function anytime but it returns only after calling
 * iotc_events_stop().
 * 
 * The event engine won't {@link iotc_events_process_tick() process events} when
 * the IoT device SDK is in the
 * {@link ::iotc_state_t IOTC_EVENT_PROCESS_STOPPED state}. If
 * the function returns IOTC_EVENT_PROCESS_STOPPED,
 * {@link iotc_shutdown() shutdown} and {@link iotc_initialize() reinitialize}
 * the IoT device SDK to process events again.
 */
extern void iotc_events_process_blocking();

/**
 * @details Invokes the event processing loop on RTOS or non-OS devices that
 * must yield for standard tick operations.
 *
 * The event engine won't {@link iotc_events_process_tick() process events} when
 * the IoT device SDK is in the
 * {@link ::iotc_state_t IOTC_EVENT_PROCESS_STOPPED state}. If
 * the function returns IOTC_EVENT_PROCESS_STOPPED,
 * {@link iotc_shutdown() shutdown} and {@link iotc_initialize() reinitialize}
 * the IoT device SDK to process events again.
 */
extern iotc_state_t iotc_events_process_tick();

/**
 * @brief Shuts down the event engine.
 */
extern void iotc_events_stop();

/**
 * @brief Connects to Cloud IoT Core.
 *
 * @param [in] iotc_h A {@link iotc_create_context() context handle}.
 * @param [in] username The MQTT username. Cloud IoT Core ignores this
 *     parameter, but some platforms will not send the password field unless the
 *     username field is specified. For best results, supply an arbitrary
 *     username like "unused" or "ignored."
 * @param [in] password The MQTT password. Cloud IoT Core requires a
 *     {@link iotc_create_iotcore_jwt() JWT}.
 * @param [in] client_id The MQTT client ID. Cloud IoT Core requires a
 *     <a href="https://cloud.google.com/iot/docs/how-tos/mqtt-bridge#configuring_mqtt_clients">
 *     device path</a>.
 * @param [in] connection_timeout The number of seconds to wait for an MQTT
 *     <code>CONNACK</code> response before closing the socket. If
 *     <code>0</code>, the TCP timeout is used.
 * @param [in] keepalive_timeout The <a href="http://docs.oasis-open.org/mqtt/mqtt/v3.1.1/os/mqtt-v3.1.1-os.html#_Toc385349238">
 *     number of seconds</a> the broker will wait for the client application to
 *     send a <code>PINGREQ</code> message. The <code>PINGREQ</code> is
 *     automatically sent at the specified interval, so you don't need to write
 *     the <code>PINGREQ</code> message contents.
 * @param [in] (Optional) client_callback The callback function. Invoked when
 *     the client connects to or is disconnected from the MQTT broker. 
 */
extern iotc_state_t iotc_connect(iotc_context_handle_t iotc_h,
                                 const char* username, const char* password,
                                 const char* client_id,
                                 uint16_t connection_timeout,
                                 uint16_t keepalive_timeout,
                                 iotc_user_callback_t* client_callback);

/**
 * @brief Connects to a custom MQTT broker endpoint.
 *
 * @details Performs the same operations as iotc_connect() but you can
 * configure the endpoint.
 *
 * @param [in] host The address at which the client connects.
 * @param [in] port The port of the host on which to connect.
 * @param [in] iotc_h A {@link iotc_create_context() context handle}.
 * @param [in] username The MQTT username. Some platforms will not send the
 *     password field unless the username field is specified. 
 * @param [in] password The MQTT password.
 * @param [in] client_id The MQTT client ID.
 * @param [in] connection_timeout The number of seconds to wait for an MQTT
 *     <code>CONNACK</code> response before closing the socket. If
 *     <code>0</code>, the TCP timeout is used.
 * @param [in] keepalive_timeout The <a href="http://docs.oasis-open.org/mqtt/mqtt/v3.1.1/os/mqtt-v3.1.1-os.html#_Toc385349238">
 *     number of seconds</a> the broker will wait for the client application to
 *     send a <code>PINGREQ</code> message. The <code>PINGREQ</code> is
 *     automatically sent at the specified interval, so you don't need to write
 *     the <code>PINGREQ</code> message contents.
 * @param [in] (Optional) client_callback The callback function. Invoked when
 *     the client connects to or is disconnected from the MQTT broker.
 */
extern iotc_state_t iotc_connect_to(iotc_context_handle_t iotc_h,
                                    const char* host, uint16_t port,
                                    const char* username, const char* password,
                                    const char* client_id,
                                    uint16_t connection_timeout,
                                    uint16_t keepalive_timeout,
                                    iotc_user_callback_t* client_callback);

/**
 * @brief Publishes a message to an MQTT topic.
 *
 * @param [in] iotc_h A {@link iotc_create_context() context handle}.
 * @param [in] topic The MQTT topic.
 * @param [in] msg The payload of the message.
 * @param [in] qos The Quality of Service (QoS) level. Can be <code>0</code> or
 *     <code>1</code>. QoS level <code>2</code> isn't supported.
 * @param [in] callback (Optional) The callback function. Invoked after a
 *     message is successfully or unsuccessfully delivered.
 * @param [in] (Optional) client_callback The callback function. Invoked when
 *     the client connects to or is disconnected from the MQTT broker. 
 * @param [in] user_data (Optional) Abstract data passed to the callback
 *     function. 
 */
extern iotc_state_t iotc_publish(iotc_context_handle_t iotc_h,
                                 const char* topic, const char* msg,
                                 const iotc_mqtt_qos_t qos,
                                 iotc_user_callback_t* callback,
                                 void* user_data);

/**
 * @brief Publishes binary data to an MQTT topic.
 *
 * @details Performs the same operations as iotc_publish() but the payload
 * can be empty.
 *
 * @param [in] iotc_h A {@link iotc_create_context() context handle}.
 * @param [in] topic The MQTT topic. 
 * @param [in] data A pointer to a buffer with the message payload.
 * @param [in] data_len The size, in bytes, of the message.
 * @param [in] qos The Quality of Service (QoS) level. Can be <code>0</code> or
 *     <code>1</code>. QoS level <code>2</code> isn't supported.
 * @param [in] callback (Optional) The callback function. Invoked after a
 *     message is successfully or unsuccessfully delivered.
 * @param [in] user_data (Optional) Abstract data passed to the callback
 *     function. 
 */
extern iotc_state_t iotc_publish_data(iotc_context_handle_t iotc_h,
                                      const char* topic, const uint8_t* data,
                                      size_t data_len,
                                      const iotc_mqtt_qos_t qos,
                                      iotc_user_callback_t* callback,
                                      void* user_data);

/**
 * @brief Subscribes to an MQTT topic.
 *
 * @param [in] iotc_h A {@link iotc_create_context() context handle}.
 * @param [in] topic The MQTT topic.
 * @param [in] qos The Quality of Service (QoS) level. Can be <code>0</code>,
 *     <code>1</code>, or <code>2</code>.
 * @param [in] callback The {@link ::iotc_user_subscription_callback_t callback}
 *     invoked after a message is published to the MQTT topic.
 * @param [in] user_data (Optional) A pointer that to the callback function's
 *     user_data parameter.
 */
extern iotc_state_t iotc_subscribe(iotc_context_handle_t iotc_h,
                                   const char* topic, const iotc_mqtt_qos_t qos,
                                   iotc_user_subscription_callback_t* callback,
                                   void* user_data);

/**
 * @brief Disconnects asynchronously from an MQTT broker.
 *
 * @details After disconnecting, the disconnection status code is passed to the
 * iotc_connect() callback. You may reuse disconnected contexts until
 * iotc_events_stop() returns; you don't need to destroy and recreate contexts.
 *
 * @param [in] iotc_h A {@link iotc_create_context() context handle}.
 */
extern iotc_state_t iotc_shutdown_connection(iotc_context_handle_t iotc_h);

/**
 * @brief Executes a function after an interval and returns a unique ID for the
 * scheduled task. If the function returns an error, the ID is the opposite of
 *     the numeric {@link ::iotc_state_t error code}.
 *
 * @param [in] iotc_h A {@link iotc_create_context() context handle}.
 * @param [in] iotc_user_task_callback_t The
 *     {@link ::iotc_user_task_callback_t function} invoked after an interval.
 * @param [in] seconds_from_now The number of seconds to wait before invoking
 *     the callback.
 * @param [in] repeats_forever If the repeats_forever parameter is set to
 *     <code>0</code>, the callback is executed only once. Otherwise, the
 *     callback is repeatedly executed at seconds_from_now intervals. 
 * @param [in] data (Optional) A pointer which will be passed to the callback
 *     function's user_data parameter.
 */
iotc_timed_task_handle_t iotc_schedule_timed_task(
    iotc_context_handle_t iotc_h, iotc_user_task_callback_t* callback,
    const iotc_time_t seconds_from_now, const uint8_t repeats_forever,
    void* data);

/**
 * @brief Removes a scheduled task from the internal event system.
 *
 * @param [in] timed_task_handle A unique ID for the scheduled task.
 */

void iotc_cancel_timed_task(iotc_timed_task_handle_t timed_task_handle);

/*-----------------------------------------------------------------------
 * HELPER FUNCTIONS
 * ---------------------------------------------------------------------- */

/**
 * @brief Sets the connection timeout. Only new connections observe this
 * timeout. 
 *
 * @details <b>Note</b>: The IoT device SDK periodically creates network traffic per
 * MQTT specifications.
 *
 * @param [in] timeout The number of seconds sockets remain open when data
 *     isn't passing through them. When the socket initializes, this parameter 
 *     is passed to the implemented networking layer to automatically keep
 *     connections open.
 */
extern void iotc_set_network_timeout(uint32_t timeout);

/**
 * @brief Gets the
 * {@link iotc_set_network_timeout() connection timeout}.
 */
extern uint32_t iotc_get_network_timeout(void);

/**
 * @details Sets the maximum heap memory that the IoT device SDK can use.
 *
 * The function is part of the
 * <a href="../../../user_guide.md#memory-limiter">memory limiter</a>.
 *
 * @param [in] max_bytes The maximum amount of heap memory, in bytes, that
 *     the IoT device SDK can use during standard execution.
 */
iotc_state_t iotc_set_maximum_heap_usage(const size_t max_bytes);

/**
 * @details Gets the amount of heap memory allocated to the IoT device SDK.
 *
 * The function is part of the
 * <a href="../../../user_guide.md#memory-limiter">memory limiter</a>. If no
 * heap memory is allocated, the function runs but returns
 * IOTC_INVALID_PARAMETER.
 */
iotc_state_t iotc_get_heap_usage(size_t* const heap_usage);

/**
 * @brief The IoT device SDK major version number.
 **/
extern const uint16_t iotc_major;

/**
 * @brief The IoT device SDK minor version number.
 **/
extern const uint16_t iotc_minor;

/**
 * @brief The IoT device SDK revision number.
 **/
extern const uint16_t iotc_revision;

/**
 * @brief The IoT device SDK "major.minor.revision" version string.
 **/
extern const char iotc_cilent_version_str[];

#ifdef IOTC_EXPOSE_FS
#include "iotc_fs_api.h"

/**
 * @brief Sets the file operations to
 * <a href="../../bsp/html/d8/dc3/iotc__bsp__io__fs_8h.html">custom file
 * management functions</a>.
 */
iotc_state_t iotc_set_fs_functions(const iotc_fs_functions_t fs_functions);

#endif /* IOTC_EXPOSE_FS */

#ifdef __cplusplus
}
#endif

#endif /* __IOTC_H__ */
