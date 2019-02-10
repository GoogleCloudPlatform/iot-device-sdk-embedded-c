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

#include "tinytest.h"

#ifndef __IOTC_UTEST_BASIC_TESTCASE_FRAME_H__
#define __IOTC_UTEST_BASIC_TESTCASE_FRAME_H__

void* iotc_utest_setup_basic(const struct testcase_t* testcase);
int iotc_utest_teardown_basic(const struct testcase_t* testcase, void* fixture);

#endif
