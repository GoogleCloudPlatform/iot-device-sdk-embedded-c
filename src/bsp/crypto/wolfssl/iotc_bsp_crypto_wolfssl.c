/* Copyright 2018-2019 Google LLC
 *
 * This is part of the Google Cloud IoT Device SDK for Embedded C.
 * It is licensed under the BSD 3-Clause license; you may not use this file
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

#include "wolfssl/wolfcrypt/coding.h"
#include "wolfssl/wolfcrypt/error-crypt.h"
#include "wolfssl/wolfcrypt/sha256.h"

#include "wolfssl/ssl.h"
#include "wolfssl/wolfcrypt/asn.h"
#include "wolfssl/wolfcrypt/ecc.h"
#include "wolfssl/wolfcrypt/logging.h"

#include <stdio.h>

static iotc_bsp_crypto_state_t _iotc_bsp_base64_encode(
    unsigned char* dst_string, size_t dst_string_size, size_t* bytes_written,
    const uint8_t* src_buf, size_t src_buf_size) {
  size_t dst_string_size_orig = dst_string_size;

  int result =
      Base64_Encode_NoNl(src_buf, src_buf_size, NULL, (word32*)bytes_written);

  IOTC_CHECK_CND(result != LENGTH_ONLY_E, result, result);

  ++*bytes_written;  // output is a null terminated string. wolfssl's output
                     // is a non-zero terminated binary, so add one extra byte
                     // for the trailing zero

  if (*bytes_written <= dst_string_size) {
    result = Base64_Encode_NoNl(src_buf, src_buf_size, dst_string,
                                (word32*)&dst_string_size);
    IOTC_CHECK_STATE(result);

    // note: wolfssl doesn't write string but binary, so terminating zero
    // is written here
    *bytes_written = dst_string_size;
    if (*bytes_written < dst_string_size_orig) {
      dst_string[*bytes_written] = 0;
    }
  } else {
    return IOTC_BSP_CRYPTO_BUFFER_TOO_SMALL_ERROR;
  }

err_handling:

  switch (result) {
    case 0:
      return IOTC_BSP_CRYPTO_STATE_OK;
    case BAD_FUNC_ARG:
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

  // Translate to url-safe alphabet
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

iotc_bsp_crypto_state_t iotc_bsp_sha256(uint8_t* dst_buf_32_bytes,
                                        const uint8_t* src_buf,
                                        uint32_t src_buf_size) {
  if (NULL == dst_buf_32_bytes) {
    return IOTC_BSP_CRYPTO_INVALID_INPUT_PARAMETER_ERROR;
  }

  Sha256 sha_ctx;

  IOTC_CHECK_STATE(wc_InitSha256(&sha_ctx));
  IOTC_CHECK_STATE(wc_Sha256Update(&sha_ctx, src_buf, src_buf_size));
  IOTC_CHECK_STATE(wc_Sha256Final(&sha_ctx, dst_buf_32_bytes));

  return IOTC_BSP_CRYPTO_STATE_OK;

err_handling:

  return IOTC_BSP_CRYPTO_SHA256_ERROR;
}

iotc_bsp_crypto_state_t iotc_bsp_ecc(
    const iotc_crypto_key_data_t* private_key_data, uint8_t* dst_buf,
    size_t dst_buf_size, size_t* bytes_written, const uint8_t* src_buf,
    size_t src_buf_len) {
  // reusing wolfcrypt_rng from BSP_RNG module
  extern WC_RNG wolfcrypt_rng;

  if (NULL == private_key_data || NULL == dst_buf || NULL == bytes_written ||
      NULL == src_buf) {
    return IOTC_BSP_CRYPTO_INVALID_INPUT_PARAMETER_ERROR;
  }

  /* wolfSSL requires a PEM format private key */
  if (IOTC_CRYPTO_KEY_UNION_TYPE_PEM !=
          private_key_data->crypto_key_union_type ||
      NULL == private_key_data->crypto_key_union.key_pem.key) {
    return IOTC_BSP_CRYPTO_INVALID_INPUT_PARAMETER_ERROR;
  }

  const char* private_key_pem = private_key_data->crypto_key_union.key_pem.key;

  int ret = 0;

  DerBuffer* pDer = NULL;
  ret = PemToDer((unsigned char*)private_key_pem, strlen(private_key_pem),
                 ECC_PRIVATEKEY_TYPE, &pDer, NULL, NULL, NULL);

  IOTC_CHECK_STATE(ret);

  ecc_key ecc_key_private;
  wc_ecc_init(&ecc_key_private);

  word32 in_out_idx = 0;
  ret = wc_EccPrivateKeyDecode(pDer->buffer, &in_out_idx, &ecc_key_private,
                               pDer->length);
  IOTC_CHECK_STATE(ret);

  mp_int r;
  ret = mp_init(&r);
  IOTC_CHECK_STATE(ret);

  mp_int s;
  ret = mp_init(&s);
  IOTC_CHECK_STATE(ret);

  ret = wc_ecc_sign_hash_ex((const byte*)src_buf, src_buf_len, &wolfcrypt_rng,
                            &ecc_key_private, &r, &s);
  IOTC_CHECK_STATE(ret);

  // two 32 byte integers build up a JWT ECC signature: r and s
  // see https://tools.ietf.org/html/rfc7518#section-3.4
  const size_t integer_size = 32;
  *bytes_written = 2 * integer_size;

  IOTC_CHECK_CND(dst_buf_size < *bytes_written, BUFFER_E, ret);

  memset(dst_buf, 0, *bytes_written);

  const size_t padding_size_r = integer_size - mp_unsigned_bin_size(&r);
  ret = mp_to_unsigned_bin(&r, dst_buf + padding_size_r);
  IOTC_CHECK_STATE(ret);
  const size_t padding_size_s = integer_size - mp_unsigned_bin_size(&s);
  ret = mp_to_unsigned_bin(&s, dst_buf + integer_size + padding_size_s);
  IOTC_CHECK_STATE(ret);

err_handling:

  FreeDer(&pDer);
  wc_ecc_free(&ecc_key_private);
  mp_free(&r);
  mp_free(&s);

  switch (ret) {
    case BUFFER_E:
      return IOTC_BSP_CRYPTO_BUFFER_TOO_SMALL_ERROR;
    case ASN_NO_PEM_HEADER:
      return IOTC_BSP_CRYPTO_KEY_PARSE_ERROR;
    default:
      return IOTC_BSP_CRYPTO_STATE_OK;
  }

  return IOTC_BSP_CRYPTO_STATE_OK;
}
