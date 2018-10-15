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

#ifndef __IOTC_ITEST_MQTT_KEEPALIVE_H__
#define __IOTC_ITEST_MQTT_KEEPALIVE_H__

extern int iotc_itest_mqtt_keepalive_setup(void** state);
extern int iotc_itest_mqtt_keepalive_teardown(void** state);

extern void
iotc_itest_mqtt_keepalive__PINGREQ_failed_to_send__client_disconnects_after_keepalive_seconds(
    void** state);
extern void
iotc_itest_mqtt_keepalive__PINGREQ_failed_to_send__broker_disconnects_first(
    void** state);
extern void
iotc_itest_mqtt_keepalive__2nd_PINGREQ_failed_to_send__broker_disconnects_first(
    void** state);

#ifdef IOTC_MOCK_TEST_PREPROCESSOR_RUN
struct CMUnitTest iotc_itests_mqtt_keepalive[] = {
    cmocka_unit_test_setup_teardown(
        iotc_itest_mqtt_keepalive__PINGREQ_failed_to_send__client_disconnects_after_keepalive_seconds,
        iotc_itest_mqtt_keepalive_setup, iotc_itest_mqtt_keepalive_teardown),
    cmocka_unit_test_setup_teardown(
        iotc_itest_mqtt_keepalive__PINGREQ_failed_to_send__broker_disconnects_first,
        iotc_itest_mqtt_keepalive_setup, iotc_itest_mqtt_keepalive_teardown),
    cmocka_unit_test_setup_teardown(
        iotc_itest_mqtt_keepalive__2nd_PINGREQ_failed_to_send__broker_disconnects_first,
        iotc_itest_mqtt_keepalive_setup, iotc_itest_mqtt_keepalive_teardown),
};
#endif

#endif /* __IOTC_ITEST_MQTT_KEEPALIVE_H__ */