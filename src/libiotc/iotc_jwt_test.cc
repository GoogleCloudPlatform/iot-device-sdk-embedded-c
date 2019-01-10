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
#include <cstdlib>
#include <iostream>
#include "gmock.h"
#include "gtest.h"

#include "iotc.h"
#include "iotc_bsp_crypto.h"
#include "iotc_heapcheck_test.h"
#include "iotc_jwt.h"
#include "iotc_openssl_utils.h"

namespace iotctest {
namespace {

constexpr char kPrivateKey[] =
    "\
-----BEGIN EC PRIVATE KEY-----\n\
MHcCAQEEINg6KhkJ2297KYO4eyLTPtVIhLloIfp3IsJo9n6KqelfoAoGCCqGSM49\n\
AwEHoUQDQgAE1Oi16oAc/+s5P5g2pzt3IDXfUBBUKUBrB8vgfyKOFb7sQTx4topE\n\
E0KOix7rJyli6tiAJJDL4lbdf0YRo45THQ==\n\
-----END EC PRIVATE KEY-----";

constexpr char kPublicKey[] =
    "\
-----BEGIN PUBLIC KEY-----\n\
MFkwEwYHKoZIzj0CAQYIKoZIzj0DAQcDQgAE1Oi16oAc/+s5P5g2pzt3IDXfUBBU\n\
KUBrB8vgfyKOFb7sQTx4topEE0KOix7rJyli6tiAJJDL4lbdf0YRo45THQ==\n\
-----END PUBLIC KEY-----";

class IotcJwt : public IotcHeapCheckTest {
 public:
  IotcJwt() {
    iotc_initialize();
    private_key_.key_signature_algorithm =
        IOTC_JWT_PRIVATE_KEY_SIGNATURE_ALGORITHM_ES256,
    private_key_.private_key_union_type = IOTC_CRYPTO_KEY_UNION_TYPE_PEM,
    private_key_.private_key_union.key_pem.key = const_cast<char*>(kPrivateKey);
  }
  ~IotcJwt() { iotc_shutdown(); }

  std::string base64_decoded_as_string(const std::string& base64_str) {
    uint8_t decoded[IOTC_JWT_SIZE] = {0};
    size_t length;
    openssl::base64_decode(
        decoded, IOTC_JWT_SIZE, &length,
        reinterpret_cast<const unsigned char*>(base64_str.c_str()),
        base64_str.length());

    return std::string(reinterpret_cast<char*>(decoded), length);
  }

