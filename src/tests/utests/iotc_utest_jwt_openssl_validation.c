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
#include "iotc_utest_basic_testcase_frame.h"
#include "tinytest.h"
#include "tinytest_macros.h"

#include "iotc_jwt.h"
#include "iotc_macros.h"

#include <openssl/bio.h>
#include <openssl/ec.h>
#include <openssl/evp.h>
#include <stdio.h>

#include "iotc_bsp_time.h"

#ifndef IOTC_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN

static const iotc_crypto_private_key_data_t DEFAULT_PRIVATE_KEY = {
    .private_key_signature_algorithm =
        IOTC_JWT_PRIVATE_KEY_SIGNATURE_ALGORITHM_ES256,
    .private_key_union_type = IOTC_CRYPTO_KEY_UNION_TYPE_PEM,
    .private_key_union.key_pem.key =
        "\
-----BEGIN EC PRIVATE KEY-----\n\
MHcCAQEEINg6KhkJ2297KYO4eyLTPtVIhLloIfp3IsJo9n6KqelfoAoGCCqGSM49\n\
AwEHoUQDQgAE1Oi16oAc/+s5P5g2pzt3IDXfUBBUKUBrB8vgfyKOFb7sQTx4topE\n\
E0KOix7rJyli6tiAJJDL4lbdf0YRo45THQ==\n\
-----END EC PRIVATE KEY-----"};

static void base64_decode_openssl(const char* base64_string, char* dst_string,
                                  size_t dst_len) {
  IOTC_UNUSED(dst_len);

  BIO* bio;
  BIO* b64;

  bio = BIO_new_mem_buf((void*)base64_string, -1);
  b64 = BIO_new(BIO_f_base64());
  bio = BIO_push(b64, bio);

  BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL);
  size_t length = BIO_read(bio, dst_string, strlen(base64_string));

  IOTC_UNUSED(length);

  BIO_free_all(bio);
}

#endif

IOTC_TT_TESTGROUP_BEGIN(utest_jwt_openssl_validation)

IOTC_TT_TESTCASE_WITH_SETUP(
    jwt_create_es256__validate_first_section, iotc_utest_setup_basic,
    iotc_utest_teardown_basic, NULL, {
      unsigned char jwt[IOTC_JWT_SIZE] = {0};
      size_t bytes_written = 0;
      const iotc_state_t ret =
          iotc_create_jwt_es256("projectID", 600, &DEFAULT_PRIVATE_KEY, jwt,
                                IOTC_JWT_SIZE, &bytes_written);

      IOTC_UNUSED(ret);

      char* first_dot_pos = strchr((char*)jwt, '.');

      tt_ptr_op(NULL, !=, first_dot_pos);

      *first_dot_pos = 0;

      char first_section[IOTC_JWT_SIZE] = {0};
      const char* first_section_b64 = (char*)jwt;

      base64_decode_openssl(first_section_b64, first_section, IOTC_JWT_SIZE);
      // first_section ~= {"alg":"ES256","typ":"JWT"}

      tt_str_op("{\"alg\":\"ES256\",\"typ\":\"JWT\"}", ==, first_section);

    end:;
    })

IOTC_TT_TESTCASE_WITH_SETUP(
    jwt_create_es256__validate_second_section, iotc_utest_setup_basic,
    iotc_utest_teardown_basic, NULL, {
      const uint32_t expiration_period_sec = 1600;
      const iotc_time_t current_time_at_start_of_test =
          iotc_bsp_time_getcurrenttime_seconds();
      const char* project_id = "projectID";

      unsigned char jwt[IOTC_JWT_SIZE] = {0};
      size_t bytes_written = 0;
      const iotc_state_t ret = iotc_create_jwt_es256(
          project_id, expiration_period_sec, &DEFAULT_PRIVATE_KEY, jwt,
          IOTC_JWT_SIZE, &bytes_written);

      IOTC_UNUSED(ret);

      const char* first_dot_pos = strchr((char*)jwt, '.');
      char* second_dot_pos = strchr(first_dot_pos + 1, '.');

      tt_ptr_op(NULL, !=, second_dot_pos);

      *second_dot_pos = 0;

      char second_section[IOTC_JWT_SIZE] = {0};
      const char* second_section_b64 = first_dot_pos + 1;

      base64_decode_openssl(second_section_b64, second_section, IOTC_JWT_SIZE);
      // second_section ~= {"iat":1536227916,"exp":1536229516,"aud":"projectID"}

      const char* expected_second_section_format =
          "{\"iat\":%lu,\"exp\":%lu,\"aud\":\"%s\"}";

      char expected_second_section[IOTC_JWT_SIZE] = {0};
      snprintf(expected_second_section, IOTC_JWT_SIZE,
               expected_second_section_format, current_time_at_start_of_test,
               current_time_at_start_of_test + expiration_period_sec,
               project_id);

#ifndef SWITCH_THIS_BLOCK_IF_THIS_TEST_CASE_BECOMES_FLAKY
      tt_str_op(expected_second_section, ==, second_section);
#else
      const int first_string_matches = !strcmp( expected_second_section, second_section );

      char expected_second_section_plus_a_second[ IOTC_JWT_SIZE ] = { 0 };
      snprintf( expected_second_section_plus_a_second, IOTC_JWT_SIZE,
                expected_second_section_format,
                current_time_at_start_of_test + 1,
                current_time_at_start_of_test + 1 + expiration_period_sec,
                project_id );

      const int second_string_matches =
          !strcmp( expected_second_section_plus_a_second, second_section );

      tt_int_op( first_string_matches, !=, second_string_matches );

#endif

    end:;
    })
IOTC_TT_TESTGROUP_END

#ifndef IOTC_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#define IOTC_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#include __FILE__
#undef IOTC_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#endif
