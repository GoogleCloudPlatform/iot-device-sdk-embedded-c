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

#include "iotc_bsp_crypto.h"
#include "iotc_macros.h"

#include <stdio.h>

#ifndef IOTC_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN

#define IOTC_SHA256_HASH_SIZE (256 / 8)

#endif

IOTC_TT_TESTGROUP_BEGIN(utest_bsp_crypto_sha256)

IOTC_TT_TESTCASE_WITH_SETUP(
    sha256_output_buffer_is_null, iotc_utest_setup_basic,
    iotc_utest_teardown_basic, NULL, {
      const iotc_bsp_crypto_state_t ret =
          iotc_bsp_sha256(NULL, (uint8_t*)"abc", 3);
      tt_assert(IOTC_BSP_CRYPTO_INVALID_INPUT_PARAMETER_ERROR == ret);

    end:;
    })

IOTC_TT_TESTCASE_WITH_SETUP(
    sha256_empty_string, iotc_utest_setup_basic, iotc_utest_teardown_basic,
    NULL, {
      uint8_t sha256[IOTC_SHA256_HASH_SIZE] = {0};
      const iotc_bsp_crypto_state_t ret =
          iotc_bsp_sha256(sha256, (uint8_t*)NULL, 0);
      tt_assert(ret == IOTC_BSP_CRYPTO_STATE_OK);

      const int8_t sha256_expected[IOTC_SHA256_HASH_SIZE] = {
          0xe3, 0xb0, 0xc4, 0x42, 0x98, 0xfc, 0x1c, 0x14, 0x9a, 0xfb, 0xf4,
          0xc8, 0x99, 0x6f, 0xb9, 0x24, 0x27, 0xae, 0x41, 0xe4, 0x64, 0x9b,
          0x93, 0x4c, 0xa4, 0x95, 0x99, 0x1b, 0x78, 0x52, 0xb8, 0x55};
      tt_int_op(0, ==, memcmp(sha256, sha256_expected, IOTC_SHA256_HASH_SIZE));

    end:;
    })

IOTC_TT_TESTCASE_WITH_SETUP(
    sha256_a, iotc_utest_setup_basic, iotc_utest_teardown_basic, NULL, {
      uint8_t sha256[IOTC_SHA256_HASH_SIZE] = {0};
      const iotc_bsp_crypto_state_t ret =
          iotc_bsp_sha256(sha256, (uint8_t*)"a", 1);
      tt_assert(ret == IOTC_BSP_CRYPTO_STATE_OK);

      const int8_t sha256_expected[IOTC_SHA256_HASH_SIZE] = {
          0xCA, 0x97, 0x81, 0x12, 0xCA, 0x1B, 0xBD, 0xCA, 0xFA, 0xC2, 0x31,
          0xB3, 0x9A, 0x23, 0xDC, 0x4D, 0xA7, 0x86, 0xEF, 0xF8, 0x14, 0x7C,
          0x4E, 0x72, 0xB9, 0x80, 0x77, 0x85, 0xAF, 0xEE, 0x48, 0xBB};
      tt_int_op(0, ==, memcmp(sha256, sha256_expected, IOTC_SHA256_HASH_SIZE));

    end:;
    })

IOTC_TT_TESTCASE_WITH_SETUP(
    sha256_longerinput, iotc_utest_setup_basic, iotc_utest_teardown_basic, NULL,
    {
      uint8_t sha256[IOTC_SHA256_HASH_SIZE] = {0};
      const char* input =
          "Dear unit test please sha256 hash this string! Thank you!";
      const iotc_bsp_crypto_state_t ret =
          iotc_bsp_sha256(sha256, (uint8_t*)input, strlen(input));
      tt_assert(ret == IOTC_BSP_CRYPTO_STATE_OK);

      const int8_t sha256_expected[IOTC_SHA256_HASH_SIZE] = {
          0x10, 0xdf, 0x52, 0xdb, 0xb0, 0xcd, 0xea, 0x43, 0x13, 0x6b, 0x5b,
          0x2b, 0x36, 0x02, 0x03, 0xb2, 0xbd, 0x22, 0xdb, 0x63, 0x04, 0x9d,
          0xa9, 0x09, 0x8f, 0xb7, 0x4d, 0x2e, 0x11, 0x19, 0xf5, 0x99};
      tt_int_op(0, ==, memcmp(sha256, sha256_expected, IOTC_SHA256_HASH_SIZE));

    end:;
    })

IOTC_TT_TESTGROUP_END

#ifndef IOTC_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#define IOTC_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#include __FILE__
#undef IOTC_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#endif
