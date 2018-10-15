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
#include "iotc_connection_data_internal.h"
#include "iotc_helpers.h"
#include "iotc_memory_checks.h"
#include "iotc_mqtt_logic_layer_data_helpers.h"

#include <errno.h>
#include <stdlib.h>
#include <string.h>

/*-----------------------------------------------------------------------*/
/* HELPER TESTS                                                          */
/*-----------------------------------------------------------------------*/
#ifndef IOTC_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN

void test_helpers_iotc_parse_payload_as_string_parse_empty_payload_help(void) {
  const char topic[] = "test_topic";
  const char content[] = "";
  char* string_payload = NULL;
  iotc_mqtt_message_t* msg = NULL;

  iotc_data_desc_t* message_payload = iotc_make_desc_from_string_copy(content);

  iotc_state_t local_state = IOTC_STATE_OK;
  IOTC_ALLOC_AT(iotc_mqtt_message_t, msg, local_state);

  tt_want_int_op(fill_with_publish_data(
                     msg, topic, message_payload, IOTC_MQTT_QOS_AT_LEAST_ONCE,
                     IOTC_MQTT_RETAIN_FALSE, IOTC_MQTT_DUP_FALSE, 17),
                 ==, IOTC_STATE_OK);

  string_payload = iotc_parse_message_payload_as_string(msg);

  tt_assert(string_payload != NULL);

  tt_want_int_op(strlen(string_payload), ==, 0);

err_handling:
end:
  IOTC_SAFE_FREE(string_payload);
  iotc_free_desc(&message_payload);
  iotc_mqtt_message_free(&msg);
  tt_want_int_op(iotc_is_whole_memory_deallocated(), >, 0);
  ;
}

#endif

IOTC_TT_TESTGROUP_BEGIN(utest_helpers)

IOTC_TT_TESTCASE(test_helpers_iotc_parse_payload_as_string_null_param, {
  char* result = iotc_parse_message_payload_as_string(NULL);

  tt_assert(NULL == result);

end:
  IOTC_SAFE_FREE(result);
  tt_want_int_op(iotc_is_whole_memory_deallocated(), >, 0);
  ;
})

IOTC_TT_TESTCASE(
    test_helpers_iotc_parse_payload_as_string_parse_legitmiate_payload, {
      const char topic[] = "test_topic";
      const char content[] = "0123456789";
      char* string_payload = NULL;
      iotc_mqtt_message_t* msg = NULL;

      iotc_data_desc_t* message_payload =
          iotc_make_desc_from_string_copy(content);

      iotc_state_t local_state = IOTC_STATE_OK;
      IOTC_ALLOC_AT(iotc_mqtt_message_t, msg, local_state);

      tt_want_int_op(fill_with_publish_data(msg, topic, message_payload,
                                            IOTC_MQTT_QOS_AT_LEAST_ONCE,
                                            IOTC_MQTT_RETAIN_FALSE,
                                            IOTC_MQTT_DUP_FALSE, 17),
                     ==, IOTC_STATE_OK);

      string_payload = iotc_parse_message_payload_as_string(msg);

      tt_assert(string_payload != NULL);

      tt_want_int_op(strncmp(content, string_payload, 10), ==, 0);
      tt_want_int_op(strlen(string_payload), ==, 10);

    err_handling:
    end:
      IOTC_SAFE_FREE(string_payload);
      iotc_free_desc(&message_payload);
      iotc_mqtt_message_free(&msg);
      tt_want_int_op(iotc_is_whole_memory_deallocated(), >, 0);
      ;
    })

IOTC_TT_TESTCASE(
    test_helpers_iotc_parse_payload_as_string_parse_empty_payload,
    { test_helpers_iotc_parse_payload_as_string_parse_empty_payload_help(); })

IOTC_TT_TESTCASE(test_helpers_iotc_crypto_private_key_data_dup_null, {
  const iotc_crypto_private_key_data_t* copied_key =
      iotc_crypto_private_key_data_dup(NULL);
  tt_ptr_op(copied_key, ==, NULL);

end:;
})

