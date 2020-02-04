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

#include <string.h>

#include "iotc_debug.h"
#include "iotc_macros.h"
#include "iotc_mqtt_message.h"
#include "iotc_mqtt_serialiser.h"

#define WRITE_8(out, byte) \
  IOTC_CHECK_STATE(iotc_data_desc_append_byte(out, byte))

#define WRITE_16(out, value) \
  WRITE_8(out, value >> 8);  \
  WRITE_8(out, value & 0xFF)

#define WRITE_STRING(out, str)                              \
  if (NULL != str) {                                        \
    WRITE_16(out, str->length);                             \
    IOTC_CHECK_STATE(iotc_data_desc_append_data(out, str)); \
  } else {                                                  \
    WRITE_16(out, 0);                                       \
  }

#define WRITE_DATA(out, data) \
  IOTC_CHECK_STATE(iotc_data_desc_append_data(out, data))

void iotc_mqtt_serialiser_init(iotc_mqtt_serialiser_t* serialiser) {
  memset(serialiser, 0, sizeof(iotc_mqtt_serialiser_t));
}

static uint8_t iotc_mqtt_get_remaining_length_bytes(
    const size_t remaining_length) {
  if (remaining_length <= 127) {
    return 1;
  } else if (remaining_length <= 16383) {
    return 2;
  } else if (remaining_length <= 2097151) {
    return 3;
  } else if (remaining_length <= 268435455) {
    return 4;
  }

  /* Terrible failure! */
  assert(0 == 1);
  return 0;
}

iotc_state_t iotc_mqtt_serialiser_size(size_t* msg_len, size_t* remaining_len,
                                       size_t* publish_payload_len,
                                       iotc_mqtt_serialiser_t* serialiser,
                                       const iotc_mqtt_message_t* message) {
  if (NULL == msg_len || NULL == remaining_len || NULL == publish_payload_len) {
    return IOTC_INVALID_PARAMETER;
  }

  /* Reset the values. */
  *msg_len = 0;
  *remaining_len = 0;
  *publish_payload_len = 0;

  /* This is currently unused, however it may be used
   * to save the state of serialiser in case of serialializing message
   * in turns. */
  (void)serialiser;

  /* First byte is for fixed header. */
  *msg_len = 1;

  if (message->common.common_u.common_bits.type == IOTC_MQTT_TYPE_CONNECT) {
    /* @TODO: Should get this first len from the actual protocol_name
     * variable, instead of hardcoding it. */
    *msg_len += 6; /* Protocol name. */
    *msg_len += 1; /* Protocol version. */
    *msg_len += 1; /* Connect flags. */
    *msg_len += 2; /* Keep alive timer. */

    *msg_len += 2; /* Size of client id length. */

    if (NULL != message->connect.client_id) {
      *msg_len += message->connect.client_id->length;
    }

    if (message->connect.flags_u.flags_bits.username_follows) {
      *msg_len += 2; /* Size of username length. */
      *msg_len += message->connect.username->length;
    }

    if (message->connect.flags_u.flags_bits.password_follows) {
      *msg_len += 2; /* Size of password length. */
      *msg_len += message->connect.password->length;
    }

    if (message->connect.flags_u.flags_bits.will) {
      *msg_len += 4; /* Size of will topic length and size of
                      * will message length. */
      *msg_len += message->connect.will_topic->length;
      *msg_len += message->connect.will_message->length;
    }
  } else if (message->common.common_u.common_bits.type ==
             IOTC_MQTT_TYPE_CONNACK) {
    *msg_len += 2;
  } else if (message->common.common_u.common_bits.type ==
             IOTC_MQTT_TYPE_PUBLISH) {
    *msg_len += 2; /* Size. */
    *msg_len += message->publish.topic_name->length;

    if (message->publish.common.common_u.common_bits.qos > 0) {
      *msg_len += 2; /* Size. */
    }

    *msg_len += message->publish.content->length;
    *publish_payload_len += message->publish.content->length;
  } else if (message->common.common_u.common_bits.type ==
             IOTC_MQTT_TYPE_PUBACK) {
    *msg_len += 2; /* Size of the msg id. */
  } else if (message->common.common_u.common_bits.type ==
             IOTC_MQTT_TYPE_DISCONNECT) {
    /* Empty. */
  } else if (message->common.common_u.common_bits.type ==
             IOTC_MQTT_TYPE_SUBSCRIBE) {
    *msg_len += 2; /* Size msgid. */
    *msg_len += 2; /* Size of topic. */

    /* @TODO add support for multiple topics per request */
    *msg_len += message->subscribe.topics->name->length;
    *msg_len += 1; /* QoS. */
  } else if (message->common.common_u.common_bits.type ==
             IOTC_MQTT_TYPE_SUBACK) {
    *msg_len += 2; /* Xize of the msg id. */
    *msg_len += 1; /* QoS */
  } else if (message->common.common_u.common_bits.type ==
                 IOTC_MQTT_TYPE_PINGREQ ||
             message->common.common_u.common_bits.type ==
                 IOTC_MQTT_TYPE_PINGRESP) {
    /* Just a fixed header. */
  } else {
    return IOTC_MQTT_SERIALIZER_ERROR;
  }

  *remaining_len = *msg_len - 1; /* Because remaining length does not contain
                                  * the size of fixed header. */
  *msg_len += iotc_mqtt_get_remaining_length_bytes(*remaining_len);

  return IOTC_STATE_OK;
}

