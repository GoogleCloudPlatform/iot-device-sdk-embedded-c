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
 * @brief Connects client applications to Cloud IoT Core
 *
 * \mainpage Google Cloud IoT Device SDK for Embedded C API
 *
 * The Device SDK securely connects client applications to
 * <a href="https://cloud.google.com/iot-core/">Cloud IoT Core</a> in order to
 * publish and subscribe to messages via
 * <a href="http://docs.oasis-open.org/mqtt/mqtt/v3.1.1/os/mqtt-v3.1.1-os.html">
 * MQTT</a>.
 *
 * # Getting started
 * To
 * <a href="https://github.com/GoogleCloudPlatform/iot-device-sdk-embedded-c#building">build</a>
 * the Device SDK:
 *     1. {@code git clone https://github.com/GoogleCloudPlatform/iot-device-sdk-embedded-c.git}
 *     2. {@code cd iot-device-sdk-embedded-c}
 *     3. (Optional) Customize the <a href="#dependencies">Board Support Package
 *        </a> for your device.
 *     4. {@code make}
 *
 * ## Device SDK functions
 * You can use the Device SDK API to create a client application that connects 
 * to and communicates with MQTT brokers. The
 * <a href="../../../user_guide.md">user guide</a> demonstrates the
 * <a href="../../../user_guide.md#typical-client-application-workflow">typical
 * client application workflow</a>. It highlights the following Device SDK
 * functions:
 *
 * **Connecting to Cloud IoT Core**
 *    * {@code iotc_initialize()}
 *    * {@code iotc_create_context()}
 *    * {@code iotc_connect(iotc_context_handle_t iotc_h,
 *                             const char *username,
 *                             const char *password,
 *                             const char *client_id,
 *                             uint16_t connection_timeout,
 *                             uint16_t keepalive_timeout,
 *                             iotc_user_callback_t *client_callback)}
 *
 * **Publishing and subscribing to messages**
 *    * {@code iotc_publish(iotc_context_handle_t iotc_h,
 *                             const char *topic,
 *                             const char *msg,
 *                             const iotc_mqtt_qos_t qos,
 *                             iotc_user_callback_t *callback,
 *                             void *user_data)}
 *    * {@code iotc_publish_data(iotc_context_handle_t iotc_h,
 *                                  const char *topic, const uint8_t *data,
 *                                  size_t data_len,
 *                                  const iotc_mqtt_qos_t qos,
 *                                  iotc_user_callback_t *callback,
 *                                  void *user_data)}
 *    * {@code iotc_subscribe(iotc_context_handle_t iotc_h,
 *                               const char *topic, 
 *                               const iotc_mqtt_qos_t qos, 
 *                               iotc_user_subscription_callback_t *callback,
 *                               void *user_data)}
 *
 * See the <a href="globals_func.html">methods index</a> for a complete
 * reference.
 *
 * # Dependencies 
 * The Device SDK calls
 * <a href="../../bsp/html/index.html">the hardware-specific drivers and
 * routines</a> in the Board Support Package (BSP) to implement the MQTT
 * protocol. The BSP is a part of the Device SDK.
 *
 * The Device SDK has a turn-key POSIX BSP, so it builds natively on POSIX
 * platforms. Customize the BSP to build the Device SDK on non-POSIX platforms.
 */

/* -----------------------------------------------------------------------
 * MAIN LIBRARY FUNCTIONS
 * ----------------------------------------------------------------------- */

/**
 * @details Initializes the BSP time and random number libraries. Applications
 * must call the function first when starting a new runtime.
 *
 * @retval IOTC_STATE_OK The libraries were initialized.
 */
extern iotc_state_t iotc_initialize();

/**
 * @details Shuts down the Device SDK and frees all resources created during
 * {@link iotc_initialize() initialization}. {@link Free iotc_delete_context()}
 * all contexts before calling the function.
 *
 * @retval IOTC_STATE_OK The Device SDK was shut down.
 *
 * @see iotc_shutdown_connection
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
 *
 * @retval IOTC_STATE_OK The context was freed.
 * @retval IOTC_INVALID_PARAMETER The {@link iotc_create_context() context handle} is invalid.
 */
extern iotc_state_t iotc_delete_context(iotc_context_handle_t context_handle);

/**
 * @brief Determines if a context is
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
 * You can call the function anytime but it returns only after calling iotc_events_stop().
 * 
 * The event engine won't {@link iotc_events_process_tick() process events} when
 * the Device SDK is in the
 * {@link ::iotc_state_t IOTC_EVENT_PROCESS_STOPPED state}. If
 * returns IOTC_EVENT_PROCESS_STOPPED is returned,
 * {@link iotc_shutdown() shutdown} and {@link iotc_initialize() reinitialize}
 * the Device SDK to process events again.
 */
extern void iotc_events_process_blocking();

/**
 * @details Invokes the event processing loop. The function is for RTOS or non-OS
 * devices that must yield for standard tick operations.
 *
 * The event engine won't {@link iotc_events_process_tick() process events} when
 * the Device SDK is in the
 * {@link ::iotc_state_t IOTC_EVENT_PROCESS_STOPPED state}. If the function
 * returns IOTC_EVENT_PROCESS_STOPPED, {@link iotc_shutdown() shutdown} and
 * {@link iotc_initialize() reinitialize} the Device SDK to process events
 * again.
 *
 * @retval IOTC_STATE_OK The event system is ongoing and can continue to
 *     operate.
 * @retval IOTC_EVENT_PROCESS_STOPPED The event processor stopped because the
 *     client application {@link iotc_events_stop() shut down the event engine}
 *     or an unrecoverable error occurred.
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
 *
 * @retval IOTC_STATE_OK The connection request was enqueued.
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
 *
 * @retval IOTC_STATE_OK The publication request was enqueued.
 * @retval IOTC_OUT_OF_MEMORY The platform doesn't have enough memory
 *     to fulfull the request.
 * @retval IOTC_BACKOFF_TERMINAL The backoff was applied.
 * @retval IOTC_INTERNAL_ERROR Something went wrong.
 */
extern iotc_state_t iotc_publish(iotc_context_handle_t iotc_h,
                                 const char* topic, const char* msg,
                                 const iotc_mqtt_qos_t qos,
                                 iotc_user_callback_t* callback,
                                 void* user_data);

/**
 * @brief Publishes binary data to an MQTT topic.
 *
 * The function accepts a pointer and data length, whereas
 * iotc_publish() accepts a null-terminated string.
 *
 * @param [in] data The message payload.
 * @param [in] data_len The size, in bytes, of the message.
 *
 * @retval IOTC_STATE_OK The publication request was enqueued.
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
 *
 * @retval IOTC_STATE_OK The subscription request was enqueued.
 * @retval IOTC_OUT_OF_MEMORY The platform doesn't have enough memory
 *     to fulfull the request.
 * @retval IOTC_INTERNAL_ERROR An unrecoverable error occurred.
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
 *
 * @retval IOTC_STATE_OK The disconnection request was enqueued.
 * @retval IOTC_SOCKET_NO_ACTIVE_CONNECTION_ERROR The device isn't connected to
 *     an MQTT broker.
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
 * @brief Cancels a timed task by removing it from the internal event system.
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
 **/
extern void iotc_set_network_timeout(uint32_t timeout);

/**
 * @brief Queries the current
 * {@link iotc_set_network_timeout() network timeout value}.
 **/
extern uint32_t iotc_get_network_timeout(void);

/**
 * @details Sets the maximum heap memory that the Device SDK can use. The
 * function is part of the Device SDK <a
 * href="../../../user_guide.md#memory-limiter">memory limiter</a>.
 *
 * @param [in] max_bytes The maximum amount of heap memory, in bytes, that
 *     the Device SDK can use during standard execution.
 *
 * @retval IOTC_STATE_OK The new memory limit was set.
 * @retval IOTC_NOT_SUPPORTED The memory limiter module isn't compiled into the
 *     Device SDK.
 * @retval IOTC_OUT_OF_MEMORY The new memory limit is too small to support
 *     the current heapspace footprint.
 */
iotc_state_t iotc_set_maximum_heap_usage(const size_t max_bytes);

/**
 * @details Queries the amount of heap memory allocated to the Device SDK. The
 * function is part of the Device SDK <a
 * href="../../../user_guide.md#memory-limiter">memory limiter</a>.
 *
 * @retval IOTC_STATE_OK The current heap usage was queried.
 * @retval IOTC_NOT_SUPPORTED The memory limiter module isn't installed.
 * @retval IOTC_INVALID_PARAMETER No heap memory allocated.
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