IOTC_TT_TESTCASE(test_helpers_iotc_crypto_private_key_data_cmp_null, {
  const iotc_crypto_private_key_data_t dummy_key = {
      .private_key_signature_algorithm =
          IOTC_JWT_PRIVATE_KEY_SIGNATURE_ALGORITHM_ES256,
      .private_key_union_type = IOTC_CRYPTO_KEY_UNION_TYPE_PEM,
      .private_key_union.key_pem.key = "dummy key"};

  tt_int_op(0, ==, iotc_crypto_private_key_data_cmp(NULL, NULL));
  tt_int_op(0, !=, iotc_crypto_private_key_data_cmp(&dummy_key, NULL));
  tt_int_op(0, !=, iotc_crypto_private_key_data_cmp(NULL, &dummy_key));

end:;
})

IOTC_TT_TESTCASE(test_helpers_iotc_crypto_private_key_data_cmp_type, {
  const iotc_crypto_private_key_data_t key_pem = {
      .private_key_signature_algorithm =
          IOTC_JWT_PRIVATE_KEY_SIGNATURE_ALGORITHM_ES256,
      .private_key_union_type = IOTC_CRYPTO_KEY_UNION_TYPE_PEM,
      .private_key_union.key_pem.key = "dummy key"};

  const iotc_crypto_private_key_data_t key_slot = {
      .private_key_signature_algorithm =
          IOTC_JWT_PRIVATE_KEY_SIGNATURE_ALGORITHM_ES256,
      .private_key_union_type = IOTC_CRYPTO_KEY_UNION_TYPE_SLOT_ID,
      .private_key_union.key_slot.slot_id = 42};

  tt_int_op(0, !=, iotc_crypto_private_key_data_cmp(&key_pem, &key_slot));
end:;
})

IOTC_TT_TESTCASE(test_helpers_iotc_crypto_private_key_data_cmp_pem, {
  const iotc_crypto_private_key_data_t keyA1 = {
      .private_key_signature_algorithm =
          IOTC_JWT_PRIVATE_KEY_SIGNATURE_ALGORITHM_ES256,
      .private_key_union_type = IOTC_CRYPTO_KEY_UNION_TYPE_PEM,
      .private_key_union.key_pem.key = "keyA"};

  const iotc_crypto_private_key_data_t keyA2 = {
      .private_key_signature_algorithm =
          IOTC_JWT_PRIVATE_KEY_SIGNATURE_ALGORITHM_ES256,
      .private_key_union_type = IOTC_CRYPTO_KEY_UNION_TYPE_PEM,
      .private_key_union.key_pem.key = "keyA"};

  const iotc_crypto_private_key_data_t keyB = {
      .private_key_signature_algorithm =
          IOTC_JWT_PRIVATE_KEY_SIGNATURE_ALGORITHM_ES256,
      .private_key_union_type = IOTC_CRYPTO_KEY_UNION_TYPE_PEM,
      .private_key_union.key_pem.key = "keyB"};

  const iotc_crypto_private_key_data_t keyNull1 = {
      .private_key_signature_algorithm =
          IOTC_JWT_PRIVATE_KEY_SIGNATURE_ALGORITHM_ES256,
      .private_key_union_type = IOTC_CRYPTO_KEY_UNION_TYPE_PEM,
      .private_key_union.key_pem.key = NULL};

  const iotc_crypto_private_key_data_t keyNull2 = {
      .private_key_signature_algorithm =
          IOTC_JWT_PRIVATE_KEY_SIGNATURE_ALGORITHM_ES256,
      .private_key_union_type = IOTC_CRYPTO_KEY_UNION_TYPE_PEM,
      .private_key_union.key_pem.key = NULL};

  tt_int_op(0, ==, iotc_crypto_private_key_data_cmp(&keyA1, &keyA2));
  tt_int_op(0, !=, iotc_crypto_private_key_data_cmp(&keyA1, &keyB));
  tt_int_op(0, ==, iotc_crypto_private_key_data_cmp(&keyNull1, &keyNull2));
  tt_int_op(0, !=, iotc_crypto_private_key_data_cmp(&keyB, &keyNull1));

end:;
})

