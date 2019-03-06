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

#include "gtest.h"
#include "iotc_bsp_crypto.h"

namespace iotctest {
namespace {

size_t base64_encoded_length(const char* str) {
  if (str == nullptr)
    return 0;
  return ((strlen(str) + 2) / 3) * 4 + 1;
}

TEST(IotcBspCryptoBase64,
     ReportsErrorAndRequiredBufferSizeWhenOutputBufferIsTooSmall) {
  const char* kSrcString =
      "not big enough output buffer for base64 encoded string";
  const size_t kBase64BufferSize = 10;
  unsigned char b64_encoded[kBase64BufferSize];
  size_t bytes_written = 0;
  EXPECT_EQ(iotc_bsp_base64_encode_urlsafe(
                b64_encoded, kBase64BufferSize, &bytes_written,
                reinterpret_cast<const unsigned char*>(kSrcString),
                strlen(kSrcString)),
            IOTC_BSP_CRYPTO_BUFFER_TOO_SMALL_ERROR);
  EXPECT_EQ(bytes_written, base64_encoded_length(kSrcString));
}

TEST(IotcBspCryptoBase64, CommonStringEncoding) {
  const char* kSrcString = "first unit test string to base64 encode";
  const char* kBase64ExpectedString =
      "Zmlyc3QgdW5pdCB0ZXN0IHN0cmluZyB0byBiYXNlNjQgZW5jb2Rl";
  const size_t kBase64BufferSize = base64_encoded_length(kSrcString);
  unsigned char b64_encoded[kBase64BufferSize];
  size_t bytes_written = 0;

  EXPECT_EQ(iotc_bsp_base64_encode_urlsafe(
                b64_encoded, kBase64BufferSize, &bytes_written,
                reinterpret_cast<const unsigned char*>(kSrcString),
                strlen(kSrcString)),
            IOTC_BSP_CRYPTO_STATE_OK);
  EXPECT_STREQ((const char*)b64_encoded, kBase64ExpectedString);
  EXPECT_EQ(bytes_written, strlen(kBase64ExpectedString));
}

TEST(IotcBspCryptoBase64,
     NotBigEnoughOutputBufferProvidedAdvisedBufferSizeWorks) {
  // Inputs for testing base64-encoded suffixes and circularity.
  const char* kSrcStrings[] = {"12345", "123456", "1234567", "12345678"};
  const char* kBase64ExpectedStrings[] = {"MTIzNDU=", "MTIzNDU2",
                                          "MTIzNDU2Nw==", "MTIzNDU2Nzg="};
  uint8_t i = 0;
  for (; i < 4; ++i) {
    const char* kSrcString = kSrcStrings[i];
    const size_t kBase64BufferSize = 1;
    unsigned char b64_encoded[128];
    size_t bytes_written = 0;
    EXPECT_EQ(iotc_bsp_base64_encode_urlsafe(
                  b64_encoded, kBase64BufferSize, &bytes_written,
                  reinterpret_cast<const unsigned char*>(kSrcString),
                  strlen(kSrcString)),
              IOTC_BSP_CRYPTO_BUFFER_TOO_SMALL_ERROR);
    EXPECT_EQ(iotc_bsp_base64_encode_urlsafe(
                  b64_encoded, bytes_written, &bytes_written,
                  reinterpret_cast<const unsigned char*>(kSrcString),
                  strlen(kSrcString)),
              IOTC_BSP_CRYPTO_STATE_OK);
    EXPECT_STREQ((const char*)b64_encoded, kBase64ExpectedStrings[i]);
  }
}

TEST(IotcBspCryptoBase64, ZeroTerminatedStringGenerated) {
  const char* kSrcString = "text to base64 encode";
  const char* kBase64ExpectedString = "dGV4dCB0byBiYXNlNjQgZW5jb2Rl";
  const size_t kBase64BufferSize = 100;
  unsigned char b64_encoded[kBase64BufferSize];
  size_t bytes_written = 0;

  // Fill dst_string with non-zero values to check if base64 impl
  // puts zero to the end of base64 encoded string.
  memset(b64_encoded, 'x', kBase64BufferSize);

  ASSERT_EQ(iotc_bsp_base64_encode_urlsafe(
                b64_encoded, kBase64BufferSize, &bytes_written,
                reinterpret_cast<const unsigned char*>(kSrcString),
                strlen(kSrcString)),
            IOTC_BSP_CRYPTO_STATE_OK);
  EXPECT_STREQ((const char*)b64_encoded, kBase64ExpectedString);
  EXPECT_EQ(bytes_written, strlen(kBase64ExpectedString));
}

TEST(IotcBspCryptoBase64, ReplacesNonURLSafeBase64Chars) {
  const char* kSrcString = "\x18\x23\xc8\xa1\x3f\xbf";
  const char* kBase64ExpectedString = "GCPIoT-_";
  const size_t kBase64BufferSize = base64_encoded_length(kSrcString);
  unsigned char b64_encoded[kBase64BufferSize];
  size_t bytes_written = 0;

  ASSERT_EQ(iotc_bsp_base64_encode_urlsafe(
                b64_encoded, kBase64BufferSize, &bytes_written,
                reinterpret_cast<const unsigned char*>(kSrcString),
                strlen(kSrcString)),
            IOTC_BSP_CRYPTO_STATE_OK);

  EXPECT_STREQ((const char*)b64_encoded, kBase64ExpectedString);
}

} // namespace
} // namespace iotctest
