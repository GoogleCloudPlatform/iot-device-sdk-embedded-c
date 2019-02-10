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

#ifndef __IOTC_OPENSSL_UTILS_H__
#define __IOTC_OPENSSL_UTILS_H__

#include <string>

namespace iotctest {
namespace openssl {

// Verifies the ECC signature against the hash and the public key.
// Returns true, if the signature is correct.
bool ecc_is_valid(const unsigned char* hash, size_t hash_size,
                  const uint8_t* signature, size_t signature_size,
                  const char* public_key_pem);

// Computes the SHA256 hash of the input string. Hash must be a buffer of at
// least 32 bytes length, preallocated.
void sha256(unsigned char* hash, const std::string& input);

// Decodes a base64 encoded string from 'src_buf' to 'dst_buf'. Returns true
// on success.
// If 'dst_buf' doesn't have enough space, returns false and indicates the
// required size in 'bytes_written'.
bool base64_decode(uint8_t* dst_buf, size_t dst_buf_size, size_t* bytes_written,
                   const unsigned char* src_buf, size_t src_buf_size);

}  // namespace openssl
}  // namespace iotctest

#endif  // __IOTC_OPENSSL_UTILS_H__
