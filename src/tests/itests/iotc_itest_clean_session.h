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

#ifndef IOTC_ITEST_CLEAN_SESSION_H
#define IOTC_ITEST_CLEAN_SESSION_H

#include "iotc_itest_helpers.h"

extern int iotc_itest_clean_session_setup(void** state);
extern int iotc_itest_clean_session_teardown(void** state);

extern void
iotc_itest_test_valid_flow__handlers_vector_should_be_empty__init_with_clean_session(
    void** state);

#ifdef IOTC_MOCK_TEST_PREPROCESSOR_RUN
struct CMUnitTest iotc_itests_clean_session[] = {cmocka_unit_test_setup_teardown(
    iotc_itest_test_valid_flow__handlers_vector_should_be_empty__init_with_clean_session,
    iotc_itest_clean_session_setup, iotc_itest_clean_session_teardown)};
#endif

#endif  // IOTC_ITEST_CLEAN_SESSION_H
