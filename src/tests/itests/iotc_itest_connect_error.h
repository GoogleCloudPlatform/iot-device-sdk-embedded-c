/* Copyright 2018-2019 Google LLC
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

#ifndef __IOTC_ITEST_CONNECT_ERROR_H__
#define __IOTC_ITEST_CONNECT_ERROR_H__

#include "iotc_itest_helpers.h"

extern int iotc_itest_connect_error_setup(void** fixture_void);
extern int iotc_itest_connect_error_teardown(void** fixture_void);

extern void
iotc_itest_test_valid_flow__call_connect_function_twice_in_a_row__second_call_returns_error(
    void** state);
extern void
iotc_itest_test_valid_flow__call_connect_function_twice__second_call_returns_error(
    void** state);
extern void
iotc_itest_test_valid_flow__call_is_context_connected_on_connecting_context__call_returns_false(
    void** state);
extern void
iotc_itest_test_valid_flow__call_is_context_connected_on_connected_context__call_returns_true(
    void** state);
extern void
iotc_itest_test_valid_flow__call_is_context_connected_on_disconnecting_context__call_returns_false(
    void** state);
extern void
iotc_itest_test_valid_flow__call_is_context_connected_on_disconnected_context__call_returns_false(
    void** state);
extern void
iotc_itest_test_valid_flow__call_disconnect_twice_on_connected_context__second_call_should_return_error(
    void** state);
extern void
iotc_itest_test_valid_flow__call_connect_function_then_disconnect_without_making_a_connection__shutdown_should_unregister_connect(
    void** state);

#ifdef IOTC_MOCK_TEST_PREPROCESSOR_RUN

struct CMUnitTest iotc_itests_connect_error[] = {
    cmocka_unit_test_setup_teardown(
        iotc_itest_test_valid_flow__call_connect_function_twice_in_a_row__second_call_returns_error,
        iotc_itest_connect_error_setup, iotc_itest_connect_error_teardown),
    cmocka_unit_test_setup_teardown(
        iotc_itest_test_valid_flow__call_connect_function_twice__second_call_returns_error,
        iotc_itest_connect_error_setup, iotc_itest_connect_error_teardown),
    cmocka_unit_test_setup_teardown(
        iotc_itest_test_valid_flow__call_disconnect_twice_on_connected_context__second_call_should_return_error,
        iotc_itest_connect_error_setup, iotc_itest_connect_error_teardown),
    cmocka_unit_test_setup_teardown(
        iotc_itest_test_valid_flow__call_connect_function_then_disconnect_without_making_a_connection__shutdown_should_unregister_connect,
        iotc_itest_connect_error_setup, iotc_itest_connect_error_teardown),
    cmocka_unit_test_setup_teardown(
        iotc_itest_test_valid_flow__call_is_context_connected_on_connecting_context__call_returns_false,
        iotc_itest_connect_error_setup, iotc_itest_connect_error_teardown),
    cmocka_unit_test_setup_teardown(
        iotc_itest_test_valid_flow__call_is_context_connected_on_connected_context__call_returns_true,
        iotc_itest_connect_error_setup, iotc_itest_connect_error_teardown),
    cmocka_unit_test_setup_teardown(
        iotc_itest_test_valid_flow__call_is_context_connected_on_disconnecting_context__call_returns_false,
        iotc_itest_connect_error_setup, iotc_itest_connect_error_teardown),
    cmocka_unit_test_setup_teardown(
        iotc_itest_test_valid_flow__call_is_context_connected_on_disconnected_context__call_returns_false,
        iotc_itest_connect_error_setup, iotc_itest_connect_error_teardown),
};

#endif /* IOTC_MOCK_TEST_PREPROCESSOR_RUN */

#endif /* __IOTC_ITEST_CONNECT_ERROR_H__ */
