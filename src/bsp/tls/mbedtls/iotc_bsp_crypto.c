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

#include "iotc_bsp_crypto.h"
#include "iotc_bsp_mem.h"
#include "iotc_helpers.h"
#include "iotc_macros.h"

#include "mbedtls/base64.h"
#include "mbedtls/sha256.h"

#include "mbedtls/ctr_drbg.h"
#include "mbedtls/ecdsa.h"
#include "mbedtls/entropy.h"
#include "mbedtls/pk.h"

#include <stdio.h>

static iotc_bsp_crypto_state_t _iotc_bsp_base64_encode(
    unsigned char* dst_string, size_t dst_string_size, size_t* bytes_written,
    const uint8_t* src_buf, size_t src_buf_size) {
  const int result = mbedtls_base64_encode(
      dst_string, dst_string_size, bytes_written, src_buf, src_buf_size);
  switch (result) {
    case 0:
      return IOTC_BSP_CRYPTO_STATE_OK;

    case MBEDTLS_ERR_BASE64_BUFFER_TOO_SMALL:
      return IOTC_BSP_CRYPTO_BUFFER_TOO_SMALL_ERROR;

    default:
      return IOTC_BSP_CRYPTO_BASE64_ERROR;
  }
}

static iotc_bsp_crypto_state_t _iotc_bsp_base64_decode(
    uint8_t* dst_buf, size_t dst_buf_size, size_t* bytes_written,
    const unsigned char* src_buf, size_t src_buf_size) {
  const int result = mbedtls_base64_decode(dst_buf, dst_buf_size, bytes_written,
                                           src_buf, src_buf_size);
  switch (result) {
    case 0:
      return IOTC_BSP_CRYPTO_STATE_OK;

    case MBEDTLS_ERR_BASE64_BUFFER_TOO_SMALL:
      return IOTC_BSP_CRYPTO_BUFFER_TOO_SMALL_ERROR;

    default:
      return IOTC_BSP_CRYPTO_BASE64_ERROR;
  }
}

iotc_bsp_crypto_state_t iotc_bsp_base64_encode_urlsafe(
    unsigned char* dst_string, size_t dst_string_size, size_t* bytes_written,
    const uint8_t* src_buf, size_t src_buf_size) {
  const iotc_bsp_crypto_state_t b64_result = _iotc_bsp_base64_encode(
      dst_string, dst_string_size, bytes_written, src_buf, src_buf_size);

  if (b64_result != IOTC_BSP_CRYPTO_STATE_OK) {
    return b64_result;
  }

  // Translate to url-safe alphabet.
  size_t i = 0;
  for (; i < *bytes_written; i++) {
    switch (dst_string[i]) {
      case '+':
        dst_string[i] = '-';
        break;
      case '/':
        dst_string[i] = '_';
        break;
      default:
        break;
    }
  }

  return IOTC_BSP_CRYPTO_STATE_OK;
}

iotc_bsp_crypto_state_t iotc_bsp_base64_decode_urlsafe(
    uint8_t* dst_buf, size_t dst_buf_size, size_t* bytes_written,
    const unsigned char* src_buf, size_t src_buf_size) {
  unsigned char src_buf_copy[src_buf_size];

  // Translate from url-safe alphabet.
  for (size_t i = 0; i < src_buf_size; i++) {
    switch (src_buf[i]) {
      case '-':
        src_buf_copy[i] = '+';
        break;
      case '_':
        src_buf_copy[i] = '/';
        break;
      default:
        src_buf_copy[i] = src_buf[i];
        break;
    }
  }

  return _iotc_bsp_base64_decode(dst_buf, dst_buf_size, bytes_written,
                                 src_buf_copy, src_buf_size);
}

iotc_bsp_crypto_state_t iotc_bsp_sha256(uint8_t* dst_buf_32_bytes,
                                        const uint8_t* src_buf,
                                        uint32_t src_buf_size) {
  if (NULL == dst_buf_32_bytes) {
    return IOTC_BSP_CRYPTO_INVALID_INPUT_PARAMETER_ERROR;
  }

  mbedtls_sha256_context sha_ctx;
  mbedtls_sha256_init(&sha_ctx);

  IOTC_CHECK_STATE(mbedtls_sha256_starts_ret(&sha_ctx, 0));
  IOTC_CHECK_STATE(mbedtls_sha256_update_ret(&sha_ctx, src_buf, src_buf_size));
  IOTC_CHECK_STATE(mbedtls_sha256_finish_ret(&sha_ctx, dst_buf_32_bytes));

  return IOTC_BSP_CRYPTO_STATE_OK;

err_handling:

  return IOTC_BSP_CRYPTO_SHA256_ERROR;
}

#if 0  // TODO(atigyi): use this struct to channel necessary application
       //               parameter to ecc signature function. Insted of
       //               usage of two mutually excluive parameters:
       //               private_key_pem, jwt_signature_data
typedef enum {
  IOTC_BSP_CRYPTO_KEY_PEM = 0,
  IOTC_BSP_CRYPTO_KEY_SLOT_ID,
  IOTC_BSP_CRYPTO_KEY_CUSTOM,
} iotc_bsp_crypto_key_type_t;

