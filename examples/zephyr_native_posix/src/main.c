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
iotc_context_handle_t iotc_context = IOTC_INVALID_CONTEXT_HANDLE;

void main(void) {
  printk("[ INFO ] GCP IoT client example for Zephyr.\n");

  int argc = 0;
  char** argv = NULL;

  native_get_cmd_line_args(&argc, &argv);

  /* Zephyr passes the "-testargs" internal command line argument too. This code
   * skips it to be compatible to the native command line argument handling.
   */
  if (1 < argc && 0 == strcmp(argv[1], "-testargs")) {
    --argc;
    ++argv;
  }

  /* Parse the GCP IoT Core parameters (project_id, registry_id, device_id,
   * private_key, publish topic, publish message).
   */
  if (0 != iotc_parse_commandline_flags(argc, argv)) {
    return;
  }

  /* Read the private key that will be used for authentication. */
  if (0 != load_ec_private_key_pem_from_posix_fs(ec_private_key_pem,
                                                 PRIVATE_KEY_BUFFER_SIZE)) {
    printk("[ FAIL ] Application exiting due to private key load error.\n\n");
    return;
  }

  /* Format the key type descriptors so the client understands
   * what type of key is being represented. In this case, a PEM encoded
   * byte array of a ES256 key.
   */
  iotc_connect_private_key_data.crypto_key_signature_algorithm =
      IOTC_CRYPTO_KEY_SIGNATURE_ALGORITHM_ES256;
  iotc_connect_private_key_data.crypto_key_union_type =
      IOTC_CRYPTO_KEY_UNION_TYPE_PEM;
  iotc_connect_private_key_data.crypto_key_union.key_pem.key =
      ec_private_key_pem;

  printk("[ INFO ] Starting GCP IoT Embedded C Client...\n");

  /* Initialize the IoTC library. */
  const iotc_state_t init_state = iotc_initialize();
  if (IOTC_STATE_OK != init_state) {
    printk("[ FAIL ] Failed to initialize IoTC library. Reason: %d\n",
           init_state);
    return;
  }

  /* Create a connection context. A context represents a Connection
   * on a single socket, and can be used to publish and subscribe
   * to numerous topics.
   */
  iotc_context = iotc_create_context();
  if (IOTC_INVALID_CONTEXT_HANDLE >= iotc_context) {
    printk("[ FAIL ] Failed to create IoTC context. Reason: %d\n",
           -iotc_context);
    return;
  }

  const uint16_t connection_timeout = 10;
  const uint16_t keepalive_timeout = 10;

  /* Generate the client authentication JWT, which will serve as the MQTT
   * password.
   */
  char jwt[IOTC_JWT_SIZE] = {0};
  size_t bytes_written = 0;
  iotc_state_t state = iotc_create_iotcore_jwt(
      iotc_core_parameters.project_id,
      /*jwt_expiration_period_sec=*/3600, &iotc_connect_private_key_data, jwt,
      IOTC_JWT_SIZE, &bytes_written);

  if (IOTC_STATE_OK != state) {
    printk("[ FAIL ] Failed to create JWT. Reason: %d", state);
    return;
  }

  iotc_connect(iotc_context, /*username=*/NULL, /*password=*/jwt,
               /*client_id=*/iotc_core_parameters.device_path,
               connection_timeout, keepalive_timeout,
               &on_connection_state_changed);

  iotc_events_process_blocking();

  /*  Cleanup the default context, releasing its memory. */
  iotc_delete_context(iotc_context);

  /* Cleanup internal allocations that were created by iotc_initialize. */
  iotc_shutdown();
}
