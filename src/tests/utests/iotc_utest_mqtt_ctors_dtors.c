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

#include "iotc_tt_testcase_management.h"
#include "tinytest.h"
#include "tinytest_macros.h"

#include "iotc.h"
#include "iotc_err.h"
#include "iotc_helpers.h"
#include "iotc_mqtt_logic_layer_data_helpers.h"
#include "iotc_mqtt_message.h"

#include "iotc_memory_checks.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifndef IOTC_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN

typedef struct iotc_utest_mqtt_message_details_s {
  unsigned qos : 2;
  unsigned retain : 1;
  unsigned dup : 1;
} iotc_utest_mqtt_message_details_t;

typedef struct iotc_utest_mqtt_message_details_to_uint16_t_s {
  union {
    iotc_utest_mqtt_message_details_t qos_retain_dup;
    uint8_t value;
  } iotc_mqtt_union;
} iotc_utest_mqtt_message_details_to_uint16_t;

static void utest__fill_with_pingreq_data__valid_data__pingreq_msg_help(void) {
  iotc_state_t local_state = IOTC_STATE_OK;

  iotc_mqtt_message_t* msg_matrix = NULL;
  iotc_mqtt_message_t* msg = NULL;

  IOTC_ALLOC_AT(iotc_mqtt_message_t, msg, local_state);
  IOTC_ALLOC_AT(iotc_mqtt_message_t, msg_matrix, local_state);

  tt_want_int_op(fill_with_pingreq_data(msg), ==, IOTC_STATE_OK);

  msg_matrix->common.common_u.common_bits.type = IOTC_MQTT_TYPE_PINGREQ;
  tt_want_int_op(memcmp(msg, msg_matrix, sizeof(iotc_mqtt_message_t)), ==, 0);

  iotc_mqtt_message_free(&msg);
  iotc_mqtt_message_free(&msg_matrix);

  tt_want_int_op(iotc_is_whole_memory_deallocated(), >, 0);

  return;

err_handling:
  tt_abort_msg(("test must not fail!"));
end:
  iotc_mqtt_message_free(&msg);
  iotc_mqtt_message_free(&msg_matrix);
  ;
}

static void
utest__fill_with_publish_data__valid_data_max_payload__publish_msg_help() {
  iotc_state_t local_state = IOTC_STATE_OK;

  iotc_mqtt_message_t* msg = NULL;
  iotc_mqtt_message_t* msg_matrix = NULL;

  const size_t cnt_size = IOTC_MQTT_MAX_PAYLOAD_SIZE;

  const char topic[] = "test_topic";

  iotc_data_desc_t* content = iotc_make_empty_desc_alloc(cnt_size);
  memset((void*)content->data_ptr, 'a', cnt_size);
  content->length = cnt_size;

  // test for all combinations of common
  size_t i = 0;
  for (i = 0; i < 16; ++i) {
    IOTC_ALLOC_AT(iotc_mqtt_message_t, msg, local_state);
    IOTC_ALLOC_AT(iotc_mqtt_message_t, msg_matrix, local_state);

    iotc_utest_mqtt_message_details_to_uint16_t mqtt_common = {{.value = i}};

    tt_want_int_op(
        fill_with_publish_data(
            msg, topic, content, mqtt_common.iotc_mqtt_union.qos_retain_dup.qos,
            mqtt_common.iotc_mqtt_union.qos_retain_dup.retain,
            mqtt_common.iotc_mqtt_union.qos_retain_dup.dup, 17),
        ==, IOTC_STATE_OK);

    msg_matrix->common.common_u.common_bits.retain =
        mqtt_common.iotc_mqtt_union.qos_retain_dup.retain;
    msg_matrix->common.common_u.common_bits.qos =
        mqtt_common.iotc_mqtt_union.qos_retain_dup.qos;
    msg_matrix->common.common_u.common_bits.dup =
        mqtt_common.iotc_mqtt_union.qos_retain_dup.dup;
    msg_matrix->common.common_u.common_bits.type = IOTC_MQTT_TYPE_PUBLISH;
    msg_matrix->common.remaining_length = 0;

    msg_matrix->publish.topic_name = msg->publish.topic_name;
    msg_matrix->publish.content = msg->publish.content;

    tt_want_int_op(
        memcmp(msg->publish.topic_name->data_ptr, topic, sizeof(topic) - 1), ==,
        0);
    tt_want_int_op(memcmp(msg->publish.content->data_ptr, content->data_ptr,
                          content->length),
                   ==, 0);

    msg_matrix->publish.message_id = 17;

    tt_want_int_op(memcmp(msg, msg_matrix, sizeof(iotc_mqtt_message_t)), ==, 0);

    msg_matrix->publish.topic_name = 0;
    msg_matrix->publish.content = 0;

    iotc_mqtt_message_free(&msg);
    iotc_mqtt_message_free(&msg_matrix);
  }

  iotc_free_desc(&content);
  tt_want_int_op(iotc_is_whole_memory_deallocated(), >, 0);

  return;

err_handling:
  tt_abort_msg(("test must not fail!"));
end:
  iotc_mqtt_message_free(&msg);
  iotc_mqtt_message_free(&msg_matrix);
  iotc_free_desc(&content);
}

#endif

IOTC_TT_TESTGROUP_BEGIN(utest_mqtt_ctors_dtors)

IOTC_TT_TESTCASE(utest__fill_with_pingreq_data__valid_data__pingreq_msg, {
  utest__fill_with_pingreq_data__valid_data__pingreq_msg_help();
})

