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

#ifndef __IOTC_EXAMPLES_COMMON_COMMANDLINE_H__
#define __IOTC_EXAMPLES_COMMON_COMMANDLINE_H__

#include <iotc_mqtt.h>

/* Parameters to GCP IoT Core. */
typedef struct iotc_core_parameters_s {
  const char* project_id;
  const char* device_path;
  const char* publish_topic;
  const char* publish_message;
  const char* private_key_filename;
  iotc_mqtt_qos_t example_qos;
} iotc_core_parameters_t;

extern iotc_core_parameters_t iotc_core_parameters;

/* Parses the commandline flags to |iotc_core_parameters| global variable.
 * Returns  0 on success,
 *         -1 on error, or when help (-h, --help) was requested.
 */
int iotc_parse_commandline_flags(int argc, char** argv);

#endif /* __IOTC_EXAMPLES_COMMON_COMMANDLINE_H__ */
