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

#include "iotc_tt_testcase_management.h"
#include "iotc_utest_basic_testcase_frame.h"
#include "tinytest.h"
#include "tinytest_macros.h"

#include "iotc_bsp_crypto.h"
#include "iotc_helpers.h"
#include "iotc_jwt.h"
#include "iotc_macros.h"

#include <openssl/bio.h>
#include <openssl/ec.h>
#include <openssl/ecdsa.h>
#include <openssl/err.h>
#include <openssl/pem.h>
#include <stdio.h>

#ifndef IOTC_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN

static const iotc_crypto_private_key_data_t DEFAULT_PRIVATE_KEY = {
    .private_key_signature_algorithm =
        IOTC_JWT_PRIVATE_KEY_SIGNATURE_ALGORITHM_ES256,
    .private_key_union_type = IOTC_CRYPTO_KEY_UNION_TYPE_PEM,
    .private_key_union.key_pem.key = "\
-----BEGIN EC PRIVATE KEY-----\n\
MHcCAQEEINg6KhkJ2297KYO4eyLTPtVIhLloIfp3IsJo9n6KqelfoAoGCCqGSM49\n\
AwEHoUQDQgAE1Oi16oAc/+s5P5g2pzt3IDXfUBBUKUBrB8vgfyKOFb7sQTx4topE\n\
E0KOix7rJyli6tiAJJDL4lbdf0YRo45THQ==\n\
-----END EC PRIVATE KEY-----"};

static const char default_public_key_pem[] = "\
-----BEGIN PUBLIC KEY-----\n\
MFkwEwYHKoZIzj0CAQYIKoZIzj0DAQcDQgAE1Oi16oAc/+s5P5g2pzt3IDXfUBBU\n\
KUBrB8vgfyKOFb7sQTx4topEE0KOix7rJyli6tiAJJDL4lbdf0YRo45THQ==\n\
-----END PUBLIC KEY-----";

static const uint8_t* default_data_to_sign =
    (const uint8_t*)"this text is ecc signed";
static size_t default_data_to_sign_len = 0;

void iotc_test_print_buffer(const uint8_t* buf, size_t len) {
  size_t i = 0;
  for (; i < len; ++i) {
    printf("%.2x", buf[i]);
  }
  printf("\nlen: %lu\n", len);
}

