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

#ifndef __IOTC_ITEST_TLS_ERROR_H__
#define __IOTC_ITEST_TLS_ERROR_H__

extern int iotc_itest_tls_error_setup(void** state);
extern int iotc_itest_tls_error_teardown(void** state);

extern void iotc_itest_tls_error__connection_flow__basic_checks(void** state);
extern void
iotc_itest_tls_error__tls_init_and_connect_errors__graceful_error_handling(
    void** state);
extern void
iotc_itest_tls_error__tls_push_CONNECT_errors__graceful_error_handling(
    void** state);
extern void
iotc_itest_tls_error__tls_push_infinite_SUBSCRIBE_errors__reSUBSCRIBE_occurs_once_in_a_second(
    void** state);
extern void
iotc_itest_tls_error__tls_push_SUBSCRIBE_errors__graceful_error_handling(
    void** state);
extern void
iotc_itest_tls_error__tls_push_PUBLISH_errors__graceful_error_handling(
    void** state);
extern void
iotc_itest_tls_error__tls_pull_CONNACK_errors__graceful_error_handling(
    void** state);
extern void
iotc_itest_tls_error__tls_pull_SUBACK_errors__graceful_error_handling(
    void** state);
extern void
iotc_itest_tls_error__tls_pull_PUBACK_errors__graceful_error_handling(
    void** state);

#ifdef IOTC_MOCK_TEST_PREPROCESSOR_RUN
struct CMUnitTest iotc_itests_tls_error[] = {
    cmocka_unit_test_setup_teardown(
        iotc_itest_tls_error__connection_flow__basic_checks,
        iotc_itest_tls_error_setup, iotc_itest_tls_error_teardown),
    cmocka_unit_test_setup_teardown(
        iotc_itest_tls_error__tls_init_and_connect_errors__graceful_error_handling,
        iotc_itest_tls_error_setup, iotc_itest_tls_error_teardown),
    cmocka_unit_test_setup_teardown(
        iotc_itest_tls_error__tls_push_CONNECT_errors__graceful_error_handling,
        iotc_itest_tls_error_setup, iotc_itest_tls_error_teardown),
    cmocka_unit_test_setup_teardown(
        iotc_itest_tls_error__tls_push_infinite_SUBSCRIBE_errors__reSUBSCRIBE_occurs_once_in_a_second,
        iotc_itest_tls_error_setup, iotc_itest_tls_error_teardown),
    cmocka_unit_test_setup_teardown(
        iotc_itest_tls_error__tls_push_SUBSCRIBE_errors__graceful_error_handling,
        iotc_itest_tls_error_setup, iotc_itest_tls_error_teardown),
    cmocka_unit_test_setup_teardown(
        iotc_itest_tls_error__tls_push_PUBLISH_errors__graceful_error_handling,
        iotc_itest_tls_error_setup, iotc_itest_tls_error_teardown),
    cmocka_unit_test_setup_teardown(
        iotc_itest_tls_error__tls_pull_CONNACK_errors__graceful_error_handling,
        iotc_itest_tls_error_setup, iotc_itest_tls_error_teardown),
    cmocka_unit_test_setup_teardown(
        iotc_itest_tls_error__tls_pull_SUBACK_errors__graceful_error_handling,
        iotc_itest_tls_error_setup, iotc_itest_tls_error_teardown),
    cmocka_unit_test_setup_teardown(
        iotc_itest_tls_error__tls_pull_PUBACK_errors__graceful_error_handling,
        iotc_itest_tls_error_setup, iotc_itest_tls_error_teardown)};
#endif

#endif /* __IOTC_ITEST_TLS_ERROR_H__ */