 protected:
  iotc_crypto_private_key_data_t private_key_;
};

TEST_F(IotcJwt, ES256JwtCreateReturnsSmallBufferError) {
  unsigned char jwt_buffer[IOTC_JWT_SIZE] = {0};
  size_t bytes_written = 0;
  EXPECT_EQ(iotc_create_jwt_es256("projectID", /*expiration_period_sec=*/600,
                                  &private_key_, jwt_buffer,
                                  /*dst_jwt_buf_len=*/1, &bytes_written),
            IOTC_BUFFER_TOO_SMALL_ERROR);
}

TEST_F(IotcJwt, ES256JwtStringConsistsOfThreeDotSeparatedStrings) {
  unsigned char jwt_buffer[IOTC_JWT_SIZE] = {0};
  size_t bytes_written = 0;
  ASSERT_EQ(iotc_create_jwt_es256("projectID", /*expiration_period_sec=*/600,
                                  &private_key_, jwt_buffer, IOTC_JWT_SIZE,
                                  &bytes_written),
            IOTC_STATE_OK);

  std::string jwt(reinterpret_cast<char*>(jwt_buffer), bytes_written);
  EXPECT_THAT(jwt, ::testing::MatchesRegex(R"(^[^.]+\.[^.]+\.[^.]+$)"));
}

TEST_F(IotcJwt, ES256JwtCreateReturnsProjectIdTooLongError) {
  const std::string kTooLongProjectId(IOTC_JWT_SIZE - 1, 'x');

  unsigned char jwt_buffer[IOTC_JWT_SIZE] = {0};
  size_t bytes_written = 0;
  EXPECT_EQ(iotc_create_jwt_es256(kTooLongProjectId.c_str(),
                                  /*expiration_period_sec=*/600, &private_key_,
                                  jwt_buffer, IOTC_JWT_SIZE, &bytes_written),
            IOTC_JWT_PROJECTID_TOO_LONG_ERROR);

  EXPECT_LT(bytes_written, (unsigned)IOTC_JWT_SIZE);

  const std::string kMaximumLengthProjectId(bytes_written, 'x');
  EXPECT_EQ(iotc_create_jwt_es256(kMaximumLengthProjectId.c_str(),
                                  /*expiration_period_sec=*/600, &private_key_,
                                  jwt_buffer, IOTC_JWT_SIZE, &bytes_written),
            IOTC_STATE_OK);
}

TEST_F(IotcJwt, ES256JwtCreateReturnsCorrectDecodableBase64Sections) {
  unsigned char jwt_buffer[IOTC_JWT_SIZE] = {0};
  size_t bytes_written = 0;
  const uint32_t expiration_period_sec = 1600;
  ASSERT_EQ(
      iotc_create_jwt_es256("projectID", expiration_period_sec, &private_key_,
                            jwt_buffer, IOTC_JWT_SIZE, &bytes_written),
      IOTC_STATE_OK);

  std::string jwt(reinterpret_cast<char*>(jwt_buffer), bytes_written);
  ASSERT_THAT(jwt, ::testing::MatchesRegex(R"(^[^.]+\.[^.]+\.[^.]+)"));

  const size_t first_dot = jwt.find_first_of('.');
  const size_t second_dot = jwt.find_last_of('.');

  const std::string first_section = jwt.substr(0, first_dot);
  const std::string second_section =
      jwt.substr(first_dot + 1, second_dot - first_dot - 1);
  const std::string third_section =
      jwt.substr(second_dot + 1, jwt.size() - second_dot);

  EXPECT_EQ(R"({"alg":"ES256","typ":"JWT"})",
            base64_decoded_as_string(first_section));

  const auto second_section_decoded = base64_decoded_as_string(second_section);
  EXPECT_THAT(second_section_decoded,
              ::testing::MatchesRegex(
                  R"(^\{"iat":[0-9]+,"exp":[0-9]+,"aud":"projectID"\}$)"));

  uint32_t iat_time, exp_time;
  sscanf(second_section_decoded.c_str(), R"({"iat":%d,"exp":%d")", &iat_time,
         &exp_time);
  EXPECT_EQ(exp_time, iat_time + expiration_period_sec);
}

TEST_F(IotcJwt, ES256JwtCreateReturnsCorrectES256) {
  unsigned char jwt_buffer[IOTC_JWT_SIZE] = {0};
  size_t bytes_written = 0;
  ASSERT_EQ(iotc_create_jwt_es256("projectID", /*expiration_period_sec=*/600,
                                  &private_key_, jwt_buffer, IOTC_JWT_SIZE,
                                  &bytes_written),
            IOTC_STATE_OK);
  std::string jwt(reinterpret_cast<char*>(jwt_buffer), bytes_written);

  const size_t second_dot = jwt.find_last_of('.');
  std::string third_section =
      jwt.substr(second_dot + 1, jwt.size() - second_dot);

  // We need to URL-unsafe the characters we might have changed when base64
  // encoding.
  std::replace(third_section.begin(), third_section.end(), '-', '+');
  std::replace(third_section.begin(), third_section.end(), '_', '/');

  // We need to SHA256 the "(first section).(second_section)", base64 decode
  // the third section (which is the ECC signature), then ECC verify with the
  // public key.
  uint8_t sha256[32] = {0};
  openssl::sha256(sha256, jwt.substr(0, second_dot));

  uint8_t ecc_signature[IOTC_JWT_SIZE] = {0};
  size_t ecc_signature_length;
  openssl::base64_decode(
      ecc_signature, IOTC_JWT_SIZE, &ecc_signature_length,
      reinterpret_cast<const unsigned char*>(third_section.c_str()),
      third_section.length());
  EXPECT_EQ(ecc_signature_length, 64u);

  EXPECT_TRUE(openssl::ecc_is_valid(sha256, 32, ecc_signature,
                                    ecc_signature_length, kPublicKey));
}

}  // namespace
}  // namespace iotctest