void* iotc_utest_setup_ecc(const struct testcase_t* testcase) {
  default_data_to_sign_len = strlen((char*)default_data_to_sign);
  return iotc_utest_setup_basic(testcase);
}

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
static int ec_verify_openssl(const uint8_t* hash, size_t hash_len,
                             const uint8_t* sig, size_t sig_len,
                             const char* pub_key_pem) {
  if (sig_len != 64) {
    iotc_debug_format("sig_len expected to be 64, was %d", sig_len);
    return -1;
  }

  int ret = -1;
  EC_KEY* ec_key_openssl = NULL;
  ECDSA_SIG* sig_openssl = NULL;

  // Parse the public key to OpenSSL representation
  BIO* pub_key_pem_bio = BIO_new(BIO_s_mem());
  BIO_puts(pub_key_pem_bio, pub_key_pem);
  ec_key_openssl = PEM_read_bio_EC_PUBKEY(pub_key_pem_bio, NULL, NULL, NULL);
  if (ec_key_openssl == NULL) {
    goto cleanup;
  }

  // Parse the signature to OpenSSL representation
  const int int_len = 32; // as per RFC7518 section-3.4
  BIGNUM* r = BN_bin2bn(sig, int_len, NULL);
  BIGNUM* s = BN_bin2bn(sig + int_len, int_len, NULL);
  sig_openssl = ECDSA_SIG_new();

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

#endif

IOTC_TT_TESTGROUP_BEGIN(utest_bsp_crypto_ecc)

IOTC_TT_TESTCASE_WITH_SETUP(
    small_buffer__buffer_too_small_error_returned, iotc_utest_setup_ecc,
    iotc_utest_teardown_basic, NULL, {
      const size_t ecc_signature_buf_len = 1;
      uint8_t ecc_signature[ecc_signature_buf_len];
      size_t bytes_written = 0;

      const iotc_bsp_crypto_state_t ecc_return_state = iotc_bsp_ecc(
          &DEFAULT_PRIVATE_KEY, ecc_signature, ecc_signature_buf_len,
          &bytes_written, default_data_to_sign, default_data_to_sign_len);

      tt_int_op(IOTC_BSP_CRYPTO_BUFFER_TOO_SMALL_ERROR, ==, ecc_return_state);

    end:;
    })

IOTC_TT_TESTCASE_WITH_SETUP(
    small_buffer__proper_min_buf_size_returned, iotc_utest_setup_ecc,
    iotc_utest_teardown_basic, NULL, {
      const size_t ecc_signature_buf_len = 1;
      uint8_t ecc_signature[ecc_signature_buf_len];
      uint8_t* ecc_signature_new = NULL;
      size_t bytes_written = 0;

      iotc_bsp_crypto_state_t ecc_return_state = iotc_bsp_ecc(
          &DEFAULT_PRIVATE_KEY, ecc_signature, ecc_signature_buf_len,
          &bytes_written, default_data_to_sign, default_data_to_sign_len);

      tt_int_op(IOTC_BSP_CRYPTO_BUFFER_TOO_SMALL_ERROR, ==, ecc_return_state);
      // two 32 byte integers build up a JWT ECC signature: r and s
      tt_int_op(64, ==, bytes_written);

      iotc_state_t state = IOTC_STATE_OK;
      const size_t ecc_signature_new_buf_len = bytes_written;

      // by definition, the expected JWT ECC signature size is 64 bytes
      // https://tools.ietf.org/html/rfc7518#section-3.4
      tt_int_op(64, ==, ecc_signature_new_buf_len);

      IOTC_ALLOC_BUFFER_AT(uint8_t, ecc_signature_new,
                           ecc_signature_new_buf_len, state);

      ecc_return_state = iotc_bsp_ecc(
          &DEFAULT_PRIVATE_KEY, ecc_signature_new, ecc_signature_new_buf_len,
          &bytes_written, default_data_to_sign, default_data_to_sign_len);

      tt_int_op(IOTC_BSP_CRYPTO_STATE_OK, ==, ecc_return_state);
      tt_int_op(bytes_written, ==, ecc_signature_new_buf_len);

    err_handling:
    end:;
      IOTC_SAFE_FREE(ecc_signature_new);
    })

IOTC_TT_TESTCASE_WITH_SETUP(
    bad_private_key__proper_error_handling, iotc_utest_setup_ecc,
    iotc_utest_teardown_basic, NULL, {
      const iotc_crypto_private_key_data_t invalid_key = {
          .private_key_signature_algorithm =
              IOTC_JWT_PRIVATE_KEY_SIGNATURE_ALGORITHM_ES256,
          .private_key_union_type = IOTC_CRYPTO_KEY_UNION_TYPE_PEM,
          .private_key_union.key_pem.key = "invalid key"};
      const size_t ecc_signature_buf_len = 128;
      uint8_t ecc_signature[ecc_signature_buf_len];
      size_t bytes_written = 0;

      const iotc_bsp_crypto_state_t ecc_return_state = iotc_bsp_ecc(
          &invalid_key, ecc_signature, ecc_signature_buf_len, &bytes_written,
          default_data_to_sign, default_data_to_sign_len);

      tt_int_op(IOTC_BSP_CRYPTO_KEY_PARSE_ERROR, ==, ecc_return_state);

    end:;
    })

IOTC_TT_TESTCASE_WITH_SETUP(
    output_buffer_is_null__proper_error_handling, iotc_utest_setup_ecc,
    iotc_utest_teardown_basic, NULL, {
      const size_t ecc_signature_buf_len = 128;
      size_t bytes_written = 0;

      const iotc_bsp_crypto_state_t ecc_return_state = iotc_bsp_ecc(
          &DEFAULT_PRIVATE_KEY, NULL, ecc_signature_buf_len, &bytes_written,
          default_data_to_sign, default_data_to_sign_len);

      tt_int_op(IOTC_BSP_CRYPTO_INVALID_INPUT_PARAMETER_ERROR, ==,
                ecc_return_state);

    end:;
    })

IOTC_TT_TESTCASE_WITH_SETUP(
    input_buffer_is_null__proper_error_handling, iotc_utest_setup_ecc,
    iotc_utest_teardown_basic, NULL, {
      const size_t ecc_signature_buf_len = 128;
      uint8_t ecc_signature[ecc_signature_buf_len];
      size_t bytes_written = 0;

      const iotc_bsp_crypto_state_t ecc_return_state = iotc_bsp_ecc(
          &DEFAULT_PRIVATE_KEY, ecc_signature, ecc_signature_buf_len,
          &bytes_written, NULL, default_data_to_sign_len);

      tt_int_op(IOTC_BSP_CRYPTO_INVALID_INPUT_PARAMETER_ERROR, ==,
                ecc_return_state);

    end:;
    })

IOTC_TT_TESTCASE_WITH_SETUP(
    happy_path_JWT_signature_validation_with_openssl, iotc_utest_setup_ecc,
    iotc_utest_teardown_basic, NULL, {
      const char* jwt_header_payload_b64 =
          "eyJhbGciOiJFUzI1NiIsInR5cCI6IkpXVCJ9."
          "eyJzdWIiOiIxMjM0NTY3ODkwIiwibmFtZSI6IkpvaG4gRG9lIiwiYWRtaW4iOnRydWUs"
          "ImlhdCI6MTUxNjIzOTAyMn0";
      uint8_t hash_sha256[32] = {0};
      iotc_bsp_sha256(hash_sha256, (const uint8_t*)jwt_header_payload_b64,
                      strlen(jwt_header_payload_b64));

      size_t bytes_written_ecc_signature = 0;
      uint8_t ecc_signature[IOTC_JWT_MAX_SIGNATURE_SIZE] = {0};

      size_t i = 0;
      for (; i < IOTC_JWT_MAX_SIGNATURE_SIZE; ++i) {
        ecc_signature[i] = 'x';
      }

      iotc_bsp_ecc(&DEFAULT_PRIVATE_KEY, ecc_signature,
                   IOTC_JWT_MAX_SIGNATURE_SIZE, &bytes_written_ecc_signature,
                   hash_sha256, /*hash_len=*/32);

      // assure no overwrite happens
      if (bytes_written_ecc_signature < sizeof(ecc_signature)) {
        tt_int_op('x', ==, ecc_signature[bytes_written_ecc_signature]);
      }

      const int openssl_verify_ret = ec_verify_openssl(
          hash_sha256, /*hash_len=*/32, ecc_signature,
          bytes_written_ecc_signature, default_public_key_pem);
      tt_int_op(0, ==, openssl_verify_ret);
      // two 32 byte integers build up a JWT ECC signature: r and s
      tt_int_op(64, ==, bytes_written_ecc_signature);

    end:;
    })

IOTC_TT_TESTCASE_WITH_SETUP(
    happy_path_simple_text_validation_with_openssl, iotc_utest_setup_ecc,
    iotc_utest_teardown_basic, NULL, {
      const char* simple_text =
          "hi, I am the sipmle text, please sign me Mr. Ecc!";

      size_t bytes_written_ecc_signature = 0;
      uint8_t ecc_signature[IOTC_JWT_MAX_SIGNATURE_SIZE] = {0};

      size_t i = 0;
      for (; i < IOTC_JWT_MAX_SIGNATURE_SIZE; ++i) {
        ecc_signature[i] = 'x';
      }

      iotc_bsp_ecc(&DEFAULT_PRIVATE_KEY, ecc_signature,
                   IOTC_JWT_MAX_SIGNATURE_SIZE, &bytes_written_ecc_signature,
                   (uint8_t*)simple_text, strlen(simple_text));

      // assure no overwrite happens
      if (bytes_written_ecc_signature < sizeof(ecc_signature)) {
        tt_int_op('x', ==, ecc_signature[bytes_written_ecc_signature]);
      }

      const int openssl_verify_ret = ec_verify_openssl(
          (uint8_t*)simple_text, strlen(simple_text), ecc_signature,
          bytes_written_ecc_signature, default_public_key_pem);

      tt_int_op(0, ==, openssl_verify_ret);
      // two 32 byte integers build up a JWT ECC signature: r and s
      tt_int_op(64, ==, bytes_written_ecc_signature);

    end:;
    })

IOTC_TT_TESTGROUP_END

#ifndef IOTC_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#define IOTC_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#include __FILE__
#undef IOTC_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#endif
