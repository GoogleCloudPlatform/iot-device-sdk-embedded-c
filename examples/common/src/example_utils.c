/* Copyright 2018-2020 Google LLC
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

#include <iotc_error.h>
#include <iotc_jwt.h>
#include <stdio.h>
#include <string.h>

#include "commandline.h"
#include "example_utils.h"

#define IOTC_UNUSED(x) (void)(x)

extern iotc_crypto_key_data_t iotc_connect_private_key_data;

static iotc_timed_task_handle_t delayed_publish_task =
    IOTC_INVALID_TIMED_TASK_HANDLE;

int iotc_example_handle_command_line_args(int argc, char* argv[]) {
  char options[] = "h:p:d:t:m:u:n:f";
  int missingparameter = 0;
  int retval = 0;

  char host_name[2048];
  char port_val[1048];

  /* log the executable name and library version */
  printf("\n%s\n%s\n", argv[0], iotc_cilent_version_str);

  /* Parse the argv array for ONLY the options specified in the options string
   */
  retval = iotc_parse(argc, argv, options, sizeof(options));

  if (-1 == retval) {
    /* iotc_parse has returned an error, and has already logged the error
       to the console. Therefore just silently exit here. */
    return -1;
  }

  if((iotc_mqtt_connect_url != NULL) && (iotc_mqtt_connect_port != NULL)){
      /* caller supplied custom hostURL and port number  */
      memcpy(host_name, iotc_mqtt_connect_url, strlen(iotc_mqtt_connect_url));
      memcpy(port_val, iotc_mqtt_connect_port, strlen(iotc_mqtt_connect_port));
      int port_i_val = atoi(port_val);
      setHostNameAndPort(host_name, port_i_val);
	}

  /* Check to see that the required parameters were all present on the command
   * line */
  if (NULL == iotc_project_id) {
    missingparameter = 1;
    printf("-p --project_id is required\n");
  }

  if (NULL == iotc_device_path) {
    missingparameter = 1;
    printf("-d --device_path is required\n");
  }

  if (NULL == iotc_publish_topic) {
    missingparameter = 1;
    printf("-t --publish_topic is required\n");
  }

  if (1 == missingparameter) {
    /* Error has already been logged, above.  Silently exit here */
    printf("\n");
    return -1;
  }

  return 0;
}

int load_ec_private_key_pem_from_posix_fs(char* buf_ec_private_key_pem,
                                          size_t buf_len) {
  FILE* fp = fopen(iotc_private_key_filename, "rb");
  if (fp == NULL) {
    printf("ERROR!\n");
    printf(
        "\tMissing Private Key required for JWT signing.\n"
        "\tPlease copy and paste your device's EC private key into\n"
        "\ta file with the following path based on this executable's\n"
        "\tcurrent working dir:\n\t\t\'%s\'\n\n"
        "\tAlternatively use the --help command line parameter to learn\n"
        "\thow to set a path to your file using command line arguments\n",
        iotc_private_key_filename);
    return -1;
  }

  fseek(fp, 0, SEEK_END);
  long file_size = ftell(fp);
  rewind(fp);

  if ((size_t)file_size > buf_len) {
    printf(
        "private key file size of %lu bytes is larger that certificate buffer "
        "size of %lu bytes\n",
        file_size, (long)buf_len);
    fclose(fp);
    return -1;
  }

  long bytes_read = fread(buf_ec_private_key_pem, 1, file_size, fp);
  fclose(fp);

  if (bytes_read != file_size) {
    printf("could not fully read private key file\n");
    return -1;
  }

  return 0;
}

void on_connection_state_changed(iotc_context_handle_t in_context_handle,
                                 void* data, iotc_state_t state) {
  iotc_connection_data_t* conn_data = (iotc_connection_data_t*)data;

  if (NULL == conn_data) {
    return;
  }

  switch (conn_data->connection_state) {
    /* IOTC_CONNECTION_STATE_OPENED means that the connection has been
       established and the IoTC Client is ready to send/recv messages */
    case IOTC_CONNECTION_STATE_OPENED:
      printf("connected to %s:%d\n", conn_data->host, conn_data->port);

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
      printf("ERROR!\tConnection has failed reason %d : %s\n\n", state,
             iotc_get_state_string(state));

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
        printf("connection closed - reason %d : %s!\n", state,
               iotc_get_state_string(state));
        /* The disconnection was unforeseen.  Try to reconnect to the server
           with the previously set username and client_id, but regenerate
           the client authentication JWT password in case the disconnection
           was due to an expired JWT. */
        char jwt[IOTC_JWT_SIZE] = {0};
        size_t bytes_written = 0;
        state = iotc_create_iotcore_jwt(iotc_project_id,
                                        /*jwt_expiration_period_sec=*/3600,
                                        &iotc_connect_private_key_data, jwt,
                                        IOTC_JWT_SIZE, &bytes_written);
        if (IOTC_STATE_OK != state) {
          printf(
              "iotc_create_iotcore_jwt returned with error"
              " when attempting to reconnect: %ul\n",
              state);
        } else {
          iotc_connect(in_context_handle, conn_data->username, jwt,
                       conn_data->client_id, conn_data->connection_timeout,
                       conn_data->keepalive_timeout,
                       &on_connection_state_changed);
        }
      }
    } break;
    default:
      printf("wrong value\n");
      break;
  }
}

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