IOTC_TT_TESTCASE(utest__fill_with_puback_data__valid_data__puback_msg, {
  iotc_state_t local_state = IOTC_STATE_OK;

  iotc_mqtt_message_t* msg = NULL;
  iotc_mqtt_message_t* msg_matrix = NULL;

  IOTC_ALLOC_AT(iotc_mqtt_message_t, msg, local_state);
  IOTC_ALLOC_AT(iotc_mqtt_message_t, msg_matrix, local_state);

  size_t i = 0;

  for (; i < 0xFFFF; ++i) {
    msg_matrix->common.common_u.common_bits.type = IOTC_MQTT_TYPE_PUBACK;
    msg_matrix->puback.message_id = i;

    tt_want_int_op(fill_with_puback_data(msg, i), ==, IOTC_STATE_OK);
    tt_want_int_op(memcmp(msg, msg_matrix, sizeof(iotc_mqtt_message_t)), ==, 0);
  }

  iotc_mqtt_message_free(&msg);
  iotc_mqtt_message_free(&msg_matrix);

  tt_want_int_op(iotc_is_whole_memory_deallocated(), >, 0);

  return;

err_handling:
  tt_abort_msg(("test must not fail!"));
end:
  iotc_mqtt_message_free(&msg);
  iotc_mqtt_message_free(&msg_matrix);
})

IOTC_TT_TESTCASE(utest__fill_with_connect_data__valid_data__connect_msg, {
  iotc_state_t local_state = IOTC_STATE_OK;

  const char username[] = "test_username";
  const char password[] = "test_password";

  iotc_mqtt_message_t* msg = NULL;
  iotc_mqtt_message_t* msg_matrix = NULL;

  IOTC_ALLOC_AT(iotc_mqtt_message_t, msg, local_state);
  IOTC_ALLOC_AT(iotc_mqtt_message_t, msg_matrix, local_state);

  tt_want_int_op(fill_with_connect_data(msg, username, password, 7,
                                        IOTC_SESSION_CLEAN, NULL, NULL, 0, 0),
                 ==, IOTC_STATE_OK);

  msg_matrix->common.common_u.common_bits.retain = IOTC_MQTT_RETAIN_FALSE;
  msg_matrix->common.common_u.common_bits.qos = IOTC_MQTT_QOS_AT_MOST_ONCE;
  msg_matrix->common.common_u.common_bits.dup = IOTC_MQTT_DUP_FALSE;
  msg_matrix->common.common_u.common_bits.type = IOTC_MQTT_TYPE_CONNECT;

  msg_matrix->connect.protocol_version = 4;
  msg_matrix->connect.protocol_name = msg->connect.protocol_name;

  msg_matrix->common.remaining_length = 0;

  tt_want_int_op(
      memcmp(msg->connect.protocol_name->data_ptr, "MQTT", sizeof("MQTT") - 1),
      ==, 0);

  tt_want_int_op(
      memcmp(msg->connect.client_id->data_ptr, username, sizeof(username) - 1),
      ==, 0);

  tt_want_int_op(
      memcmp(msg->connect.password->data_ptr, password, sizeof(password) - 1),
      ==, 0);

  msg_matrix->connect.client_id = msg->connect.client_id;
  msg_matrix->connect.username = msg->connect.username;
  msg_matrix->connect.password = msg->connect.password;
  msg_matrix->connect.keepalive = msg->connect.keepalive;
  msg_matrix->connect.flags_u = msg->connect.flags_u;

  tt_want_int_op(memcmp(msg, msg_matrix, sizeof(iotc_mqtt_message_t)), ==, 0);

  msg_matrix->connect.protocol_name = 0;
  msg_matrix->connect.client_id = 0;
  msg_matrix->connect.username = 0;
  msg_matrix->connect.password = 0;

  iotc_mqtt_message_free(&msg);
  iotc_mqtt_message_free(&msg_matrix);

  tt_want_int_op(iotc_is_whole_memory_deallocated(), >, 0);

  return;

err_handling:
  tt_abort_msg(("test must not fail!"));
end:
  iotc_mqtt_message_free(&msg);
  iotc_mqtt_message_free(&msg_matrix);
})

IOTC_TT_TESTCASE(
    utest__fill_with_connect_data__valid_data_no_password__connect_msg, {
      iotc_state_t local_state = IOTC_STATE_OK;

      const char username[] = "test_username";
      const char* password = 0;

      iotc_mqtt_message_t* msg = NULL;
      iotc_mqtt_message_t* msg_matrix = NULL;

      IOTC_ALLOC_AT(iotc_mqtt_message_t, msg, local_state);
      IOTC_ALLOC_AT(iotc_mqtt_message_t, msg_matrix, local_state);

      tt_want_int_op(
          fill_with_connect_data(msg, username, password, 7, IOTC_SESSION_CLEAN,
                                 NULL, NULL, 0, 0),
          ==, IOTC_STATE_OK);

      msg_matrix->common.common_u.common_bits.retain = IOTC_MQTT_RETAIN_FALSE;
      msg_matrix->common.common_u.common_bits.qos = IOTC_MQTT_QOS_AT_MOST_ONCE;
      msg_matrix->common.common_u.common_bits.dup = IOTC_MQTT_DUP_FALSE;
      msg_matrix->common.common_u.common_bits.type = IOTC_MQTT_TYPE_CONNECT;

      msg_matrix->connect.protocol_version = 4;
      msg_matrix->connect.protocol_name = msg->connect.protocol_name;

      msg_matrix->common.remaining_length = 0;

      tt_want_int_op(memcmp(msg->connect.protocol_name->data_ptr, "MQTT",
                            sizeof("MQTT") - 1),
                     ==, 0);

      tt_want_int_op(memcmp(msg->connect.client_id->data_ptr, username,
                            sizeof(username) - 1),
                     ==, 0);

      tt_want_int_op(msg->connect.password, ==, 0);

      msg_matrix->connect.client_id = msg->connect.client_id;
      msg_matrix->connect.username = msg->connect.username;
      msg_matrix->connect.password = msg->connect.password;
      msg_matrix->connect.keepalive = msg->connect.keepalive;
      msg_matrix->connect.flags_u = msg->connect.flags_u;

      tt_want_int_op(memcmp(msg, msg_matrix, sizeof(iotc_mqtt_message_t)), ==,
                     0);

      msg_matrix->connect.protocol_name = 0;
      msg_matrix->connect.client_id = 0;
      msg_matrix->connect.username = 0;
      msg_matrix->connect.password = 0;

      iotc_mqtt_message_free(&msg);
      iotc_mqtt_message_free(&msg_matrix);

      tt_want_int_op(iotc_is_whole_memory_deallocated(), >, 0);

      return;

    err_handling:
      tt_abort_msg(("test must not fail!"));
    end:
      iotc_mqtt_message_free(&msg);
      iotc_mqtt_message_free(&msg_matrix);
    })

