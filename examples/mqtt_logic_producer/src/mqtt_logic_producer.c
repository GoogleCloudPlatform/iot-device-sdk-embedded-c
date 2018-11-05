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

/*
 * This example application connects to the GCP IoT Core Service with
 * a credentials you mus specify on the command line.   It then publishes
 * test messages to a topic that you also must specify.
 *
 * Run the example with the flag --help for more information.
 */

#include <iotc.h>
#include "../../common/src/commandline.h"
#include "../../common/src/example_init.h"

#include <stdio.h>
#include "../src/libiotc/iotc_jwt.h"

#define IOTC_UNUSED(x) (void)(x)

/* Application variables. */
iotc_context_handle_t iotc_context = IOTC_INVALID_CONTEXT_HANDLE;

static iotc_timed_task_handle_t delayed_publish_task =
    IOTC_INVALID_TIMED_TASK_HANDLE;

/* Function definitions. */
void on_connection_state_changed(iotc_context_handle_t in_context_handle,
                                 void* data, iotc_state_t state);

void publish_function(iotc_context_handle_t context_handle,
                      iotc_timed_task_handle_t timed_task, void* user_data);

/*  -main-
    The main entry point for this example binary.

    For information on creating the credentials required for your device to the
    GCP IoT Core Service during development then please see the service's
    quick start guide. */

int main(int argc, char* argv[]) {
  if (0 != iotc_example_handle_command_line_args(argc, argv)) {
    return -1;
  }

  char ec_private_key_pem[PRIVATE_KEY_BUFFER_SIZE] = {0};

  if (0 != load_ec_private_key_pem_from_posix_fs(ec_private_key_pem, PRIVATE_KEY_BUFFER_SIZE)) {
    printf("\nApplication exiting due to private key load error.\n\n");
    return -1;
  }

  /* initialize iotc library and create a context to use to connect to the
   * GCP IoT Core Service. */
  const iotc_state_t error_init = iotc_initialize();

  if (IOTC_STATE_OK != error_init) {
    printf(" iotc failed to initialize, error: %d\n", error_init);
    return -1;
  }

  /*  Create a connection context. A context represents a Connection
      on a single socket, and can be used to publish and subscribe
      to numerous topics. */
  iotc_context = iotc_create_context();
  if (IOTC_INVALID_CONTEXT_HANDLE >= iotc_context) {
    printf(" iotc failed to create context, error: %d\n", -iotc_context);
    return -1;
  }

  /*  Queue a connection request to be completed asynchronously.
      The 'on_connection_state_changed' parameter is the name of the
      callback function after the connection request completes, and its
      implementation should handle both successful connections and
      unsuccessful connections as well as disconnections. */
  const uint16_t connection_timeout = 10;
  const uint16_t keepalive_timeout = 20;

  iotc_crypto_private_key_data_t key_data;
  key_data.private_key_signature_algorithm =
      IOTC_JWT_PRIVATE_KEY_SIGNATURE_ALGORITHM_ES256;
  key_data.private_key_union_type = IOTC_CRYPTO_KEY_UNION_TYPE_PEM;
  key_data.private_key_union.key_pem.key = ec_private_key_pem;

  iotc_connect(iotc_context, iotc_project_id, iotc_device_path, &key_data,
               /*{jwt_expiration_period_sec=*/3600, connection_timeout,
               keepalive_timeout, &on_connection_state_changed);

  /* The IoTC Client was designed to be able to run on single threaded devices.
     As such it does not have its own event loop thread. Instead you must
     regularly call the function iotc_events_process_blocking() to process
     connection requests, and for the client to regularly check the sockets for
     incoming data. This implementation has the loop operate endlessly. The loop
     will stop after closing the connection, using iotc_shutdown_connection() as
     defined in on_connection_state_change logic, and exit the event handler
     handler by calling iotc_events_stop(); */
  iotc_events_process_blocking();

  /*  Cleanup the default context, releasing its memory */
  iotc_delete_context(iotc_context);

  /* Cleanup internal allocations that were created by iotc_initialize. */
  iotc_shutdown();

  return 0;
}

/* A callback function that will be invoked whenever the connection state
   has changed.

   Information about the status of the connection will be provided by the
   IoTC library via the function parameters of this callback.

   DATA:
    a iotc_connection_data_t structure which holds a connection_state parameter
    that can be monitored to determine the health of the connection:

    IOTC_CONNECTION_STATE_OPENED - the connection has been successfully
   established, and TLS handshaking has verified the server certificate.  The
   client is read to publish and subscribe to topics.
    IOTC_CONNECTION_STATE_OPEN_FAILED - there was an error in the connection
        process and the connection could not be made. You can determine the
        reason in the iotc_state_t, which contains a standard IoTC error code.
    IOTC_CONNECTION_STATE_CLOSED - disconnected from Google Cloud IoT Core
   service, with the reason provided in iotc_state_t. Note: there are two
   possible reasons of being disconnected: a) the application initiated the
   disconnection procedure through a call to iotc_shutdown_connection b) there
   has been a problem with maintaining the connection, or the service shutdown
   the connection expectantly.

        Determining which of these scenarios has occured can be done by
        checking the iotc_state_t parameter of the callback.

        if state == IOTC_STATE_OK , the application intentionally disconnected
        if state != IOTC_STATE_OK , the disconnection was unexpected, and the
   state contains an error code as to what happened.

        In the case of this example,
        the application's response to a successful connection is to create a
            repeating timed task to publish a message on the configured topic.
            If an intentional disconnection occurs, then the client application
            exists.  If an unforeseen disconnection occurs, or if the connection
            fails, then the application reattempts the connection.  */
