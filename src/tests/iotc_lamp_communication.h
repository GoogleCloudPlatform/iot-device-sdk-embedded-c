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

#ifndef __IOTC_LAMP_COMUNNICATION_H__
#define __IOTC_LAMP_COMUNNICATION_H__

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef IOTC_PLATFORM_IS_OSX

static const char IOTC_TEST_RESULT_COMMAND_TEMPLATE[] =
    "open -g \"unittest-notifier://iotc.unittest/{"
    "\\\"id\\\":\\\"%s\\\","
    "\\\"initiale\\\":\\\"%s\\\","
    "\\\"testdetailsfile\\\":\\\"none\\\","
    "\\\"running\\\":\\\"%s\\\","
    "\\\"passed\\\":\\\"%s\\\"}\"";

void iotc_test_report_result(const char* const test_id,
                             const char* const initiale, const uint8_t running,
                             uint8_t failed) {
  char test_result_command_buffer[1000] = {0};

  snprintf(test_result_command_buffer, sizeof(test_result_command_buffer),
           IOTC_TEST_RESULT_COMMAND_TEMPLATE, test_id, initiale,
           running ? "true" : "false", failed ? "false" : "true");

  system(test_result_command_buffer);
}

#else
#define iotc_test_report_result(test_id, initiale, running, failed)
#endif

#ifdef __cplusplus
}
#endif

#endif /* __IOTC_LAMP_COMUNNICATION_H__ */
