/* 2018-2019- 2019 Google LLC
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

#ifndef __IOTC_MQTT_PARSER_H__
#define __IOTC_MQTT_PARSER_H__

#include <stddef.h>
#include <stdint.h>

#include "iotc_common.h"
#include "iotc_layer.h"
#include "iotc_mqtt_errors.h"
#include "iotc_mqtt_message.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum iotc_mqtt_parser_rc_e {
  IOTC_MQTT_PARSER_RC_ERROR,
  IOTC_MQTT_PARSER_RC_CONTINUE,
  IOTC_MQTT_PARSER_RC_INCOMPLETE,
  IOTC_MQTT_PARSER_RC_DONE,
  IOTC_MQTT_PARSER_RC_WANT_MEMORY,
} iotc_mqtt_parser_rc_t;

typedef struct iotc_mqtt_parser_s {
  iotc_mqtt_error_t error;
  uint16_t cs;
  uint16_t read_cs;
  char buffer_pending;
  uint8_t* buffer;
  size_t buffer_length;
  size_t digit_bytes;
  size_t multiplier;
  size_t remaining_length;
  size_t str_length;
  size_t data_length;
} iotc_mqtt_parser_t;

extern void iotc_mqtt_parser_init(iotc_mqtt_parser_t* parser);
extern void iotc_mqtt_parser_buffer(iotc_mqtt_parser_t* parser, uint8_t* buffer,
                                    size_t buffer_length);

extern iotc_state_t iotc_mqtt_parse_suback_response(
    iotc_mqtt_suback_status_t* dst, const uint8_t resp);
extern iotc_state_t iotc_mqtt_parser_execute(iotc_mqtt_parser_t* parser,
                                             iotc_mqtt_message_t* message,
                                             iotc_data_desc_t*);

#ifdef __cplusplus
}
#endif

#endif /* __IOTC_MQTT_PARSER_H__ */
