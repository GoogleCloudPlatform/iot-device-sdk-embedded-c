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
 * @mainpage Google Cloud IoT device SDK for embedded C 1.0.2
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
 * The repository has example client applications for connecting:
 *
 * - <a href="../../../../examples/freertos_linux/Linux_gcc_gcp_iot">Zephyr
 * <code>native_posix</code></a> board applications to Cloud IoT Core
 * - <a href="../../../../examples/freertos_linux/Linux_gcc_gcp_iot">FreeRTOS</a>
 * applications to Cloud IoT Core
 * - <a href="../../../../examples/freertos_linux/Linux_gcc_gcp_iot">Native Linux</a>
 * applications to Cloud IoT Core
 * 
 * # Quickstart on a POSIX platform
 * This section shows you how to create a client application on a Linux device and
 * publish telemetry events to Cloud IoT Core.
 * 
 * Client applications build natively on POSIX platforms, so you don't need to
 * customize the Board Support Package. You'll build this client application with
 * mbedTLS, the default TLS library.
 * 
 * Before connecting to Cloud IoT Core,
 * <a href="https://cloud.google.com/iot/docs/how-tos/devices">register the device</a>
 * with an
 * <a href="https://cloud.google.com/iot/docs/how-tos/credentials/keys#generating_an_elliptic_curve_keys">Elliptic Curve key pair</a>.
 * 
 * ## Download the SDK
 *
 * @code{.sh}
 * git clone https://github.com/GoogleCloudPlatform/iot-device-sdk-embedded-c.git
 * @endcode
 * 
 * ## Create a client application
 * 
 * The client application is a C program that connects to an interacts with
 * Cloud IoT Core. This client application publishes telemetry events from a
 * native Linux device to Cloud IoT Core.
 * 
 * To start, create a C file called my_iot_app.c and paste in the code snippets
 * to:
 * 
 * 1. Initialize the SDK
 * 2. Connect to Cloud IoT Core
 * 3. Publish telemetry events to Cloud IoT Core
 * 
 * ### Initialize the SDK
 * 
 * <ol>
 *   <li>
 *     Link the SDK to your client application:
 *
 *     @code{.c}
 *     #include <iotc.h>
 *     #include <iotc_error.h>
 *     #include <iotc_jwt.h>
 *     @endcode
 *   </li>
 *   <li>
 *     Initialize the BSP time and random number libraries:
 *
 *     @code{.c}
 *     const iotc_state_t error_init = iotc_initialize();
 *     @endcode
 * 
 *     The TLS library uses these functions to create JWTs.
 *   </li>
 * </ol>
 * 
 * ### Communicate with Cloud IoT Core
 * 
 * <ol>
 *   <li>
 *     Create a connection context:
 *
 *     @code{.c}
 *     iotc_context = iotc_create_context();
 *     @endcode
 *
 *     The context represents the socket connection, so the client applications
 *     knows where to publish telemetry events.
 *   </li>
 *   <li>
 *     Create a JWT:
 *
 *     @code{.c}
 *     char jwt[IOTC_JWT_SIZE] = {0};
 *     size_t bytes_written = 0;
 *     iotc_state_t state = iotc_create_iotcore_jwt(
 *       iotc_project_id,
 *       3600,
 *       &iotc_connect_private_key_data,
 *       jwt,
 *       IOTC_JWT_SIZE,
 *       &bytes_written);
 *     @endcode
 *   </li>
 *   <li>
 *     Connect to Cloud IoT Core:
 *
 *     @code{.c}
 *     const char iotc_device_path = 'projects/PROJECT_ID/locations/REGION/registries/REGISTRY_ID/devices/DEVICE_ID';
 *     const uint16_t connection_timeout = 10;
 *     const uint16_t keepalive_timeout = 20;
 *
 *     iotc_connect(iotc_context, /NULL, jwt,
 *                iotc_device_path, connection_timeout,
 *                keepalive_timeout);
 *     @endcode
 *
 *     The {@link iotc_connect()} function sends a CONNECT message to the Cloud IoT
 *     Core MQTT broker. The MQTT topic is the
 *     <a href="https://cloud.google.com/iot/docs/how-tos/mqtt-bridge#configuring_mqtt_clients">device path</a>.
 *   </li>
 *   <li>
 *     Publish a telemetry event to Cloud IoT Core:
 *
 *     @code{.c}
 *     const char msg = 'Hello world';
 *     const iotc_mqtt_qos_t qos = 1;
 *
 *     iotc_publish(iotc_context, iotc_device_path
 *                msg, qos);
 *     @endcode 
 *
 *     The {@link iotc_publish()} function sends a PUBLISH message to the MQTT
 *     broker. The payload of the message is the telemetry event.
 *   </li>
 * </ol>
 * 
 * ## Build the client application
 * 
 * The SDK builds with <code>make</code>, so the build configuration is in
 * makefiles. Build client applications on the target device—not your
 * workstation.
 * 
 * To build a client application with the default build configuration:
 * 
 * <ol>
 *   <li>
 *     Run <code>make</code> from the
 *     <a href="https://github.com/GoogleCloudPlatform/iot-edge-sdk-embedded-c">root directory</a>
 *     of the SDK.
 *   </li>
 *   <li>
 *     Create a makefile (called <code>Makefile</code>) in the root directory of
 *     the client application. This makefile specifies build rules and targets
 *     for your client application.
 * 
 *     On native Linux devices, you can use the configurations in
 *     <a href="../../../examples/common/rules.mk"><code>rules.mk</code></a> and
 *     <a href="../../../examples/common/targets.mk"><code>targets.mk</code></a>.
 *     Copy them to the root directory of your client application and include them
 *     in the makefile:
 *     
 *     @code{.mk}
 *     include rules.mk
 *     include targets.mk
 *     @endcode
 *   </li>
 *   <li>
 *     Run <code>make</code> from the root directory of your client application.
 *     <code>make</code> compiles the program and generates an executable.
 * </ol>
 * 
 * ## Start the client application
 *
 * Run the executable to start your client application:
 *
 * @code{.sh}
 * ./my_iot_app.exe
 * @endcode
 *
 * # File summary
 * The library functions are in:
 *
 * | File | Description |
 * | --- | --- |
 * | iotc.h | Connects to and communicates with Cloud IoT Core |
 * | iotc_error.h | Defines state messages and their numeric codes |
 * | iotc_jwt.h | Creates JSON Web Tokens |
 *
 * See [File list]() for a complete file reference, including files with only
 * helper functions.
 *
 * # Function summary
 * | Function | Description |
 * | --- | --- |
 * | iotc_cancel_timed_task() | Removes a scheduled task from the internal event system. |
 * | iotc_connect() | Connects to Cloud IoT Core. |
 * | iotc_connect_to() | Connects to a custom MQTT broker endpoint. |
 * | iotc_create_context() | Creates a connection context. |
 * | iotc_create_iotcore_jwt() | Creates a JSON Web Token. |
 * | iotc_delete_context() | Frees the provided context. |
 * | iotc_events_process_blocking() | Invokes the event processing loop and executes the Device SDKs event engine as the main application process. |
 * | iotc_events_process_tick() | Invokes the event processing loop on RTOS or non-OS devices that must yield for standard tick operations. |
 * | iotc_events_stop() | Shuts down the event engine. |
 * | iotc_get_heap_usage() | Gets the amount of heap memory allocated to the Device SDK. |
 * | iotc_get_network_timeout() | Gets the {@link iotc_set_network_timeout() connection timeout}.
 * | iotc_get_state_string() | Gets the {@link ::iotc_state_t state message} associated with a numeric code. |
 * | iotc_initialize() | Initializes the BSP time and random number libraries. |
 * | iotc_is_context_connected() | Checks if a context is {@link iotc_connect() connected to Cloud IoT Core}. |
 * | iotc_publish() | Publishes a message to an MQTT topic. |
 * | iotc_publish_data() | Publishes binary data to an MQTT topic. |
 * | iotc_schedule_timed_task() | Executes a function after an interval and returns a unique ID for the scheduled task. |
 * | iotc_set_fs_functions() | Sets the file operations to <a href="../../bsp/html/d8/dc3/iotc__bsp__io__fs_8h.html">custom file management functions</a>. |
 * | iotc_set_maximum_heap_usage() | Sets the maximum heap memory that the Device SDK can use. |
 * | iotc_set_network_timeout() | Sets the connection timeout. |
 * | iotc_shutdown() | Shuts down the Device SDK and frees all resources created during {@link iotc_initialize() initialization}. |
 * | iotc_shutdown_connection() | Disconnects asynchronously from an MQTT broker. |
 * | iotc_subscribe() | Subscribes to an MQTT topic. |
 *
 * # Board Support Package 
 * The Device SDK calls
 * <a href="../../bsp/html/index.html">the hardware-specific drivers and
 * routines</a> in the Board Support Package (BSP) to implement the MQTT
 * protocol.
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
 * @details Shuts down the Device SDK and frees all resources created during
 * {@link iotc_initialize() initialization}. {@link Free iotc_delete_context()}
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
 * @details On POSIX systems, the client application must free the context
 * after iotc_process_blocking() returns.
 *
 * On non-POSIX systems, the application must free the context on the
 * event loop tick after the disconnection event (not in the disconnection
 * callback itself).
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
 * @details Invokes the event processing loop and executes the Device SDKs event
 * engine as the main application process. The function processes events on platforms with main application
 * loops that can block indefinitely. For other platforms, call
 * iotc_events_process_tick().
 *
 * You can call the function anytime but it returns only after calling
 * iotc_events_stop().
 * 
 * The event engine won't {@link iotc_events_process_tick() process events} when
 * the Device SDK is in the
 * {@link ::iotc_state_t IOTC_EVENT_PROCESS_STOPPED state}. If
 * the function returns IOTC_EVENT_PROCESS_STOPPED,
 * {@link iotc_shutdown() shutdown} and {@link iotc_initialize() reinitialize}
 * the Device SDK to process events again.
 */
