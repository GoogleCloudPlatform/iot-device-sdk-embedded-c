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

#include <cstring>
#include <iostream>

#include "gtest.h"
#include "iotc.h"
#include "iotc_bsp_crypto.h"
#include "iotc_heapcheck_test.h"
#include "iotc_helpers.h"
#include "iotc_jwt.h"
#include "iotc_macros.h"
#include "iotc_types.h"
#include <openssl/bio.h>
#include <openssl/ec.h>
#include <openssl/ecdsa.h>
#include <openssl/err.h>
#include <openssl/pem.h>

namespace iotctest {
namespace {

constexpr char kPemKey[] = "\
-----BEGIN PUBLIC KEY-----\n\
MFkwEwYHKoZIzj0CAQYIKoZIzj0DAQcDQgAE1Oi16oAc/+s5P5g2pzt3IDXfUBBU\n\
KUBrB8vgfyKOFb7sQTx4topEE0KOix7rJyli6tiAJJDL4lbdf0YRo45THQ==\n\
-----END PUBLIC KEY-----";

constexpr char kPrivateKey[] = "\
-----BEGIN EC PRIVATE KEY-----\n\
MHcCAQEEINg6KhkJ2297KYO4eyLTPtVIhLloIfp3IsJo9n6KqelfoAoGCCqGSM49\n\
AwEHoUQDQgAE1Oi16oAc/+s5P5g2pzt3IDXfUBBUKUBrB8vgfyKOFb7sQTx4topE\n\
E0KOix7rJyli6tiAJJDL4lbdf0YRo45THQ==\n\
-----END EC PRIVATE KEY-----";

class IotcBspCryptoEcc : public IotcHeapCheckTest {
  public:
    IotcBspCryptoEcc() {
      iotc_initialize();
      DefaultPrivateKey.private_key_signature_algorithm =
          IOTC_JWT_PRIVATE_KEY_SIGNATURE_ALGORITHM_ES256,
      DefaultPrivateKey.private_key_union_type = IOTC_CRYPTO_KEY_UNION_TYPE_PEM,
      DefaultPrivateKey.private_key_union.key_pem.key =
          const_cast<char*>(kPrivateKey);
    }
    ~IotcBspCryptoEcc() { iotc_shutdown(); }
    static constexpr uint8_t* kDefaultDataToSign =
        (uint8_t*)"this text is ecc signed";
    static constexpr size_t kDefaultDataToSignLength =
        strlen((char*)kDefaultDataToSign);

  protected:
    iotc_crypto_private_key_data_t DefaultPrivateKey;
    static int ec_verify_openssl(const uint8_t* hash, size_t hash_len,
                                const uint8_t* sig, size_t sig_len,
                                const char* pub_key_pem);
};

/**
 * Verify an EC signature using OpenSSL.
 *
 * @param hash the hash that was signed.
 * @param hash_len length of the hash in bytes.
 * @param sig signature to verify, formated according to rfc7518 section 3.4.
 * @param sig_len length of the signature in bytes.
 * @param pub_key_pem public key in PEM format.
 * @returns 0 if the verification succeeds.
 */
int IotcBspCryptoEcc ::ec_verify_openssl(const uint8_t* hash, size_t hash_len,
                                         const uint8_t* sig, size_t sig_len,
                                         const char* pub_key_pem) {
  if (sig_len != 64) {
    iotc_debug_format("sig_len expected to be 64, was %d", sig_len);
    return -1;
  }

  int ret = -1;
  EC_KEY* ec_key_openssl = nullptr;
  ECDSA_SIG* sig_openssl = nullptr;

  // Parse the signature to OpenSSL representation
  constexpr int kIntLength = 32; // as per RFC7518 section-3.4
  BIGNUM* r = BN_bin2bn(sig, kIntLength, nullptr);
  BIGNUM* s = BN_bin2bn(sig + kIntLength, kIntLength, nullptr);
  sig_openssl = ECDSA_SIG_new();

  // Parse the public key to OpenSSL representation
  BIO* pub_key_pem_bio = BIO_new(BIO_s_mem());
  BIO_puts(pub_key_pem_bio, pub_key_pem);
  ec_key_openssl =
      PEM_read_bio_EC_PUBKEY(pub_key_pem_bio, nullptr, nullptr, nullptr);
  if (ec_key_openssl == nullptr) {
    goto cleanup;
  }

// 1.1.0+, see opensslv.h for versions
#if 0x10100000L <= OPENSSL_VERSION_NUMBER
  if (1 != ECDSA_SIG_set0(sig_openssl, r, s)) {
    goto cleanup;
  }
#else // Travis CI's 14.04 trusty supports openssl 1.0.1f
  sig_openssl->r = r;
  sig_openssl->s = s;
#endif

  if (1 == ECDSA_do_verify((const unsigned char*)hash, hash_len, sig_openssl,
                           ec_key_openssl)) {
    ret = 0; // Verify success
  }

cleanup:
  ERR_print_errors_fp(stderr);
  EC_KEY_free(ec_key_openssl);
  ECDSA_SIG_free(sig_openssl);
  BIO_free_all(pub_key_pem_bio);

  return ret;
}

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
  const iotc_crypto_private_key_data_t kInvalidKey = {
      IOTC_CRYPTO_KEY_UNION_TYPE_PEM, const_cast<char*>(kInvalid),
      IOTC_JWT_PRIVATE_KEY_SIGNATURE_ALGORITHM_ES256};
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
  EXPECT_EQ(0, ec_verify_openssl(hash_sha256, /*hash len=*/32, ecc_signature,
                                 bytes_written_ecc_signature, kPemKey));
  // Two 32-byte integers(r and s) build up a JWT ECC signature
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
  // Assure no everwriting happens.
  if (bytes_written_ecc_signature < sizeof(ecc_signature)) {
    ASSERT_EQ('x', ecc_signature[bytes_written_ecc_signature]);
  }
  EXPECT_EQ(ec_verify_openssl((uint8_t*)kSimpleText, strlen(kSimpleText),
                              ecc_signature, bytes_written_ecc_signature,
                              kPemKey),
            0);
  EXPECT_EQ(bytes_written_ecc_signature, 64u);
}

} // namespace
} // namespace iotctest