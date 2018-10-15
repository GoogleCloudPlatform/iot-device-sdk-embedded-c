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

#include "iotc_jwt.h"
#include "iotc_bsp_crypto.h"
#include "iotc_bsp_time.h"
#include "iotc_macros.h"

#include <stdio.h>

#define IOTC_CHECK_CRYPTO(s)             \
  if ((s) != IOTC_BSP_CRYPTO_STATE_OK) { \
    goto err_handling;                   \
  }

#define IOTC_JWT_PROJECTID_MAX_LEN 200

/**
 * Creates the first two parts of the token: b64(header) + . + b64(payload)
 *
 * Helper function for iotc_create_jwt_es256.
 */
static iotc_bsp_crypto_state_t _iotc_create_jwt_b64h_b64p(
    unsigned char* dst_string, size_t dst_string_size, size_t* bytes_written,
    const char* project_id, uint32_t expiration_period_sec, const char* algo) {
  iotc_bsp_crypto_state_t ret = IOTC_BSP_CRYPTO_ERROR;
  size_t bytes_written_payload = 0;

  // create header
  char header[IOTC_JWT_HEADER_BUF_SIZE] = {0};
  snprintf(header, IOTC_JWT_HEADER_BUF_SIZE, "{\"alg\":\"%s\",\"typ\":\"JWT\"}",
           algo);

  // create payload
  const iotc_time_t current_time_in_sec =
      iotc_bsp_time_getcurrenttime_seconds();

  char payload[IOTC_JWT_PAYLOAD_BUF_SIZE] = {0};
  snprintf(payload, IOTC_JWT_PAYLOAD_BUF_SIZE,
           "{\"iat\":%lu,\"exp\":%lu,\"aud\":\"%s\"}", current_time_in_sec,
           current_time_in_sec + expiration_period_sec, project_id);

  // base64 encode, header
  *bytes_written = 0;
  IOTC_CHECK_CRYPTO(ret = iotc_bsp_base64_encode_urlsafe(
                        dst_string, dst_string_size, bytes_written,
                        (const uint8_t*)header, strlen(header)));

  IOTC_CHECK_CND(*bytes_written >= dst_string_size,
                 IOTC_BSP_CRYPTO_BUFFER_TOO_SMALL_ERROR, ret);

  // add first dot, separating b64(h) and b64(p)
  dst_string[*bytes_written] = '.';
  ++*bytes_written;

  // base64 encode, payload
  IOTC_CHECK_CRYPTO(
      ret = iotc_bsp_base64_encode_urlsafe(
          dst_string + *bytes_written, dst_string_size - *bytes_written,
          &bytes_written_payload, (const uint8_t*)payload, strlen(payload)));

  *bytes_written += bytes_written_payload;
  return IOTC_BSP_CRYPTO_STATE_OK;

err_handling:
  *bytes_written += bytes_written_payload;
  return ret;
}

// create the JWT: b64(h).b64(p).b64(ecc(sha256(b64(h).b64(p))))
// h = header
// p = payload
// b64 = base64
// sha = Secure Hash Algorithm
// ecc = Elliptic Curve Cryptography
iotc_state_t iotc_create_jwt_es256(
    const char* project_id, uint32_t expiration_period_sec,
    const iotc_crypto_private_key_data_t* private_key_data,
    unsigned char* dst_jwt_buf, size_t dst_jwt_buf_len, size_t* bytes_written) {
  if (IOTC_JWT_PROJECTID_MAX_LEN < strlen(project_id)) {
    *bytes_written = IOTC_JWT_PROJECTID_MAX_LEN;
    return IOTC_JWT_PROJECTID_TOO_LONG_ERROR;
  }

  iotc_bsp_crypto_state_t ret = IOTC_BSP_CRYPTO_ERROR;

  // create base64 encoded header and payload: b64(h).b64(p)
  IOTC_CHECK_CRYPTO(ret = _iotc_create_jwt_b64h_b64p(
                        dst_jwt_buf, dst_jwt_buf_len, bytes_written, project_id,
                        expiration_period_sec, "ES256"));

  // create sha256 hash of b64(h).b64(p): sha256(b64(h).b64(p))
  uint8_t sha256_b64h_b64p[32] = {0};
  IOTC_CHECK_CRYPTO(ret = iotc_bsp_sha256(sha256_b64h_b64p,
                                          (const uint8_t*)dst_jwt_buf,
                                          *bytes_written));

  // add second dot, separating b64(h).b64(p) and b64(eccsignature)
  dst_jwt_buf[*bytes_written] = '.';
  ++*bytes_written;

  // create ecc signature: ecc(sha256(b64(h).b64(p)))
  size_t bytes_written_ecc_signature = 0;
  uint8_t ecc_signature[IOTC_JWT_MAX_SIGNATURE_SIZE] = {0};
  IOTC_CHECK_CRYPTO(ret = iotc_bsp_ecc(private_key_data, ecc_signature,
                                       IOTC_JWT_MAX_SIGNATURE_SIZE,
                                       &bytes_written_ecc_signature,
                                       sha256_b64h_b64p, 32));

  // base64 encode the ecc signature
  size_t bytes_written_ecc_signature_base64 = 0;
  ret = iotc_bsp_base64_encode_urlsafe(
      dst_jwt_buf + *bytes_written, dst_jwt_buf_len - *bytes_written,
      &bytes_written_ecc_signature_base64, ecc_signature,
      bytes_written_ecc_signature);

  *bytes_written += bytes_written_ecc_signature_base64;

  IOTC_CHECK_CRYPTO(ret);

  return IOTC_STATE_OK;

err_handling:

  switch (ret) {
    case IOTC_BSP_CRYPTO_STATE_OK:
      return IOTC_STATE_OK;
    case IOTC_BSP_CRYPTO_BUFFER_TOO_SMALL_ERROR:
      return IOTC_BUFFER_TOO_SMALL_ERROR;
    case IOTC_BSP_CRYPTO_INVALID_INPUT_PARAMETER_ERROR:
      return IOTC_INVALID_PARAMETER;
    default:
      return IOTC_JWT_FORMATTION_ERROR;
  }

  return IOTC_JWT_FORMATTION_ERROR;
}
