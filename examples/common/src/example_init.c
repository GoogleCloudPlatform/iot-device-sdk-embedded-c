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

#include <stdio.h>

#include "example_init.h"
#include "commandline.h"
#include <iotc.h>

int iotc_example_handle_command_line_args(int argc, char* argv[]) {
  char options[] = "h:p:d:t:m:f:";
  int missingparameter = 0;
  int retval = 0;

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

/* Attempts to load the client's identifying private key from disk so that the
   byte data may be passed to the 'iotc_connect function'.  Please note that the
   IoTC API and Board Support Package have various means to use private keys.
   This example assumes the use of one that must be provided to a TLS
   implementation in buffer, but secure chips with slot-based key stores can
   also be used. Please see the Crypto BSP for more information. */
int load_ec_private_key_pem_from_posix_fs(char* buf_ec_private_key_pem, size_t buf_len) {
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
        file_size, buf_len);
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