typedef struct {
  iotc_bsp_crypto_key_type_t key_type;
  union {
    struct {
      const char* key;
    } key_pem;

    struct {
      uint8_t slot_id;
    } key_slot_id;

    struct {
      const void* data;
      size_t data_size;
    } key_custom;
  };
} iotc_bsp_crypto_key_t;
#endif

iotc_bsp_crypto_state_t iotc_bsp_ecc(
    const iotc_crypto_private_key_data_t* private_key_data, uint8_t* dst_buf,
    size_t dst_buf_size, size_t* bytes_written, const uint8_t* src_buf,
    size_t src_buf_len) {
  // reusing ctr_drbg from BSP_RNG module
  extern mbedtls_ctr_drbg_context ctr_drbg;

  if (NULL == private_key_data || NULL == dst_buf || NULL == bytes_written ||
      NULL == src_buf) {
    return IOTC_BSP_CRYPTO_INVALID_INPUT_PARAMETER_ERROR;
  }

  /* mbedTLS requires a PEM format private key */
  if (IOTC_CRYPTO_KEY_UNION_TYPE_PEM !=
          private_key_data->private_key_union_type ||
      NULL == private_key_data->private_key_union.key_pem.key) {
    return IOTC_BSP_CRYPTO_INVALID_INPUT_PARAMETER_ERROR;
  }

  const char* private_key_pem = private_key_data->private_key_union.key_pem.key;

  iotc_bsp_crypto_state_t return_code = IOTC_BSP_CRYPTO_STATE_OK;

  int mbedtls_ret = -1;

  mbedtls_mpi r, s;
  mbedtls_pk_context pk;
  mbedtls_ecdsa_context ecdsa_sign;

  mbedtls_mpi_init(&r);
  mbedtls_mpi_init(&s);
  mbedtls_pk_init(&pk);
  mbedtls_ecdsa_init(&ecdsa_sign);

  IOTC_CHECK_CND_DBGMESSAGE(
      (mbedtls_ret =
           mbedtls_pk_parse_key(&pk, (const unsigned char*)private_key_pem,
                                strlen(private_key_pem) + 1, NULL, 0)) != 0,
      IOTC_BSP_CRYPTO_KEY_PARSE_ERROR, return_code, "mbedtls_pk_parse_key");

  IOTC_CHECK_CND_DBGMESSAGE(
      (mbedtls_ret = mbedtls_ecdsa_from_keypair(&ecdsa_sign, pk.pk_ctx)) != 0,
      IOTC_BSP_CRYPTO_ECC_ERROR, return_code, "mbedtls_ecdsa_from_keypair");

#if 1  // non-deterministic ecc signature
  IOTC_CHECK_CND_DBGMESSAGE(
      (mbedtls_ret = mbedtls_ecdsa_sign(
           &ecdsa_sign.grp, &r, &s, &ecdsa_sign.d, src_buf, src_buf_len,
           mbedtls_ctr_drbg_random, &ctr_drbg)) != 0,
      IOTC_BSP_CRYPTO_ECC_ERROR, return_code, "mbedtls_ecdsa_sign");
#else  // deterministic ecc signature
  IOTC_CHECK_CND_DBGMESSAGE((mbedtls_ret = mbedtls_ecdsa_sign_det(
                                 &ecdsa_sign.grp, &r, &s, &ecdsa_sign.d,
                                 src_buf, src_buf_len, MBEDTLS_MD_SHA256)) != 0,
                            IOTC_BSP_CRYPTO_ECC_ERROR, return_code,
                            "mbedtls_ecdsa_sign_det");
#endif

  // two 32 byte integers build up a JWT ECC signature: r and s
  // see https://tools.ietf.org/html/rfc7518#section-3.4
  const size_t integer_size = 32;
  *bytes_written = 2 * integer_size;

  IOTC_CHECK_CND(dst_buf_size < *bytes_written,
                 IOTC_BSP_CRYPTO_BUFFER_TOO_SMALL_ERROR, return_code);

  IOTC_CHECK_CND_DBGMESSAGE(
      (mbedtls_ret = mbedtls_mpi_write_binary(&r, dst_buf, integer_size)) != 0,
      IOTC_BSP_CRYPTO_SERIALIZE_ERROR, return_code, "failed to write 'r'");

  IOTC_CHECK_CND_DBGMESSAGE(
      (mbedtls_ret = mbedtls_mpi_write_binary(&s, dst_buf + integer_size,
                                              integer_size)) != 0,
      IOTC_BSP_CRYPTO_SERIALIZE_ERROR, return_code, "failed to write 's'");

err_handling:
  if (0 != mbedtls_ret) {
    iotc_debug_format("mbedtls_ret: %d", mbedtls_ret);
  }

  mbedtls_mpi_free(&r);
  mbedtls_mpi_free(&s);

  mbedtls_ecdsa_free(&ecdsa_sign);
  mbedtls_pk_free(&pk);

  return return_code;
}