IOTC_TT_TESTCASE(
    utest__fill_with_connect_data__valid_data_no_password_no_username__connect_msg,
    {
      iotc_state_t local_state = IOTC_STATE_OK;

      const char* username = 0;
      const char* password = 0;

      iotc_mqtt_message_t* msg = NULL;
      iotc_mqtt_message_t* msg_matrix = NULL;

      IOTC_ALLOC_AT(iotc_mqtt_message_t, msg, local_state);
      IOTC_ALLOC_AT(iotc_mqtt_message_t, msg_matrix, local_state);

      tt_want_int_op(
          fill_with_connect_data(msg, username, password, 7, IOTC_SESSION_CLEAN,
                                 NULL, NULL, 0, 0),
          ==, IOTC_STATE_OK);

      msg_matrix->common.common_u.common_bits.retain = IOTC_MQTT_RETAIN_FALSE;
      msg_matrix->common.common_u.common_bits.qos = IOTC_MQTT_QOS_AT_MOST_ONCE;
      msg_matrix->common.common_u.common_bits.dup = IOTC_MQTT_DUP_FALSE;
      msg_matrix->common.common_u.common_bits.type = IOTC_MQTT_TYPE_CONNECT;

      msg_matrix->connect.protocol_version = 4;
      msg_matrix->connect.protocol_name = msg->connect.protocol_name;

      msg_matrix->common.remaining_length = 0;

      tt_want_int_op(memcmp(msg->connect.protocol_name->data_ptr, "MQTT",
                            sizeof("MQTT") - 1),
                     ==, 0);

      tt_want_int_op(msg->connect.client_id, ==, 0);
      tt_want_int_op(msg->connect.password, ==, 0);

      msg_matrix->connect.client_id = msg->connect.client_id;
      msg_matrix->connect.username = msg->connect.username;
      msg_matrix->connect.password = msg->connect.password;
      msg_matrix->connect.keepalive = msg->connect.keepalive;
      msg_matrix->connect.flags_u = msg->connect.flags_u;

      tt_want_int_op(memcmp(msg, msg_matrix, sizeof(iotc_mqtt_message_t)), ==,
                     0);

      msg_matrix->connect.protocol_name = 0;
      msg_matrix->connect.client_id = 0;
      msg_matrix->connect.username = 0;
      msg_matrix->connect.password = 0;

      iotc_mqtt_message_free(&msg);
      iotc_mqtt_message_free(&msg_matrix);

      tt_want_int_op(iotc_is_whole_memory_deallocated(), >, 0);

      return;

    err_handling:
      tt_abort_msg(("test must not fail!"));
    end:
      iotc_mqtt_message_free(&msg);
      iotc_mqtt_message_free(&msg_matrix);
    })

