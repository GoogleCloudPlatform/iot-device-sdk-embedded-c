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

#include "iotc_utest_basic_testcase_frame.h"
#include <stdio.h>
#include "iotc.h"
#include "iotc_macros.h"
#include "iotc_memory_checks.h"
#include "tinytest_macros.h"

void* iotc_utest_setup_basic(const struct testcase_t* testcase) {
  IOTC_UNUSED(testcase);

  iotc_memory_limiter_tearup();

  iotc_initialize();

  return (intptr_t*)1;
}

int iotc_utest_teardown_basic(const struct testcase_t* testcase,
                              void* fixture) {
  IOTC_UNUSED(testcase);
  IOTC_UNUSED(fixture);

  iotc_shutdown();

  // 1 - OK, 0 - NOT OK
  return iotc_memory_limiter_teardown();
}
