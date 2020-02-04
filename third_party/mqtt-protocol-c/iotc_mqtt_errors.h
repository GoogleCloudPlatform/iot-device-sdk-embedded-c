/* 2018-2020 Google LLC
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

#ifndef __IOTC_MQTT_ERRORS_H__
#define __IOTC_MQTT_ERRORS_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef enum iotc_mqtt_error_e {
  IOTC_MQTT_ERROR_NONE = 0,
  IOTC_MQTT_ERROR_PARSER_INVALID_STATE = 1,
  IOTC_MQTT_ERROR_PARSER_INVALID_REMAINING_LENGTH = 2,
  IOTC_MQTT_ERROR_PARSER_INVALID_MESSAGE_ID = 3,
  IOTC_MQTT_ERROR_SERIALISER_INVALID_MESSAGE_ID = 4,
} iotc_mqtt_error_t;

#ifdef __cplusplus
}
#endif

#endif /* __IOTC_MQTT_ERRORS_H__ */
