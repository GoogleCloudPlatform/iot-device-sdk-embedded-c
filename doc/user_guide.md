# Google Cloud IoT Device SDK for Embedded C User Guide

##### Copyright (C) 2018-2019 Google Inc.

This document explains how applications can use the Google Cloud IoT Device Embedded C Client to connect to Google Cloud IoT Core. It also describes the security and communication features of the client.

For a complete API reference and code samples, see the following directories in the [Google Cloud IoT Edge Embedded C Client GitHub repository](https://github.com/googlecloudplatform/iot-edge-sdk-embedded-c).

 * **`examples/`**: Includes an example of how to connect and then publish/subscribe to Cloud IoT Core MQTT topics
 * **`doc/doxygen/`**: HTML documentation for the client API and BSP functions

The first part of this user guide summarizes features and requirements. If you're ready to start using the client, review the [Typical client application workflow](#typical-client-application-workflow).


## Feature overview

The Google Cloud IoT Device Embedded C Client provides MQTT connectivity over TLS. The client architecture targets constrained devices running embedded Linux, an RTOS, or a no-OS configuration. An event system enables your applications to publish and receive MQTT messages. The client can run on a single thread with a non-blocking socket connection and can publish, subscribe and receive messages concurrently.

The Cloud IoT Device Embedded C Client offers the following features.

### Flexibility

The client scales to meet the needs of the platform.

* The client operates on a single thread and runs in non-blocking mode by default.
* The client doesn't impact the CPU while waiting for operations. Power consumption can be minimized as required.
* The client operates a thread-safe event queue and an optional thread pool for callbacks. These characteristics support robust applications and platforms beyond standard embedded environments.
* The client includes a Transport Layer Security (TLS) Board Support Package (BSP) that leverages features in embedded Wi-Fi chips and software libraries like [mbedTLS](https://tls.mbed.org/) and [wolfSSL](https://www.wolfssl.com). For more information on device security, see [Platform security requirements](#platform-security-requirements).

### Asynchronous publish/subscribe

The client uses [coroutines](http://en.wikipedia.org/wiki/Coroutine) to cocurrently handle multiple publish and subscribe requests. This facilitates multiple ongoing communications, even on no-OS devices.

* The client can simultaneously send and receive information on a single socket, and, if required, return to the OS for tick operations.
* Because the client is non-blocking, your application won't interfere with the usability of your device.

### Distributed Denial of Service (DDoS) prevention

The client includes a backoff system that uses intelligent networking behavior to prevent fleets of devices from causing unintentional DDoS attacks on Google Cloud IoT Core.

* The backoff system prevents individual clients from performing tight-loop reconnection attempts.
* Client applications are informed of pending connection attempts, disconnects, and backoff status changes.

### Abstracted implementation

The client is designed to adapt to the rapid evolution of the IoT landscape:

* The client is written in C with a Board Support Package (BSP) architecture. The client can be easily ported to support custom network stacks, operating systems, toolchains, and third-party TLS implementations.
* The client uses MQTT for client/broker communications.

### Client footprint

The client footprint is approximately 25 KB for embedded devices with optimized toolchains. This footprint includes the following components.

* Event dispatcher and scheduler
* Connection backoff system
* Board Support Package implementations (networking, time, random number generator, memory management and crypto functions)

The footprint also includes a TLS adaptation layer but doesn't include a TLS implementation itself. If the platform already has a TLS implementation, the size requirement for interfacing with it is negligible.

### MQTT v3.1.1

The Cloud IoT Device Embedded C Client uses MQTT messages to communicate over publish/subscribe topics. The client connects a TCP socket to the Cloud IoT Core MQTT bridge. The client then requests subscriptions to one or many topics via the socket. After the client subscribes to a topic, all incoming data is published to the client application on the embedded device. 

Similarly, devices can publish to one or many topics in order to perform outgoing communication with Cloud IoT Core. The device doesn't need to keep track of numerous connections, report connection state, or broadcast messages to multiple addresses. Instead, the device simply publishes a message to a topic. Cloud IoT Core automatically routes the message to the associated [subscriptions](https://cloud.google.com/iot/docs/how-tos/devices#creating_a_device_registry). Because routing and permissions are handled in the cloud, the client reduces communication overhead on the device.

* The MQTT specification defines three Quality of Service (QoS) [levels](https://cloud.google.com/iot/docs/how-tos/mqtt-bridge#quality_of_service_qos).

    * **QoS 0 (AT\_MOST\_ONCE)**: The published message is sent to the broker. No response is sent to the client, so the client has no way to confirm receipt and the message might not be delivered at all.
    * **QoS 1 (AT\_LEAST\_ONCE)**: When the message is delivered to Cloud IoT Core, a "receipt" is sent to the client. The message may be sent multiple times before it is acknowledged by the service.
    * **QoS 2 (EXACTLY\_ONCE)**: Includes acknowledgment as in QoS 1, but the message is guaranteed to reach the target only once.

Note: Google Cloud IoT Core does not support QoS 2. Visit the [MQTT Standard v3.1.1 Reference](http://docs.oasis-open.org/mqtt/mqtt/v3.1.1/os/mqtt-v3.1.1-os.html) for more information about the MQTT protocol.


### TLS Support

The Cloud IoT Device Embedded C Client includes two working implementations of its TLSv1.2 Board Support Package (BSP) for embedded devices: [mbedTLS](https://tls.mbed.org) and [wolfSSL](https://www.wolfssl.com/). The GitHub repository includes the source required for the client to interface with either of these implementations (but does not provide the source for the TLS libraries themselves).

If you have your own TLS implementation or one is included in your platform software, you can use the modular Networking and TLS BSPs. For more information on how to write and build with a custom TLS or Networking BSP, see the [porting guide](https://github.com/googlecloudplatform/iot-edge-sdk-embedded-c/blob/master/doc/porting_guide.md) and [TLS implementation requirements](#tls-implementation-requirements).


### RTOS support

The client is adapts to the platform it runs on. Applications call the client's event system to process incoming and outgoing messages.

On Linux systems, call a function that processes an event loop continuously, blocking and returning only if a shutdown or an unrecoverable error occurs. But this architecture is insufficient for no-OS (and some RTOS) devices because these devices yield control to the OS or platform software to handle execution on common tasks, ticking network stacks, timers, and similar operations.

For embedded systems, the event loop processes a small series of events before returning control to the main platform loop. The platform loop maintains system tick tasks and then invokes the client to process another series of events. This makes the client nonblocking; it handles both incoming and outgoing publishing in a tick operation.

For more information on event processing, see the functions **`iotc_events_process_blocking()`** and **`iotc_events_process_tick()`** in `include/iotc.h`.


### Backoff

The Cloud IoT Device Embedded C Client includes a backoff system that monitors connection state on the client. The client counts disconnects, tracks errant MQTT operations, and monitors healthy connection durations in order to improve recovery after service disruptions.

* A device that produces errant behavior or becomes disconnected will enter backoff mode. In this mode, connection requests are queued in the client's internal event system and processed later.
* The delay between request is determined by the backoff severity level:
    * A single disconnection event delays the device's next connection attempt by approximately one second.
    * Repeated connection issues will increase the penalty over time, up to a max of approximately 15 minutes.
    * Some randomness is added to each delay calculation in order to distribute connection attempts across a fleet of devices.

Backoff penalties don't affect devices that have successfully connected to Cloud IoT Core.

### Memory limiter

The Cloud IoT Device Embedded C Client includes an optional memory limiter that sets thresholds on the heap memory allocated during client operations. Memory is allocated and freed for various operations, including MQTT serialization/deserialization and TLS system tasks. The memory limiter sets strict contracts that ensure the client does not exceed a certain runtime heap footprint.

* If certain operations cause the client to allocate memory beyond the defined bounds, IOTC_OUT_OF_MEMORY errors are returned.
* If memory is exhausted while processing an enqueued task (send, receive, etc.), the client shuts down the connection, cleans up its allocated resources, and calls the connect callback method with an error.

The memory limiter doesn't preallocate its heap, so heap monitoring software will not register an initial jump in usage. Additionally, if memory is consumed on the device by another subsystem, heap exhaustion could still occur at the system level, thereby hanging your device.

#### Memory limiter functions

Use the following functions to work with the memory limiter. If the memory limiter is not compiled into the current client, these functions return `XI_NOT_SUPPORTED`.

**`iotc_state_t iotc_set_maximum_heap_usage( const size_t max_bytes)`**

* Sets the maximum number of bytes to be allocated on the heap during runtime operations. If a hardware-based TLS BSP implementation offloads TLS from the runtime environment, the maximum number of bytes won't include the TLS handshake.

* Returns an error if the specified limit is less than the amount of memory currently allocated or if the specified limit is smaller than the Embedded C Client's system allocation pool (see below for details).

**`iotc_state_t iotc_get_heap_usage( size_t* const heap_usage )`**

* Queries the amount of memory currently allocated for Embedded C Client operations, including buffers for ongoing MQTT transmissions, TLS encodings, and scheduled event handlers.

### Cloud IoT Device Embedded C Client system allocation reservations

The client reserves some space in memory so that it can continue operations during a memory exhaustion event. This reservation allows the client to continue processing ongoing coroutines, clean up scheduled tasks, unroll ongoing transmissions and buffers, and shut down sockets.

By default, this memory space is set to 2 KB. For example, if you run `iotc_maximum_heap_usage` to set a maximum heap size of 20 KB, 2 KB is reserved for cleanup scenarios. 18 KB are then available for all other operations.


## Platform security requirements

The client requires TLS v1.2 to securely connect to Cloud IoT Core. In addition, the embedded device must include all of the following components.

### True random number generator

During TLS handshaking, the embedded device generates a [cryptographic nonce](http://en.wikipedia.org/wiki/Cryptographic_nonce). If this nonce is predictable, the device's TLS connection can be compromised and information can be stolen, including credentials and telemetry data.

A true random number generator ensures that the nonce created during TLS handshaking is unpredictable. Random number generators seeded by the current time are not truly random, because the seed can be predicted.

### Accurate real-time clock

The embedded device must keep time in order to perform the following security processes when it initially connects with Cloud IoT Core.

* To [authenticate to Cloud IoT Core](https://cloud.google.com/iot/docs/how-tos/credentials/jwts), connecting clients create a signed JSON Web Token (JWT) that includes the current date/time. The JWT is valid for one day and must be regenerated with the accurate time on subsequent connections.
* The TLS implementation on the device checks the service's identifying certificate during TLS handshaking: the current date/time must be within the certificate's date/time validity range. If the date is outside the range, the TLS handshake is aborted and the device doesn't connect.

## TLS implementation requirements

If you want to use a TLS library other than [mbedTLS](https://tls.mbed.org) or [wolfSSL](https://www.wolfssl.com), review this section and read the [porting guide](https://github.com/googlecloudplatform/iot-edge-sdk-embedded-c/blob/master/doc/porting_guide.md) for details on configuring builds to support other TLS BSPs. All TLS implementations must meet the following requirements.

* Server certificate root CA validation
    * The TLS implementation must accept a series of root CA certificates stored on the client to determines whether the server certificate (provided at TLS handshaking) has been signed by one of the root CAs.
        * The root certificates are currently provided in the [Google root CA PEM file](https://pki.google.com/roots.pem).
    * Requires an accurate clock.

* Server domain checking
    * Wildcard support for checking the domain name of server certificates.

* [Online Certificate Status Protocol (OCSP)](http://en.wikipedia.org/wiki/Online_Certificate_Status_Protocol) (optional, highly recommended)
    * Actively used at the time of connection to determine whether a server certificate has been revoked by building a separate connection to the root certificate authority.
    * Circumvents the need for certificate revocation lists on the device.
    * Requires the use of two sockets per connection, at least for the duration of the TLS handshaking process.
    * Requires an accurate clock.\

Some versions of TLS in Wi-Fi hardware do not have all of the required features, such as OCSP support. In such cases, the best practice is to use a software library for TLS (assuming the platform has sufficient flash storage available).

## Typical client application workflow

The Google Cloud IoT Device Embedded C Client is an asynchronous communication tool. Client applications use callbacks to monitor the state of connections and subscriptions. This section summarizes the high-level steps.

Note: this section provides an overview rather than complete details. For more information, see the API reference in the `doc/doxygen` directory of the [Cloud IoT Edge Embedded C Client GitHub repository](https://github.com/googlecloudplatform/iot-edge-sdk-embedded-c/tree/master/).

### Provisioning credentials

Before you begin building a client application, [generate device credentials](https://cloud.google.com/iot/docs/how-tos/credentials/keys) with Cloud IoT Core. Make sure that the following information is available:

 - project ID
 - [device path](https://cloud.google.com/iot/docs/how-tos/mqtt-bridge)
 - private key (e.g., `./ec_private.pem`)
 - path to Google's [roots.pem](https://pki.google.com/roots.pem) file (e.g., `./roots.pem`)

### Step 1: Create a context

A Cloud IoT Edge Embedded C context represents a socket connection with the Cloud IoT Core service.

To create a context, call **`iotc_create_context()`**. Then, follow the instructions below to pass the context to the **`iotc_connect`** function and connect to Cloud IoT Core.

### Step 2: Connect

To connect to Cloud IoT Core, call **`iotc_connect()`**. This function enqueues an event that requests a socket connection with Cloud IoT Core. The event is processed in the subsequent tick of the client's event processor. Executing the **`iotc_connect()`** function initiates the following operations.

* Domain name resolution of the Cloud IoT Core service
* Building the TCP/IP socket connection
* TLS handshaking and certificate validation
* MQTT credential handshaking

Note: the call to connect returns immediately. The connection operation is fulfilled on subsequent calls to the client's event processing functions, as described in [**Step 3: Process Events**](step-3-process-events).

#### Connect callback

When the **`iotc_connect()`** function runs, the Cloud IoT Device SDK initalizes a callback function. The callback function is invoked when a connection to Cloud IoT Core is established or when the connection is unsuccessful. The callback function is also invoked when an established connection is lost or shut down. See [**Step 6: Disconnect and Shut Down**](step-6-disconnect-and-shut-down) for details.

### Step 3: Process events

After a connection is established (indicated by the client invoking the connect callback function), the client application automatically subscribes to the Cloud Pub/Sub topics [associated with the device](https://cloud.google.com/iot/docs/how-tos/devices).

To manually enqueue a subscription request, call **`iotc_subscribe()`**. [Subscription requests](https://github.com/GoogleCloudPlatform/iot-device-sdk-embedded-c/blob/5e6fa0111690d01bc048f3e91570aeabbe284163/src/libiotc/iotc.c) must include the topic name, a subscriptions callback function, and a QoS level.

#### Topics

There is no limit of the number of topics an application can subscribe to. However, each subscription has memory overhead.

#### Subscription callback

The Cloud IoT Device Embedded C CLient invokes a subscription callback function when Cloud IoT Core acknowledges an outgoing subscription request or when the client recieves an incoming message on a subscribed topic.

Incoming messages include the topic data as part of the callback parameters. Specify a callback function for each subscription. You can either use the same subscription callback function for multiple topics, or have a unique callback function for each topic your client application subscribes to.

#### QoS level

The [specified QoS level](#MQTT v3.1.1) is the maximum QoS level of incoming messages. For instance, if you set the QoS level to 0, QoS 1 messages are downgraded to QoS 0. 

The QoS level also affects the client's memory overhead. QoS 2 messages use the most memory overhead, while QoS 0 messages use the least.

### Step 5: Publish

One the embedded device is connected to Cloud IoT Core, the client can publish messages.

To publish a message to a topic, run **`iotc_publish()`** or **`iotc_publish_data()`** functions. Each message must include the following parameters.

* The topic name
* A message payload
* A QoS level

#### Publish callback

The client can also supply a function pointer to a publish callback. This callback function is optional; it notifies embedded devices when messages are sent from the client, the messages are acknowledged by Cloud IoT Core and the client deletes buffered data.

This callback function is for environments with severe memory restrictions. For example, the callback function helps gate pending publications and helps track the status of large messages in order to free up resources after the messages are published.

### Step 6: Disconnect and shut down

To disconnect from Cloud IoT Core, invoke the **`iotc_shutdown_connection()`** function. This function enqueues an event that cleanly closes the socket connection. After the connection is terminated, the client invokes the [connect callback](#Step 2: Connect) function.

Note: Do not delete the context until the the connect callback is invoked.

#### Disconnection types and reconnecting

The client checks the parameters of the connect callback function to determine if a device was intentionally or incidentally disconnected. See the [**Connect callback**](#connect-callback) section for more information.

If an error disconnects a device, the client immediately calls the connect callback function again from within the ongoing connect callback in order to que a new [connection request](#Step 2: Connect). the client application can call connect again immediately from within the connect callback.

If the device is intentionally disconnected, it retains the [existing context](#Step 1: Create a context).

#### Freeing memory and shutting down

To free memory after intentially disconnecting a device, invoke the **`iotc_delete_context`** to clean the context. After deleting all contexts, call **`iotc_shutdown()`** to free more memory.

If memory needs to be freed after an intentional disconnection, the context can be cleaned up by invoking **`iotc_delete_context`**. Further memory can be freed by calling **`iotc_shutdown()`**, but only after all contexts have been deleted.

Don't call the **`iotc_shutdown()`** function on every disconnection event because the **`iotc_shutdown()`** the destroys the backoff status cache that guards Cloud IoT Core service from accidental DDoS attacks by field devices. For more information, see [Backoff](#backoff).

Note: In blocking mode, stop the event loop in the disconnection callback and then delete the context. In a ticking event loop, delete the context in the next tick after disconnecting.

## Standard operation callbacks (connect, subscribe, publish, and shutdown)

The network dependent functions—connect, subscribe, publish, and shutdown—are asynchronous. Although these functions return errors immediately after sanity checks fail, normal operation queues requests in the Device SDK. Requests are then processed in the next iteration of the event processor.

To determine whether a request was successful, provide a pointer to a callback function. These callback functions are detailed below.

Note: The connect, publish, and shutdown callbacks share the same function signature. The subscribe callback function requires more parameters. This section details the parameters in each callback scenario.

### Connect callback

The connect callback function tracks the current state of the connection process and monitors disconnection behavior.

#### Function signature

```
void
on_connection_state_changed( iotc_context_handle_t in_context_handle, void* data, iotc_state_t state )
```

#### Function parameters

**`in_context_handle`**

Tracks the state of the ongoing connection and queues new events from within the callback.

**`data`**

Cast this as `iotc_connection_data_t*` and observe the `connection_state` to determine the type of connection state change. Possible responses are listed below.

* **IOTC_CONNECTION_STATE_OPENED:** The connection successfully opened and TLS handshaking occurred.
* **IOTC_CONNECTION_STATE_OPEN_FAILED:** The connection failed. The state parameter is set to an `iotc_err` that explains why the connection failed.
* **IOTC_CONNECTION_STATE_CLOSED:** A previously opened connection was shut down. If the state parameter is `IOTC_STATE_OK`, the disruption was due to the client application queuing a shutdown request via `iotc_shutdown_connection()`. Otherwise, the connection was closed either because of a network interruption or because the Cloud IoT Core service encountered errant client behavior or an expired JWT.

**`state`**

IOTC_CONNECTION_STATE_CLOSED messages use this parameter when the device was intentially disconnected and the response is `IOTC_STATE_OK`. Other state values indicate that a connection error occurred.

### Subscription callback

The subscription callback is invoked when Cloud IoT Core sends a MQTT SUBACK response with the granted QoS level of the subscription request. This function is also invoked each time Cloud IoT Core delivers a message to the subscribed topic.

#### Function signature

```
void on_message( iotc_context_handle_t in_context_handle,
                 iotc_sub_call_type_t call_type,
                 const iotc_sub_call_params_t* const params,
                 iotc_state_t state,
                 void* user_data )
```

#### Function parameters

**`in_context_handle`**

Tracks the state of the ongoing connection and queues new events from within the callback.

**`call_type`**

Indicates the reason for invoking the function. Possible responses are listed below.

* **IOTC_SUB_CALL_SUBACK:** The callback provides a status update of subscription process.
* **IOTC_SUB_CALL_MESSAGE:** The callback invocation carries the payload data of the message sent by Cloud IoT Core.
* **IOTC_SUB_CALL_UNKNOWN:** Signifies a serious issue; data might be corrupted. Report an error but do not take other action.


**`params`**

This parameter holds a pointer to a union that contains either subscription status information (if a change in the status occurs) or the payload of a incoming message.

If `IOTC_SUB_CALL_SUBACK` is set, the **`params suback`** structure's fields are as follows.

* const char\* **topic**
    * Contains the topic name this invocation is related to
* iotc_mqtt_suback_status_t **suback_status**
    * May be one of:
        - IOTC_MQTT_QOS_0_GRANTED
        - IOTC_MQTT_QOS_1_GRANTED
        - IOTC_MQTT_QOS_2_GRANTED
        - IOTC_MQTT_SUBACK_FAILED

If `call_type` is set to `IOTC_SUB_CALL_MESSAGE`, **`params`** holds a **`message`** structure. This structure type can include the following information.

* const char\* **topic**
    - Topic name the message arrived on
* const uint8_t\* **temporary_payload_data**
    - Holds memory of length temporary_payload_data_length bytes, to be freed after the invocation is completed. The data must be copied in order to be retained.
* size_t **temporary_payload_data_length**
    - The length of the temporary_payload_data array
* iotc_mqtt_retain_t **retain**
    - Can be IOTC_MQTT_RETAIN_FALSE or IOTC_MQTT_RETAIN_TRUE
* iotc_mqtt_qos_t **qos**
    - Can be one of: IOTC_MQTT_QOS_AT_MOST_ONCE, IOTC_MQTT_QOS_AT_LEAST_ONCE, IOTC_MQTT_QOS_EXACTLY_ONCE
* iotc_mqtt_dup_t **dup_flag**
    - Can be one of: IOTC_MQTT_DUP_FALSE, IOTC_MQTT_DUP_TRUE

**`state`**

Provides information about the state of the execution. If the value is something other than IOTC_STATE_OK, an error occurred and it should be reported.

**`user_data`**

The `void* data` variable can be associated with each subscription request. This allows a single callback function to handle multiple message subscriptions.

Tip: This data can contain application-specific information or it can differentiate channels more efficiently than by comparing topic names.

### Publication callback

The publication callback is invoked when the Device SDK successfully publishes to Cloud IoT Core. This callback helps devices with low memory capacity gate the publication rate. The publication callback accepts a `void* data` variable to mark the specific publication that this callback is tracking.

#### Function signature

```
void
on_publication( iotc_context_handle_t in_context_handle, void* data, iotc_state_t state )
```

#### Function parameters

**`in_context_handle`**

Tracks the state of the ongoing connection and queues new events from within the callback.

**`data`**

Always NULL.

**`state`**

Returns IOTC_STATE_WRITTEN after successful publications. Otherwise, this parameter returns an `iotc_state_t` error.

### Event system

The Device SDK has its own event processor that handles asynchronous communications (such as publishing, subscribing, and connecting). To use this event system to schedule callbacks, pass a function pointer, a valid context, and the elapsed time in seconds to the respective function.

The event systems helps arrange periodic publications, set timeout timers or schedule new connections.

#### Function signature

Callback functions require three parameters.

* **`context_handle`**: Created via the `iotc_create_context()` function call
* **`timed_task_handle`**: For identifying the function
* **`user_data`**: Any data you want to pass to the function

For example, the following function call executes periodic communications.

```
void scheduled_function( const iotc_context_handle_t context_handle,
                         const iotc_timed_task_handle_t timed_task_handle,
                         void* user_data )
{
    /* things to do. */
}
```

#### Scheduling an event

Functions can execute only once or repeat indefinitely. Store an event handle to cancel a schedule event before it backfires or stop repeating function calls.

The example below executes a task that repeats indefinitely.


`/* You need to create this context handle only once in the application lifespan */`

`iotc_context_handle_t iotc_context_handle = iotc_create_context();`

`/* Schedule a callback to scheduled_function() defined above */`

```
iotc_timed_task_handle_t timed_task_handle
    = iotc_schedule_timed_task( iotc_context_handle
         , &scheduled_function
         , 5 /* 5 seconds */
         , 1 /* It will be called repeatedly forever.
                Pass 0 if you need your function called only once */
         , NULL /* User defined value, not used in this example. */ );
```

#### Canceling an Event

The timed task handle cancels upcoming event callbacks.

`/* Assumes a timed_task_handle was set when the callback was scheduled */`

```
if( IOTC_INVALID_TIMED_TASK_HANDLE != timed_task_handle )
{
    iotc_cancel_timed_task( timed_task_handle );
}
```


#### Sample usage

Visit the `examples/` directory for an sample implementation of a scheduled callback. The sample schedules a callback to `delayed_publish()`, connects to Cloud IoT Core, and then publishes a message every five seconds.

## Learn more

### Additional documentation

The following documentation is also available:

* [Google Cloud IoT Device SDK for Embedded C Releases](https://github.com/googlecloudplatform/iot-device-sdk-embedded-c/releases)
* [Google Cloud IoT Device SDK for Embedded C GitHub](https://github.com/googlecloudplatform/iot-device-sdk-embedded-c)
* [Google Cloud IoT Device SDK for Embedded C Porting Guide](https://github.com/googlecloudplatform/iot-device-sdk-embedded-c/blob/master/doc/porting_guide.md)
* [Google Cloud IoT Core Quickstart](https://cloud.google.com/iot/docs/quickstart)
* Google Cloud IoT Device SDK for Embedded C API reference, in `doc/doxygen/api` within the GitHub repo.
* Google Cloud IoT Device SDK for Embedded C BSP reference, in `doc/doxygen/bsp` within the GitHub repo.


### External resources

#### MQTT

* [MQTT Wikipedia](https://en.wikipedia.org/wiki/MQTT)
* [MQTT 3.1.1 Specification](http://docs.oasis-open.org/mqtt/mqtt/v3.1.1/os/mqtt-v3.1.1-os.html)

#### TLS security

* Transport Layer Security (TLS)
    - [TLS v1.2](https://tools.ietf.org/html/rfc5246)
* [Cryptographic Nonce](http://en.wikipedia.org/wiki/Cryptographic_nonce)
* [Online Certificate Status Protocol (OCSP)](http://en.wikipedia.org/wiki/Online_Certificate_Status_Protocol)
* [OCSP Stapling](http://en.wikipedia.org/wiki/OCSP_stapling)

## Glossary

### Backoff

A state in which the Device SDK delays connection attempts to prevent accidental Distributed Denial of Service attacks.

### Client application

Software written for the target platform. Resides above the Device SDK for Embedded C on the application software stack. The client application calls into the Google Cloud IoT Device SDK for Embedded C to fulfill Cloud IoT Core communication requests.

### Google Cloud IoT Device SDK for Embedded C

Software written in C and intended for embedded devices. The Device SDK connects client applications to Cloud IoT Core.
