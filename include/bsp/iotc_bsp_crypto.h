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
 * @brief Implements a TLS library for creating JSON Web Tokens.
 * 
 * @details The TLS library:
 *     - Generates JWT credentials
 *     - Signs JWTs with Elliptic Curve cryptography and SHA256
 *     - Encodes signed JWTs as URL-safe base64 strings.
 *
 * The SDK has two <a href="../../bsp/html/index.html">turn-key TLS libraries
 * for POSIX platforms: <a href="../../../src/bsp/crypto/mbedtls">mbedTLS</a>
 * and <a href="../../../src/bsp/crypto/wolfssl">wolfSSL</a>. You can modify the
 * functions in this file to create a new TLS library.
 */

/**
 * @typedef iotc_bsp_crypto_state_t
 * @brief The crytography function states.
 *
 * @see #iotc_bsp_crypto_state_e
 */
typedef enum iotc_bsp_crypto_state_e {
  /** The cryptography function succeeded. */
  IOTC_BSP_CRYPTO_STATE_OK = 0,
  /** Something went wrong. */
  IOTC_BSP_CRYPTO_ERROR,
  /** Can't base64-encode string. */
  IOTC_BSP_CRYPTO_BASE64_ERROR,
  /** Can't create SHA256 digest. */
  IOTC_BSP_CRYPTO_SHA256_ERROR,
  /** Can't create ECC signature. */
  IOTC_BSP_CRYPTO_ECC_ERROR,
  /** Can't write data to the buffer because the data is larger than the buffer. */
  IOTC_BSP_CRYPTO_BUFFER_TOO_SMALL_ERROR,
  /** Can't parse private key data. */
  IOTC_BSP_CRYPTO_KEY_PARSE_ERROR,
  /** Can't serialize data. */
  IOTC_BSP_CRYPTO_SERIALIZE_ERROR,
  /** Invalid parameter. */
  IOTC_BSP_CRYPTO_INVALID_INPUT_PARAMETER_ERROR,
} iotc_bsp_crypto_state_t;

/**
 * @brief Encodes a string as a URL-safe, base64 string by replacing all
 * URL-unsafe characters with a - (dash) or _ (underscore).
 *
 * @param [in,out] dst_string A pointer to a buffer that stores the URL-safe,
 *     base64 string. The Device SDK allocates the buffer before calling this
 *     function implementation.
 * @param [in] dst_string_size The length, in bytes, of the
 *     dst_string buffer.
 * @param [out] bytes_written The bytes written to dst_string. If the buffer is
 *     too small, bytes_written is the required buffer size.
 * @param [in] src_buf A pointer to a buffer with the string to encode.
 * @param [in] src_buf_size The size, in bytes, of buffer to which
 *     src_buf points.
 *
 * @returns A {@link iotc_bsp_crypto_state_e crytography function status}.
 */
iotc_bsp_crypto_state_t iotc_bsp_base64_encode_urlsafe(
    unsigned char* dst_string, size_t dst_string_size, size_t* bytes_written,
    const uint8_t* src_buf, size_t src_buf_size);

/**
 * @brief Generates a SHA256 cryptographic hash.
 *
 * @param [in,out] dst_buf_32_bytes A pointer to 32-byte buffer into which
 *     this function stores the digest. The buffer is already
 *     allocated by the SDK.
 * @param [in] src_buf A pointer to buffer with the string to encode.
 * @param [in] src_buf_size The size, in bytes, of the buffer to which
 *     src_buf points.
 */
iotc_bsp_crypto_state_t iotc_bsp_sha256(uint8_t* dst_buf_32_bytes,
                                        const uint8_t* src_buf,
                                        uint32_t src_buf_size);

/**
 * @brief Generates an Elliptic Curve signature for a private key.
 *
 * @param [in] private_key_pem The private key data or slot number.
 *     Implementations of this function must use the same private key data or
 *     slot number provided to iotc_create_iotcore_jwt().
 * @param [in,out] dst_buf A pointer to a buffer into which the function
 *     stores the Elliptic Curve signature. The buffer is already allocated by
 *     the SDK.
 * @param [in] dst_buf_size The size, in bytes, of the buffer to which
 *     dst_buf points.
 * @param [out] bytes_written The number of bytes written to dst_buf.
 * @param [in] src_buf A pointer to a buffer of data to sign.
 * @param [in] src_buf_size The size, in bytes, of the buffer to which
 *     src_buf points.
 */
iotc_bsp_crypto_state_t iotc_bsp_ecc(const iotc_crypto_key_data_t* private_key,
                                     uint8_t* dst_buf, size_t dst_buf_size,
                                     size_t* bytes_written,
                                     const uint8_t* src_buf,
                                     size_t src_buf_size);

#ifdef __cplusplus
}
#endif

#endif /* __IOTC_BSP_CRYPTO_H__ */
