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

#include "gmock.h"
#include "gtest.h"

#include <openssl/bio.h>
#include <openssl/ec.h>
#include <openssl/evp.h>
#include "iotc.h"
#include "iotc_heapcheck_test.h"
#include "iotc_jwt.h"

namespace iotctest {
namespace {

constexpr char kPemKey[] =
    "\
-----BEGIN EC PRIVATE KEY-----\n\
MHcCAQEEINg6KhkJ2297KYO4eyLTPtVIhLloIfp3IsJo9n6KqelfoAoGCCqGSM49\n\
AwEHoUQDQgAE1Oi16oAc/+s5P5g2pzt3IDXfUBBUKUBrB8vgfyKOFb7sQTx4topE\n\
E0KOix7rJyli6tiAJJDL4lbdf0YRo45THQ==\n\
-----END EC PRIVATE KEY-----";

class IotcJwt : public IotcHeapCheckTest {
 public:
  IotcJwt() {
    iotc_initialize();
    private_key_.private_key_signature_algorithm =
        IOTC_JWT_PRIVATE_KEY_SIGNATURE_ALGORITHM_ES256,
    private_key_.private_key_union_type = IOTC_CRYPTO_KEY_UNION_TYPE_PEM,
    private_key_.private_key_union.key_pem.key = const_cast<char*>(kPemKey);
  }
  ~IotcJwt() { iotc_shutdown(); }

  std::string base64_decode_openssl(const std::string& base64_string) {
    BIO* input = BIO_new_mem_buf(
        static_cast<void*>(const_cast<char*>(base64_string.c_str())),
        /*len=*/-1);
    BIO* b64 = BIO_new(BIO_f_base64());
    BIO* bio = BIO_push(b64, input);

    char decoded[IOTC_JWT_SIZE] = {0};
    BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL);
    BIO_read(bio, decoded, base64_string.size());
    BIO_free_all(bio);

    return std::string(decoded, strlen(decoded));
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
      jwt.substr(first_dot + 1, second_dot - first_dot);
  const std::string third_section =
      jwt.substr(second_dot + 1, jwt.size() - second_dot);

  EXPECT_EQ(R"({"alg":"ES256","typ":"JWT"})",
            base64_decode_openssl(first_section));

  const auto second_section_decoded = base64_decode_openssl(second_section);
  EXPECT_THAT(second_section_decoded,
              ::testing::MatchesRegex(
                  R"(^\{"iat":[0-9]+,"exp":[0-9]+,"aud":"projectID"\}$)"));

  uint32_t iat_time, exp_time;
  sscanf(second_section_decoded.c_str(), R"({"iat":%d,"exp":%d")", &iat_time,
         &exp_time);
  EXPECT_EQ(exp_time, iat_time + expiration_period_sec);
}

}  // namespace
}  // namespace iotctest
