/* Copyright 2018-2019 Google LLC
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

#include <cstring>
#include <iostream>

#include "gtest.h"
#include "iotc.h"
#include "iotc_bsp_crypto.h"
#include "iotc_heapcheck_test.h"
#include "iotc_helpers.h"
#include "iotc_jwt.h"
#include "iotc_macros.h"
#include "iotc_openssl_utils.h"
#include "iotc_types.h"

namespace iotctest {
namespace {

constexpr char kPublicKey[] = R"(
-----BEGIN PUBLIC KEY-----
MFkwEwYHKoZIzj0CAQYIKoZIzj0DAQcDQgAE1Oi16oAc/+s5P5g2pzt3IDXfUBBU
KUBrB8vgfyKOFb7sQTx4topEE0KOix7rJyli6tiAJJDL4lbdf0YRo45THQ==
-----END PUBLIC KEY-----)";

constexpr char kPrivateKey[] = R"(
-----BEGIN EC PRIVATE KEY-----
MHcCAQEEINg6KhkJ2297KYO4eyLTPtVIhLloIfp3IsJo9n6KqelfoAoGCCqGSM49
AwEHoUQDQgAE1Oi16oAc/+s5P5g2pzt3IDXfUBBUKUBrB8vgfyKOFb7sQTx4topE
E0KOix7rJyli6tiAJJDL4lbdf0YRo45THQ==
-----END EC PRIVATE KEY-----)";

static const uint8_t* kDefaultDataToSign =
    (uint8_t*)"this text is ecc signed";
static const size_t kDefaultDataToSignLength =
    strlen((char*)kDefaultDataToSign);

class IotcBspCryptoEcc : public IotcHeapCheckTest {
  public:
    IotcBspCryptoEcc() {
      iotc_initialize();
      DefaultPrivateKey.crypto_key_signature_algorithm =
          IOTC_CRYPTO_KEY_SIGNATURE_ALGORITHM_ES256,
      DefaultPrivateKey.crypto_key_union_type = IOTC_CRYPTO_KEY_UNION_TYPE_PEM,
      DefaultPrivateKey.crypto_key_union.key_pem.key =
          const_cast<char*>(kPrivateKey);
    }
    ~IotcBspCryptoEcc() { iotc_shutdown(); }

  protected:
    iotc_crypto_key_data_t DefaultPrivateKey;
};

TEST_F(IotcBspCryptoEcc, ReportsErrorWhenBufferIsTooSmall) {
  constexpr size_t kEccSignatureBufferLength = 1;
  uint8_t ecc_signature[kEccSignatureBufferLength];
  size_t bytes_written = 0;
  EXPECT_EQ(iotc_bsp_ecc(&DefaultPrivateKey, ecc_signature,
                         kEccSignatureBufferLength, &bytes_written,
                         kDefaultDataToSign, kDefaultDataToSignLength),
            IOTC_BSP_CRYPTO_BUFFER_TOO_SMALL_ERROR);
}

TEST_F(IotcBspCryptoEcc, SmallBufferProperMinSize) {
  constexpr size_t kEccSignatureBufferLength = 1;
  uint8_t ecc_signature[kEccSignatureBufferLength];
  uint8_t ecc_signature_new[64];
  size_t bytes_written = 0;

  EXPECT_EQ(iotc_bsp_ecc(&DefaultPrivateKey, ecc_signature,
                         kEccSignatureBufferLength, &bytes_written,
                         kDefaultDataToSign, kDefaultDataToSignLength),
            IOTC_BSP_CRYPTO_BUFFER_TOO_SMALL_ERROR);
  // Two 32-byte integers(r and s) build up a JWT ECC signature.
  EXPECT_EQ(bytes_written, 64u);

  const size_t kEccSignatureNewBufferLen = bytes_written;

  // By definition, the expected JWT ECC signature size is 64 bytes.
  // https://tools.ietf.org/html/rfc7518#section-3.4
  ASSERT_EQ(64u, kEccSignatureNewBufferLen);
  EXPECT_EQ(iotc_bsp_ecc(&DefaultPrivateKey, ecc_signature_new,
                         kEccSignatureNewBufferLen, &bytes_written,
                         kDefaultDataToSign, kDefaultDataToSignLength),
            IOTC_BSP_CRYPTO_STATE_OK);
  EXPECT_EQ(bytes_written, kEccSignatureNewBufferLen);
}

TEST_F(IotcBspCryptoEcc, ReportsErrorOnInvalidPrivateKey) {
  constexpr char kInvalid[] = "invalid key";
  const iotc_crypto_key_data_t kInvalidKey = {
      IOTC_CRYPTO_KEY_UNION_TYPE_PEM, const_cast<char*>(kInvalid),
      IOTC_CRYPTO_KEY_SIGNATURE_ALGORITHM_ES256};
  const size_t kEccSignatureBufferLength = 128;
  uint8_t ecc_signature[kEccSignatureBufferLength];
  size_t bytes_written = 0;

  EXPECT_EQ(iotc_bsp_ecc(&kInvalidKey, ecc_signature, kEccSignatureBufferLength,
                         &bytes_written, kDefaultDataToSign,
                         kDefaultDataToSignLength),
            IOTC_BSP_CRYPTO_KEY_PARSE_ERROR);
}

TEST_F(IotcBspCryptoEcc, ReportsErrorWhenOutputBufferIsNull) {
  size_t bytes_written = 0;

  EXPECT_EQ(iotc_bsp_ecc(&DefaultPrivateKey, nullptr,
                         /*ecc_signature_buffer_len=*/128, &bytes_written,
                         kDefaultDataToSign, kDefaultDataToSignLength),
            IOTC_BSP_CRYPTO_INVALID_INPUT_PARAMETER_ERROR);
}