IOTC_TT_TESTCASE(test_helpers_iotc_crypto_private_key_data_cmp_slot_id, {
  const iotc_crypto_private_key_data_t key1A = {
      .private_key_signature_algorithm =
          IOTC_JWT_PRIVATE_KEY_SIGNATURE_ALGORITHM_ES256,
      .private_key_union_type = IOTC_CRYPTO_KEY_UNION_TYPE_SLOT_ID,
      .private_key_union.key_slot.slot_id = 1};

  const iotc_crypto_private_key_data_t key1B = {
      .private_key_signature_algorithm =
          IOTC_JWT_PRIVATE_KEY_SIGNATURE_ALGORITHM_ES256,
      .private_key_union_type = IOTC_CRYPTO_KEY_UNION_TYPE_SLOT_ID,
      .private_key_union.key_slot.slot_id = 1};

  const iotc_crypto_private_key_data_t key2 = {
      .private_key_signature_algorithm =
          IOTC_JWT_PRIVATE_KEY_SIGNATURE_ALGORITHM_ES256,
      .private_key_union_type = IOTC_CRYPTO_KEY_UNION_TYPE_SLOT_ID,
      .private_key_union.key_slot.slot_id = 2};

  tt_int_op(0, ==, iotc_crypto_private_key_data_cmp(&key1A, &key1B));
  tt_int_op(0, !=, iotc_crypto_private_key_data_cmp(&key1A, &key2));
end:;
})

IOTC_TT_TESTCASE(test_helpers_iotc_crypto_private_key_data_cmp_custom, {
  const char* dataA1 = "IoT-A";
  const size_t dataA_size = strlen(dataA1);
  const iotc_crypto_private_key_data_t keyA1 = {
      .private_key_signature_algorithm =
          IOTC_JWT_PRIVATE_KEY_SIGNATURE_ALGORITHM_ES256,
      .private_key_union_type = IOTC_CRYPTO_KEY_UNION_TYPE_CUSTOM,
      .private_key_union.key_custom = {.data = (void*)dataA1,
                                       .data_size = dataA_size}};

  const char* dataA2 = "IoT-A";
  const iotc_crypto_private_key_data_t keyA2 = {
      .private_key_signature_algorithm =
          IOTC_JWT_PRIVATE_KEY_SIGNATURE_ALGORITHM_ES256,
      .private_key_union_type = IOTC_CRYPTO_KEY_UNION_TYPE_CUSTOM,
      .private_key_union.key_custom = {.data = (void*)dataA2,
                                       .data_size = dataA_size}};

  const char* dataB = "IoT-B";
  const size_t dataB_size = strlen(dataB);
  const iotc_crypto_private_key_data_t keyB = {
      .private_key_signature_algorithm =
          IOTC_JWT_PRIVATE_KEY_SIGNATURE_ALGORITHM_ES256,
      .private_key_union_type = IOTC_CRYPTO_KEY_UNION_TYPE_CUSTOM,
      .private_key_union.key_custom = {.data = (void*)dataB,
                                       .data_size = dataB_size}};

  // prefix of B
  const size_t dataBShorter_size = strlen(dataB) - 1;
  const iotc_crypto_private_key_data_t keyBShorter = {
      .private_key_signature_algorithm =
          IOTC_JWT_PRIVATE_KEY_SIGNATURE_ALGORITHM_ES256,
      .private_key_union_type = IOTC_CRYPTO_KEY_UNION_TYPE_CUSTOM,
      .private_key_union.key_custom = {.data = (void*)dataB,
                                       .data_size = dataBShorter_size}};

  const iotc_crypto_private_key_data_t keyNull1 = {
      .private_key_signature_algorithm =
          IOTC_JWT_PRIVATE_KEY_SIGNATURE_ALGORITHM_ES256,
      .private_key_union_type = IOTC_CRYPTO_KEY_UNION_TYPE_CUSTOM,
      .private_key_union.key_custom = {.data = NULL, .data_size = 0}};

  const iotc_crypto_private_key_data_t keyNull2 = {
      .private_key_signature_algorithm =
          IOTC_JWT_PRIVATE_KEY_SIGNATURE_ALGORITHM_ES256,
      .private_key_union_type = IOTC_CRYPTO_KEY_UNION_TYPE_CUSTOM,
      .private_key_union.key_custom = {.data = NULL, .data_size = 0}};

  tt_int_op(0, ==, iotc_crypto_private_key_data_cmp(&keyA1, &keyA2));
  tt_int_op(0, !=, iotc_crypto_private_key_data_cmp(&keyA1, &keyB));
  tt_int_op(0, !=, iotc_crypto_private_key_data_cmp(&keyB, &keyBShorter));
  tt_int_op(0, !=, iotc_crypto_private_key_data_cmp(&keyB, &keyNull1));
  tt_int_op(0, ==, iotc_crypto_private_key_data_cmp(&keyNull1, &keyNull2));
end:;
})

