/* Copyright 2018-2019 Google LLC
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
 * This module implements a command line argument parser
 */

#include "commandline.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#ifndef IOTC_CROSS_TARGET
#include <getopt.h>
#endif

#include "iotc_mqtt.h"

#ifndef IOTC_CROSS_TARGET
const iotc_mqtt_qos_t kDefaultMqttQos = IOTC_MQTT_QOS_AT_LEAST_ONCE;
#else
const iotc_mqtt_qos_t kDefaultMqttQos = IOTC_MQTT_QOS_AT_MOST_ONCE;
#endif /* IOTC_CROSS_TARGET */

void iotc_print_usage(const char* binary_name);

const char kDefaultPrivateKeyFilename[] = "ec_private.pem";

iotc_core_parameters_t iotc_core_parameters;

int iotc_parse_commandline_flags(int argc, char** argv) {
  int c;
  int has_error = 0;
  int help_flag_present = 0;
  static char valid_options[] = "hp:d:t:m:f:";

  iotc_core_parameters.project_id = NULL;
  iotc_core_parameters.device_path = NULL;
  iotc_core_parameters.publish_topic = NULL;
  iotc_core_parameters.private_key_filename = kDefaultPrivateKeyFilename;
  iotc_core_parameters.publish_message = "Hello From Your GCP IoT client!";
  iotc_core_parameters.example_qos = kDefaultMqttQos;

  while (1) {
    static struct option long_options[] = {
        {"help", no_argument, 0, 'h'},
        {"project_id", required_argument, 0, 'p'},
        {"device_path", required_argument, 0, 'd'},
        {"publish_topic", required_argument, 0, 't'},
        {"publish_message", required_argument, 0, 'm'},
        {"private_key_filename", optional_argument, 0, 'f'},
        {0, 0, 0, 0}};

    /* getopt_long stores the option index here. */
    int option_index = 0;

    c = getopt_long(argc, argv, valid_options, long_options, &option_index);

    /* Detect the end of the options. */
    if (-1 == c) {
      break;
    }

    switch (c) {
      case 'p':
        iotc_core_parameters.project_id = optarg;
        break;
      case 'd':
        iotc_core_parameters.device_path = optarg;
        break;
      case 't':
        iotc_core_parameters.publish_topic = optarg;
        break;
      case 'm':
        iotc_core_parameters.publish_message = optarg;
        break;
      case 'f':
        iotc_core_parameters.private_key_filename = optarg;
        break;
      case 'h':
      default:
        help_flag_present = 1;
        break;
    }
  }

  /* Print any unrecognized command line arguments. */
  if (optind < argc) {
    printf("[ FAIL ] Invalid commandline flags: ");
    while (optind < argc) {
      printf("%s ", argv[optind++]);
    }
    putchar('\n');
  }
  putchar('\n');

  /* Print the usage statement */
  if (1 == help_flag_present) {
    iotc_print_usage(argv[0]);
    return -1;
  }

  /* Check that all required parameters were present. */
  if (NULL == iotc_core_parameters.project_id) {
    has_error = 1;
    printf("[ FAIL ] -p --project_id is required\n");
  }

  if (NULL == iotc_core_parameters.device_path) {
    printf("[ FAIL ] -d --device_path is required\n");
  }

  if (NULL == iotc_core_parameters.publish_topic) {
    has_error = 1;
    printf("[ FAIL ] -t --publish_topic is required\n");
  }

  if (1 == has_error) {
    return -1;
  }

  return 0;
}

void iotc_print_usage(const char* binary_name) {
  printf("Usage: %s <options> \n\n", binary_name);
  printf("Supported options are:\n");
  printf(
      "\t -p --project_id (required)\n"
      "\t\tThe project_id of your device, registered in Google Cloud IoT "
      "Core.\n");
  printf(
      "\t-d --device_path (required)\n"
      "\t\tThe path to your device. Example:\n"
      "\t\tprojects/<project_id>/locations/<cloud_region>/registries/"
      "<registry_id>/devices/<device_id>\n");
  printf(
      "\t-t --publish_topic (required)\n"
      "\t\tThe topic to subscribe to.\n");
  printf(
      "\t-m --publish_message (required)\n"
      "\t\tThe message to publish. A shell quoted list of characters.\n");
  printf(
      "\t-f --private_key_filename (optional) default: %s\n"
      "\t\tRelative path to the private key file.\n",
      kDefaultPrivateKeyFilename);
  printf("\n");
}
