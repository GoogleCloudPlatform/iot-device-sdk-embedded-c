# Google Cloud IoT Device SDK for Embedded C User Guide

##### Copyright (C) 2018-2019 Google Inc.

This document explains how applications can use the Google Cloud IoT Device SDK for Embedded C to connect to Google Cloud IoT Core. It also describes the security and communication features of the SDK.

This User Guide does not provide full API details or code examples. For more information, see the following directories in the [Google Cloud IoT Device SDK for Embedded C Client GitHub repository](https://github.com/googlecloudplatform/iot-device-sdk-embedded-c):

 * **`examples/`**: Includes an example of how to connect and then publish/subscribe to Cloud IoT Core MQTT topics
 * **`doc/doxygen/`**: HTML documentation for the SDK's API and BSP functions

The first part of this document summarizes features and requirements. If you're ready to start using the SDK, review the [Typical SDK application workflow](#typical-sdk-application-workflow).


## Feature overview

The Google Cloud IoT Device SDK provides MQTT connectivity over TLS. The SDK's architecture targets constrained devices running embedded Linux, an RTOS, or a no-OS configuration. An event system is provided to enable your applications to publish and receive MQTT messages asynchronously. The client can run on a single thread with a non-blocking socket connection and can publish, subscribe and receive messages concurrently.

This Cloud IoT Device SDK offers the following features:

### Flexibility

The SDK scales to meet the needs of the platform:

* Operates on a single thread and runs in non-blocking mode by default.
* Does not impact the CPU while waiting for operations. Power consumption can be minimized as required.
* Has a thread-safe event queue and an optional thread pool for callbacks. These support robust applications and platforms beyond standard embedded environments.
* Includes a Transport Layer Security (TLS) Board Support Package (BSP) that can be used to leverage features in embedded Wi-Fi chips, as well as software libraries like [mbedTLS](https://tls.mbed.org/) and [wolfSSL](https://www.wolfssl.com). For more information, see [Platform security requirements](#platform-security-requirements).

### Asynchronous publish/subscribe

Through the use of [coroutines](http://en.wikipedia.org/wiki/Coroutine), the client can handle multiple publish and subscribe requests concurrently. This facilitates multiple ongoing communications, even on no-OS devices.

* The client can send and receive simultaneously on a single socket, and, if required, return to the OS for tick operations.
* Because the client is non-blocking, your client application will not interfere with the usability of your device.

### Distributed Denial of Service (DDoS) prevention

The client includes a backoff system that uses intelligent networking behavior to prevent fleets of devices from causing unintentional DDoS attacks on Google Cloud IoT Core.

* Prevents individual clients from performing tight-loop reconnection attempts.
* Client applications are informed of pending connection attempts, disconnects, and backoff status changes.

### Abstracted implementation

The SDK is designed to adapt to the rapid evolution of the IoT landscape:

* Written in C with a Board Support Package (BSP) architecture: can be easily ported to support custom network stacks, operating systems, tool chains, and third-party TLS implementations.
* Uses MQTT for client/broker communications, with the ability to support other IoT protocols in the future.

### Client footprint

The client footprint is approximately 25 KB for embedded devices with optimized tool chains. This footprint includes:

* the event dispatcher and scheduler,
* the connection backoff system,
* the Board Support Package implementations (networking, time, random number generator, memory management and crypto functions.)

The footprint also includes a TLS adaptation layer but does not include a TLS implementation itself. If the platform already has a TLS implementation, the size requirement for interfacing with it is negligible.

### MQTT v3.1.1

[MQTT Standard v3.1.1 Reference](http://docs.oasis-open.org/mqtt/mqtt/v3.1.1/os/mqtt-v3.1.1-os.html)

The Cloud IoT Device SDK uses MQTT messages to communicate over a publish/subscribe topic structure. The client connects a TCP socket to the Cloud IoT Core MQTT bridge and uses that socket to request subscriptions to one or many topics. After the client subscribes, all data published to those topics is delivered to the client application on the embedded device. Similarly, devices can publish to one or many topics to communicate to listeners through Cloud IoT Core.

In the pub/sub paradigm, the device can be relatively unaware of listeners. It doesn't need to keep track of numerous connections, report connection state, or broadcast messages to multiple addresses. Instead, the device simply publishes a message to a topic and lets Cloud IoT Core route the message to any and all listeners.

* The MQTT specification defines three Quality of Service (QoS) levels:

    * **QoS 0 (AT\_MOST\_ONCE)**: The published message is sent to the broker. No response is sent to the client, so the client has no way to confirm receipt and the message might not be delivered at all.
    * **QoS 1 (AT\_LEAST\_ONCE)**: When the message is delivered to Cloud IoT Core, a "receipt" is sent to the client. The message may be sent multiple times before it is acknowledged by the service.
    * **QoS 2 (EXACTLY\_ONCE)**: Includes acknowledgment as in QoS 1, but the message is guaranteed to reach the target only once.

Note: While this SDK supports QoS 2, the Google Cloud IoT Core does not.

With the MQTT toolbox, embedded devices can subscribe to and publish to various topics, and messages are automatically routed to the correct recipients based on the current subscriptions and permissions set. Because routing and permissions are handled in the cloud, communication overhead is reduced on the device.


### TLS Support

The Cloud IoT Device SDK includes two working implementations of its TLSv1.2 Board Support Package (BSP) for embedded devices: [mbedTLS](https://tls.mbed.org) and [wolfSSL](https://www.wolfssl.com/). The GitHub repository includes the source required to interface the Cloud IoT Device SDK with either of these implementations (but does not provide the source for the TLS libraries themselves).

If you wish to use another hardware or software based TLS you can use the modular Networking and TLS BSPs to invoke it instead. For more information on how to write and build with a custom TLS or Networking BSP, see the [Cloud IoT Device SDK for Embedded C Client Porting Guide](https://github.com/googlecloudplatform/iot-device-sdk-embedded-c/blob/master/doc/porting_guide.md). Be sure to also refer to the [TLS implementation requirements](#tls-implementation-requirements) in this document.


### RTOS support

The SDK was designed to be adaptable to the platform it's running on. Applications call the SDK's event system to process incoming and outgoing messages.

On Linux systems, you may call a function that processes an event loop continuously, blocking and returning only if shutdown or an unrecoverable error occurs. But this architecture is insufficient for no-OS and some RTOS devices. These devices need to yield control to the OS or platform software to handle execution on a common task, ticking network stacks, timers, and so on.

For these limited systems, the event loop processes a small series of events before returning control to the main platform loop. The platform loop can maintain system tick tasks, and then invoke the SDK to process another series of events.

For more information, see the functions **`iotc_events_process_blocking()`** and **`iotc_events_process_tick()`** in `include/iotc.h`.


### Backoff

Deploying a large fleet of connected devices can be problematic. If, for example, a Cloud IoT Core service disruption were to occur, millions of devices could attempt to reconnect to the service simultaneously. This would tax the service heavily, and the high load would make recovery even more difficult.

In order to avoid such scenarios, the Cloud IoT Device SDK includes a backoff system that monitors connection state on the device. It counts disconnects and errant MQTT operations, and monitors healthy connection durations to determine if a device might contribute to a DDoS attack.

* A device that produces errant behavior or becomes disconnected will enter backoff mode. In this mode, connection requests are queued in the client's internal event system and processed later.
* The delay in fulfilling the request is determined by the backoff severity level:
    * A single disconnection event might delay the device's next connection attempt by only a second.
    * Repeated connection issues will increase the penalty over time, up to a max of approximately 15 minutes (using a standard penalty growth schedule).
    * Some randomness is added to each delay calculation, to help distribute connection attempts across a fleet of devices.
    * Penalties diminish over time if a connection is healthy.

Backoff penalties do not affect devices that have successfully connected to Cloud IoT Core, only in reconnection attempts.

### Memory limiter

When working with restricted systems, memory usage is always an issue. The Cloud IoT Device SDK includes an optional memory limiter that sets thresholds on the heap memory allocated during client operations. Memory is allocated and freed for various operations, including MQTT serialization/deserialization operations, TLS system tasks, and so on. By setting memory limits, you create a strict contract which ensures the client does not exceed a certain runtime heap footprint.

* If certain operations would cause the client to allocate memory beyond the defined bounds, IOTC_OUT_OF_MEMORY errors are returned.
* If the memory exhaustion event occurs while processing an already queued task (send, receive, etc.), the client shuts down the connection, cleans up its allocated resources, and calls the connect callback method with an error.

Note: The memory limiter does not preallocate its heap, so heap monitoring software will not register an initial jump in usage. Additionally, if memory is being consumed on the device by another subsystem, heap exhaustion could still occur at the system level, hanging your device.

#### Memory limiter functions

Use the following functions to work with the memory limiter. Note: if the memory limiter was not compiled into the current client, these functions will return `XI_NOT_SUPPORTED`.

**`iotc_state_t iotc_set_maximum_heap_usage( const size_t max_bytes)`**

Sets the maximum number of bytes to be allocated on the heap during runtime operations. Depending on the TLS BSP implementation, this may include the TLS handshake.

Returns an error if the specified limit is less than the amount of memory currently allocated or smaller than the IoT Device SDK's system allocation pool (see below for details).

**`iotc_state_t iotc_get_heap_usage( size_t* const heap_usage )`**

Queries the amount of memory currently allocated for IoT Device SDK operations, including buffers for ongoing MQTT transmissions; TLS encodings; and scheduled event handlers.

### Cloud IoT Device SDK system allocation reservations

The SDK reserves some space in memory so that it can continue operations during a memory exhaustion event. This reservation allows the SDK to continue processing ongoing coroutines, clean up scheduled tasks, unroll ongoing transmissions and buffers, and shut down sockets properly.

By default, this memory space is set to 2 KB. For example, if you run `iotc_maximum_heap_usage` to set a max heap size of 20 KB, 2 KB will still be reserved for cleanup scenarios, resulting in 18 KB being available for all other operations -- including creating connections, sending and receiving buffers, and TLS processing.


## Platform security requirements

The IoT Device SDK requires TLS v1.2 to securely connect to the Cloud IoT Core service. However, simply connecting with TLS isn't sufficient. For a truly secure connection, the following embedded device components must be in place.

### True random number generator

During TLS handshaking, a [cryptographic Nonce](http://en.wikipedia.org/wiki/Cryptographic_nonce) must be generated by the embedded device. If this nonce is predictable, the device's TLS connection can be compromised and data can be stolen, including credentials and device sensor data.

A true random number generator ensures that the nonce created during TLS handshaking is unpredictable.

Note: Random number generators seeded by the current time are not truly random, because the seed can be predicted.

### Accurate real-time clock

The device's ability to know the time is essential for two major security processes which occur during the initial connection:

* In order to [authenticate to Cloud IoT Core](https://cloud.google.com/iot/docs/how-tos/credentials/jwts), connecting clients must create a signed JSON Web Token (JWT) that includes the current date/time. The JWT is valid for only one day and must be regenerated with the accurate time on subsequent connections.
* The TLS implementation on the device checks the service's identifying certificate during TLS handshaking: the current date/time must be within the certificate's date/time validity range. If the date is outside the range, the TLS handshake is aborted and the device cannot connect.

## TLS implementation requirements

If you want to use a TLS library other than [mbedTLS](https://tls.mbed.org) or [wolfSSL](https://www.wolfssl.com), review this section to ensure that your TLS implementation meets the requirements. See also the [Client Porting Guide](https://github.com/googlecloudplatform/iot-device-sdk-embedded-c/blob/master/doc/porting_guide.md) for information on configuring builds to support other TLS BSPs.

Note: Some versions of TLS in Wi-Fi hardware do not have all of the required features, such as OCSP support. In such cases, the best practice is to use a software library for TLS (assuming the platform has sufficient flash storage available.)

* Server certificate root CA validation
    * The TLS implementation must accept a series of root CA certificates stored on the client. These are used to determine whether the server certificate (provided at TLS handshaking) has been signed by one of the root CAs.
        * The root certificates are currently provided in the [Google root CA PEM file](https://pki.google.com/roots.pem).
    * Requires an accurate clock.

* Server domain checking
    * Wildcard support for domain name checks of server certificates.

* [Online Certificate Status Protocol (OCSP)](http://en.wikipedia.org/wiki/Online_Certificate_Status_Protocol) - Highly recommended
    * Actively used at the time of connection to determine whether a server certificate has been revoked by building a separate connection to the root certificate authority.
    * Circumvents the need for certificate revocation lists on the device.
    * Requires the use of two sockets per connection, at least for the duration of the TLS handshaking process.
    * Requires an accurate clock.


## Typical SDK client application workflow

The Google Cloud IoT Device SDK is an asynchronous communication tool. In order to maintain a communication channel with Cloud IoT Core, client applications must use callbacks to monitor the state of connections and subscriptions. This section summarizes the high-level steps.

Note that this section provides an overview rather than complete details. For more information, see the API reference in the `doc/doxygen` directory of the [Cloud IoT Device SDK for Embedded C Client GitHub repository](https://github.com/googlecloudplatform/iot-device-sdk-embedded-c/tree/master/).

**Note: Provisioning credentials**

Before you begin, you must generate device credentials as described in [Creating Public/Private Key Pairs](https://cloud.google.com/iot/docs/how-tos/credentials/keys). Make sure that you have the following information readily available:

 - project ID
 - [device path](https://cloud.google.com/iot/docs/how-tos/mqtt-bridge)
 - private key (e.g., `./ec_private.pem`)
 - path to Google's [roots.pem](https://pki.google.com/roots.pem) file (e.g., `./roots.pem`)

Additional information about callbacks is provided after the summary steps.

### Step 1: Create a context

A Cloud IoT Device SDK context represents a socket connection to the  Cloud IoT Core service. All pending messages, incoming MQTT messages, subscription callbacks function pointers, and so on are associated with the context.

Contexts are created by calling **`iotc_create_context()`**. Creating the context does not connect you to the Cloud IoT Core service directly. To start the connection process, provide the context to the **`iotc_connect`** function, as described below.

### Step 2: Connect

To create a connection, call **`iotc_connect()`**. This function enqueues an event that, once processed, attempts a connection to Cloud IoT Core. The event will be processed in the next tick of the client's event processor and will kick off a series of operations, including the following:

* Domain name resolution of the Cloud IoT Core service
* Building the TCP/IP socket connection
* TLS handshaking and certificate validation
* MQTT credential handshaking

Note: The call to connect returns immediately. The connection operation is fulfilled on subsequent calls to the client's event processing functions, as described in [**Step 3: Process Events**](step-3-process-events).

#### Connect callback

When making a connection request, your client operation must provide a callback function. This function is invoked when a connection to the Cloud IoT Core service has been established, as well as when the connection was unsuccessful, disrupted, or purposefully shutdown. See [**Step 6: Disconnect and Shut Down**](step-6-disconnect-and-shut-down) for details.

### Step 3: Process events

After a connection has been established (as indicated by the client's invocation of the connect callback function, described in the preceding step), the client application should subscribe to the relevant topics.

To enqueue a subscription request, call **`iotc_subscribe()`**.
Subscription requests must include the following:

* The topic name
* A subscription callback function
* A QoS level

#### Topics

There is no hard limit to the number of topics an application can subscribe to with a particular context. Note, however, that each subscription has some memory overhead (topic name and a function pointer.)

#### Subscription callback

The subscription callback function is invoked by Cloud IoT Device SDKwhen the IoT Core service has acknowledged the subscription request, as well as when a message arrives to your device on a subscribed topic.

Incoming messages include the topic data as part of the callback parameters. You can either use the same subscription callback function for multiple topics, or have a unique callback function for each topic your client application subscribes to.

#### QoS level

The QoS level is the *maximum* QoS level of incoming messages. That is, if you set the QoS level to 0, QoS level 1 messages directed to the client will be downgraded to QoS level 0 by the Cloud IoT Core service. The QoS level affects the memory overhead and behavior of message processing on the client (for example, QoS 1 messages have more memory overhead than QoS 0 messages).

### Step 5: Publish

Once a connection has been established, your application can publish messages to the Cloud IoT Core service.

To publish a message to a topic, use either **`iotc_publish()`** or **`iotc_publish_data()`**. Each message must include the following:

* The topic name
* A message payload
* A QoS level

#### Publish callback

The client application can also supply a function pointer to a publish callback. This callback function is optional; it is intended to notify embedded devices after the message has been fully sent/acknowledged and any buffered data on the client has been deleted.

The callback is useful in environments with severe memory restrictions. For example, the device may gate the number of pending publications at a time, or you may need to track the status of very large publications to free up resources when they’re complete.

### Step 6: Disconnect and shut down

As noted in **Step 2: Connect**, the Cloud IoT Device SDK invokes the connect callback upon a disconnection event. Disconnection can occur for various reasons, including loss of internet connection, the Cloud IoT Core service closing the connection due to errant behavior (such as malformed MQTT packets), or an expired JWT. Additionally, the client application itself can instigate a shutdown.

To close the connection directly, the client application can invoke **`iotc_shutdown_connection()`**. This function enqueues an event to close the socket connection in a clean manner, invoking the connect callback once the connection has been fully terminated.

Note: Do not delete the context until the the connect callback has been invoked with the disconnect state.

#### Disconnection types and reconnecting

Your client application can determine the type of disconnection event (valid disconnect versus error) by checking various parameters of the connect callback. See the [**Connect callback**](#connect-callback) section for more information.

If the disconnection wasn't expected, your client application can call connect again immediately from within the connect callback. This will queue a new connection request, just as in **Step 2: Connect**.

If your client application closed the connection intentionally, it can retain the IoT Device SDK context and invoke the connect function again later.

#### Freeing memory and shutting down

If memory needs to be freed after an intentional disconnection, the context can be cleaned up by invoking **`iotc_delete_context`**. Further memory can be freed by calling **`iotc_shutdown()`**, but only after all contexts have been deleted.

Note: If you’re running in blocking mode, stop the event loop in the disconnection callback and then delete the context. If you’re in a ticking event loop, then delete the context in the next tick after the disconnection occurs.

The **`iotc_shutdown()`** function should be called sparingly -- not on every disconnection event. **`iotc_shutdown()`** destroys the backoff status cache that helps guard the Cloud IoT Core service from accidental DDoS attacks by devices in the field. For more information, see [Backoff](#backoff).

## Standard operation callbacks (connect, subscribe, publish, and shutdown)

The network dependent functions -- connect, subscribe, publish, and shutdown -- are all asynchronous. Although these functions return errors immediately if some simple sanity checks fail, normal operation will cause requests to be queued in the IoT Device SDK event system. They will be processed in the next iteration of the event processor.

In order for client applications to determine whether a request was successful, you'll need to provide a pointer to a callback function for these requests. These callback functions are detailed below.

**Note:** For system simplification, the connect, publish, and shutdown callbacks share the same function signature. The only distinction is the subscribe callback which requires more information than the others. This section details the use of parameters in each callback scenario.

### Connect callback

The connect callback function is used to track the current state of the connection process, and to monitor disconnection behavior.

#### Function signature

```
void
on_connection_state_changed( iotc_context_handle_t in_context_handle, void* data, iotc_state_t state )
```

#### Function parameters

**`in_context_handle`**

Used internally to track the state of the ongoing connection. Can be provided to the event system to queue new events from within the callback.

**`data`**

Cast this as `iotc_connection_data_t*` and observe the `connection_state` to determine the type of connection state change. Possible values:

* **IOTC_CONNECTION_STATE_OPENED:** The connection was successfully opened and TLS handshaking has occurred.
* **IOTC_CONNECTION_STATE_OPEN_FAILED:** The connection was not successfully opened. The state parameter will be set to an `iotc_err` noting why the connection failed.
* **IOTC_CONNECTION_STATE_CLOSED:** A previously opened connection was shut down. If the state parameter is `IOTC_STATE_OK`, the disruption was due to the client application queuing a shutdown request via `iotc_shutdown_connection()`. Otherwise, the connection was closed either because of a network interruption or because the Cloud IoT Core service encountered errant client behavior or an expired JWT.

**`state`**

Used in IOTC_CONNECTION_STATE_CLOSED messages, where `IOTC_STATE_OK` is set if the connection was an intentionally scheduled disconnection request made by the client application. Other state values indicate that a connection error occurred.

### Subscription callback

The subscription callback is invoked when the Cloud IoT Core service sends a MQTT SUBACK response with the granted QoS level of the subscription request. This function is also invoked each time the Cloud IoT Core service delivers a message to the device on the subscribed topic.

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

Used internally to track the state of the ongoing connection. Can be provided to the event system to queue new events from within the callback.

**`call_type`**

Indicates the reason for invoking the function. Possible values:

* **IOTC_SUB_CALL_SUBACK:** The callback provides a status update of subscription process.
* **IOTC_SUB_CALL_MESSAGE:** The callback invocation carries the payload data of the message sent by Cloud IoT Core.
* **IOTC_SUB_CALL_UNKNOWN:** Signifies a serious issue; data might be corrupted. Report an error but do not take other action.


**`params`**

This parameter holds a pointer to a union which contains either subscription status information (if a change in the status has occurred), or the payload of a received incoming message.

If `IOTC_SUB_CALL_SUBACK `is set, the **`params suback`** structure's fields are as follows:

* const char\* **topic**
    * Contains the topic name this invocation is related to
* iotc_mqtt_suback_status_t **suback_status**
    * May be one of:
        - IOTC_MQTT_QOS_0_GRANTED
        - IOTC_MQTT_QOS_1_GRANTED
        - IOTC_MQTT_QOS_2_GRANTED
        - IOTC_MQTT_SUBACK_FAILED

If `call_type` is set to `IOTC_SUB_CALL_MESSAGE`, **`params`** holds a **`message`** structure. This structure type can include the following information:

* const char\* **topic**
    - Topic name the message arrived on.
* const uint8_t\* **temporary_payload_data**
    - Holds memory of length temporary_payload_data_length bytes, to be freed after the invocation is completed. The data must be copied if it needs to be retained.
* size_t **temporary_payload_data_length**
    - The length of the temporary_payload_data array.
* iotc_mqtt_retain_t **retain**
    - Can be IOTC_MQTT_RETAIN_FALSE or IOTC_MQTT_RETAIN_TRUE.
* iotc_mqtt_qos_t **qos**
    - Can be one of: IOTC_MQTT_QOS_AT_MOST_ONCE, IOTC_MQTT_QOS_AT_LEAST_ONCE, IOTC_MQTT_QOS_EXACTLY_ONCE.
* iotc_mqtt_dup_t **dup_flag**
    - Can be one of: IOTC_MQTT_DUP_FALSE, IOTC_MQTT_DUP_TRUE.

**`state`**

Provides information about the state of the execution. If the value is something other than IOTC_STATE_OK, an error has occurred that should be reported.

**`user_data`**

`void* data` can be associated with each subscription request. This allows a single callback function implementation to handle multiple message subscriptions.

**Tip:** This data can contain application-specific information, or it can be used as a custom tag to differentiate topics more quickly and easily than by comparing topic names.

### Publication callback

The publication callback is invoked when the Cloud IoT Device SDK successfully publishes to Cloud IoT Core. This callback is intended for use with devices that have very little memory capacity and need to gate the rate of publication. To facilitate this, the publication callback accepts a `void* data` variable for marking the specific publication that this callback is tracking.

#### Function signature

```
void
on_publication( iotc_context_handle_t in_context_handle, void* data, iotc_state_t state )
```

#### Function parameters

**`in_context_handle`**

Used internally to track the state of the ongoing connection. Can be provided to the event system to queue new events from within the callback.

**`data`**

Always NULL.

**`state`**

On publication success, set to IOTC_STATE_WRITTEN. Otherwise, provides an `iotc_state_t` error.

### Event system

The Cloud IoT Device SDK Client has its own event processor for handle asynchronous communication (such as publication, subscription, and connection). To use this event system to schedule callbacks, provide a function pointer, a valid context, and the elapsed time in seconds when the function should be called.

By making use of the event system, you can schedule publications to occur regularly (for example, to get device status), set a timer for a timeout, or schedule new connections. The latter can be useful if the device actively communicates with the Cloud IoT Core service only a few times per day.

#### Function signature

Your callback function should have three parameters:

* **`context_handle`**: Created via the `iotc_create_context()` function call
* **`timed_task_handle`**: For identifying the function
* **`user_data`**: Any data you want to pass to the function

Example:

```
void scheduled_function( const iotc_context_handle_t context_handle,
                         const iotc_timed_task_handle_t timed_task_handle,
                         void* user_data )
{
    /* things to do. */
}
```

#### Scheduling an event

You can choose whether to have your function called only once, or called repeatedly indefinitely on a specified cadence until you cancel it. Storing a handle to the event is highly recommended so that you can cancel a scheduled event before it fires, or stop repeated calls of functions.

Example:


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

If you need to cancel an upcoming event callback before it occurs or stop a repeated callback, use the timed task handle:

`/* Assumes a timed_task_handle was set when the callback was scheduled */`

```
if( IOTC_INVALID_TIMED_TASK_HANDLE != timed_task_handle )
{
    iotc_cancel_timed_task( timed_task_handle );
}
```


#### Sample usage

An example of a scheduled callback is provided in the base `examples/` directory of the Cloud IoT Device SDK GitHub repository. It includes source that schedules a callback to `delayed_publish()`, to publish a message every five seconds after the client has connected to the Cloud IoT Core service.

## Learn more

### Additional documentation

The following documentation is available:

* [Google Cloud IoT Device SDK for Embedded C Releases](https://github.com/googlecloudplatform/iot-device-sdk-embedded-c/releases)
* [Google Cloud IoT Device SDK for Embedded C GitHub](https://github.com/googlecloudplatform/iot-device-sdk-embedded-c)
* [Google Cloud IoT Device SDK for Embedded C Porting Guide](https://github.com/googlecloudplatform/iot-device-sdk-embedded-c/blob/master/doc/porting_guide.md)
* [Google Cloud Google Cloud IoT Core Quickstart](https://cloud.google.com/iot/docs/quickstart)
* Cloud IoT Device SDK for Embedded C API reference, in `doc/doxygen/api` within the GitHub repo.
* Cloud IoT Device SDK for Embedded C BSP reference, in `doc/doxygen/bsp` within the GitHub repo.


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

A state in which the Cloud IoT Device SDK delays connection attempts to the Cloud IoT Core service, in order to prevent accidental Distributed Denial of Service attacks.

### Client application

Software written for the target platform. Resides above the Cloud IoT Device SDK on the application software stack. The client application calls into the Google Cloud IoT Device SDK to fulfill its communication requests to the Cloud IoT Core service.

### Cloud IoT Device SDK for Embedded C

Software written in C and intended for embedded devices. The SDK facilitates connections to the Cloud IoT Core service on behalf of the client application running on the device.
