/* Copyright 2019 Google LLC
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

#include <cmdline.h>
#include <string.h>
#include <zephyr.h>

#include <commandline.h>
#include <example_utils.h>
#include <iotc.h>
#include <iotc_jwt.h>

iotc_crypto_key_data_t iotc_connect_private_key_data;
char ec_private_key_pem[PRIVATE_KEY_BUFFER_SIZE] = {0};

void main(void) {
  printk("Example for Zephyr port.\n");

  /* commandline sample:
    zephyr.exe -testargs -p <GCP IoT Core Project ID> -d projects/<GCP IoT Core
    Project ID>/locations/<Region>/registries/<GCP IoT Core Registry
    ID>/devices/<GCP IoT Core Device ID> -t /devices/<GCP IoT Core
    DeviceID>/state
    */

  int argc = 0;
  char** argv = NULL;

  native_get_cmd_line_args(&argc, &argv);

  /* Zephyr passes the "-testargs" internal command line argument too. This code
   * skips it to be compatible to the native command line argument handlin. */
  if (1 < argc && 0 == strcmp(argv[1], "-testargs")) {
    --argc;
    ++argv;
  }

  /* parsing GCP IoT related command line arguments */
  if (0 != iotc_example_handle_command_line_args(argc, argv)) {
    return;
  }

  if (0 != load_ec_private_key_pem_from_posix_fs(ec_private_key_pem,
                                                 PRIVATE_KEY_BUFFER_SIZE)) {
    printk("\nApplication exiting due to private key load error.\n\n");
    return;
  }

  /* Format the key type descriptors so the client understands
     what type of key is being represented. In this case, a PEM encoded
     byte array of a ES256 key. */
  iotc_connect_private_key_data.crypto_key_signature_algorithm =
      IOTC_CRYPTO_KEY_SIGNATURE_ALGORITHM_ES256;
  iotc_connect_private_key_data.crypto_key_union_type =
      IOTC_CRYPTO_KEY_UNION_TYPE_PEM;
  iotc_connect_private_key_data.crypto_key_union.key_pem.key =
      ec_private_key_pem;

  printk("Starting GCP IoT Embedded C Client...\n");

  iotc_initialize();

  iotc_context_handle_t iotc_context = iotc_create_context();

  const uint16_t connection_timeout = 10;
  const uint16_t keepalive_timeout = 10;

  /* Generate the client authentication JWT, which will serve as the MQTT
   * password. */
  char jwt[IOTC_JWT_SIZE] = {0};
  size_t bytes_written = 0;
  iotc_state_t state = iotc_create_iotcore_jwt(
      iotc_project_id,
      /*jwt_expiration_period_sec=*/3600, &iotc_connect_private_key_data, jwt,
      IOTC_JWT_SIZE, &bytes_written);

  if (IOTC_STATE_OK != state) {
    printk("iotc_create_iotcore_jwt returned with error: %ul", state);
    return;
  }

  iotc_connect(iotc_context, /*username=*/NULL, /*password=*/jwt,
               /*client_id=*/iotc_device_path, connection_timeout,
               keepalive_timeout, &on_connection_state_changed);

  iotc_events_process_blocking();

  /*  Cleanup the default context, releasing its memory */
  iotc_delete_context(iotc_context);

  /* Cleanup internal allocations that were created by iotc_initialize. */
  iotc_shutdown();
}