extern void iotc_events_process_blocking();

/**
 * @details Invokes the event processing loop on RTOS or non-OS devices that
 * must yield for standard tick operations.
 *
 * The event engine won't {@link iotc_events_process_tick() process events} when
 * the Device SDK is in the
 * {@link ::iotc_state_t IOTC_EVENT_PROCESS_STOPPED state}. If
 * the function returns IOTC_EVENT_PROCESS_STOPPED,
 * {@link iotc_shutdown() shutdown} and {@link iotc_initialize() reinitialize}
 * the Device SDK to process events again.
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
 *     parameter.
 * @param [in] password The MQTT password. Cloud IoT Core requires a
 *     {@link iotc_create_iotcore_jwt() JWT}.
 * @param [in] client_id The MQTT client ID. Cloud IoT Core requires a
 *     <a href="https://cloud.google.com/iot/docs/how-tos/mqtt-bridge#device_authentication">
 *     device path</a>.
 * @param [in] connection_timeout The number of seconds to wait for a
 *     <code>CONNACK</code> response before closing the socket. If
 *     <code>0</code>, the TCP timeout is used.
 * @param [in] keepalive_timeout The number of seconds the broker will wait for
 *     the client application to send a message. If no message is sent to the
 *     broker during the interval, the broker automatically closes the
 *     connection.
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
 * @param [in] host The address at which the client connects.
 * @param [in] port The port of the host on which to connect.
 * @param [in] iotc_h A {@link iotc_create_context() context handle}.
 * @param [in] username The MQTT username.
 * @param [in] password The MQTT password.
 * @param [in] client_id The MQTT client ID.
 * @param [in] connection_timeout The number of seconds to wait for a
 *     <code>CONNACK</code> response before closing the socket. If
 *     <code>0</code>, the TCP timeout is used.
 * @param [in] keepalive_timeout The number of seconds the broker will wait for
 *     the client application to send a message. If no message is sent to the
 *     broker during the interval, the broker automatically closes the
 *     connection.
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
 * @param [in] qos The Quality of Service (QoS) level. Can be <code>0</code>,
 *     <code>1</code>, or <code>2</code>.
 * @param [in] callback (Optional) The callback function. Invoked after a
 *     message is successfully or unsuccessfully delivered.
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
 * @param [in] data A pointer to a buffer with the message payload.
 * @param [in] data_len The size, in bytes, of the message.
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
 * @details After disconnecting, the Device SDK passes the disconnection status
 * code to iotc_connect(). You may reuse disconnected contexts to reconnect
 * to Cloud IoT Core; you don't need to destroy and recreate contexts.
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
 * @param [in] iotc_user_task_callback_t The function invoked after an interval.
 *     The signature must be:
 *     {@code typedef void() iotc_user_task_callback_t(const iotc_context_handle_t
 *     context_handle, const iotc_timed_task_handle_t timed_task_handle, void
 *     *user_data)}.
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
 * @details <b>Note</b>: The Device SDK periodically creates network traffic per
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
 * @details Sets the maximum heap memory that the Device SDK can use.
 *
 * The function is part of the
 * <a href="../../../user_guide.md#memory-limiter">memory limiter</a>.
 *
 * @param [in] max_bytes The maximum amount of heap memory, in bytes, that
 *     the Device SDK can use during standard execution.
 */
iotc_state_t iotc_set_maximum_heap_usage(const size_t max_bytes);

/**
 * @details Gets the amount of heap memory allocated to the Device SDK.
 *
 * The function is part of the
 * <a href="../../../user_guide.md#memory-limiter">memory limiter</a>. If no
 * heap memory is allocated, the function runs but returns
 * IOTC_INVALID_PARAMETER.
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
