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

#ifndef __IOTC_JWT_H__
#define __IOTC_JWT_H__

#include <iotc_types.h>

#ifdef __cplusplus
extern "C" {
#endif

#define IOTC_JWT_HEADER_BUF_SIZE 40
#define IOTC_JWT_HEADER_BUF_SIZE_BASE64 \
  (((IOTC_JWT_HEADER_BUF_SIZE + 2) / 3) * 4)

#define IOTC_JWT_PAYLOAD_BUF_SIZE 256
#define IOTC_JWT_PAYLOAD_BUF_SIZE_BASE64 \
  (((IOTC_JWT_PAYLOAD_BUF_SIZE + 2) / 3) * 4)

#define IOTC_JWT_MAX_SIGNATURE_SIZE 132
#define IOTC_JWT_MAX_SIGNATURE_SIZE_BASE64 \
  (((IOTC_JWT_MAX_SIGNATURE_SIZE + 2) / 3) * 4)

#define IOTC_JWT_SIZE                                                       \
  (IOTC_JWT_HEADER_BUF_SIZE_BASE64 + 1 + IOTC_JWT_PAYLOAD_BUF_SIZE_BASE64 + \
   1 + IOTC_JWT_MAX_SIGNATURE_SIZE_BASE64)

/**
 * @brief    Create a JWT.
 * @detailed Create a JWT to authenticate the client application with Cloud IoT
 * Core. To connect to Cloud IoT Core, pass a JWT to the CONNECT message
 * password field.
 *
 * The function only supports ES256 key types in order to operate with key
 * footprint requirements.
 *
 * Note: This function invokes <code>iotc_bsp_sha256()</code>, 
 * <code>iotc_bsp_ecc()</code>, and <code>iotc_bsp_base64_encode_urlsafe()</code>
 * to enable string encoding and signatures.
 *
 * @param [in] expiration_period_sec the number of seconds before this JWT
 * expires.
 * @param [in] project_id the GCP project ID.
 * @param [in] private_key_data ES256 <a href"https://cloud.google.com/iot/docs/how-tos/credentials/keys">private key data</a>.
 * @param [in/out] a pointer to a buffer with a formatted and signed JWT.
 * @param [in] dst_jwt_buf_len the length of the dst_jwt_buf buffer, in bytes.
 * @param [out] bytes_written the number of bytes written to dst_jwt_buf.
 *
 * @returns IOTC_STATE_OK a JWT is generated.
 * @returns IOTC_INVALID_PARAMETER the project_id, private_key_data or
 * dst_jwt_buf parameters are NULL, or either crypto BSP function returns
 * IOTC_BSP_CRYPTO_INVALID_INPUT_PARAMETER_ERROR.
 * @returns IOTC_ALG_NOT_SUPPORTED_ERROR the provided private key isn't
 * an ES256 key.
 * @returns IOTC_NULL_KEY_DATA_ERROR the provided private key is a PEM file
 * but the crypto_key_union pointer is NULL.
 * @returns IOTC_NOT_IMPLEMENTED the crypto_key_union pointer type is
 * unknown.
 * @returns IOTC_BUFFER_TOO_SMALL_ERROR the provided buffer is too small for
 * JWTs.
 */

iotc_state_t iotc_create_iotcore_jwt(
    const char* project_id, uint32_t expiration_period_sec,
    const iotc_crypto_key_data_t* private_key_data, char* dst_jwt_buf,
    size_t dst_jwt_buf_len, size_t* bytes_written);

#ifdef __cplusplus
}
#endif

#endif /* __IOTC_JWT_H__ */
