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
#include "iotc_bsp_crypto.h"
#include "iotc_heapcheck_test.h"
#include "iotc_helpers.h"
#include "iotc_jwt.h"
#include "iotc_macros.h"
#include "iotc_types.h"

#include <cstring>
#include <iostream>
#include <openssl/bio.h>
#include <openssl/ec.h>
#include <openssl/ecdsa.h>
#include <openssl/err.h>
#include <openssl/pem.h>

namespace iotctest
{
namespace
{
static const uint8_t *default_data_to_sign =
		(const uint8_t *)"this text is ecc signed";
static size_t default_data_to_sign_len = strlen((char *)default_data_to_sign);

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

class IotcBspCryptoEcc : public IotcHeapCheckTest
{
public:
	IotcBspCryptoEcc()
	{
		iotc_initialize();
		DefaultPrivateKey.private_key_signature_algorithm =
				IOTC_JWT_PRIVATE_KEY_SIGNATURE_ALGORITHM_ES256,
		DefaultPrivateKey.private_key_union_type = IOTC_CRYPTO_KEY_UNION_TYPE_PEM,
		DefaultPrivateKey.private_key_union.key_pem.key =
				const_cast<char *>(kPrivateKey);
	}
	~IotcBspCryptoEcc() { iotc_shutdown(); }

protected:
	iotc_crypto_private_key_data_t DefaultPrivateKey;
	static int ec_verify_openssl(const uint8_t *hash, size_t hash_len,
															 const uint8_t *sig, size_t sig_len,
															 const char *pub_key_pem);
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
int IotcBspCryptoEcc ::ec_verify_openssl(const uint8_t *hash, size_t hash_len,
																				 const uint8_t *sig, size_t sig_len,
																				 const char *pub_key_pem)
{
	if (sig_len != 64)
	{
		iotc_debug_format("sig_len expected to be 64, was %d", sig_len);
		return -1;
	}

	int ret = -1;
	EC_KEY *ec_key_openssl = nullptr;
	ECDSA_SIG *sig_openssl = nullptr;

	// Parse the signature to OpenSSL representation
	const int int_len = 32; // as per RFC7518 section-3.4
	BIGNUM *r = BN_bin2bn(sig, int_len, nullptr);
	BIGNUM *s = BN_bin2bn(sig + int_len, int_len, nullptr);
	sig_openssl = ECDSA_SIG_new();

	// Parse the public key to OpenSSL representation
	BIO *pub_key_pem_bio = BIO_new(BIO_s_mem());
	BIO_puts(pub_key_pem_bio, pub_key_pem);
	ec_key_openssl =
			PEM_read_bio_EC_PUBKEY(pub_key_pem_bio, nullptr, nullptr, nullptr);
	if (ec_key_openssl == nullptr)
	{
		goto cleanup;
	}

// 1.1.0+, see opensslv.h for versions
#if 0x10100000L <= OPENSSL_VERSION_NUMBER
	if (1 != ECDSA_SIG_set0(sig_openssl, r, s))
	{
		goto cleanup;
	}
#else // Travis CI's 14.04 trusty supports openssl 1.0.1f
	sig_openssl->r = r;
	sig_openssl->s = s;
#endif

	if (1 == ECDSA_do_verify((const unsigned char *)hash, hash_len, sig_openssl,
													 ec_key_openssl))
	{
		ret = 0; // Verify success
	}

cleanup:
	ERR_print_errors_fp(stderr);
	EC_KEY_free(ec_key_openssl);
	ECDSA_SIG_free(sig_openssl);
	BIO_free_all(pub_key_pem_bio);

	return ret;
}

TEST_F(IotcBspCryptoEcc, SmallBuffer)
{
	const size_t ecc_signature_buf_len = 1;
	uint8_t ecc_signature[ecc_signature_buf_len];
	size_t bytes_written = 0;
	EXPECT_EQ(iotc_bsp_ecc(&DefaultPrivateKey, ecc_signature,
												 ecc_signature_buf_len, &bytes_written,
												 default_data_to_sign, default_data_to_sign_len),
						IOTC_BSP_CRYPTO_BUFFER_TOO_SMALL_ERROR);
}

TEST_F(IotcBspCryptoEcc, SmallBufferProperMinSize)
{
	const size_t ecc_signature_buf_len = 1;
	uint8_t ecc_signature[ecc_signature_buf_len];
	// uint8_t *ecc_signature_new = nullptr;
	uint8_t ecc_signature_new[64];
	size_t bytes_written = 0;

	EXPECT_EQ(iotc_bsp_ecc(&DefaultPrivateKey, ecc_signature,
												 ecc_signature_buf_len, &bytes_written,
												 default_data_to_sign, default_data_to_sign_len),
						IOTC_BSP_CRYPTO_BUFFER_TOO_SMALL_ERROR);
	// two 32 byte integers build up a JWT ECC signature: r and s
	EXPECT_EQ(64u, bytes_written);

	const size_t ecc_signature_new_buf_len = bytes_written;

	// by definition, the expected JWT ECC signature size is 64 bytes
	// https://tools.ietf.org/html/rfc7518#section-3.4
	ASSERT_EQ(64u, ecc_signature_new_buf_len);
	EXPECT_EQ(iotc_bsp_ecc(&DefaultPrivateKey, ecc_signature_new,
												 ecc_signature_new_buf_len, &bytes_written,
												 default_data_to_sign, default_data_to_sign_len),
						IOTC_BSP_CRYPTO_STATE_OK);
	EXPECT_EQ(bytes_written, ecc_signature_new_buf_len);
}

TEST_F(IotcBspCryptoEcc, ReportsErrorOnInvalidPrivateKey)
{
	constexpr char invalid[] = "invalid key";
	const iotc_crypto_private_key_data_t invalid_key = {
			IOTC_CRYPTO_KEY_UNION_TYPE_PEM, const_cast<char *>(invalid),
			IOTC_JWT_PRIVATE_KEY_SIGNATURE_ALGORITHM_ES256};
	const size_t ecc_signature_buf_len = 128;
	uint8_t ecc_signature[ecc_signature_buf_len];
	size_t bytes_written = 0;

	EXPECT_EQ(iotc_bsp_ecc(&invalid_key, ecc_signature, ecc_signature_buf_len,
												 &bytes_written, default_data_to_sign,
												 default_data_to_sign_len),
						IOTC_BSP_CRYPTO_KEY_PARSE_ERROR);
}

TEST_F(IotcBspCryptoEcc, ReportsErrorWhenOutputBufferIsNull)
{
	const size_t ecc_signature_buf_len = 128;
	size_t bytes_written = 0;

	EXPECT_EQ(iotc_bsp_ecc(&DefaultPrivateKey, nullptr, ecc_signature_buf_len,
												 &bytes_written, default_data_to_sign,
												 default_data_to_sign_len),
						IOTC_BSP_CRYPTO_INVALID_INPUT_PARAMETER_ERROR);
}

TEST_F(IotcBspCryptoEcc, ReportsErrorWhenInputBufferIsNull)
{
	const size_t ecc_signature_buf_len = 128;
	uint8_t ecc_signature[ecc_signature_buf_len];
	size_t bytes_written = 0;

	EXPECT_EQ(iotc_bsp_ecc(&DefaultPrivateKey, ecc_signature,
												 ecc_signature_buf_len, &bytes_written, nullptr,
												 default_data_to_sign_len),
						IOTC_BSP_CRYPTO_INVALID_INPUT_PARAMETER_ERROR);
}

TEST_F(IotcBspCryptoEcc, JwtSignatureValidation)
{
	const char *jwt_header_payload_b64 =
			"eyJhbGciOiJFUzI1NiIsInR5cCI6IkpXVCJ9."
			"eyJzdWIiOiIxMjM0NTY3ODkwIiwibmFtZSI6IkpvaG4gRG9lIiwiYWRtaW4iOnRydWUs"
			"ImlhdCI6MTUxNjIzOTAyMn0";
	uint8_t hash_sha256[32] = {0};
	iotc_bsp_sha256(hash_sha256, (const uint8_t *)jwt_header_payload_b64,
									strlen(jwt_header_payload_b64));
	size_t bytes_written_ecc_signature = 0;
	uint8_t ecc_signature[IOTC_JWT_MAX_SIGNATURE_SIZE] = {0};

	size_t i = 0;
	for (; i < IOTC_JWT_MAX_SIGNATURE_SIZE; ++i)
	{
		ecc_signature[i] = 'x';
	}

	iotc_bsp_ecc(&DefaultPrivateKey, ecc_signature, IOTC_JWT_MAX_SIGNATURE_SIZE,
							 &bytes_written_ecc_signature, hash_sha256, /*hash_len=*/32);

	// assure no overwrite happens
	if (bytes_written_ecc_signature < sizeof(ecc_signature))
	{
		ASSERT_EQ('x', ecc_signature[bytes_written_ecc_signature]);
	}
	EXPECT_EQ(0, ec_verify_openssl(hash_sha256, /*hash len=*/32, ecc_signature,
																 bytes_written_ecc_signature, kPemKey));
	// two 32 byte integers build up a JWT ECC signature: r and s
	EXPECT_EQ(64u, bytes_written_ecc_signature);
}

TEST_F(IotcBspCryptoEcc, SimpleTextValidation)
{
	const char *simple_text = "hi, I am the sipmle text, please sign me Mr. Ecc!";
	size_t bytes_written_ecc_signature = 0;
	uint8_t ecc_signature[IOTC_JWT_MAX_SIGNATURE_SIZE] = {0};

	size_t i = 0;
	for (; i < IOTC_JWT_MAX_SIGNATURE_SIZE; i++)
	{
		ecc_signature[i] = 'x';
	}
	iotc_bsp_ecc(&DefaultPrivateKey, ecc_signature, IOTC_JWT_MAX_SIGNATURE_SIZE,
							 &bytes_written_ecc_signature, (uint8_t *)simple_text,
							 strlen(simple_text));
	// assure no everwriting happens
	if (bytes_written_ecc_signature < sizeof(ecc_signature))
	{
		ASSERT_EQ('x', ecc_signature[bytes_written_ecc_signature]);
	}
	EXPECT_EQ(0, ec_verify_openssl((uint8_t *)simple_text, strlen(simple_text),
																 ecc_signature, bytes_written_ecc_signature,
																 kPemKey));
	EXPECT_EQ(64u, bytes_written_ecc_signature);
}

} // namespace
} // namespace iotctest