iotc_mqtt_serialiser_rc_t iotc_mqtt_serialiser_write(
    iotc_mqtt_serialiser_t* serialiser, const iotc_mqtt_message_t* message,
    iotc_data_desc_t* buffer, const size_t message_len,
    const size_t remaining_len) {
  IOTC_UNUSED(message_len);

  size_t tmp_remaining_len = 0;
  uint32_t value = 0;

  WRITE_8(buffer, message->common.common_u.common_value);

  tmp_remaining_len = remaining_len;

  do {
    /* Mask the least significant 7 bits. */
    value = tmp_remaining_len & 0x7f;

    /* Shift so the rest of the number become availible. */
    tmp_remaining_len >>= 7;

    /* If the value is greater set the continuation bit. */
    WRITE_8(buffer, value | (tmp_remaining_len > 0 ? 0x80 : 0x0));
  } while (tmp_remaining_len > 0);

  switch (message->common.common_u.common_bits.type) {
    case IOTC_MQTT_TYPE_CONNECT: {
      WRITE_STRING(buffer, message->connect.protocol_name);

      WRITE_8(buffer, message->connect.protocol_version);
      WRITE_8(buffer, message->connect.flags_u.flags_value);

      WRITE_16(buffer, message->connect.keepalive);

      WRITE_STRING(buffer, message->connect.client_id);

      if (message->connect.flags_u.flags_bits.will) {
        WRITE_STRING(buffer, message->connect.will_topic);
        WRITE_STRING(buffer, message->connect.will_message);
      }

      if (message->connect.flags_u.flags_bits.username_follows) {
        WRITE_STRING(buffer, message->connect.username);
      }

      if (message->connect.flags_u.flags_bits.password_follows) {
        WRITE_STRING(buffer, message->connect.password);
      }

      break;
    }

    case IOTC_MQTT_TYPE_CONNACK: {
      WRITE_8(buffer, message->connack._unused);
      WRITE_8(buffer, message->connack.return_code);

      break;
    }

    case IOTC_MQTT_TYPE_PUBLISH: {
      WRITE_STRING(buffer, message->publish.topic_name);

      if (message->common.common_u.common_bits.qos > 0) {
        WRITE_16(buffer, message->publish.message_id);
      }

      /* Because the publish payload is being sent in separation by the
       * mqtt codec layer. Writing the payload data is no longer part
       * of a serialisation process so that line is obsolete in current
       * version.
       *
       * This comment is to remember about that very important line of code
       * whenever we will change the implementation of the mqtt_codec_layer
       * WRITE_DATA( buffer,message->publish.content ) */

      break;
    }

    case IOTC_MQTT_TYPE_PUBACK: {
      WRITE_16(buffer, message->puback.message_id);

      break;
    }

    case IOTC_MQTT_TYPE_SUBSCRIBE: {
      /* Write the message identifier the subscribe is using
       * the QoS 1 anyway. */

      WRITE_16(buffer, message->subscribe.message_id);

      WRITE_STRING(buffer, message->subscribe.topics->name);

      WRITE_8(
          buffer,
          message->subscribe.topics->iotc_mqtt_topic_pair_payload_u.qos & 0xFF);
      break;
    }

    case IOTC_MQTT_TYPE_SUBACK: {
      WRITE_16(buffer, message->suback.message_id);

      WRITE_8(buffer,
              message->subscribe.topics->iotc_mqtt_topic_pair_payload_u.status &
                  0xFF);
      break;
    }

    case IOTC_MQTT_TYPE_DISCONNECT: {
      /* Empty. */
      break;
    }

    case IOTC_MQTT_TYPE_PINGREQ:
    case IOTC_MQTT_TYPE_PINGRESP: {
      /* Empty. */
      break;
    }

    default: {
      serialiser->error = IOTC_MQTT_ERROR_SERIALISER_INVALID_MESSAGE_ID;
      return IOTC_MQTT_SERIALISER_RC_ERROR;
    }
  }

  return IOTC_MQTT_SERIALISER_RC_SUCCESS;

err_handling:
  return IOTC_MQTT_SERIALISER_RC_ERROR;
}