IOTC_TT_TESTCASE(test_helpers_iotc_crypto_private_key_data_dup_pem, {
  const iotc_crypto_private_key_data_t src_key = {
      .private_key_signature_algorithm =
          IOTC_JWT_PRIVATE_KEY_SIGNATURE_ALGORITHM_ES256,
      .private_key_union_type = IOTC_CRYPTO_KEY_UNION_TYPE_PEM,
      .private_key_union.key_pem.key = "dummy key"};

  iotc_crypto_private_key_data_t* copied_key =
      iotc_crypto_private_key_data_dup(&src_key);
  tt_assert(copied_key != NULL);
  tt_int_op(copied_key->private_key_signature_algorithm, ==,
            IOTC_JWT_PRIVATE_KEY_SIGNATURE_ALGORITHM_ES256);
  tt_int_op(copied_key->private_key_union_type, ==,
            IOTC_CRYPTO_KEY_UNION_TYPE_PEM);
  tt_ptr_op(copied_key->private_key_union.key_pem.key, !=,
            src_key.private_key_union.key_pem.key);
  tt_str_op(copied_key->private_key_union.key_pem.key, ==, "dummy key");

end:
  iotc_crypto_private_key_data_free(copied_key);
})

IOTC_TT_TESTCASE(test_helpers_iotc_crypto_private_key_data_dup_slot_id, {
  const iotc_crypto_private_key_data_t src_key = {
      .private_key_signature_algorithm =
          IOTC_JWT_PRIVATE_KEY_SIGNATURE_ALGORITHM_ES256,
      .private_key_union_type = IOTC_CRYPTO_KEY_UNION_TYPE_SLOT_ID,
      .private_key_union.key_slot.slot_id = 42};

  iotc_crypto_private_key_data_t* copied_key =
      iotc_crypto_private_key_data_dup(&src_key);
  tt_assert(copied_key != NULL);
  tt_int_op(copied_key->private_key_union_type, ==,
            IOTC_CRYPTO_KEY_UNION_TYPE_SLOT_ID);
  tt_int_op(copied_key->private_key_union.key_slot.slot_id, ==, 42);

end:
  iotc_crypto_private_key_data_free(copied_key);
})

IOTC_TT_TESTCASE(test_helpers_iotc_crypto_private_key_data_dup_custom, {
  const char* data = "IoT";
  const size_t data_size = strlen(data);
  const iotc_crypto_private_key_data_t src_key = {
      .private_key_signature_algorithm =
          IOTC_JWT_PRIVATE_KEY_SIGNATURE_ALGORITHM_ES256,
      .private_key_union_type = IOTC_CRYPTO_KEY_UNION_TYPE_CUSTOM,
      .private_key_union.key_custom = {.data = (void*)data,
                                       .data_size = data_size}};

  iotc_crypto_private_key_data_t* copied_key =
      iotc_crypto_private_key_data_dup(&src_key);
  tt_assert(copied_key != NULL);
  tt_int_op(copied_key->private_key_union_type, ==,
            IOTC_CRYPTO_KEY_UNION_TYPE_CUSTOM);
  tt_ptr_op(copied_key->private_key_union.key_custom.data, !=,
            src_key.private_key_union.key_custom.data);
  tt_assert(memcmp(copied_key->private_key_union.key_custom.data, data,
                   data_size) == 0);
  tt_int_op(copied_key->private_key_union.key_custom.data_size, ==, data_size);

end:
  iotc_crypto_private_key_data_free(copied_key);
})

IOTC_TT_TESTGROUP_END

#ifndef IOTC_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#define IOTC_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#include __FILE__
#undef IOTC_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#endif
