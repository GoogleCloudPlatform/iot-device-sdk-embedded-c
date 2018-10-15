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

#include <stdio.h>

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

#endif

IOTC_TT_TESTGROUP_BEGIN(utest_jwt)

IOTC_TT_TESTCASE_WITH_SETUP(jwt_create_es256__small_buffer_provided,
                            iotc_utest_setup_basic, iotc_utest_teardown_basic,
                            NULL, {
                              unsigned char jwt[IOTC_JWT_SIZE] = {0};
                              size_t bytes_written = 0;
                              const iotc_state_t ret = iotc_create_jwt_es256(
                                  "projectID", 600, &DEFAULT_PRIVATE_KEY, jwt,
                                  1, &bytes_written);

                              printf("ret: %d\n", ret);
                              tt_int_op(IOTC_BUFFER_TOO_SMALL_ERROR, ==, ret);

                            end:;
                            })

IOTC_TT_TESTCASE_WITH_SETUP(
    jwt_create_es256__jwt_string_consists_of_three_dot_separated_strings,
    iotc_utest_setup_basic, iotc_utest_teardown_basic, NULL, {
      unsigned char jwt[IOTC_JWT_SIZE] = {0};
      size_t bytes_written = 0;
      const iotc_state_t ret =
          iotc_create_jwt_es256("projectID", 600, &DEFAULT_PRIVATE_KEY, jwt,
                                IOTC_JWT_SIZE, &bytes_written);

      tt_int_op(IOTC_STATE_OK, ==, ret);

      const char* first_dot_pos = strchr((char*)jwt, '.');
      const char* second_dot_pos = strchr(first_dot_pos + 1, '.');
      const char* third_dot_pos = strchr(second_dot_pos + 1, '.');

      tt_ptr_op(NULL, !=, first_dot_pos);
      tt_ptr_op(NULL, !=, second_dot_pos);
      tt_ptr_op(NULL, ==, third_dot_pos);

    end:;
    })

IOTC_TT_TESTCASE_WITH_SETUP(
    jwt_create_es256__long_projectid, iotc_utest_setup_basic,
    iotc_utest_teardown_basic, NULL, {
      const uint32_t expiration_period_sec = 1600;
      char project_id_too_long[IOTC_JWT_SIZE] = {0};

      size_t i = 0;
      for (; i < IOTC_JWT_SIZE - 1; ++i) {
        project_id_too_long[i] = 'x';
      }

      unsigned char jwt[IOTC_JWT_SIZE] = {0};
      size_t bytes_written = 0;
      iotc_state_t ret = iotc_create_jwt_es256(
          project_id_too_long, expiration_period_sec, &DEFAULT_PRIVATE_KEY, jwt,
          IOTC_JWT_SIZE, &bytes_written);

      tt_int_op(IOTC_JWT_PROJECTID_TOO_LONG_ERROR, ==, ret);
      printf("bytes_written: %lu\n", bytes_written);

      // make project id the desired max long
      tt_int_op(bytes_written, <, IOTC_JWT_SIZE);
      project_id_too_long[bytes_written] = 0;

      ret = iotc_create_jwt_es256(project_id_too_long, expiration_period_sec,
                                  &DEFAULT_PRIVATE_KEY, jwt, IOTC_JWT_SIZE,
                                  &bytes_written);

      tt_int_op(IOTC_STATE_OK, ==, ret);
      printf("ret: %d\n", ret);
    end:;
    })

IOTC_TT_TESTGROUP_END

#ifndef IOTC_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#define IOTC_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#include __FILE__
#undef IOTC_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#endif
