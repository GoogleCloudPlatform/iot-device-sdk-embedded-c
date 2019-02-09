/* Copyright 2018 - 2019 Google LLC
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

/*
 * This example application connects to the GCP IoT Core Service with
 * a credentials you mus specify on the command line.   It then publishes
 * test messages to a topic that you also must specify.
 *
 * Run the example with the flag --help for more information.
 */

#include <iotc.h>
#include "../../common/src/commandline.h"
#include "../../common/src/example_utils.h"

#include <stdio.h>
#include <iotc_jwt.h>

/* Application variables. */
iotc_crypto_key_data_t iotc_connect_private_key_data;
char ec_private_key_pem[PRIVATE_KEY_BUFFER_SIZE] = {0};
iotc_context_handle_t iotc_context = IOTC_INVALID_CONTEXT_HANDLE;

/*  -main-
    The main entry point for this example binary.

    For information on creating the credentials required for your device to the
    GCP IoT Core Service during development then please see the service's
    quick start guide. */

int main(int argc, char* argv[]) {
  if (0 != iotc_example_handle_command_line_args(argc, argv)) {
    return -1;
  }

  /* This example assumes the private key to be used to  sign the IoT Core
     Connect JWT credential is a PEM encoded ES256 private key,
     and passes it IoT Core Device Client functions as a byte array.
     There are other ways of passing key data to the client, including
     passing Key Slot IDs for using keys stored in secure elements.
     For more information, please see the iotc_crypto_key_data_t
     documentation in include/iotc_types.h. */

  /* Use the example utils function to load the key from disk into memory. */
  if (0 != load_ec_private_key_pem_from_posix_fs(ec_private_key_pem,
                                                 PRIVATE_KEY_BUFFER_SIZE)) {
    printf("\nError loading IoT Core private key from disk.\n\n");
    return -1;
  }

  /* Format the key type descriptors so the client understands
     which type of key is being reprenseted. In this case, a PEM encoded
     byte array of a ES256 key. */
  iotc_connect_private_key_data.crypto_key_signature_algorithm =
      IOTC_CRYPTO_KEY_SIGNATURE_ALGORITHM_ES256;
  iotc_connect_private_key_data.crypto_key_union_type =
      IOTC_CRYPTO_KEY_UNION_TYPE_PEM;
  iotc_connect_private_key_data.crypto_key_union.key_pem.key =
      ec_private_key_pem;

  /* Initialize iotc library and create a context to use to connect to the
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

  /* Generate the client authentication JWT, which will serve as the MQTT
   * password. */
  char jwt[IOTC_JWT_SIZE] = {0};
  size_t bytes_written = 0;
  iotc_state_t state =
    iotc_create_iotcore_jwt( iotc_project_id,
                             /*jwt_expiration_period_sec=*/3600,
                             &iotc_connect_private_key_data, jwt,
                             IOTC_JWT_SIZE, &bytes_written);

  if (IOTC_STATE_OK != state ) {
    printf("iotc_create_iotcore_jwt returned with error: %ul", state);
    return -1;
  }

  iotc_connect(iotc_context, /*username=*/NULL, /*password=*/jwt,
               /*client_id=*/ iotc_device_path, connection_timeout, keepalive_timeout,
               &on_connection_state_changed);

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
