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

#include "iotc_tt_testcase_management.h"
#include "iotc_utest_basic_testcase_frame.h"
#include "tinytest.h"
#include "tinytest_macros.h"

#include "iotc_bsp_crypto.h"
#include "iotc_macros.h"

#include <stdio.h>

#ifndef IOTC_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN

#endif

IOTC_TT_TESTGROUP_BEGIN(utest_bsp_crypto_base64)

IOTC_TT_TESTCASE_WITH_SETUP(
    not_big_enough_output_buffer_provided__proper_error_and_required_buffer_size_expected,
    iotc_utest_setup_basic, iotc_utest_teardown_basic, NULL, {
      const char* src_string =
          "not big enough output buffer for base64 encoded string";
      const size_t dst_string_size = 10;
      unsigned char dst_string[dst_string_size];
      size_t bytes_written = 0;

      const iotc_bsp_crypto_state_t result = iotc_bsp_base64_encode_urlsafe(
          dst_string, dst_string_size, &bytes_written, (uint8_t*)src_string,
          strlen(src_string));

      tt_int_op(IOTC_BSP_CRYPTO_BUFFER_TOO_SMALL_ERROR, ==, result);
      // The math below calculates the required base 64 encoded buffer size
      // including trailing zero.
      // 1) (x + 2) / 3 equals to ceil(x / 3)
      // 2) * 4 is there since base64 generates 4 bytes from each 3 byte by
      // definition
      // 3) + 1 is the trailing zero
      tt_uint_op((strlen(src_string) + 2) / 3 * 4 + 1, ==, bytes_written);
    end:;
    })

IOTC_TT_TESTCASE_WITH_SETUP(
    output_buffer_too_small_urlsafe, iotc_utest_setup_basic,
    iotc_utest_teardown_basic, NULL, {
      const char* src_string =
          "not big enough output buffer for base64 encoded string";
      const size_t dst_string_size = 10;
      unsigned char dst_string[dst_string_size];
      size_t bytes_written = 0;

      const iotc_bsp_crypto_state_t result = iotc_bsp_base64_encode_urlsafe(
          dst_string, dst_string_size, &bytes_written, (uint8_t*)src_string,
          strlen(src_string));

      tt_int_op(IOTC_BSP_CRYPTO_BUFFER_TOO_SMALL_ERROR, ==, result);
    end:;
    })

IOTC_TT_TESTCASE_WITH_SETUP(
    common_string_encoding, iotc_utest_setup_basic, iotc_utest_teardown_basic,
    NULL, {
      const char* src_string = "first unit test string to base64 encode";
      // The math below calculates the required base 64 encoded buffer size
      // including trailing zero.
      // 1) (x + 2) / 3 equals to ceil(x / 3)
      // 2) * 4 is there since base64 generates 4 bytes from each 3 byte by
      // definition
      // 3) + 1 is the trailing zero
      const size_t dst_string_size = ((strlen(src_string) + 2) / 3) * 4 + 1;
      unsigned char dst_string[dst_string_size];
      size_t bytes_written = 0;

      const iotc_bsp_crypto_state_t result = iotc_bsp_base64_encode_urlsafe(
          dst_string, dst_string_size, &bytes_written, (uint8_t*)src_string,
          strlen(src_string));

      const char* b64_expected =
          "Zmlyc3QgdW5pdCB0ZXN0IHN0cmluZyB0byBiYXNlNjQgZW5jb2Rl";

      tt_str_op(b64_expected, ==, dst_string);
      tt_uint_op(strlen(b64_expected), ==, bytes_written);
      tt_int_op(IOTC_BSP_CRYPTO_STATE_OK, ==, result);

    end:;
    })

IOTC_TT_TESTCASE_WITH_SETUP(
    not_big_enough_output_buffer_provided__advised_buffer_size_works,
    iotc_utest_setup_basic, iotc_utest_teardown_basic, NULL, {
      char* strings_to_encode[] = {"12345", "123456", "1234567", "12345678"};
      const char* base64_encoded_strings[] = {"MTIzNDU=", "MTIzNDU2",
                                              "MTIzNDU2Nw==", "MTIzNDU2Nzg="};

      uint8_t i = 0;
      for (; i < 4; ++i) {
        char* src_string = strings_to_encode[i];
        const size_t dst_string_size = 1;
        unsigned char dst_string[128];
        size_t bytes_written = 0;

        iotc_bsp_crypto_state_t result = iotc_bsp_base64_encode_urlsafe(
            dst_string, dst_string_size, &bytes_written, (uint8_t*)src_string,
            strlen(src_string));

        tt_int_op(IOTC_BSP_CRYPTO_BUFFER_TOO_SMALL_ERROR, ==, result);

        result = iotc_bsp_base64_encode_urlsafe(
            dst_string, bytes_written, &bytes_written, (uint8_t*)src_string,
            strlen(src_string));

        tt_str_op(base64_encoded_strings[i], ==, dst_string);
      }
    end:;
    })

IOTC_TT_TESTCASE_WITH_SETUP(
    zero_terminated_string_generated, iotc_utest_setup_basic,
    iotc_utest_teardown_basic, NULL, {
      const char* src_string = "text to base64 encode";
      const size_t dst_string_size = 100;
      unsigned char dst_string[dst_string_size];
      size_t bytes_written = 0;

      // fill dst_string with non-zero values to check if base64 impl
      // puts zero to the end of base64 encoded string
      size_t i = 0;
      for (; i < dst_string_size; ++i) {
        dst_string[i] = 'x';
      }

      const iotc_bsp_crypto_state_t result = iotc_bsp_base64_encode_urlsafe(
          dst_string, dst_string_size, &bytes_written, (uint8_t*)src_string,
          strlen(src_string));

      const char* b64_expected = "dGV4dCB0byBiYXNlNjQgZW5jb2Rl";

      tt_str_op(b64_expected, ==, dst_string);
      tt_uint_op(strlen(b64_expected), ==, bytes_written);
      tt_int_op(IOTC_BSP_CRYPTO_STATE_OK, ==, result);

    end:;
    })

IOTC_TT_TESTCASE_WITH_SETUP(
    common_string_encoding_urlsafe, iotc_utest_setup_basic,
    iotc_utest_teardown_basic, NULL, {
      const char* src_string = "\x18#\xc8\xa1?\xbf";
      // The math below calculates the required base 64 encoded buffer size
      // including trailing zero.
      // 1) (x + 2) / 3 equals to ceil(x / 3)
      // 2) * 4 is there since base64 generates 4 bytes from each 3 byte by
      // definition
      // 3) + 1 is the trailing zero
      const size_t dst_string_size = ((strlen(src_string) + 2) / 3) * 4 + 1;
      unsigned char dst_string[dst_string_size];
      size_t bytes_written = 0;

      const iotc_bsp_crypto_state_t result = iotc_bsp_base64_encode_urlsafe(
          dst_string, dst_string_size, &bytes_written, (uint8_t*)src_string,
          strlen(src_string));

      const char* b64_expected = "GCPIoT-_";

      tt_str_op(b64_expected, ==, dst_string);
      tt_uint_op(strlen(b64_expected), ==, bytes_written);
      tt_int_op(IOTC_BSP_CRYPTO_STATE_OK, ==, result);

    end:;
    })

IOTC_TT_TESTGROUP_END

#ifndef IOTC_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#define IOTC_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#include __FILE__
#undef IOTC_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#endif
