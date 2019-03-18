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
 * @brief Create a JWT.
 * 
 * Implementations of this function must create a JWT string. To connect to
 * Cloud IoT Core, pass the JWT to the CONNECT message password field.
 *
 * BSP implementations must also invoke <code>iotc_bsp_sha256()</code>, 
 * <code>iotc_bsp_ecc()</code> and <code>iotc_bsp_base64_encode_urlsafe()</code>
 * to enable string encoding and signatures.
 *
 * This function only supports ES256 key types.
 *
 * @param [in] expiration_period_sec The number of seconds before this JWT
 * expires.
 * @param [in] project_id The GCP project ID.
 * @param [in] private_key_data ES256 <a href"https://cloud.google.com/iot/docs/how-tos/credentials/keys">private key data</a>.
 * @param [in,out] dst_jwt_buf A pointer to a buffer that stores a formatted and signed JWT.
 * @param [in] dst_jwt_buf_len The length of the dst_jwt_buf buffer, in bytes.
 * @param [out] bytes_written The number of bytes written to dst_jwt_buf.
 *
 * @retval IOTC_STATE_OK A JWT is successfully generated.
 * @retval IOTC_INVALID_PARAMETER The project_id, private_key_data or
 * dst_jwt_buf parameters are NULL, or a crypto BSP function returns
 *     IOTC_BSP_CRYPTO_INVALID_INPUT_PARAMETER_ERROR.
 * @retval IOTC_ALG_NOT_SUPPORTED_ERROR The provided private key isn't
 *     an ES256 key.
 * @retval IOTC_NULL_KEY_DATA_ERROR The provided private key is a PEM file
 *     but the crypto_key_union pointer is NULL.
 * @retval IOTC_NOT_IMPLEMENTED The crypto_key_union pointer type is
 *     unknown.
 * @retval IOTC_BUFFER_TOO_SMALL_ERROR The provided buffer is too small for
 *     the JWT.
 */

iotc_state_t iotc_create_iotcore_jwt(
    const char* project_id, uint32_t expiration_period_sec,
    const iotc_crypto_key_data_t* private_key_data, char* dst_jwt_buf,
    size_t dst_jwt_buf_len, size_t* bytes_written);

#ifdef __cplusplus
}
#endif

#endif /* __IOTC_JWT_H__ */
