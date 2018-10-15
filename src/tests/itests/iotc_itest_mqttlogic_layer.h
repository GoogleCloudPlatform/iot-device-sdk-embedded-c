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

#ifndef __IOTC_ITEST_MQTTLOGIC_LAYER_H__
#define __IOTC_ITEST_MQTTLOGIC_LAYER_H__

extern int iotc_itest_mqttlogic_layer_setup(void** state);
extern int iotc_itest_mqttlogic_layer_teardown(void** state);

extern void
iotc_itest_mqttlogic_layer__backoff_class_error_PUSH__layerchain_closure_is_expected(
    void** state);
extern void
iotc_itest_mqttlogic_layer__backoff_class_error_PULL__layerchain_closure_is_expected(
    void** state);
extern void
iotc_itest_mqtt_logic_layer__subscribe_success__success_suback_callback_invocation(
    void** state);
extern void
iotc_itest_mqtt_logic_layer__subscribe_failure__failed_suback_callback_invocation(
    void** state);
extern void
iotc_itest_mqtt_logic_layer__subscribe_success__success_message_callback_invocation(
    void** state);

#ifdef IOTC_MOCK_TEST_PREPROCESSOR_RUN
struct CMUnitTest iotc_itests_mqttlogic_layer[] = {
    cmocka_unit_test_setup_teardown(
        iotc_itest_mqttlogic_layer__backoff_class_error_PUSH__layerchain_closure_is_expected,
        iotc_itest_mqttlogic_layer_setup, iotc_itest_mqttlogic_layer_teardown),
    cmocka_unit_test_setup_teardown(
        iotc_itest_mqttlogic_layer__backoff_class_error_PULL__layerchain_closure_is_expected,
        iotc_itest_mqttlogic_layer_setup, iotc_itest_mqttlogic_layer_teardown),
    cmocka_unit_test_setup_teardown(
        iotc_itest_mqtt_logic_layer__subscribe_success__success_suback_callback_invocation,
        iotc_itest_mqttlogic_layer_setup, iotc_itest_mqttlogic_layer_teardown),
    cmocka_unit_test_setup_teardown(
        iotc_itest_mqtt_logic_layer__subscribe_failure__failed_suback_callback_invocation,
        iotc_itest_mqttlogic_layer_setup, iotc_itest_mqttlogic_layer_teardown)};
#endif

#endif /* __IOTC_ITEST_MQTTLOGIC_LAYER_H__ */