/* Test the last_will entries */
IOTC_TT_TESTCASE(
    utest__fill_with_connect_data__valid_data_will_topic_will_message__connect_msg,
    {
      iotc_state_t local_state = IOTC_STATE_OK;

      const char* username = 0;
      const char* password = 0;
      const char will_topic[] = "device_last_will";
      const char will_message[] = "device quit unexpectedly";

      iotc_mqtt_message_t* msg = NULL;
      iotc_mqtt_message_t* msg_matrix = NULL;

      IOTC_ALLOC_AT(iotc_mqtt_message_t, msg, local_state);
      IOTC_ALLOC_AT(iotc_mqtt_message_t, msg_matrix, local_state);

      tt_want_int_op(
          fill_with_connect_data(
              msg, username, password, 7, IOTC_SESSION_CLEAN, will_topic,
              will_message, IOTC_MQTT_QOS_AT_LEAST_ONCE, IOTC_MQTT_RETAIN_TRUE),
          ==, IOTC_STATE_OK);

      msg_matrix->common.common_u.common_bits.retain = IOTC_MQTT_RETAIN_FALSE;
      msg_matrix->common.common_u.common_bits.qos = IOTC_MQTT_QOS_AT_MOST_ONCE;
      msg_matrix->common.common_u.common_bits.dup = IOTC_MQTT_DUP_FALSE;
      msg_matrix->common.common_u.common_bits.type = IOTC_MQTT_TYPE_CONNECT;

      msg_matrix->connect.protocol_version = 4;
      msg_matrix->connect.protocol_name = msg->connect.protocol_name;

      msg_matrix->common.remaining_length = 0;

      tt_want_int_op(memcmp(msg->connect.protocol_name->data_ptr, "MQTT",
                            sizeof("MQTT") - 1),
                     ==, 0);

      tt_want_int_op(msg->connect.client_id, ==, 0);
      tt_want_int_op(msg->connect.password, ==, 0);

      /* Check the last will elements */
      tt_want_int_op(memcmp(msg->connect.will_topic->data_ptr, will_topic,
                            sizeof(will_topic) - 1),
                     ==, 0);
      tt_want_int_op(memcmp(msg->connect.will_message->data_ptr, will_message,
                            sizeof(will_message) - 1),
                     ==, 0);
      /* The will bit should be set since we passed a will_topic AND
       * will_message */
      tt_want_int_op(msg->connect.flags_u.flags_bits.will, ==, 1);
      tt_want_int_op(msg->connect.flags_u.flags_bits.will_retain, ==,
                     IOTC_MQTT_RETAIN_TRUE);
      tt_want_int_op(msg->connect.flags_u.flags_bits.will_qos, ==,
                     IOTC_MQTT_QOS_AT_LEAST_ONCE);

      msg_matrix->connect.client_id = msg->connect.client_id;
      msg_matrix->connect.username = msg->connect.username;
      msg_matrix->connect.password = msg->connect.password;
      msg_matrix->connect.keepalive = msg->connect.keepalive;
      msg_matrix->connect.flags_u = msg->connect.flags_u;
      msg_matrix->connect.will_topic = msg->connect.will_topic;
      msg_matrix->connect.will_message = msg->connect.will_message;

      tt_want_int_op(memcmp(msg, msg_matrix, sizeof(iotc_mqtt_message_t)), ==,
                     0);

      msg_matrix->connect.protocol_name = 0;
      msg_matrix->connect.client_id = 0;
      msg_matrix->connect.username = 0;
      msg_matrix->connect.password = 0;
      msg_matrix->connect.will_topic = 0;
      msg_matrix->connect.will_message = 0;

      iotc_mqtt_message_free(&msg);
      iotc_mqtt_message_free(&msg_matrix);

      tt_want_int_op(iotc_is_whole_memory_deallocated(), >, 0);

      return;

    err_handling:
      tt_abort_msg(("test must not fail!"));
    end:
      iotc_mqtt_message_free(&msg);
      iotc_mqtt_message_free(&msg_matrix);
    })
IOTC_TT_TESTCASE(
    utest__fill_with_connect_data__valid_data_will_topic_no_will_message__connect_msg,
    {
      iotc_state_t local_state = IOTC_STATE_OK;

      const char* username = 0;
      const char* password = 0;
      const char will_topic[] = "device_last_will";
      const char* will_message = 0;

      iotc_mqtt_message_t* msg = NULL;
      iotc_mqtt_message_t* msg_matrix = NULL;

      IOTC_ALLOC_AT(iotc_mqtt_message_t, msg, local_state);
      IOTC_ALLOC_AT(iotc_mqtt_message_t, msg_matrix, local_state);

      tt_want_int_op(
          fill_with_connect_data(
              msg, username, password, 7, IOTC_SESSION_CLEAN, will_topic,
              will_message, IOTC_MQTT_QOS_AT_LEAST_ONCE, IOTC_MQTT_RETAIN_TRUE),
          ==, IOTC_NULL_WILL_MESSAGE);

      msg_matrix->connect.protocol_name = 0;
      msg_matrix->connect.client_id = 0;
      msg_matrix->connect.username = 0;
      msg_matrix->connect.password = 0;
      msg_matrix->connect.will_topic = 0;
      msg_matrix->connect.will_message = 0;

      iotc_mqtt_message_free(&msg);
      iotc_mqtt_message_free(&msg_matrix);

      tt_want_int_op(iotc_is_whole_memory_deallocated(), >, 0);

      return;

    err_handling:
      tt_abort_msg(("test must not fail!"));
    end:
      iotc_mqtt_message_free(&msg);
      iotc_mqtt_message_free(&msg_matrix);
    })
IOTC_TT_TESTCASE(
    utest__fill_with_connect_data__valid_data_no_will_topic_will_message__connect_msg,
    {
      iotc_state_t local_state = IOTC_STATE_OK;

      const char* username = 0;
      const char* password = 0;
      const char* will_topic = 0;
      const char will_message[] = "device quit unexpectedly";

      iotc_mqtt_message_t* msg = NULL;
      iotc_mqtt_message_t* msg_matrix = NULL;

      IOTC_ALLOC_AT(iotc_mqtt_message_t, msg, local_state);
      IOTC_ALLOC_AT(iotc_mqtt_message_t, msg_matrix, local_state);

      tt_want_int_op(
          fill_with_connect_data(
              msg, username, password, 7, IOTC_SESSION_CLEAN, will_topic,
              will_message, IOTC_MQTT_QOS_AT_LEAST_ONCE, IOTC_MQTT_RETAIN_TRUE),
          ==, IOTC_NULL_WILL_TOPIC);

      msg_matrix->connect.protocol_name = 0;
      msg_matrix->connect.client_id = 0;
      msg_matrix->connect.username = 0;
      msg_matrix->connect.password = 0;
      msg_matrix->connect.will_topic = 0;
      msg_matrix->connect.will_message = 0;

      iotc_mqtt_message_free(&msg);
      iotc_mqtt_message_free(&msg_matrix);

      tt_want_int_op(iotc_is_whole_memory_deallocated(), >, 0);

      return;

    err_handling:
      tt_abort_msg(("test must not fail!"));
    end:
      iotc_mqtt_message_free(&msg);
      iotc_mqtt_message_free(&msg_matrix);
    })
