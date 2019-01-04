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

#include "iotc_openssl_utils.h"

#include <openssl/bio.h>
#include <openssl/ec.h>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/sha.h>

#include <algorithm>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <sstream>

namespace iotctest {
namespace openssl {

bool ecc_is_valid(const unsigned char* hash, size_t hash_size,
                  const uint8_t* signature, size_t signature_size,
                  const char* public_key_pem) {
  if (signature_size != 64) {
    return false;
  }

  EC_KEY* ec_key_openssl = nullptr;
  ECDSA_SIG* sig_openssl = nullptr;
  BIO* pub_key_pem_bio = nullptr;

  auto cleanup = [&ec_key_openssl, &sig_openssl, &pub_key_pem_bio]() {
    ERR_print_errors_fp(stderr);
    EC_KEY_free(ec_key_openssl);
    ECDSA_SIG_free(sig_openssl);
    BIO_free_all(pub_key_pem_bio);
  };

  // Parse the public key to OpenSSL representation.
  pub_key_pem_bio = BIO_new(BIO_s_mem());
  BIO_puts(pub_key_pem_bio, public_key_pem);
  ec_key_openssl =
      PEM_read_bio_EC_PUBKEY(pub_key_pem_bio, nullptr, nullptr, nullptr);
  if (ec_key_openssl == nullptr) {
    cleanup();
    return false;
  }

  // Parse the signature to OpenSSL representation.
  const int int_len = 32;  // as per RFC7518 section-3.4
  BIGNUM* r = BN_bin2bn(signature, int_len, nullptr);
  BIGNUM* s = BN_bin2bn(signature + int_len, int_len, nullptr);
  sig_openssl = ECDSA_SIG_new();

// 1.1.0+, see opensslv.h for versions
#if 0x10100000L <= OPENSSL_VERSION_NUMBER
  if (1 != ECDSA_SIG_set0(sig_openssl, r, s)) {
    cleanup();
    return false;
  }
#else  // Travis CI's 14.04 trusty supports openssl 1.0.1f
  sig_openssl->r = r;
  sig_openssl->s = s;
#endif
  if (ECDSA_do_verify(hash, hash_size, sig_openssl, ec_key_openssl) != 1) {
    cleanup();
    return false;
  }

  cleanup();
  return true;
}

void sha256(unsigned char* hash, const std::string& input) {
  SHA256_CTX sha256;
  SHA256_Init(&sha256);
  SHA256_Update(&sha256, input.c_str(), input.length());
  SHA256_Final(hash, &sha256);
}

}  // namespace openssl
}  // namespace iotctest
