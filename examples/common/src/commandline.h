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
 * This module implements a command line argument parser.
 */

#include <iotc_mqtt.h>

/* Flags set by commandline arguments. */
extern iotc_mqtt_qos_t iotc_example_qos;

/* Parameters returned by the parser. These will be in a structure someday. */
extern const char* iotc_project_id;
extern const char* iotc_device_path;
extern const char* iotc_publish_topic;
extern const char* iotc_publish_message;
extern const char* iotc_private_key_filename;

int iotc_parse(int argc, char** argv, char* valid_options,
               const unsigned options_length);
