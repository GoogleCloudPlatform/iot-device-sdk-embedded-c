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

/*
 * This module implements a command line argument parser
 */

#include "commandline.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "iotc.h"

#ifndef IOTC_CROSS_TARGET
#include <getopt.h>
#define IOTC_EXAMPLE_DEFAULT_QOS IOTC_MQTT_QOS_AT_LEAST_ONCE
#else
#define IOTC_EXAMPLE_DEFAULT_QOS IOTC_MQTT_QOS_AT_MOST_ONCE
#endif /* IOTC_CROSS_TARGET */

void iotc_usage(const char* options, unsigned options_length);

iotc_mqtt_qos_t iotc_example_qos = IOTC_EXAMPLE_DEFAULT_QOS;

#define DEFAULT_PRIVATE_KEY_FIILENAME "ec_private.pem"

const char* iotc_project_id;
const char* iotc_device_path;
const char* iotc_publish_topic;
const char* iotc_publish_message;
const char* iotc_private_key_filename;

int iotc_parse(int argc, char** argv, char* valid_options,
               unsigned options_length) {
  int c;
  int iotc_help_flag = 0;
  iotc_project_id = NULL;
  iotc_device_path = NULL;
  iotc_publish_topic = NULL;
  iotc_private_key_filename = DEFAULT_PRIVATE_KEY_FIILENAME;
  iotc_publish_message = "Hello From Your IoTC client!";

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
        iotc_project_id = optarg;
        break;
      case 'd':
        iotc_device_path = optarg;
        break;
      case 't':
        iotc_publish_topic = optarg;
        break;
      case 'm':
        iotc_publish_message = optarg;
        break;
      case 'f':
        iotc_private_key_filename = optarg;
        break;
      case 'h':
      default:
        iotc_help_flag = 1;
        break;
    }
  }

  /* Print any unrecognized command line arguments. */
  if (optind < argc) {
    printf(
        "The application could not recognize the following non-option "
        "arguments: ");
    while (optind < argc) {
      printf("%s ", argv[optind++]);
    }
    putchar('\n');
  }
  putchar('\n');

  if (1 == iotc_help_flag) /* Print the usage statement */
  {
    iotc_usage(valid_options, options_length);
    return (
        -1); /* Don't run the application if -h --help was on the commandline */
  }

  return (0);
}

void iotc_usage(const char* options, unsigned options_length) {
  assert(NULL != options);

  /* For debugging printf( "options = %s %d\n", options, options_length ); */

  printf("Usage:\n");
  while (0 < options_length) {
    /* printf( "parsing option %c\n", *options ); */
    switch (*options) {
      case 'p':
        printf(
            "-p --project_id\n\tProvide the project_id your device is "
            "registered in "
            "Cloud IoT Core.\n");
        break;
      case 'd':
        printf(
            "-d --device_path\n\tProvide the full path of your device. For "
            "example:\n"
            "\t\tprojects/<project_id>/locations/<cloud_region>/registries/"
            "<registry_id>/devices/<device_id>\n");
        break;
      case 't':
        printf("-t --publish_topic\n\tThe topic on which to subscribe.\n");
        break;
      case 'm':
        printf(
            "-m --publish_message\n\tThe message to publish. A shell quoted "
            "string of characters.\n");
        break;
      case 'f':
        printf(
            "-f --private_key_filename\n\tThe filename, including path from "
            "cwd,\n");
        printf(" \t of the device identifying private_key. Defaults to: %s\n",
               DEFAULT_PRIVATE_KEY_FIILENAME);
        break;
      case 'h': /* Don't print anything for the help option since we're printing
                   usage */
        break;
      case ':': /* We'll skip the ':' character since it's not an option. */
        break;
      case '\0':
        break;
      default:
        printf("WARNING: Option %c not recognized by usage()\n", *options);
    }
    ++options;
    --options_length;
  }
  printf("\n");
}
