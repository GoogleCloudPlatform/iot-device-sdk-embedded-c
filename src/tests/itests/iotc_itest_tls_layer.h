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

#ifndef __IOTC_ITEST_TLS_LAYER_H__
#define __IOTC_ITEST_TLS_LAYER_H__

/**
 * @name    iotc_itest_tls_layer_setup
 * @brief   Test case setup, called on start of each test case.
 *
 * @param [in] state: Test case specific custom data.
 *
 * @retval 0              OK.
 * @retval -1             Tells cmocka error occurred.
 */
extern int iotc_itest_tls_layer_setup(void** state);

/**
 * @name    iotc_itest_tls_layer_teardown
 * @brief   Test case setup, called on finish of each test case.
 *
 * @param [in] state: Test case specific custom data.
 *
 * @retval 0              OK.
 * @retval -1             Tells cmocka error occurred.
 */
extern int iotc_itest_tls_layer_teardown(void** state);

extern void iotc_itest_tls_layer__null_host__graceful_closure(void** state);
extern void iotc_itest_tls_layer__valid_dependencies__successful_init(
    void** state);
extern void iotc_itest_tls_layer__bad_handshake_response__graceful_closure(
    void** state);

#ifdef IOTC_MOCK_TEST_PREPROCESSOR_RUN
struct CMUnitTest iotc_itests_tls_layer[] = {
    cmocka_unit_test_setup_teardown(
        iotc_itest_tls_layer__null_host__graceful_closure,
        iotc_itest_tls_layer_setup, iotc_itest_tls_layer_teardown),
    cmocka_unit_test_setup_teardown(
        iotc_itest_tls_layer__valid_dependencies__successful_init,
        iotc_itest_tls_layer_setup, iotc_itest_tls_layer_teardown),
    cmocka_unit_test_setup_teardown(
        iotc_itest_tls_layer__bad_handshake_response__graceful_closure,
        iotc_itest_tls_layer_setup, iotc_itest_tls_layer_teardown)};
#endif

#endif /* __IOTC_ITEST_TLS_LAYER_H__ */