void on_connection_state_changed(iotc_context_handle_t in_context_handle,
                                 void* data, iotc_state_t state) {
  iotc_connection_data_t* conn_data = (iotc_connection_data_t*)data;

  switch (conn_data->connection_state) {
    /* IOTC_CONNECTION_STATE_OPENED means that the connection has been
       established and the IoTC Client is ready to send/recv messages */
    case IOTC_CONNECTION_STATE_OPENED:
      printf("connected!\n");

      /* Publish immediately upon connect. 'publish_function' is defined
         in this example file and invokes the IoTC API to publish a
         message. */
      publish_function(in_context_handle, IOTC_INVALID_TIMED_TASK_HANDLE,
                       /*user_data=*/NULL);

      /* Create a timed task to publish every 5 seconds. */
      delayed_publish_task =
          iotc_schedule_timed_task(in_context_handle, publish_function, 5, 15,
                                   /*user_data=*/NULL);
      break;

    /* IOTC_CONNECTION_STATE_OPEN_FAILED is set when there was a problem
       when establishing a connection to the server. The reason for the error
       is contained in the 'state' variable. Here we log the error state and
       exit out of the application. */
    case IOTC_CONNECTION_STATE_OPEN_FAILED:
      printf("ERROR!\tConnection has failed reason %d\n\n", state);

      /* exit it out of the application by stopping the event loop. */
      iotc_events_stop();
      break;

    /* IOTC_CONNECTION_STATE_CLOSED is set when the IoTC Client has been
        disconnected. The disconnection may have been caused by some external
        issue, or user may have requested a disconnection. In order to
       distinguish between those two situation it is advised to check the state
       variable value. If the state == IOTC_STATE_OK then the application has
       requested a disconnection via 'iotc_shutdown_connection'. If the state !=
       IOTC_STATE_OK then the connection has been closed from one side.
      */
    case IOTC_CONNECTION_STATE_CLOSED: {
      /* When the connection is closed it's better to cancel some of previously
         registered activities. Using cancel function on handler will remove the
         handler from the timed queue which prevents the registered handle to be
         called when there is no connection. */
      if (IOTC_INVALID_TIMED_TASK_HANDLE != delayed_publish_task) {
        iotc_cancel_timed_task(delayed_publish_task);
        delayed_publish_task = IOTC_INVALID_TIMED_TASK_HANDLE;
      }

      if (state == IOTC_STATE_OK) {
        /* The connection has been closed intentionally. Therefore, stop
         * the event processing loop as there's nothing left to do
         * in this example. */
        iotc_events_stop();
      } else {
        printf("connection closed - reason %d!\n", state);
        /* The disconnection was unforeseen.  Try reconnect to the server
           with previously set configuration, which has been provided
           to this callback in the conn_data structure. */
        iotc_connect(
            in_context_handle, conn_data->project_id, conn_data->device_path,
            conn_data->private_key_data, conn_data->jwt_expiration_period_sec,
            conn_data->connection_timeout, conn_data->keepalive_timeout,
            &on_connection_state_changed);
      }
    } break;
    default:
      printf("wrong value\n");
      break;
  }
}

/* A function that publishes to the topic that was specified in the command
   line parameters of the application. This is invoked directly upon connect
   in the 'on_connection_state_changed' function, but also by the IoTC Client's
   event system on a 5 second interval. */
void publish_function(iotc_context_handle_t context_handle,
                      iotc_timed_task_handle_t timed_task, void* user_data) {
  /* timed_task is a handle to the reoccurring task that may have invoked
     this function.  We could use it to cancel the task here, but we want
     it to keep repeating.*/
  IOTC_UNUSED(timed_task);

  /* user data is a anonymous payload that the application can use to
     feed non-typed information to scheduled function calls / events.
     Here the application feeds a value of NULL (as defined in the
     function 'on_connection_state_changed' above, so we can ignore it.) */
  IOTC_UNUSED(user_data);

  printf("publishing msg \"%s\" to topic: \"%s\"\n", iotc_publish_message,
         iotc_publish_topic);

  iotc_publish(context_handle, iotc_publish_topic, iotc_publish_message,
               iotc_example_qos,
               /*callback=*/NULL, /*user_data=*/NULL);
}