IOTC_TT_TESTCASE(
    utest__fill_with_connect_data__valid_data_no_will_topic_no_will_message__connect_msg,
    {
      iotc_state_t local_state = IOTC_STATE_OK;

      const char* username = 0;
      const char* password = 0;
      const char* will_topic = 0;
      const char* will_message = 0;

      iotc_mqtt_message_t* msg = NULL;
      iotc_mqtt_message_t* msg_matrix = NULL;

      IOTC_ALLOC_AT(iotc_mqtt_message_t, msg, local_state);
      IOTC_ALLOC_AT(iotc_mqtt_message_t, msg_matrix, local_state);

      tt_want_int_op(
          fill_with_connect_data(
              msg, username, password, 7, IOTC_SESSION_CLEAN, will_topic,
              will_message, IOTC_MQTT_QOS_AT_LEAST_ONCE, IOTC_MQTT_RETAIN_TRUE),
          ==, IOTC_STATE_OK);

      msg_matrix->common.common_u.common_bits.retain = IOTC_MQTT_RETAIN_FALSE;
      msg_matrix->common.common_u.common_bits.qos = IOTC_MQTT_QOS_AT_MOST_ONCE;
      msg_matrix->common.common_u.common_bits.dup = IOTC_MQTT_DUP_FALSE;
      msg_matrix->common.common_u.common_bits.type = IOTC_MQTT_TYPE_CONNECT;

      msg_matrix->connect.protocol_version = 4;
      msg_matrix->connect.protocol_name = msg->connect.protocol_name;

      msg_matrix->common.remaining_length = 0;

      tt_want_int_op(memcmp(msg->connect.protocol_name->data_ptr, "MQTT",
                            sizeof("MQTT") - 1),
                     ==, 0);

      tt_want_int_op(msg->connect.client_id, ==, 0);
      tt_want_int_op(msg->connect.password, ==, 0);

      /* Check the last will elements */
      tt_want_int_op(msg->connect.will_topic, ==, 0);
      tt_want_int_op(msg->connect.will_message, ==, 0);

      /* The will bit must not be set since we passed no will_topic and no
       * will_message */
      tt_want_int_op(msg->connect.flags_u.flags_bits.will, ==, 0);
      /* We will check to see that the will_retain and will_qos are ZERO
       * instead of one of */
      /* the enumerated types since the protocol demands that they be exactly
       * zero */
      tt_want_int_op(msg->connect.flags_u.flags_bits.will_retain, ==, 0);
      tt_want_int_op(msg->connect.flags_u.flags_bits.will_qos, ==, 0);

      msg_matrix->connect.client_id = msg->connect.client_id;
      msg_matrix->connect.username = msg->connect.username;
      msg_matrix->connect.password = msg->connect.password;
      msg_matrix->connect.keepalive = msg->connect.keepalive;
      msg_matrix->connect.flags_u = msg->connect.flags_u;
      msg_matrix->connect.will_topic = msg->connect.will_topic;
      msg_matrix->connect.will_message = msg->connect.will_message;

      tt_want_int_op(memcmp(msg, msg_matrix, sizeof(iotc_mqtt_message_t)), ==,
                     0);

      msg_matrix->connect.protocol_name = 0;
      msg_matrix->connect.client_id = 0;
      msg_matrix->connect.username = 0;
      msg_matrix->connect.password = 0;
      msg_matrix->connect.will_topic = 0;
      msg_matrix->connect.will_message = 0;

      iotc_mqtt_message_free(&msg);
      iotc_mqtt_message_free(&msg_matrix);

      tt_want_int_op(iotc_is_whole_memory_deallocated(), >, 0);

      return;

    err_handling:
      tt_abort_msg(("test must not fail!"));
    end:
      iotc_mqtt_message_free(&msg);
      iotc_mqtt_message_free(&msg_matrix);
    })
IOTC_TT_TESTCASE(
    utest__fill_with_connect_data__valid_data_will_topic_will_message_no_retain__connect_msg,
    {
      iotc_state_t local_state = IOTC_STATE_OK;

      const char* username = 0;
      const char* password = 0;
      const char will_topic[] = "device_last_will";
      const char will_message[] = "device quit unexpectedly";

      iotc_mqtt_message_t* msg = NULL;
      iotc_mqtt_message_t* msg_matrix = NULL;

      IOTC_ALLOC_AT(iotc_mqtt_message_t, msg, local_state);
      IOTC_ALLOC_AT(iotc_mqtt_message_t, msg_matrix, local_state);

      tt_want_int_op(fill_with_connect_data(
                         msg, username, password, 7, IOTC_SESSION_CLEAN,
                         will_topic, will_message, IOTC_MQTT_QOS_AT_LEAST_ONCE,
                         IOTC_MQTT_RETAIN_FALSE),
                     ==, IOTC_STATE_OK);

      msg_matrix->common.common_u.common_bits.retain = IOTC_MQTT_RETAIN_FALSE;
      msg_matrix->common.common_u.common_bits.qos = IOTC_MQTT_QOS_AT_MOST_ONCE;
      msg_matrix->common.common_u.common_bits.dup = IOTC_MQTT_DUP_FALSE;
      msg_matrix->common.common_u.common_bits.type = IOTC_MQTT_TYPE_CONNECT;

      msg_matrix->connect.protocol_version = 4;
      msg_matrix->connect.protocol_name = msg->connect.protocol_name;

      msg_matrix->common.remaining_length = 0;

      tt_want_int_op(memcmp(msg->connect.protocol_name->data_ptr, "MQTT",
                            sizeof("MQTT") - 1),
                     ==, 0);

      tt_want_int_op(msg->connect.client_id, ==, 0);
      tt_want_int_op(msg->connect.password, ==, 0);

      /* Check the last will elements */
      tt_want_int_op(memcmp(msg->connect.will_topic->data_ptr, will_topic,
                            sizeof(will_topic) - 1),
                     ==, 0);
      tt_want_int_op(memcmp(msg->connect.will_message->data_ptr, will_message,
                            sizeof(will_message) - 1),
                     ==, 0);

      /* The will bit should be set since we passed a will_topic AND
       * will_message */
      tt_want_int_op(msg->connect.flags_u.flags_bits.will, ==, 1);
      tt_want_int_op(msg->connect.flags_u.flags_bits.will_retain, ==,
                     IOTC_MQTT_RETAIN_FALSE);
      tt_want_int_op(msg->connect.flags_u.flags_bits.will_qos, ==,
                     IOTC_MQTT_QOS_AT_LEAST_ONCE);

      msg_matrix->connect.client_id = msg->connect.client_id;
      msg_matrix->connect.username = msg->connect.username;
      msg_matrix->connect.password = msg->connect.password;
      msg_matrix->connect.keepalive = msg->connect.keepalive;
      msg_matrix->connect.flags_u = msg->connect.flags_u;
      msg_matrix->connect.will_topic = msg->connect.will_topic;
      msg_matrix->connect.will_message = msg->connect.will_message;

      tt_want_int_op(memcmp(msg, msg_matrix, sizeof(iotc_mqtt_message_t)), ==,
                     0);

      msg_matrix->connect.protocol_name = 0;
      msg_matrix->connect.client_id = 0;
      msg_matrix->connect.username = 0;
      msg_matrix->connect.password = 0;
      msg_matrix->connect.will_topic = 0;
      msg_matrix->connect.will_message = 0;

      iotc_mqtt_message_free(&msg);
      iotc_mqtt_message_free(&msg_matrix);

      tt_want_int_op(iotc_is_whole_memory_deallocated(), >, 0);

      return;

    err_handling:
      tt_abort_msg(("test must not fail!"));
    end:
      iotc_mqtt_message_free(&msg);
      iotc_mqtt_message_free(&msg_matrix);
    })
