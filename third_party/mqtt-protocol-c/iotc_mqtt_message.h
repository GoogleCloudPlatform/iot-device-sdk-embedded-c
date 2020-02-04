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

#ifndef __IOTC_MQTT_MESSAGE_H__
#define __IOTC_MQTT_MESSAGE_H__

#include <stdint.h>
#include <stdio.h>

#include <iotc_mqtt.h>
#include "iotc_data_desc.h"
#include "iotc_debug_data_desc_dump.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum iotc_mqtt_type_e {
  IOTC_MQTT_TYPE_NONE = 0,
  IOTC_MQTT_TYPE_CONNECT = 1,
  IOTC_MQTT_TYPE_CONNACK = 2,
  IOTC_MQTT_TYPE_PUBLISH = 3,
  IOTC_MQTT_TYPE_PUBACK = 4,
  IOTC_MQTT_TYPE_PUBREC = 5,
  IOTC_MQTT_TYPE_PUBREL = 6,
  IOTC_MQTT_TYPE_PUBCOMP = 7,
  IOTC_MQTT_TYPE_SUBSCRIBE = 8,
  IOTC_MQTT_TYPE_SUBACK = 9,
  IOTC_MQTT_TYPE_UNSUBSCRIBE = 10,
  IOTC_MQTT_TYPE_UNSUBACK = 11,
  IOTC_MQTT_TYPE_PINGREQ = 12,
  IOTC_MQTT_TYPE_PINGRESP = 13,
  IOTC_MQTT_TYPE_DISCONNECT = 14,
} iotc_mqtt_type_t;

typedef enum {
  IOTC_MQTT_MESSAGE_CLASS_UNKNOWN = 0,
  IOTC_MQTT_MESSAGE_CLASS_TO_SERVER,
  IOTC_MQTT_MESSAGE_CLASS_FROM_SERVER
} iotc_mqtt_message_class_t;

typedef struct iotc_mqtt_topic_s {
  struct mqtt_topic_s* next;
  iotc_data_desc_t* name;
} iotc_mqtt_topic_t;

typedef struct iotc_mqtt_topicpair_s {
  struct iotc_mqtt_topicpair_s* next;
  iotc_data_desc_t* name;
  union {
    iotc_mqtt_suback_status_t status;
    iotc_mqtt_qos_t qos;
  } iotc_mqtt_topic_pair_payload_u;
} iotc_mqtt_topicpair_t;

typedef union iotc_mqtt_message_u {
  struct common_s {
    union {
      struct {
        unsigned int retain : 1;
        unsigned int qos : 2;
        unsigned int dup : 1;
        unsigned int type : 4;
      } common_bits;
      uint8_t common_value;
    } common_u;
    uint32_t remaining_length;
  } common;

  struct {
    struct common_s common;

    iotc_data_desc_t* protocol_name;
    uint8_t protocol_version;

    union {
      struct {
        unsigned int reserverd : 1;
        unsigned int clean_session : 1;
        unsigned int will : 1;
        unsigned int will_qos : 2;
        unsigned int will_retain : 1;
        unsigned int password_follows : 1;
        unsigned int username_follows : 1;
      } flags_bits;
      uint8_t flags_value;
    } flags_u;

    uint16_t keepalive;

    iotc_data_desc_t* client_id;

    iotc_data_desc_t* will_topic;
    iotc_data_desc_t* will_message;

    iotc_data_desc_t* username;
    iotc_data_desc_t* password;
  } connect;

  struct {
    struct common_s common;

    uint8_t _unused;
    uint8_t return_code;
  } connack;

  struct {
    struct common_s common;

    iotc_data_desc_t* topic_name;
    uint16_t message_id;

    iotc_data_desc_t* content;
  } publish;

  struct {
    struct common_s common;

    uint16_t message_id;
  } puback;

  struct {
    struct common_s common;

    uint16_t message_id;
  } pubrec;

  struct {
    struct common_s common;

    uint16_t message_id;
  } pubrel;

  struct {
    struct common_s common;

    uint16_t message_id;
  } pubcomp;

  struct {
    struct common_s common;

    uint16_t message_id;
    iotc_mqtt_topicpair_t* topics;
  } subscribe;

  struct {
    struct common_s common;

    uint16_t message_id;
    iotc_mqtt_topicpair_t* topics;
  } suback;

  struct {
    struct common_s common;

    uint16_t message_id;
    iotc_mqtt_topic_t* topics;
  } unsubscribe;

  struct {
    struct common_s common;

    uint16_t message_id;
  } unsuback;

  struct {
    struct common_s common;
  } pingreq;

  struct {
    struct common_s common;
  } pingresp;

  struct {
    struct common_s common;
  } disconnect;
} iotc_mqtt_message_t;

#if IOTC_DEBUG_OUTPUT
extern void iotc_debug_mqtt_message_dump(const iotc_mqtt_message_t* msg);
#else
#define iotc_debug_mqtt_message_dump(...)
#endif

extern void iotc_mqtt_message_free(iotc_mqtt_message_t** msg);

/**
 * @name    iotc_mqtt_class_msg_type_receiving
 * @brief   Classifies the message while executing the receiving code.
 *
 * Message classification itself is required to pick up the proper qeueue of
 * logic tasks to avoid message id collision for incoming chains ( like
 * receiving PUBLISH ) with outgoing chains (like sending SUBSCRIBE or PUBLISH.)
 *
 * There are two situation that we might want to classify the message:
 *      - sending the message   ( push )
 *      - receiving the message ( pull )
 *
 * There are two possible scenarios that can happen during classification:
 *      - message is classified as a part of outgoing message chain ( like
 * sending PUBLISH, or KEEPALIVE. )
 *      - message is classified as a part of incoming message chain ( like
 * receiving PUBLISH on 1st and 2nd QoS levels. )
 *
 */
extern iotc_mqtt_message_class_t iotc_mqtt_class_msg_type_receiving(
    const iotc_mqtt_type_t msg_type);

/**
 * @name    iotc_mqtt_class_msg_type_sending
 * @brief   Classifies the message while executing the sending code
 *
 * @see iotc_mqtt_class_msg_type_receiving
 */
extern iotc_mqtt_message_class_t iotc_mqtt_class_msg_type_sending(
    const iotc_mqtt_type_t msg_type);

extern uint16_t iotc_mqtt_get_message_id(const iotc_mqtt_message_t* msg);

extern iotc_state_t iotc_mqtt_convert_to_qos(unsigned int qos,
                                             iotc_mqtt_qos_t* qos_out);

extern iotc_state_t iotc_mqtt_convert_to_dup(unsigned int dup,
                                             iotc_mqtt_dup_t* dup_out);

extern iotc_state_t iotc_mqtt_convert_to_retain(unsigned int retain,
                                                iotc_mqtt_retain_t* retain_out);

#ifdef __cplusplus
}
#endif

#endif /* __IOTC_MQTT_MESSAGE_H__ */
