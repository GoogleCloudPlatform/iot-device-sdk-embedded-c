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

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "iotc_allocator.h"
#include "iotc_coroutine.h"
#include "iotc_layer.h"
#include "iotc_macros.h"
#include "iotc_mqtt_errors.h"
#include "iotc_mqtt_message.h"
#include "iotc_mqtt_parser.h"

static iotc_state_t read_string(iotc_mqtt_parser_t* parser,
                                iotc_data_desc_t** dst, iotc_data_desc_t* src) {
  assert(NULL != parser);
  assert(NULL != dst);
  assert(NULL != src);

  /* Local state. */
  iotc_state_t local_state = IOTC_STATE_OK;
  size_t to_read = 0;
  size_t src_left = 0;
  size_t len_to_read = 0;

  /* Check for the existance of the data descriptor. */
  if (NULL == *dst) {
    IOTC_CHECK_MEMORY(*dst = iotc_make_empty_desc_alloc(4), local_state);
  }

  /* Local variables. */
  to_read = parser->str_length - (*dst)->length;
  src_left = src->length - src->curr_pos;
  len_to_read = IOTC_MIN(to_read, src_left);

  IOTC_CR_START(parser->read_cs);

  IOTC_CR_YIELD_ON(parser->read_cs, ((src->curr_pos - src->length) == 0),
                   IOTC_STATE_WANT_READ);
  parser->str_length = (src->data_ptr[src->curr_pos] << 8);
  src->curr_pos += 1;
  parser->data_length += 1;

  IOTC_CR_YIELD_ON(parser->read_cs, ((src->curr_pos - src->length) == 0),
                   IOTC_STATE_WANT_READ);
  parser->str_length += src->data_ptr[src->curr_pos];
  src->curr_pos += 1;
  parser->data_length += 1;

  to_read = parser->str_length - (*dst)->length;
  src_left = src->length - src->curr_pos;
  len_to_read = IOTC_MIN(to_read, src_left);

  IOTC_CR_YIELD_ON(parser->read_cs, ((src->curr_pos - src->length) == 0),
                   IOTC_STATE_WANT_READ);

  /* @TODO code duplication with read_data -> solve this via
   * code extraction to separate function */
  while (to_read > 0) {
    IOTC_CHECK_STATE(
        local_state = iotc_data_desc_append_data_resize(
            (*dst), (const char*)src->data_ptr + src->curr_pos, len_to_read));

    src->curr_pos += len_to_read;
    to_read -= len_to_read;
    parser->data_length += len_to_read;

    IOTC_CR_YIELD_UNTIL(parser->read_cs, (to_read > 0), IOTC_STATE_WANT_READ);
  }

  IOTC_CR_RESTART(parser->read_cs,
                  (to_read == 0 ? IOTC_STATE_OK : IOTC_MQTT_PARSER_ERROR));

err_handling:
  IOTC_CR_EXIT(parser->read_cs, local_state);

  IOTC_CR_END();
}

iotc_state_t iotc_mqtt_parse_suback_response(iotc_mqtt_suback_status_t* dst,
                                             const uint8_t resp) {
  assert(NULL != dst);

  switch (resp) {
    case IOTC_MQTT_QOS_0_GRANTED:
    case IOTC_MQTT_QOS_1_GRANTED:
    case IOTC_MQTT_QOS_2_GRANTED:
    case IOTC_MQTT_SUBACK_FAILED:
      *dst = (iotc_mqtt_suback_status_t)resp;
      break;
    default:
      return IOTC_MQTT_PARSER_ERROR;
  }

  return IOTC_STATE_OK;
}

