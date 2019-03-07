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

#ifndef __IOTC_BSP_CRYPTO_H__
#define __IOTC_BSP_CRYPTO_H__

#include <stdint.h>
#include <unistd.h>

#include "iotc_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file  iotc_bsp_crypto.h
 * @brief Encode and sign JWT credentials.
 * 
 * These functions implement a cryptography library to: generate and sign JWTs 
 * via ECC keys and SHA256; or format signed JWTs as URL-safe base64 strings.
 * Reference implementations are in the <a href="~/src/bsp/crypto"><code>src/bsp/crypto</code></a>
 * directory.
 *
 * The BSP uses the private key data or slot number provided to the <code>iotc_connect()</code>
 * function. The BSP doesn't format original JWT itself; the BSP only formats
 * signed JWT. The <a href="../../api/html/index.html">Device SDK API</a>
 * formats the original JWT.
 *
 * All cryptography functions issue an <code>iotc_bsp_crypto_state_e</code> 
 * status message.
 */

/**
 * @typedef iotc_bsp_crypto_state_t
 * @brief Cryptographic function status.
 * 
 * All cryptographic functions return a status message to the client
 * application. IOTC_BSP_CRYPTO_STATE_OK represents success and others
 * represent errors.
 */
typedef enum iotc_bsp_crypto_state_e {
  /** Operation successful. */
  IOTC_BSP_CRYPTO_STATE_OK = 0,
  /** Operation failed. Generic error. */
  IOTC_BSP_CRYPTO_ERROR,
  /** Cannot base64-encode string. */
  IOTC_BSP_CRYPTO_BASE64_ERROR,
  /** Cannot create SHA256 digest. */
  IOTC_BSP_CRYPTO_SHA256_ERROR,
  /** Cannot create ECC signature. */
  IOTC_BSP_CRYPTO_ECC_ERROR,
  /** Cannot write data to buffer because the data is larger than the buffer. */
  IOTC_BSP_CRYPTO_BUFFER_TOO_SMALL_ERROR,
  /** Cannot parse private key file. */
  IOTC_BSP_CRYPTO_KEY_PARSE_ERROR,
  /** Cannot serialize data. */
  IOTC_BSP_CRYPTO_SERIALIZE_ERROR,
  /** Invalid parameter. */
  IOTC_BSP_CRYPTO_INVALID_INPUT_PARAMETER_ERROR,
} iotc_bsp_crypto_state_t;

/**
 * @function
 * @brief Encode a string as a URL-safe, base64 string.
 *
 * All characters that aren't URL safe are replaced with a - (dash) or _
 * (underscore). 
 *
 * @param [in,out] dst_string a pointer to a buffer that stores the URL-safe,
 * base64 string. The buffer is automatically allocated.
 * @param [in] dst_string_size the length, in bytes, of the dst_string buffer.
 * @param [out] bytes_written the bytes written to dst_string. If the buffer is
 * too small, bytes_written is the required buffer size.
 * @param [in] src_buf a pointer to a buffer with the string to encode.
 * @param [in] src_buf_size the size, in bytes, of buffer to which src_buf
 * points.

 * @retval IOTC_BSP_TLS_STATE_OK the string is successfully encoded.
 * @retval IOTC_BSP_CRYPTO_BUFFER_TOO_SMALL_ERROR the dst_string buffer is too
 * small for the encoded string.
 */
iotc_bsp_crypto_state_t iotc_bsp_base64_encode_urlsafe(
    unsigned char* dst_string, size_t dst_string_size, size_t* bytes_written,
    const uint8_t* src_buf, size_t src_buf_size);

/**
 * @function
 * @brief Generate a SHA256 cryptographic hash.
 *
 * @param [in,out] dst_buf_32_bytes a pointer to 32-byte buffer that stores the
 * SHA256 digest. The buffer is automatically allocated.
 * @param [in] src_buf a pointer to buffer with the string to encode.
 * @param [in] src_buf_size the size, in bytes, of the buffer to which src_buf
 * points.
 *
 * @see iotc_bsp_crypto_state_t
 *
 * @retval IOTC_BSP_TLS_STATE_OK the digest is successfully created.
 * @retval IOTC_BSP_CRYPTO_ERROR can't create digest.
 */
iotc_bsp_crypto_state_t iotc_bsp_sha256(uint8_t* dst_buf_32_bytes,
                                        const uint8_t* src_buf,
                                        uint32_t src_buf_size);

/**
 * @function
 * @brief Generate an ECC signature for a private key.
 *
 * The BSP uses the private key data or slot number provided to the <code>iotc_connect()</code>
 * function.
 *
 *
 * @param [in] private_key_pem the private key data or slot number provided to
 * the <code>iotc_connect()</code> function.
 * @param [in,out] dst_buf a pointer to a buffer that stores the ECC signature.
 * The buffer is automatically allocated.
 * @param [in] dst_buf_size the size, in bytes, of the buffer to which dst_buf
 * points.
 * @param [out] bytes_written the number of bytes written to dst_buf.
 * @param [in] src_buf a buffer with the private key data or slot number.
 * @param [in] src_buf_size the size, in bytes, of the buffer to which src_buf
 * points.
 *
 * @retval IOTC_BSP_TLS_STATE_OK the ECC signature is successfully created.
 * @retval IOTC_BSP_CRYPTO_ERROR can't create ECC signature.
 */
iotc_bsp_crypto_state_t iotc_bsp_ecc(
    const iotc_crypto_key_data_t* private_key_pem, uint8_t* dst_buf,
    size_t dst_buf_size, size_t* bytes_written, const uint8_t* src_buf,
    size_t src_buf_size);

#ifdef __cplusplus
}
#endif

#endif /* __IOTC_BSP_CRYPTO_H__ */