IOTC_TT_TESTCASE(
    utest__fill_with_connect_data__valid_data_will_topic_will_message_retain_willqosAMO__connect_msg,
    {
      iotc_state_t local_state = IOTC_STATE_OK;

      const char* username = 0;
      const char* password = 0;
      const char will_topic[] = "device_last_will";
      const char will_message[] = "device quit unexpectedly";

      iotc_mqtt_message_t* msg = NULL;
      iotc_mqtt_message_t* msg_matrix = NULL;

      IOTC_ALLOC_AT(iotc_mqtt_message_t, msg, local_state);
      IOTC_ALLOC_AT(iotc_mqtt_message_t, msg_matrix, local_state);

      tt_want_int_op(
          fill_with_connect_data(
              msg, username, password, 7, IOTC_SESSION_CLEAN, will_topic,
              will_message, IOTC_MQTT_QOS_AT_MOST_ONCE, IOTC_MQTT_RETAIN_TRUE),
          ==, IOTC_STATE_OK);

      msg_matrix->common.common_u.common_bits.retain = IOTC_MQTT_RETAIN_FALSE;
      msg_matrix->common.common_u.common_bits.qos = IOTC_MQTT_QOS_AT_MOST_ONCE;
      msg_matrix->common.common_u.common_bits.dup = IOTC_MQTT_DUP_FALSE;
      msg_matrix->common.common_u.common_bits.type = IOTC_MQTT_TYPE_CONNECT;

      msg_matrix->connect.protocol_version = 4;
      msg_matrix->connect.protocol_name = msg->connect.protocol_name;

      msg_matrix->common.remaining_length = 0;

      tt_want_int_op(memcmp(msg->connect.protocol_name->data_ptr, "MQTT",
                            sizeof("MQTT") - 1),
                     ==, 0);

      tt_want_int_op(msg->connect.client_id, ==, 0);
      tt_want_int_op(msg->connect.password, ==, 0);

      /* Check the last will elements */
      tt_want_int_op(memcmp(msg->connect.will_topic->data_ptr, will_topic,
                            sizeof(will_topic) - 1),
                     ==, 0);
      tt_want_int_op(memcmp(msg->connect.will_message->data_ptr, will_message,
                            sizeof(will_message) - 1),
                     ==, 0);
      /* The will bit should be set since we passed a will_topic AND
       * will_message */
      tt_want_int_op(msg->connect.flags_u.flags_bits.will, ==, 1);
      tt_want_int_op(msg->connect.flags_u.flags_bits.will_retain, ==,
                     IOTC_MQTT_RETAIN_TRUE);
      tt_want_int_op(msg->connect.flags_u.flags_bits.will_qos, ==,
                     IOTC_MQTT_QOS_AT_MOST_ONCE);

      msg_matrix->connect.client_id = msg->connect.client_id;
      msg_matrix->connect.username = msg->connect.username;
      msg_matrix->connect.password = msg->connect.password;
      msg_matrix->connect.keepalive = msg->connect.keepalive;
      msg_matrix->connect.flags_u = msg->connect.flags_u;
      msg_matrix->connect.will_topic = msg->connect.will_topic;
      msg_matrix->connect.will_message = msg->connect.will_message;

      tt_want_int_op(memcmp(msg, msg_matrix, sizeof(iotc_mqtt_message_t)), ==,
                     0);

      msg_matrix->connect.protocol_name = 0;
      msg_matrix->connect.client_id = 0;
      msg_matrix->connect.username = 0;
      msg_matrix->connect.password = 0;
      msg_matrix->connect.will_topic = 0;
      msg_matrix->connect.will_message = 0;

      iotc_mqtt_message_free(&msg);
      iotc_mqtt_message_free(&msg_matrix);

      tt_want_int_op(iotc_is_whole_memory_deallocated(), >, 0);

      return;

    err_handling:
      tt_abort_msg(("test must not fail!"));
    end:
      iotc_mqtt_message_free(&msg);
      iotc_mqtt_message_free(&msg_matrix);
    })
