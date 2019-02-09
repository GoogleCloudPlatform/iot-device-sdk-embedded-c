/* Copyright 2018 - 2019 Google LLC
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
#include <stdio.h>
#include <string.h>

#include "iotc_allocator.h"
#include "iotc_debug.h"
#include "iotc_macros.h"
#include "iotc_mqtt_message.h"

#if IOTC_DEBUG_OUTPUT
void iotc_debug_mqtt_message_dump(const iotc_mqtt_message_t* message) {
  iotc_debug_printf("message\n");
  iotc_debug_printf("  type:              %d\n",
                    message->common.common_u.common_bits.type);
  iotc_debug_printf("  qos:               %d\n",
                    message->common.common_u.common_bits.qos);
  iotc_debug_printf(
      "  dup:               %s\n",
      message->common.common_u.common_bits.dup ? "true" : "false");
  iotc_debug_printf(
      "  retain:            %s\n",
      message->common.common_u.common_bits.retain ? "true" : "false");

  if (message->common.common_u.common_bits.type == IOTC_MQTT_TYPE_CONNECT) {
    iotc_debug_printf("  protocol name:     ");
    iotc_debug_data_desc_dump((message->connect.protocol_name));
    iotc_debug_printf("\n");

    iotc_debug_printf("  protocol version:  %d\n",
                      message->connect.protocol_version);

    iotc_debug_printf("  has username:      %s\n",
                      message->connect.flags_u.flags_bits.username_follows
                          ? "true"
                          : "false");
    iotc_debug_printf("  has password:      %s\n",
                      message->connect.flags_u.flags_bits.password_follows
                          ? "true"
                          : "false");
    iotc_debug_printf(
        "  has will:          %s\n",
        message->connect.flags_u.flags_bits.will ? "true" : "false");
    iotc_debug_printf("  will qos:          %d\n",
                      message->connect.flags_u.flags_bits.will_qos);
    iotc_debug_printf(
        "  retains will:      %s\n",
        message->connect.flags_u.flags_bits.will_retain ? "true" : "false");
    iotc_debug_printf(
        "  clean session:     %s\n",
        message->connect.flags_u.flags_bits.clean_session ? "true" : "false");

    iotc_debug_printf("  keep alive:        %d\n", message->connect.keepalive);

    iotc_debug_printf("  client id:         ");
    iotc_debug_data_desc_dump((message->connect.client_id));
    iotc_debug_printf("\n");

    iotc_debug_printf("  will topic:        ");
    iotc_debug_data_desc_dump((message->connect.will_topic));
    iotc_debug_printf("\n");
    iotc_debug_printf("  will message:      ");
    iotc_debug_data_desc_dump((message->connect.will_message));
    iotc_debug_printf("\n");

    iotc_debug_printf("  username:          ");
    iotc_debug_data_desc_dump((message->connect.username));
    iotc_debug_printf("\n");
    iotc_debug_printf("  password:          ");
    iotc_debug_data_desc_dump((message->connect.password));
    iotc_debug_printf("\n");
  } else if (message->common.common_u.common_bits.type ==
             IOTC_MQTT_TYPE_PUBLISH) {
    iotc_debug_printf("  message_id:        %d\n", message->publish.message_id);
    iotc_debug_printf("topic_name: \n");
    if (message->publish.topic_name)
      iotc_debug_data_desc_dump(message->publish.topic_name);
    iotc_debug_printf("\n");
    iotc_debug_printf("content: \n");
    if (message->publish.content)
      iotc_debug_data_desc_dump(message->publish.content);
    iotc_debug_printf("\n");
  } else if (message->common.common_u.common_bits.type ==
             IOTC_MQTT_TYPE_PUBACK) {
    iotc_debug_printf("  message_id:        %d\n", message->puback.message_id);
  }
}
#endif

void iotc_mqtt_message_free(iotc_mqtt_message_t** msg) {
  if (msg == NULL || *msg == NULL) {
    return;
  }

  iotc_mqtt_message_t* m = *msg;

  switch (m->common.common_u.common_bits.type) {
    case IOTC_MQTT_TYPE_CONNECT:
      if (m->connect.client_id) iotc_free_desc(&m->connect.client_id);
      if (m->connect.password) iotc_free_desc(&m->connect.password);
      if (m->connect.username) iotc_free_desc(&m->connect.username);
      if (m->connect.protocol_name) iotc_free_desc(&m->connect.protocol_name);
      if (m->connect.will_message) iotc_free_desc(&m->connect.will_message);
      if (m->connect.will_topic) iotc_free_desc(&m->connect.will_topic);
      break;
    case IOTC_MQTT_TYPE_PUBLISH:
      if (m->publish.content) iotc_free_desc(&m->publish.content);
      if (m->publish.topic_name) iotc_free_desc(&m->publish.topic_name);
      break;
    case IOTC_MQTT_TYPE_SUBSCRIBE:
      if (m->subscribe.topics) {
        if (m->subscribe.topics) iotc_free_desc(&m->subscribe.topics->name);
        IOTC_SAFE_FREE(m->subscribe.topics);
      }
      break;
    case IOTC_MQTT_TYPE_SUBACK:
      if (m->suback.topics) {
        if (m->suback.topics->name) iotc_free_desc(&m->suback.topics->name);
        IOTC_SAFE_FREE(m->suback.topics);
      }
      break;
  }

  IOTC_SAFE_FREE(*msg);
}

uint16_t iotc_mqtt_get_message_id(const iotc_mqtt_message_t* msg) {
  switch (msg->common.common_u.common_bits.type) {
    case IOTC_MQTT_TYPE_CONNECT:
      return 0;
    case IOTC_MQTT_TYPE_CONNACK:
      return 0;
    case IOTC_MQTT_TYPE_PUBLISH:
      return msg->publish.message_id;
    case IOTC_MQTT_TYPE_PUBACK:
      return msg->puback.message_id;
    case IOTC_MQTT_TYPE_PUBREC:
      return msg->pubrec.message_id;
    case IOTC_MQTT_TYPE_PUBREL:
      return msg->pubrel.message_id;
    case IOTC_MQTT_TYPE_PUBCOMP:
      return msg->pubcomp.message_id;
    case IOTC_MQTT_TYPE_SUBSCRIBE:
      return msg->subscribe.message_id;
    case IOTC_MQTT_TYPE_SUBACK:
      return msg->suback.message_id;
    case IOTC_MQTT_TYPE_UNSUBACK:
      return msg->unsuback.message_id;
    case IOTC_MQTT_TYPE_PINGREQ:
      return 0;
    case IOTC_MQTT_TYPE_PINGRESP:
      return 0;
    case IOTC_MQTT_TYPE_DISCONNECT:
      return 0;
    default:
      iotc_debug_logger("unhandled msg type... ");
  }

  return 0;
}

iotc_mqtt_message_class_t iotc_mqtt_class_msg_type_receiving(
    const iotc_mqtt_type_t msg_type) {
  switch (msg_type) {
    case IOTC_MQTT_TYPE_PUBLISH:
    case IOTC_MQTT_TYPE_PUBREL:
      return IOTC_MQTT_MESSAGE_CLASS_FROM_SERVER;
    case IOTC_MQTT_TYPE_PUBACK:
    case IOTC_MQTT_TYPE_PUBREC:
    case IOTC_MQTT_TYPE_PUBCOMP:
    case IOTC_MQTT_TYPE_SUBACK:
    case IOTC_MQTT_TYPE_UNSUBACK:
    case IOTC_MQTT_TYPE_PINGRESP:
      return IOTC_MQTT_MESSAGE_CLASS_TO_SERVER;
    default:
      iotc_debug_logger("unhandled msg type...");
      return IOTC_MQTT_MESSAGE_CLASS_UNKNOWN;
  }
}

iotc_mqtt_message_class_t iotc_mqtt_class_msg_type_sending(
    const iotc_mqtt_type_t msg_type) {
  switch (msg_type) {
    case IOTC_MQTT_TYPE_PUBACK:
    case IOTC_MQTT_TYPE_PUBREC:
    case IOTC_MQTT_TYPE_PUBCOMP:
      return IOTC_MQTT_MESSAGE_CLASS_FROM_SERVER;
    case IOTC_MQTT_TYPE_PUBLISH:
    case IOTC_MQTT_TYPE_PUBREL:
    case IOTC_MQTT_TYPE_SUBSCRIBE:
    case IOTC_MQTT_TYPE_UNSUBSCRIBE:
    case IOTC_MQTT_TYPE_PINGREQ:
    case IOTC_MQTT_TYPE_DISCONNECT:
      return IOTC_MQTT_MESSAGE_CLASS_TO_SERVER;
    default:
      iotc_debug_logger("unhandled msg type...");
      return IOTC_MQTT_MESSAGE_CLASS_UNKNOWN;
  }
}

iotc_state_t iotc_mqtt_convert_to_qos(unsigned int qos,
                                      iotc_mqtt_qos_t* qos_out) {
  if (NULL == qos_out) {
    return IOTC_INVALID_PARAMETER;
  }

  switch (qos) {
    case 0:
      *qos_out = IOTC_MQTT_QOS_AT_MOST_ONCE;
      break;
    case 1:
      *qos_out = IOTC_MQTT_QOS_AT_LEAST_ONCE;
      break;
    case 2:
      *qos_out = IOTC_MQTT_QOS_EXACTLY_ONCE;
      break;
    default:
      return IOTC_INVALID_PARAMETER;
  }

  return IOTC_STATE_OK;
}

iotc_state_t iotc_mqtt_convert_to_dup(unsigned int dup,
                                      iotc_mqtt_dup_t* dup_out) {
  if (NULL == dup_out) {
    return IOTC_INVALID_PARAMETER;
  }

  switch (dup) {
    case 0:
      *dup_out = IOTC_MQTT_DUP_FALSE;
      break;
    case 1:
      *dup_out = IOTC_MQTT_DUP_TRUE;
      break;
    default:
      return IOTC_INVALID_PARAMETER;
  }

  return IOTC_STATE_OK;
}

iotc_state_t iotc_mqtt_convert_to_retain(unsigned int retain,
                                         iotc_mqtt_retain_t* retain_out) {
  if (NULL == retain_out) {
    return IOTC_INVALID_PARAMETER;
  }

  switch (retain) {
    case 0:
      *retain_out = IOTC_MQTT_RETAIN_FALSE;
      break;
    case 1:
      *retain_out = IOTC_MQTT_RETAIN_TRUE;
      break;
    default:
      return IOTC_INVALID_PARAMETER;
  }

  return IOTC_STATE_OK;
}