TEST_F(IotcBspCryptoEcc, ReportsErrorWhenInputBufferIsNull) {
  const size_t kEccSignatureBufferLength = 128;
  uint8_t ecc_signature[kEccSignatureBufferLength];
  size_t bytes_written = 0;

  EXPECT_EQ(iotc_bsp_ecc(&DefaultPrivateKey, ecc_signature,
                         kEccSignatureBufferLength, &bytes_written, nullptr,
                         kDefaultDataToSignLength),
            IOTC_BSP_CRYPTO_INVALID_INPUT_PARAMETER_ERROR);
}

TEST_F(IotcBspCryptoEcc, JwtSignatureValidation) {
  const char* kJwtHeaderPayloadB64 =
      R"(eyJhbGciOiJFUzI1NiIsInR5cCI6IkpXVCJ9.
      eyJzdWIiOiIxMjM0NTY3ODkwIiwibmFtZSI6IkpvaG4gRG9lIiwiYWRtaW4iOnRydWUs
      ImlhdCI6MTUxNjIzOTAyMn0)";
  uint8_t hash_sha256[32] = {0};
  iotc_bsp_sha256(hash_sha256, (const uint8_t*)kJwtHeaderPayloadB64,
                  strlen(kJwtHeaderPayloadB64));
  size_t bytes_written_ecc_signature = 0;
  uint8_t ecc_signature[IOTC_JWT_MAX_SIGNATURE_SIZE] = {0};

  for (size_t i = 0; i < IOTC_JWT_MAX_SIGNATURE_SIZE; ++i) {
    ecc_signature[i] = 'x';
  }

  iotc_bsp_ecc(&DefaultPrivateKey, ecc_signature, IOTC_JWT_MAX_SIGNATURE_SIZE,
               &bytes_written_ecc_signature, hash_sha256, /*hash_len=*/32);

  // Assure no overwrite happens.
  if (bytes_written_ecc_signature < sizeof(ecc_signature)) {
    ASSERT_EQ('x', ecc_signature[bytes_written_ecc_signature]);
  }
  EXPECT_TRUE(openssl::ecc_is_valid(hash_sha256, /*hash len=*/32, ecc_signature,
                                    bytes_written_ecc_signature, kPublicKey));
  // Two 32-byte integers(r and s) build up a JWT ECC signature.
  EXPECT_EQ(bytes_written_ecc_signature, 64u);
}

TEST_F(IotcBspCryptoEcc, SimpleTextValidation) {
  const char* kSimpleText = "hi, I am the sipmle text, please sign me Mr. Ecc!";
  size_t bytes_written_ecc_signature = 0;
  uint8_t ecc_signature[IOTC_JWT_MAX_SIGNATURE_SIZE] = {0};

  for (size_t i = 0; i < IOTC_JWT_MAX_SIGNATURE_SIZE; i++) {
    ecc_signature[i] = 'x';
  }
  iotc_bsp_ecc(&DefaultPrivateKey, ecc_signature, IOTC_JWT_MAX_SIGNATURE_SIZE,
               &bytes_written_ecc_signature, (uint8_t*)kSimpleText,
               strlen(kSimpleText));
  // Assure no overwriting happens.
  if (bytes_written_ecc_signature < sizeof(ecc_signature)) {
    ASSERT_EQ('x', ecc_signature[bytes_written_ecc_signature]);
  }
  EXPECT_TRUE(openssl::ecc_is_valid((uint8_t*)kSimpleText, strlen(kSimpleText),
                                    ecc_signature, bytes_written_ecc_signature,
                                    kPublicKey));
  EXPECT_EQ(bytes_written_ecc_signature, 64u);
}

} // namespace
} // namespace iotctest