IOTC_TT_TESTCASE(
    utest__fill_with_connect_data__valid_data_will_topic_will_message_retain_willqosEO__connect_msg,
    {
      iotc_state_t local_state = IOTC_STATE_OK;

      const char* username = 0;
      const char* password = 0;
      const char will_topic[] = "device_last_will";
      const char will_message[] = "device quit unexpectedly";

      iotc_mqtt_message_t* msg = NULL;
      iotc_mqtt_message_t* msg_matrix = NULL;

      IOTC_ALLOC_AT(iotc_mqtt_message_t, msg, local_state);
      IOTC_ALLOC_AT(iotc_mqtt_message_t, msg_matrix, local_state);

      tt_want_int_op(
          fill_with_connect_data(
              msg, username, password, 7, IOTC_SESSION_CLEAN, will_topic,
              will_message, IOTC_MQTT_QOS_EXACTLY_ONCE, IOTC_MQTT_RETAIN_TRUE),
          ==, IOTC_STATE_OK);

      msg_matrix->common.common_u.common_bits.retain = IOTC_MQTT_RETAIN_FALSE;
      msg_matrix->common.common_u.common_bits.qos = IOTC_MQTT_QOS_AT_MOST_ONCE;
      msg_matrix->common.common_u.common_bits.dup = IOTC_MQTT_DUP_FALSE;
      msg_matrix->common.common_u.common_bits.type = IOTC_MQTT_TYPE_CONNECT;

      msg_matrix->connect.protocol_version = 4;
      msg_matrix->connect.protocol_name = msg->connect.protocol_name;

      msg_matrix->common.remaining_length = 0;

      tt_want_int_op(memcmp(msg->connect.protocol_name->data_ptr, "MQTT",
                            sizeof("MQTT") - 1),
                     ==, 0);

      tt_want_int_op(msg->connect.client_id, ==, 0);
      tt_want_int_op(msg->connect.password, ==, 0);

      /* Check the last will elements */
      tt_want_int_op(memcmp(msg->connect.will_topic->data_ptr, will_topic,
                            sizeof(will_topic) - 1),
                     ==, 0);
      tt_want_int_op(memcmp(msg->connect.will_message->data_ptr, will_message,
                            sizeof(will_message) - 1),
                     ==, 0);
      /* The will bit should be set since we passed a will_topic AND
       * will_message */
      tt_want_int_op(msg->connect.flags_u.flags_bits.will, ==, 1);
      tt_want_int_op(msg->connect.flags_u.flags_bits.will_retain, ==,
                     IOTC_MQTT_RETAIN_TRUE);
      tt_want_int_op(msg->connect.flags_u.flags_bits.will_qos, ==,
                     IOTC_MQTT_QOS_EXACTLY_ONCE);

      msg_matrix->connect.client_id = msg->connect.client_id;
      msg_matrix->connect.username = msg->connect.username;
      msg_matrix->connect.password = msg->connect.password;
      msg_matrix->connect.keepalive = msg->connect.keepalive;
      msg_matrix->connect.flags_u = msg->connect.flags_u;
      msg_matrix->connect.will_topic = msg->connect.will_topic;
      msg_matrix->connect.will_message = msg->connect.will_message;

      tt_want_int_op(memcmp(msg, msg_matrix, sizeof(iotc_mqtt_message_t)), ==,
                     0);

      msg_matrix->connect.protocol_name = 0;
      msg_matrix->connect.client_id = 0;
      msg_matrix->connect.username = 0;
      msg_matrix->connect.password = 0;
      msg_matrix->connect.will_topic = 0;
      msg_matrix->connect.will_message = 0;

      iotc_mqtt_message_free(&msg);
      iotc_mqtt_message_free(&msg_matrix);

      tt_want_int_op(iotc_is_whole_memory_deallocated(), >, 0);

      return;

    err_handling:
      tt_abort_msg(("test must not fail!"));
    end:
      iotc_mqtt_message_free(&msg);
      iotc_mqtt_message_free(&msg_matrix);
    })

IOTC_TT_TESTCASE(utest__fill_with_publish_data__valid_data__publish_msg, {
  iotc_state_t local_state = IOTC_STATE_OK;

  iotc_mqtt_message_t* msg = NULL;
  iotc_mqtt_message_t* msg_matrix = NULL;

  const char topic[] = "test_topic";

  iotc_data_desc_t* content = iotc_make_desc_from_string_copy("test_content");

  // test for all combinations of common
  size_t i = 0;
  for (i = 0; i < 16; ++i) {
    IOTC_ALLOC_AT(iotc_mqtt_message_t, msg, local_state);
    IOTC_ALLOC_AT(iotc_mqtt_message_t, msg_matrix, local_state);

    iotc_utest_mqtt_message_details_to_uint16_t mqtt_common = {{.value = i}};

    tt_want_int_op(
        fill_with_publish_data(
            msg, topic, content, mqtt_common.iotc_mqtt_union.qos_retain_dup.qos,
            mqtt_common.iotc_mqtt_union.qos_retain_dup.retain,
            mqtt_common.iotc_mqtt_union.qos_retain_dup.dup, 17),
        ==, IOTC_STATE_OK);

    msg_matrix->common.common_u.common_bits.retain =
        mqtt_common.iotc_mqtt_union.qos_retain_dup.retain;
    msg_matrix->common.common_u.common_bits.qos =
        mqtt_common.iotc_mqtt_union.qos_retain_dup.qos;
    msg_matrix->common.common_u.common_bits.dup =
        mqtt_common.iotc_mqtt_union.qos_retain_dup.dup;
    msg_matrix->common.common_u.common_bits.type = IOTC_MQTT_TYPE_PUBLISH;
    msg_matrix->common.remaining_length = 0;

    msg_matrix->publish.topic_name = msg->publish.topic_name;
    msg_matrix->publish.content = msg->publish.content;

    tt_want_int_op(
        memcmp(msg->publish.topic_name->data_ptr, topic, sizeof(topic) - 1), ==,
        0);
    tt_want_int_op(memcmp(msg->publish.content->data_ptr, content->data_ptr,
                          content->length),
                   ==, 0);

    msg_matrix->publish.message_id = 17;

    tt_want_int_op(memcmp(msg, msg_matrix, sizeof(iotc_mqtt_message_t)), ==, 0);

    msg_matrix->publish.topic_name = 0;
    msg_matrix->publish.content = 0;

    iotc_mqtt_message_free(&msg);
    iotc_mqtt_message_free(&msg_matrix);
  }

  iotc_free_desc(&content);

  tt_want_int_op(iotc_is_whole_memory_deallocated(), >, 0);

  return;

err_handling:
  tt_abort_msg(("test must not fail!"));
end:
  iotc_mqtt_message_free(&msg);
  iotc_mqtt_message_free(&msg_matrix);
  iotc_free_desc(&content);
})

