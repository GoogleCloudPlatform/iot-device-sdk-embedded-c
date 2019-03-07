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

#ifndef __IOTC_MQTT_H__
#define __IOTC_MQTT_H__

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @name iotc_mqtt_retain_t
 * @brief MQTT retain flag.
 *
 * For details, see the <a href"http://docs.oasis-open.org/mqtt/mqtt/v3.1.1/os/mqtt-v3.1.1-os.html#_Toc398718030">MQTT retain specifications</a>.
 */
typedef enum iotc_mqtt_retain_e {
  IOTC_MQTT_RETAIN_FALSE = 0,
  IOTC_MQTT_RETAIN_TRUE = 1,
} iotc_mqtt_retain_t;

/**
 * @name iotc_mqtt_qos_t
 * @brief MQTT Quality of Service levels.
 *
 * For details, see the <a href"http://docs.oasis-open.org/mqtt/mqtt/v3.1.1/os/mqtt-v3.1.1-os.html#_Toc398718099">MQTT QoS specifications</a>.
 */
typedef enum iotc_mqtt_qos_e {
  IOTC_MQTT_QOS_AT_MOST_ONCE = 0,
  IOTC_MQTT_QOS_AT_LEAST_ONCE = 1,
  IOTC_MQTT_QOS_EXACTLY_ONCE = 2,
} iotc_mqtt_qos_t;

/**
 * @name iotc_mqtt_suback_status_t
 * @brief MQTT SUBACK status.
 *
 * For details, see the <a href"http://docs.oasis-open.org/mqtt/mqtt/v3.1.1/os/mqtt-v3.1.1-os.html#_Toc398718068">MQTT SUBACK payload specifications</a>.
 */
typedef enum iotc_mqtt_suback_status_e {
  IOTC_MQTT_QOS_0_GRANTED = 0x00,
  IOTC_MQTT_QOS_1_GRANTED = 0x01,
  IOTC_MQTT_QOS_2_GRANTED = 0x02,
  IOTC_MQTT_SUBACK_FAILED = 0x80
} iotc_mqtt_suback_status_t;

/**
 * @name iotc_mqtt_dup_t
 * @brief MQTT DUP flag.
 *
 * For details, see the <a href"http://docs.oasis-open.org/mqtt/mqtt/v3.1.1/os/mqtt-v3.1.1-os.html#_Toc398718038">MQTT DUP specifications</a>.
 */
typedef enum iotc_mqtt_dup_e {
  IOTC_MQTT_DUP_FALSE = 0,
  IOTC_MQTT_DUP_TRUE = 1,
} iotc_mqtt_dup_t;

#ifdef __cplusplus
}
#endif

#endif /* __IOTC_MQTT_H__ */
