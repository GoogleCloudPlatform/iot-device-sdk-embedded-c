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

#include "gmock.h"
#include "gtest.h"

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

 protected:
  iotc_crypto_private_key_data_t private_key_;
};

TEST_F(IotcJwt, JwtCreateReturnsSmallBufferError) {
  unsigned char jwt_buffer[IOTC_JWT_SIZE] = {0};
  size_t bytes_written = 0;
  EXPECT_EQ(iotc_create_jwt_es256("projectID", /*expiration_period_sec=*/600,
                                  &private_key_, jwt_buffer,
                                  /*dst_jwt_buf_len=*/1, &bytes_written),
            IOTC_BUFFER_TOO_SMALL_ERROR);
}

TEST_F(IotcJwt, JwtStringConsistsOfThreeDotSeparatedStrings) {
  unsigned char jwt_buffer[IOTC_JWT_SIZE] = {0};
  size_t bytes_written = 0;
  ASSERT_EQ(iotc_create_jwt_es256("projectID", /*expiration_period_sec=*/600,
                                  &private_key_, jwt_buffer, IOTC_JWT_SIZE,
                                  &bytes_written),
            IOTC_STATE_OK);

  std::string jwt(reinterpret_cast<char*>(jwt_buffer), bytes_written);
  EXPECT_THAT(jwt, ::testing::MatchesRegex(R"(^[^.]+\.[^.]+\.[^.]+)"));
}

TEST_F(IotcJwt, JwtCreateReturnsProjectIdTooLongError) {
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

}  // namespace
}  // namespace iotctest