static iotc_state_t read_data(iotc_mqtt_parser_t* parser,
                              iotc_data_desc_t** dst, iotc_data_desc_t* src) {
  assert(NULL != parser);
  assert(NULL != dst);
  assert(NULL != src);

  /* local state */
  iotc_state_t local_state = IOTC_STATE_OK;
  size_t to_read = 0;
  size_t src_left = 0;
  size_t len_to_read = 0;

  if (NULL == *dst) {
    IOTC_CHECK_MEMORY(*dst = iotc_make_empty_desc_alloc(4), local_state);
  }

  /* Local variables. */
  to_read = parser->str_length - (*dst)->length;
  src_left = src->length - src->curr_pos;
  len_to_read = IOTC_MIN(to_read, src_left);

  IOTC_CR_START(parser->read_cs);

  IOTC_CR_YIELD_ON(parser->read_cs, ((src->curr_pos - src->length) == 0),
                   IOTC_STATE_WANT_READ);

  while (to_read > 0) {
    IOTC_CHECK_STATE(
        local_state = iotc_data_desc_append_data_resize(
            (*dst), (const char*)src->data_ptr + src->curr_pos, len_to_read));

    src->curr_pos += len_to_read;
    to_read -= len_to_read;

    IOTC_CR_YIELD_UNTIL(parser->read_cs, (to_read > 0), IOTC_STATE_WANT_READ);
  }

  IOTC_CR_RESTART(parser->read_cs,
                  (to_read == 0 ? IOTC_STATE_OK : IOTC_MQTT_PARSER_ERROR));

err_handling:
  IOTC_CR_EXIT(parser->read_cs, local_state);

  IOTC_CR_END();
}

#define READ_STRING(into)                                                  \
  do {                                                                     \
    local_state = read_string(parser, into, src);                          \
    IOTC_CR_YIELD_UNTIL(parser->cs, (local_state == IOTC_STATE_WANT_READ), \
                        IOTC_STATE_WANT_READ);                             \
    if (local_state != IOTC_STATE_OK) {                                    \
      IOTC_CR_EXIT(parser->cs, local_state);                               \
    }                                                                      \
  } while (local_state != IOTC_STATE_OK)

#define READ_DATA(into)                                                    \
  do {                                                                     \
    local_state = read_data(parser, into, src);                            \
    IOTC_CR_YIELD_UNTIL(parser->cs, (local_state == IOTC_STATE_WANT_READ), \
                        IOTC_STATE_WANT_READ);                             \
    if (local_state != IOTC_STATE_OK) {                                    \
      IOTC_CR_EXIT(parser->cs, local_state);                               \
    }                                                                      \
  } while (local_state != IOTC_STATE_OK)

void iotc_mqtt_parser_init(iotc_mqtt_parser_t* parser) {
  memset(parser, 0, sizeof(iotc_mqtt_parser_t));
}

void iotc_mqtt_parser_buffer(iotc_mqtt_parser_t* parser, uint8_t* buffer,
                             size_t buffer_length) {
  parser->buffer_pending = 1;
  parser->buffer = buffer;
  parser->buffer_length = buffer_length;
}

