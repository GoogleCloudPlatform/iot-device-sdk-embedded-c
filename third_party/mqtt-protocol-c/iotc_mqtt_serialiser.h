/* 2018-2019- 2019 Google LLC
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

#ifndef __IOTC_MQTT_SERIALISER_H__
#define __IOTC_MQTT_SERIALISER_H__

#include "iotc_mqtt_errors.h"
#include "iotc_mqtt_message.h"

#ifdef __cplusplus
extern "C" {
#endif

/* @TODO: Is the iotc_mqtt_buffer still used? */

typedef enum iotc_mqtt_serialiser_rc_e {
  IOTC_MQTT_SERIALISER_RC_ERROR,
  IOTC_MQTT_SERIALISER_RC_SUCCESS,
} iotc_mqtt_serialiser_rc_t;

typedef struct iotc_mqtt_serialiser_s {
  iotc_mqtt_error_t error;
} iotc_mqtt_serialiser_t;

void iotc_mqtt_serialiser_init(iotc_mqtt_serialiser_t* serialiser);

iotc_state_t iotc_mqtt_serialiser_size(size_t* msg_len, size_t* remaining_len,
                                       size_t* publish_payload_len,
                                       iotc_mqtt_serialiser_t* serialiser,
                                       const iotc_mqtt_message_t* message);

iotc_mqtt_serialiser_rc_t iotc_mqtt_serialiser_write(
    iotc_mqtt_serialiser_t* serialiser, const iotc_mqtt_message_t* message,
    iotc_data_desc_t* buffer, const size_t message_len,
    const size_t remaining_len);

#ifdef __cplusplus
}
#endif

#endif /* __IOTC_MQTT_SERIALISER_H__ */
