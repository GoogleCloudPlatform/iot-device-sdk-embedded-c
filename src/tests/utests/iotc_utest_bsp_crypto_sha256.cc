/* Copyright 2018-2019 Google LLC
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

#include <iostream>

#include "gmock.h"
#include "gtest.h"
#include "iotc_bsp_crypto.h"

namespace iotctest {
namespace {

using ::testing::ElementsAreArray;

constexpr unsigned int kSHA256HashSize = 32;

TEST(IotcBspCryptoSha256, OutputBufferIsNull) {
  EXPECT_EQ(iotc_bsp_sha256(nullptr,
                            reinterpret_cast<const unsigned char*>("abc"),
                            /* src_buf_size = */ 3),
            IOTC_BSP_CRYPTO_INVALID_INPUT_PARAMETER_ERROR);
}

TEST(IotcBspCryptoSha256, EmptyInputStrings) {
  uint8_t sha256_encoded[kSHA256HashSize] = {0};

  EXPECT_EQ(iotc_bsp_sha256(sha256_encoded, nullptr, /* src_buf_size = */ 0),
            IOTC_BSP_CRYPTO_STATE_OK);
  EXPECT_THAT(
      sha256_encoded,
      ElementsAreArray({0xe3, 0xb0, 0xc4, 0x42, 0x98, 0xfc, 0x1c, 0x14,
                        0x9a, 0xfb, 0xf4, 0xc8, 0x99, 0x6f, 0xb9, 0x24,
                        0x27, 0xae, 0x41, 0xe4, 0x64, 0x9b, 0x93, 0x4c,
                        0xa4, 0x95, 0x99, 0x1b, 0x78, 0x52, 0xb8, 0x55}));
}

TEST(IotcBspCryptoSha256, ExtremeInputString) {
  uint8_t sha256_encoded[kSHA256HashSize] = {0};

  EXPECT_EQ(iotc_bsp_sha256(sha256_encoded,
                            reinterpret_cast<const unsigned char*>("a"),
                            /* src_buf_size = */ 1),
            IOTC_BSP_CRYPTO_STATE_OK);
  EXPECT_THAT(
      sha256_encoded,
      ElementsAreArray({0xCA, 0x97, 0x81, 0x12, 0xCA, 0x1B, 0xBD, 0xCA,
                        0xFA, 0xC2, 0x31, 0xB3, 0x9A, 0x23, 0xDC, 0x4D,
                        0xA7, 0x86, 0xEF, 0xF8, 0x14, 0x7C, 0x4E, 0x72,
                        0xB9, 0x80, 0x77, 0x85, 0xAF, 0xEE, 0x48, 0xBB}));

  EXPECT_EQ(iotc_bsp_sha256(nullptr, nullptr, /* src_buf_size = */ 0),
            IOTC_BSP_CRYPTO_INVALID_INPUT_PARAMETER_ERROR);

  EXPECT_EQ(iotc_bsp_sha256(sha256_encoded,
                            reinterpret_cast<const unsigned char*>(""),
                            /* src_buf_size = */ 0),
            IOTC_BSP_CRYPTO_STATE_OK);
  EXPECT_THAT(
      sha256_encoded,
      ElementsAreArray({0xe3, 0xb0, 0xc4, 0x42, 0x98, 0xfc, 0x1c, 0x14,
                        0x9a, 0xfb, 0xf4, 0xc8, 0x99, 0x6f, 0xb9, 0x24,
                        0x27, 0xae, 0x41, 0xe4, 0x64, 0x9b, 0x93, 0x4c,
                        0xa4, 0x95, 0x99, 0x1b, 0x78, 0x52, 0xb8, 0x55}));

  EXPECT_EQ(
      iotc_bsp_sha256(sha256_encoded,
                      reinterpret_cast<const unsigned char*>("abcde\0fghi"),
                      /* src_buf_size = */ sizeof("abcde\0fghi") - 1),
      IOTC_BSP_CRYPTO_STATE_OK);
  EXPECT_THAT(
      sha256_encoded,
      ElementsAreArray({0x28, 0x32, 0x48, 0xe7, 0x48, 0x68, 0x68, 0xf7,
                        0xa4, 0x2a, 0xc0, 0xea, 0xb3, 0x15, 0xc1, 0xa6,
                        0x8,  0xfd, 0xab, 0xfa, 0xaf, 0x7a, 0xee, 0x25,
                        0x5d, 0x4,  0xb,  0xb8, 0x1b, 0x72, 0xaf, 0xac}));
}

TEST(IotcBspCryptoSha256, LongerInputString) {
  uint8_t sha256_encoded[kSHA256HashSize] = {0};
  const char* kSrcString =
      "Dear unit test please sha256 hash this string! Thank you!";
  EXPECT_EQ(iotc_bsp_sha256(sha256_encoded,
                            reinterpret_cast<const unsigned char*>(kSrcString),
                            /* src_buf_size = */ strlen(kSrcString)),
            IOTC_BSP_CRYPTO_STATE_OK);
  EXPECT_THAT(
      sha256_encoded,
      ElementsAreArray({0x10, 0xdf, 0x52, 0xdb, 0xb0, 0xcd, 0xea, 0x43,
                        0x13, 0x6b, 0x5b, 0x2b, 0x36, 0x02, 0x03, 0xb2,
                        0xbd, 0x22, 0xdb, 0x63, 0x04, 0x9d, 0xa9, 0x09,
                        0x8f, 0xb7, 0x4d, 0x2e, 0x11, 0x19, 0xf5, 0x99}));
}

} // namespace
} // namespace iotctest