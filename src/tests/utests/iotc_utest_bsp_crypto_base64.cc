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

namespace iotctest{
namespace {

TEST(IotcBspCryptoBase64, EnoughOutputBufferExpectedSize){
	const char* src_string =
			"not big enough output buffer for base64 encoded string";
	const size_t b64_actual_size = 10;
	unsigned char b64_actual[b64_actual_size];
	size_t bytes_written = 0;
	const iotc_bsp_crypto_state_t ret = iotc_bsp_base64_encode_urlsafe(
			b64_actual, b64_actual_size, &bytes_written, (uint8_t*)src_string, strlen(src_string));
	EXPECT_EQ(ret, IOTC_BSP_CRYPTO_BUFFER_TOO_SMALL_ERROR);
	//EXPECT_EQ(ret, IOTC_BSP_CRYPTO_BUFFER_TOO_SMALL_ERROR);
	// The math below calculates the required base 64 encoded buffer size
  	// including trailing zero.
  	// 1) (x + 2) / 3 equals to ceil(x / 3)
  	// 2) * 4 is there since base64 generates 4 bytes from each 3 byte by
  	// definition
  	// 3) + 1 is the trailing zero
	EXPECT_EQ((strlen(src_string) + 2) / 3 * 4 + 1, bytes_written);
}

TEST(IotcBspCryptoBase64, EnoughOutputBuffer){
	const char* src_string = 
			"not big enough output buffer for base64 encoded string";
	const size_t b64_actual_size = 10;
	unsigned char b64_actual[b64_actual_size];
	size_t bytes_written = 0;
	const iotc_bsp_crypto_state_t ret = iotc_bsp_base64_encode_urlsafe(
			b64_actual, b64_actual_size, &bytes_written, (uint8_t*)src_string, strlen(src_string));
	EXPECT_EQ(ret, IOTC_BSP_CRYPTO_BUFFER_TOO_SMALL_ERROR);
}

TEST(IotcBspCryptoBase64, EncodingFromSrc){
	const char* src_string =
			"first unit test string to base64 encode";
	// The math below calculates the required base 64 encoded buffer size
  	// including trailing zero.
  	// 1) (x + 2) / 3 equals to ceil(x / 3)
  	// 2) * 4 is there since base64 generates 4 bytes from each 3 byte by
  	// definition
  	// 3) + 1 is the trailing zero
	const size_t b64_actual_size = 
			((strlen(src_string) + 2) / 3) * 4 + 1;
	unsigned char b64_actual[b64_actual_size];
	size_t bytes_written = 0;
	const iotc_bsp_crypto_state_t ret = iotc_bsp_base64_encode_urlsafe(
			b64_actual, b64_actual_size, &bytes_written, (uint8_t*)src_string, strlen(src_string));
	const char* b64_expected = 
			"Zmlyc3QgdW5pdCB0ZXN0IHN0cmluZyB0byBiYXNlNjQgZW5jb2Rl";
	EXPECT_STREQ(b64_expected, (const char*)b64_actual);
	EXPECT_EQ(strlen(b64_expected), bytes_written);
	EXPECT_EQ(ret, IOTC_BSP_CRYPTO_STATE_OK);
}

TEST(IotcBspCryptoBase64, EnoughOutputBufferAdvisedSize){
	const char* strings_to_encode[] = 
			{"12345", "123456", "1234567", "12345678"};
	const char* base64_encoded_strings[] = 
			{"MTIzNDU=", "MTIzNDU2", "MTIzNDU2Nw==", "MTIzNDU2Nzg="};
	uint8_t i = 0;
	iotc_bsp_crypto_state_t ret;
	for(;i < 4; ++i){
		const char* src_string = strings_to_encode[i];
		const size_t b64_actual_size = 1;
		unsigned char b64_actual[128];
		size_t bytes_written = 0;
		ret = iotc_bsp_base64_encode_urlsafe(
			   	b64_actual, b64_actual_size, &bytes_written, (uint8_t*)src_string, strlen(src_string));
		EXPECT_EQ(ret, IOTC_BSP_CRYPTO_BUFFER_TOO_SMALL_ERROR);
		ret = iotc_bsp_base64_encode_urlsafe(
			   	b64_actual, bytes_written, &bytes_written, (uint8_t*)src_string, strlen(src_string));
		EXPECT_STREQ(base64_encoded_strings[i], (const char*)b64_actual);
	}
}

TEST(IotcBspCryptoBase64, ZeroTerminatedString){
	const char* src_string = 
			"text to base64 encode";
	const size_t b64_actual_size = 100;
	unsigned char b64_actual[b64_actual_size];
	size_t bytes_written = 0;

	// fill dst_string with non-zero values to check if base64 impl
     // puts zero to the end of base64 encoded string
	size_t i = 0;
	for(; i < b64_actual_size; ++i){
		b64_actual[i] = 'x';
	}
	const iotc_bsp_crypto_state_t ret = iotc_bsp_base64_encode_urlsafe(
			b64_actual, b64_actual_size, &bytes_written, (uint8_t*)src_string, strlen(src_string));
	const char* b64_expected = 
			"dGV4dCB0byBiYXNlNjQgZW5jb2Rl";
	EXPECT_STREQ(b64_expected, (const char*)b64_actual);
	EXPECT_EQ(strlen(b64_expected), bytes_written);
	EXPECT_EQ(ret, IOTC_BSP_CRYPTO_STATE_OK);
}

TEST(IotcBspCryptoBase64, EncodingToDest){
	const char* src_string = "\x18#\xc8\xa1?\xbf";
	// The math below calculates the required base 64 encoded buffer size
  	// including trailing zero.
  	// 1) (x + 2) / 3 equals to ceil(x / 3)
  	// 2) * 4 is there since base64 generates 4 bytes from each 3 byte by
  	// definition
  	// 3) + 1 is the trailing zero
	const size_t b64_actual_size = 
			((strlen(src_string) + 2) / 3) * 4 + 1;
	unsigned char b64_actual[b64_actual_size];
	size_t bytes_written = 0;
	const iotc_bsp_crypto_state_t ret = iotc_bsp_base64_encode_urlsafe(
			b64_actual, b64_actual_size, &bytes_written, (uint8_t*)src_string, strlen(src_string));
	const char* b64_expected = "GCPIoT-_";
	EXPECT_STREQ(b64_expected, (const char*)b64_actual);
	EXPECT_EQ(strlen(b64_expected), bytes_written);
	EXPECT_EQ(ret, IOTC_BSP_CRYPTO_STATE_OK);
}
}	// namespace
}	// namespace iotctest