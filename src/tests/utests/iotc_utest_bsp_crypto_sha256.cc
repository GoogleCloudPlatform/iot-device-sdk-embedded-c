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

extern "C" {
#include "iotc_bsp_crypto.h"
}

namespace iotctest {
namespace {

using ::testing::ElementsAreArray;

#define IOTC_SHA256_HASH_SIZE (256 / 8)


TEST(IotcBspCryptoSha256, OutputBufferIsNull) {
    const iotc_bsp_crypto_state_t ret = iotc_bsp_sha256(NULL, (uint8_t*)"abc", 3);
    EXPECT_EQ(ret, IOTC_BSP_CRYPTO_INVALID_INPUT_PARAMETER_ERROR);
}

TEST(IotcBspCryptoSha256, EmptyInputString) {
    uint8_t sha256_actual[IOTC_SHA256_HASH_SIZE] = {0};
    const uint8_t sha256_expected[IOTC_SHA256_HASH_SIZE] = {
          0xe3, 0xb0, 0xc4, 0x42, 0x98, 0xfc, 0x1c, 0x14, 0x9a, 0xfb, 0xf4,
          0xc8, 0x99, 0x6f, 0xb9, 0x24, 0x27, 0xae, 0x41, 0xe4, 0x64, 0x9b,
          0x93, 0x4c, 0xa4, 0x95, 0x99, 0x1b, 0x78, 0x52, 0xb8, 0x55};

    const iotc_bsp_crypto_state_t ret = iotc_bsp_sha256(sha256_actual, (uint8_t*)NULL, 0);
    EXPECT_EQ(ret, IOTC_BSP_CRYPTO_STATE_OK);
    EXPECT_THAT(sha256_actual, ElementsAreArray(sha256_expected));
}


}  // namespace
}  // namespace iotctest
