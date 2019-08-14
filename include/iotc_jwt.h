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

/*! \file
 * @brief Creates JSON Web Tokens.
 */

/** The size, in bytes, of the JWT header. */
#define IOTC_JWT_HEADER_BUF_SIZE 40
/** The size, in bytes, of the URL-encoded JWT header. */
#define IOTC_JWT_HEADER_BUF_SIZE_BASE64 \
  (((IOTC_JWT_HEADER_BUF_SIZE + 2) / 3) * 4)

/** The size, in bytes, of the JWT payload. */
#define IOTC_JWT_PAYLOAD_BUF_SIZE 256
/** The size, in bytes, of the URL-encoded JWT payload. */
#define IOTC_JWT_PAYLOAD_BUF_SIZE_BASE64 \
  (((IOTC_JWT_PAYLOAD_BUF_SIZE + 2) / 3) * 4)

/** The maximum size, in bytes, of the JWT signature. */
#define IOTC_JWT_MAX_SIGNATURE_SIZE 132
/** The maxiumum size, in bytes, of the URL-encoded JWT. */
#define IOTC_JWT_MAX_SIGNATURE_SIZE_BASE64 \
  (((IOTC_JWT_MAX_SIGNATURE_SIZE + 2) / 3) * 4)

/** The size, in bytes, of the JWT. */
#define IOTC_JWT_SIZE                                                       \
  (IOTC_JWT_HEADER_BUF_SIZE_BASE64 + 1 + IOTC_JWT_PAYLOAD_BUF_SIZE_BASE64 + \
   1 + IOTC_JWT_MAX_SIGNATURE_SIZE_BASE64)

/**
 * @details Creates a JWT with the
 * <a href="../../bsp/html/d6/d01/iotc__bsp__crypto_8h.html">BSP cyptrography
 * library</a>.
 *
 * @param [in] expiration_period_sec The number of seconds before this JWT
 *     expires.
 * @param [in] project_id The GCP project ID.
 * @param [in] private_key_data ES256 private key data.
 * @param [in,out] dst_jwt_buf A pointer to a buffer that stores a formatted and
 *     signed JWT.
 * @param [in] dst_jwt_buf_len The length, in bytes, of the buffer to which
 *     dst_jwt_buf points.
 * @param [out] bytes_written The number of bytes written to the buffer to which
 *     dst_jwt_buf points.
 */
iotc_state_t iotc_create_iotcore_jwt(
    const char* project_id, uint32_t expiration_period_sec,
    const iotc_crypto_key_data_t* private_key_data, char* dst_jwt_buf,
    size_t dst_jwt_buf_len, size_t* bytes_written);

#ifdef __cplusplus
}
#endif

#endif /* __IOTC_JWT_H__ */