#ifndef IOTC_EMBEDDED_TESTS
IOTC_TT_TESTCASE(
    utest__fill_with_publish_data__valid_data_max_payload__publish_msg, {
      utest__fill_with_publish_data__valid_data_max_payload__publish_msg_help();
    })

IOTC_TT_TESTCASE(
    utest__fill_with_publish_data__too_big_payload_data__iotc_mqtt_too_big_paload,
    {
      iotc_state_t local_state = IOTC_STATE_OK;

      iotc_mqtt_message_t* msg = NULL;
      iotc_mqtt_message_t* msg_matrix = NULL;

      const size_t cnt_size = IOTC_MQTT_MAX_PAYLOAD_SIZE + 1;

      const char topic[] = "test_topic";

      iotc_data_desc_t* content = iotc_make_empty_desc_alloc(cnt_size);
      memset((void*)content->data_ptr, 'a', cnt_size);
      content->length = cnt_size;

      IOTC_ALLOC_AT(iotc_mqtt_message_t, msg, local_state);
      IOTC_ALLOC_AT(iotc_mqtt_message_t, msg_matrix, local_state);

      tt_want_int_op(fill_with_publish_data(
                         msg, topic, content, IOTC_MQTT_QOS_AT_LEAST_ONCE,
                         IOTC_MQTT_RETAIN_FALSE, IOTC_MQTT_DUP_FALSE, 17),
                     ==, IOTC_MQTT_PAYLOAD_SIZE_TOO_LARGE);

      iotc_mqtt_message_free(&msg);
      iotc_mqtt_message_free(&msg_matrix);
      iotc_free_desc(&content);

      tt_want_int_op(iotc_is_whole_memory_deallocated(), >, 0);

      return;

    err_handling:
      tt_abort_msg(("test must not fail!"));
    end:
      iotc_mqtt_message_free(&msg);
      iotc_mqtt_message_free(&msg_matrix);
      iotc_free_desc(&content);
    })
#endif  // IOTC_EMBEDDED_TESTS

IOTC_TT_TESTCASE(utest__fill_with_subscribe_data__valid_data__publish_msg, {
  iotc_state_t local_state = IOTC_STATE_OK;

  iotc_mqtt_message_t* msg = NULL;
  iotc_mqtt_message_t* msg_matrix = NULL;

  const char topic[] = "test_topic";

  IOTC_ALLOC_AT(iotc_mqtt_message_t, msg, local_state);
  IOTC_ALLOC_AT(iotc_mqtt_message_t, msg_matrix, local_state);

  tt_want_int_op(
      fill_with_subscribe_data(msg, topic, 17, IOTC_MQTT_QOS_AT_LEAST_ONCE,
                               IOTC_MQTT_DUP_FALSE),
      ==, IOTC_STATE_OK);

  msg_matrix->common.common_u.common_bits.retain = IOTC_MQTT_RETAIN_FALSE;
  msg_matrix->common.common_u.common_bits.qos = IOTC_MQTT_QOS_AT_LEAST_ONCE;
  msg_matrix->common.common_u.common_bits.dup = IOTC_MQTT_DUP_FALSE;
  msg_matrix->common.common_u.common_bits.type = IOTC_MQTT_TYPE_SUBSCRIBE;

  msg_matrix->common.remaining_length = 0;

  msg_matrix->subscribe.topics = msg->subscribe.topics;

  tt_want_int_op(
      memcmp(msg->subscribe.topics->name->data_ptr, topic, sizeof(topic) - 1),
      ==, 0);

  msg_matrix->subscribe.message_id = 17;

  tt_want_int_op(memcmp(msg, msg_matrix, sizeof(iotc_mqtt_message_t)), ==, 0);

  msg_matrix->subscribe.topics = 0;

  iotc_mqtt_message_free(&msg);
  iotc_mqtt_message_free(&msg_matrix);

  tt_want_int_op(iotc_is_whole_memory_deallocated(), >, 0);

  return;

err_handling:
  tt_abort_msg(("test must not fail!"));
end:
  iotc_mqtt_message_free(&msg);
  iotc_mqtt_message_free(&msg_matrix);
})

IOTC_TT_TESTGROUP_END

#ifndef IOTC_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#define IOTC_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#include __FILE__
#undef IOTC_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#endif
