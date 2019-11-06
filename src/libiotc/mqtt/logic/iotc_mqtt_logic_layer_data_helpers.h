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

#ifndef __IOTC_MQTT_LOGIC_LAYER_DATA_HELPERS_H__
#define __IOTC_MQTT_LOGIC_LAYER_DATA_HELPERS_H__

#include "iotc_mqtt_logic_layer_data.h"
#include "iotc_mqtt_message.h"
#include "string.h"

#ifdef __cplusplus
extern "C" {
#endif

static inline int8_t match_topics(const union iotc_vector_selector_u* a,
                                const union iotc_vector_selector_u* b) {
  const iotc_mqtt_task_specific_data_t* ca =
      (const iotc_mqtt_task_specific_data_t*)
          a->ptr_value; /* This is supposed to be the one from the vector. */
  const iotc_data_desc_t* cb =
      (const iotc_data_desc_t*)b->ptr_value; /* This is supposed to be the one
                                                from the msg. */

  // check if we have a wildcard in our subscription
  uint8_t subscription_length = strlen(ca->subscribe.topic);
  if (subscription_length == 0) {
    // bail out in case we have an empty string at this point
    return 1;
  }

  char last_token = ca->subscribe.topic[subscription_length-1];
  // since MQTT only allows the last character of a topic to be a wildcard, we 
  // can check the last character and if a wildcard, only compare up the
  // character before the /#
  if (last_token == '#') {
    if (subscription_length == 1) {
      return 0; // we are matching all topics with root wildcard '#'
    } else if (memcmp(ca->subscribe.topic, 
              cb->data_ptr, 
              subscription_length-2) == 0) {
      return 0;
    }
  } else { // subscription is NOT any kind of wildcard
    uint8_t published_topic_length = cb->length;
    // do fast length check first
    if (published_topic_length != subscription_length) {
      return 1;
    // if incoming topic and subscription are the same length, perform memcmp
    } else if (memcmp(ca->subscribe.topic, cb->data_ptr, published_topic_length) == 0) {
      return 0;
    }
  }

  return 1;
}

static inline iotc_state_t fill_with_pingreq_data(iotc_mqtt_message_t* msg) {
  memset(msg, 0, sizeof(iotc_mqtt_message_t));
  msg->common.common_u.common_bits.type = IOTC_MQTT_TYPE_PINGREQ;

  return IOTC_STATE_OK;
}

static inline iotc_state_t fill_with_puback_data(iotc_mqtt_message_t* msg,
                                                 uint16_t msg_id) {
  memset(msg, 0, sizeof(iotc_mqtt_message_t));
  msg->common.common_u.common_bits.type = IOTC_MQTT_TYPE_PUBACK;
  msg->puback.message_id = msg_id;

  return IOTC_STATE_OK;
}

static inline iotc_state_t fill_with_connack_data(iotc_mqtt_message_t* msg,
                                                  uint8_t return_code) {
  memset(msg, 0, sizeof(iotc_mqtt_message_t));
  msg->common.common_u.common_bits.type = IOTC_MQTT_TYPE_CONNACK;
  msg->connack._unused = 0;
  msg->connack.return_code = return_code;

  return IOTC_STATE_OK;
}

static inline iotc_state_t fill_with_connect_data(
    iotc_mqtt_message_t* msg, const char* username, const char* password,
    const char* client_id, uint16_t keepalive_timeout,
    iotc_session_type_t session_type, const char* will_topic,
    const char* will_message, iotc_mqtt_qos_t will_qos,
    iotc_mqtt_retain_t will_retain) {
  iotc_state_t local_state = IOTC_STATE_OK;

  memset(msg, 0, sizeof(iotc_mqtt_message_t));

  /* MQTT Client Identifiers are not optional. */
  if (NULL == client_id) {
    return IOTC_NULL_CLIENT_ID_ERROR;
  }

  msg->common.common_u.common_bits.retain = IOTC_MQTT_RETAIN_FALSE;
  msg->common.common_u.common_bits.qos = IOTC_MQTT_QOS_AT_MOST_ONCE;
  msg->common.common_u.common_bits.dup = IOTC_MQTT_DUP_FALSE;
  msg->common.common_u.common_bits.type = IOTC_MQTT_TYPE_CONNECT;
  /* These are filled in during serialization if used. */
  msg->common.remaining_length = 0;
  msg->connect.will_topic = NULL;
  msg->connect.will_message = NULL;
  msg->connect.flags_u.flags_bits.will = 0;
  msg->connect.flags_u.flags_bits.will_retain = 0;
  msg->connect.flags_u.flags_bits.will_qos = 0;

  IOTC_CHECK_MEMORY(
      msg->connect.protocol_name = iotc_make_desc_from_string_copy("MQTT"),
      local_state);
  msg->connect.protocol_version = 4;

  IOTC_CHECK_MEMORY(
      msg->connect.client_id = iotc_make_desc_from_string_copy(client_id),
      local_state);

  if (NULL != username) {
    msg->connect.flags_u.flags_bits.username_follows = 1;
    IOTC_CHECK_MEMORY(
        msg->connect.username = iotc_make_desc_from_string_copy(username),
        local_state);
  } else {
    msg->connect.flags_u.flags_bits.username_follows = 0;
  }

  if (1 == msg->connect.flags_u.flags_bits.username_follows &&
      NULL != password) {
    msg->connect.flags_u.flags_bits.password_follows = 1;
    IOTC_CHECK_MEMORY(
        msg->connect.password = iotc_make_desc_from_string_copy(password),
        local_state);
  } else {
    msg->connect.flags_u.flags_bits.password_follows = 0;
  }

  /* If only ONE of the will_topic or will_message are set, return an error. */
  if ((NULL == will_message) && (NULL != will_topic)) {
    return IOTC_NULL_WILL_MESSAGE;
  }
  if ((NULL != will_message) && (NULL == will_topic)) {
    return IOTC_NULL_WILL_TOPIC;
  }

  /* If both the will_topic and will_message are set, set the will variables. */
  if ((NULL != will_message) && (NULL != will_topic)) {
    IOTC_CHECK_MEMORY(
        msg->connect.will_topic = iotc_make_desc_from_string_copy(will_topic),
        local_state);
    IOTC_CHECK_MEMORY(msg->connect.will_message =
                          iotc_make_desc_from_string_copy(will_message),
                      local_state);
    msg->connect.flags_u.flags_bits.will = 1;
    msg->connect.flags_u.flags_bits.will_retain = will_retain;
    msg->connect.flags_u.flags_bits.will_qos = will_qos;
  }

  msg->connect.flags_u.flags_bits.clean_session =
      session_type == IOTC_SESSION_CLEAN ? 1 : 0;

  msg->connect.keepalive = keepalive_timeout;

err_handling:
  return local_state;
}

static inline iotc_state_t fill_with_publish_data(
    iotc_mqtt_message_t* msg, const char* topic, const iotc_data_desc_t* cnt,
    const iotc_mqtt_qos_t qos, const iotc_mqtt_retain_t retain,
    const iotc_mqtt_dup_t dup, const uint16_t id) {
  iotc_state_t local_state = IOTC_STATE_OK;

  if (cnt->length > IOTC_MQTT_MAX_PAYLOAD_SIZE) {
    return IOTC_MQTT_PAYLOAD_SIZE_TOO_LARGE;
  }

  memset(msg, 0, sizeof(iotc_mqtt_message_t));

  msg->common.common_u.common_bits.retain = retain;
  msg->common.common_u.common_bits.qos = qos;
  msg->common.common_u.common_bits.dup = dup;
  msg->common.common_u.common_bits.type = IOTC_MQTT_TYPE_PUBLISH;
  msg->common.remaining_length = 0;  // This is filled during the serialization.

  IOTC_CHECK_MEMORY(
      msg->publish.topic_name = iotc_make_desc_from_string_share(topic),
      local_state);

  IOTC_CHECK_MEMORY(msg->publish.content = iotc_make_desc_from_buffer_share(
                        cnt->data_ptr, cnt->length),
                    local_state);

  msg->publish.message_id = id;

err_handling:
  return local_state;
}

static inline iotc_state_t fill_with_subscribe_data(iotc_mqtt_message_t* msg,
                                                    const char* topic,
                                                    const uint16_t msg_id,
                                                    const iotc_mqtt_qos_t qos,
                                                    const iotc_mqtt_dup_t dup) {
  iotc_state_t local_state = IOTC_STATE_OK;

  memset(msg, 0, sizeof(iotc_mqtt_message_t));

  msg->common.common_u.common_bits.retain = IOTC_MQTT_RETAIN_FALSE;
  msg->common.common_u.common_bits.qos =
      IOTC_MQTT_QOS_AT_LEAST_ONCE; /* Forced by the protocol. */
  msg->common.common_u.common_bits.dup = dup;
  msg->common.common_u.common_bits.type = IOTC_MQTT_TYPE_SUBSCRIBE;
  msg->common.remaining_length =
      0; /* This is filled during the serialization. */

  IOTC_ALLOC_AT(iotc_mqtt_topicpair_t, msg->subscribe.topics, local_state);

  IOTC_CHECK_MEMORY(
      msg->subscribe.topics->name = iotc_make_desc_from_string_copy(topic),
      local_state);

  msg->subscribe.message_id = msg_id;
  msg->subscribe.topics->iotc_mqtt_topic_pair_payload_u.qos = qos;

err_handling:
  return local_state;
}

static inline iotc_state_t fill_with_disconnect_data(iotc_mqtt_message_t* msg) {
  memset(msg, 0, sizeof(iotc_mqtt_message_t));

  msg->common.common_u.common_bits.type = IOTC_MQTT_TYPE_DISCONNECT;

  return IOTC_STATE_OK;
}

#ifdef __cplusplus
}
#endif

#endif /* __IOTC_MQTT_LOGIC_LAYER_DATA_HELPERS_H__ */