iotc_state_t iotc_mqtt_parser_execute(iotc_mqtt_parser_t* parser,
                                      iotc_mqtt_message_t* message,
                                      iotc_data_desc_t* data_buffer_desc) {
  iotc_data_desc_t* src = data_buffer_desc;
  static iotc_state_t local_state = IOTC_STATE_OK;

  IOTC_CR_START(parser->cs);

  local_state = IOTC_STATE_OK;

  IOTC_CR_YIELD_ON(parser->cs, ((src->curr_pos - src->length) == 0),
                   IOTC_STATE_WANT_READ);

  message->common.common_u.common_value = src->data_ptr[src->curr_pos];

  /* Remaining length. */
  parser->digit_bytes = 0;
  parser->multiplier = 1;
  parser->remaining_length = 0;
  parser->data_length = 1;

  do {
    src->curr_pos += 1;
    parser->digit_bytes += 1;

    IOTC_CR_YIELD_ON(parser->cs, ((src->curr_pos - src->length) == 0),
                     IOTC_STATE_WANT_READ);

    parser->remaining_length +=
        (src->data_ptr[src->curr_pos] & 0x7f) * parser->multiplier;
    parser->multiplier *= 128;
  } while (((uint8_t)src->data_ptr[src->curr_pos] & 0x80) != 0 &&
           parser->digit_bytes < 4);

  if ((uint8_t)src->data_ptr[src->curr_pos] >= 0x80) {
    parser->error = IOTC_MQTT_ERROR_PARSER_INVALID_REMAINING_LENGTH;

    IOTC_CR_EXIT(parser->cs, IOTC_MQTT_PARSER_ERROR);
  }

  message->common.remaining_length = parser->remaining_length;

  src->curr_pos += 1;
  parser->data_length += 1;

  if (message->common.common_u.common_bits.type == IOTC_MQTT_TYPE_CONNECT) {
    READ_STRING(&message->connect.protocol_name);

    IOTC_CR_YIELD_ON(parser->cs, ((src->curr_pos - src->length) == 0),
                     IOTC_STATE_WANT_READ);

    message->connect.protocol_version = src->data_ptr[src->curr_pos];

    src->curr_pos += 1;
    parser->data_length += 1;

    IOTC_CR_YIELD_ON(parser->cs, ((src->curr_pos - src->length) == 0),
                     IOTC_STATE_WANT_READ);

    message->connect.flags_u.flags_value = src->data_ptr[src->curr_pos];

    src->curr_pos += 1;
    parser->data_length += 1;

    IOTC_CR_YIELD_ON(parser->cs, ((src->curr_pos - src->length) == 0),
                     IOTC_STATE_WANT_READ);

    message->connect.keepalive = (src->data_ptr[src->curr_pos] << 8);
    src->curr_pos += 1;
    parser->data_length += 1;

    IOTC_CR_YIELD_ON(parser->cs, ((src->curr_pos - src->length) == 0),
                     IOTC_STATE_WANT_READ);

    message->connect.keepalive += src->data_ptr[src->curr_pos];
    src->curr_pos += 1;
    parser->data_length += 1;

    READ_STRING(&message->connect.client_id);

    if (message->connect.flags_u.flags_bits.will) {
      READ_STRING(&message->connect.will_topic);
    }

    if (message->connect.flags_u.flags_bits.will) {
      READ_STRING(&message->connect.will_message);
    }

    if (message->connect.flags_u.flags_bits.username_follows) {
      READ_STRING(&message->connect.username);
    }

    if (message->connect.flags_u.flags_bits.password_follows) {
      READ_STRING(&message->connect.password);
    }

    IOTC_CR_EXIT(parser->cs, IOTC_STATE_OK);
  } else if (message->common.common_u.common_bits.type ==
             IOTC_MQTT_TYPE_CONNACK) {
    IOTC_CR_YIELD_ON(parser->cs, ((src->curr_pos - src->length) == 0),
                     IOTC_STATE_WANT_READ);

    message->connack._unused = src->data_ptr[src->curr_pos];
    src->curr_pos += 1;
    parser->data_length += 1;

    IOTC_CR_YIELD_ON(parser->cs, ((src->curr_pos - src->length) == 0),
                     IOTC_STATE_WANT_READ);

    message->connack.return_code = src->data_ptr[src->curr_pos];
    src->curr_pos += 1;
    parser->data_length += 1;

    IOTC_CR_EXIT(parser->cs, IOTC_STATE_OK);
  } else if (message->common.common_u.common_bits.type ==
             IOTC_MQTT_TYPE_PUBLISH) {
    IOTC_CR_YIELD_ON(parser->cs, ((src->curr_pos - src->length) == 0),
                     IOTC_STATE_WANT_READ);

    READ_STRING(&message->publish.topic_name);

    IOTC_CR_YIELD_ON(parser->cs, ((src->curr_pos - src->length) == 0),
                     IOTC_STATE_WANT_READ);

    if (message->common.common_u.common_bits.qos > 0) {
      message->publish.message_id = (src->data_ptr[src->curr_pos] << 8);
      src->curr_pos += 1;
      parser->data_length += 1;

      IOTC_CR_YIELD_ON(parser->cs, ((src->curr_pos - src->length) == 0),
                       IOTC_STATE_WANT_READ);

      message->publish.message_id += src->data_ptr[src->curr_pos] & 0xFF;
      src->curr_pos += 1;
      parser->data_length += 1;
    }

    parser->str_length = (parser->remaining_length + 2) - parser->data_length;

    if (parser->str_length > 0) {
      READ_DATA(&message->publish.content);
    }

    IOTC_CR_EXIT(parser->cs, IOTC_STATE_OK);
  } else if (message->common.common_u.common_bits.type ==
             IOTC_MQTT_TYPE_PUBACK) {
    IOTC_CR_YIELD_ON(parser->cs, ((src->curr_pos - src->length) == 0),
                     IOTC_STATE_WANT_READ)

    message->puback.message_id = (src->data_ptr[src->curr_pos] << 8);
    src->curr_pos += 1;
    parser->data_length += 1;

    IOTC_CR_YIELD_ON(parser->cs, ((src->curr_pos - src->length) == 0),
                     IOTC_STATE_WANT_READ)

    message->puback.message_id += src->data_ptr[src->curr_pos] & 0xFF;
    src->curr_pos += 1;
    parser->data_length += 1;

    IOTC_CR_EXIT(parser->cs, IOTC_STATE_OK);
  } else if (message->common.common_u.common_bits.type ==
             IOTC_MQTT_TYPE_PUBREC) {
    IOTC_CR_YIELD_ON(parser->cs, ((src->curr_pos - src->length) == 0),
                     IOTC_STATE_WANT_READ)

    message->pubrec.message_id = (src->data_ptr[src->curr_pos] << 8);
    src->curr_pos += 1;
    parser->data_length += 1;

    IOTC_CR_YIELD_ON(parser->cs, ((src->curr_pos - src->length) == 0),
                     IOTC_STATE_WANT_READ)

    message->pubrec.message_id += src->data_ptr[src->curr_pos] & 0xFF;
    src->curr_pos += 1;
    parser->data_length += 1;

    IOTC_CR_EXIT(parser->cs, IOTC_STATE_OK);
  } else if (message->common.common_u.common_bits.type ==
             IOTC_MQTT_TYPE_PUBREL) {
    IOTC_CR_YIELD_ON(parser->cs, ((src->curr_pos - src->length) == 0),
                     IOTC_STATE_WANT_READ);

    message->pubrel.message_id = (src->data_ptr[src->curr_pos] << 8);
    src->curr_pos += 1;
    parser->data_length += 1;

    IOTC_CR_YIELD_ON(parser->cs, ((src->curr_pos - src->length) == 0),
                     IOTC_STATE_WANT_READ);

    message->pubrel.message_id += src->data_ptr[src->curr_pos] & 0xFF;
    src->curr_pos += 1;
    parser->data_length += 1;

    IOTC_CR_EXIT(parser->cs, IOTC_STATE_OK);
  } else if (message->common.common_u.common_bits.type ==
             IOTC_MQTT_TYPE_PUBCOMP) {
    IOTC_CR_YIELD_ON(parser->cs, ((src->curr_pos - src->length) == 0),
                     IOTC_STATE_WANT_READ);

    message->pubcomp.message_id = (src->data_ptr[src->curr_pos] << 8);
    src->curr_pos += 1;
    parser->data_length += 1;

    IOTC_CR_YIELD_ON(parser->cs, ((src->curr_pos - src->length) == 0),
                     IOTC_STATE_WANT_READ);

    message->pubcomp.message_id += src->data_ptr[src->curr_pos] & 0xFF;
    src->curr_pos += 1;
    parser->data_length += 1;

    IOTC_CR_EXIT(parser->cs, IOTC_STATE_OK);
  } else if (message->common.common_u.common_bits.type ==
             IOTC_MQTT_TYPE_SUBSCRIBE) {
    IOTC_CR_YIELD_ON(parser->cs, ((src->curr_pos - src->length) == 0),
                     IOTC_STATE_WANT_READ)

    message->subscribe.message_id = (src->data_ptr[src->curr_pos] << 8);
    src->curr_pos += 1;
    parser->data_length += 1;

    IOTC_CR_YIELD_ON(parser->cs, ((src->curr_pos - src->length) == 0),
                     IOTC_STATE_WANT_READ);

    message->subscribe.message_id += src->data_ptr[src->curr_pos] & 0xFF;
    src->curr_pos += 1;
    parser->data_length += 1;

    IOTC_CR_YIELD_ON(parser->cs, ((src->curr_pos - src->length) == 0),
                     IOTC_STATE_WANT_READ);

    IOTC_ALLOC_AT(iotc_mqtt_topicpair_t, message->subscribe.topics,
                  local_state);

    READ_STRING(&message->subscribe.topics->name);

    IOTC_CR_YIELD_ON(parser->cs, ((src->curr_pos - src->length) == 0),
                     IOTC_STATE_WANT_READ);

    message->subscribe.topics->iotc_mqtt_topic_pair_payload_u.qos =
        (iotc_mqtt_qos_t)src->data_ptr[src->curr_pos];
    src->curr_pos += 1;
    parser->data_length += 1;

    IOTC_CR_EXIT(parser->cs, IOTC_STATE_OK);
  } else if (message->common.common_u.common_bits.type ==
             IOTC_MQTT_TYPE_SUBACK) {
    IOTC_CR_YIELD_ON(parser->cs, ((src->curr_pos - src->length) == 0),
                     IOTC_STATE_WANT_READ);

    message->suback.message_id = (src->data_ptr[src->curr_pos] << 8);
    src->curr_pos += 1;
    parser->data_length += 1;

    IOTC_CR_YIELD_ON(parser->cs, ((src->curr_pos - src->length) == 0),
                     IOTC_STATE_WANT_READ);

    message->suback.message_id += src->data_ptr[src->curr_pos] & 0xFF;
    src->curr_pos += 1;
    parser->data_length += 1;

    IOTC_CR_YIELD_ON(parser->cs, ((src->curr_pos - src->length) == 0),
                     IOTC_STATE_WANT_READ);

    IOTC_ALLOC_AT(iotc_mqtt_topicpair_t, message->suback.topics, local_state);

    IOTC_CHECK_STATE(
        local_state = iotc_mqtt_parse_suback_response(
            &message->suback.topics->iotc_mqtt_topic_pair_payload_u.status,
            src->data_ptr[src->curr_pos]));

    src->curr_pos += 1;
    parser->data_length += 1;

    IOTC_CR_EXIT(parser->cs, IOTC_STATE_OK);
  } else if (message->common.common_u.common_bits.type ==
                 IOTC_MQTT_TYPE_PINGREQ ||
             message->common.common_u.common_bits.type ==
                 IOTC_MQTT_TYPE_PINGRESP) {
    /* Nothing to parse. */
    IOTC_CR_EXIT(parser->cs, IOTC_STATE_OK);
  } else if (message->common.common_u.common_bits.type ==
             IOTC_MQTT_TYPE_DISCONNECT) {
    /* Nothing to parse. */
    IOTC_CR_EXIT(parser->cs, IOTC_STATE_OK);
  } else {
    iotc_debug_logger("IOTC_MQTT_ERROR_PARSER_INVALID_MESSAGE_ID");
    parser->error = IOTC_MQTT_ERROR_PARSER_INVALID_MESSAGE_ID;
    IOTC_CR_RESET(parser->read_cs);
    IOTC_CR_EXIT(parser->cs, IOTC_MQTT_PARSER_ERROR);
  }

err_handling:
  IOTC_CR_EXIT(parser->cs, local_state);

  IOTC_CR_END();
}
