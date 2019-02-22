/* Copyright 2018-2019 Google LLC
 *
 * This is part of the Google Cloud IoT Device SDK for Embedded C,
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

#ifndef __IOTC_BSP_CRYPTO_H__
#define __IOTC_BSP_CRYPTO_H__

#include <stdint.h>
#include <unistd.h>

#include "iotc_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file iotc_bsp_crypto.h
 * @brief The IoT Device SDK Board Support Package (BSP) for using a Cryptography
 * library implementation to encode and sign portions of clients Google Cloud
 * IoT Core JWT credential.
 *
 * This file defines the API of a Crypto Board Support Package (BSP) that the
 * IoTC Client would use to generate and sign a JWT using ECC keys and SHA256
 * digests, and formatting results in a URL-Safe Base64 string. Reference
 * implementations reside in the src/bsp/crypto directory of this repository.
 *
 * The BSP has been created in a way that allows for flexibility in
 * implementation. The private key data and/or private key slot number
 * provided in the iotc_connect call is provided for your use here, and one
 * or both may be NULL depending on your Client Application and your
 * implementation of this BSP.
 *
 * Note that this implementation need not worry about how to format the JWT
 * itself, as this has been already done by the IoT Device SDK in a previous
 * step.
 *
 * These functions should return one of the Crypto BSP enumerated states
 * defined in iotc_bsp_crypto_state_t to report success or failures.
 */

/**
 * @typedef iotc_bsp_crypto_state_t
 * @brief Return value of the BSP Crypto API functions.
 *
 * An implementation of the Crypto BSP must return internal status
 * to IoTC Client through these state values.
 */
typedef enum iotc_bsp_crypto_state_e {
  IOTC_BSP_CRYPTO_STATE_OK = 0,
  IOTC_BSP_CRYPTO_ERROR,
  IOTC_BSP_CRYPTO_BASE64_ERROR,
  IOTC_BSP_CRYPTO_SHA256_ERROR,
  IOTC_BSP_CRYPTO_ECC_ERROR,
  IOTC_BSP_CRYPTO_BUFFER_TOO_SMALL_ERROR,
  IOTC_BSP_CRYPTO_KEY_PARSE_ERROR,
  IOTC_BSP_CRYPTO_SERIALIZE_ERROR,
  IOTC_BSP_CRYPTO_INVALID_INPUT_PARAMETER_ERROR,
} iotc_bsp_crypto_state_t;

/**
 * @function
 * @brief Encodes a source buffer into URL Safe Base64 string.
 *
 * This function should Base64 encode the provided source string and place
 * the results in the dest_string buffer.  Additionally, any URL Unsafe
 * characters in the encoding result should be replaced with URL safe
 * characters before this function returns.  That is, instead of the common
 * base64 encoding chars at ASCII index 62 and 63, these characters should
 * be replaced - (dash) and _ (underscore) respectively to be URL safe.
 *
 * @param [in|out] dst_string pointer to a byte buffer that should be
 * populated with the resulting URL-safe Base64 string.  This buffer
 * has already been allocated.
 * @param [in] dst_string_size The length of the dst_string buffer.
 * @param [out] bytes_written the number of bytes written to the
 * dst_string, or if the buffer is not large enough then return
 * the required buffer size.
 * @param [in] src_buf the source data to be encoded.
 * @param [in] src_buf_size the length of the src_buf in bytes.
 * @return
 *  - IOTC_BSP_TLS_STATE_OK in case of success
 *  - IOTC_BSP_CRYPTO_BUFFER_TOO_SMALL_ERROR in the case that the
 * dst_string is too small to hold the resulting data.
 */
iotc_bsp_crypto_state_t iotc_bsp_base64_encode_urlsafe(
    unsigned char* dst_string, size_t dst_string_size, size_t* bytes_written,
    const uint8_t* src_buf, size_t src_buf_size);

/**
 * @function
 * @brief Generates the SHA256 hash of the provided source data.
 *
 * This function should create a SHA256 hash of the provided
 * data, and write it into the destination buffer.
 *
 * @param [in|out] dst_buf_32_bytes pointer to 32 bytes of data
 * which should be filled with the resulting SHA digest. This
 * buffer has already been allocated.
 * @param [in] src_buf a pointer the source data to digest.
 * @param [in] src_buf_size the number of bytes of src_buf
 * to encode.
 * @return
 *  - IOTC_BSP_TLS_STATE_OK in case of success
 *  - IOTC_BSP_CRYPTO_ERROR this value or any of the other
 * iotc_bsp_crypto_state_t values, when applicable.
 */
iotc_bsp_crypto_state_t iotc_bsp_sha256(uint8_t* dst_buf_32_bytes,
                                        const uint8_t* src_buf,
                                        uint32_t src_buf_size);

/**
 * @function
 * @brief Creates a ECC signature of the data in src_buf and writes
 * the result into the dest_buf.
 *
 * The private key data provided at the time of the invocation
 * of iotc_connect are provided to this function. You may use
 * which ever combination of data to work with the device SDK
 * to have the provided data signed.
 *
 * Some of the provided private key data may be null, depending
 * on the Client Application's invocation of iotc_connect.  It
 * is the intention that this BSP implementation must
 * orchestrate itself with the Client Application to have the
 * correct private_key related data.
 *
 * For instance, on a secure chip where the private key is held within
 * a secure element, the private_key data that the client application
 * provides should be NULL and this implementation should invoke
 * the secure element SDK to have the src data signed internally.
 *
 * @param [in] private_key_pem a structure containing private key
 * data, or a key slot id for private key use via a secure hardware
 * element, or abstracted void* custom data which will be passed
 * to the BSP implemetnation for use in signing the provided buffer.
 * private key that was provided to the IoTC client upon the client
 * application's invocation of iot_connect.
 * @param [in|out] dst_buf a buffer into which the resulting signature
 * shall be stored.  This buffer has already been allocated.
 * @param [in] dst_buf_size The size of the dst_buf in bytes.
 * @param [out] bytes_written The number of bytes that this function
 * has written into the dst_buf
 * @param [in] src_buf a pointer the source data to have signed.
 * @param [in] src_buf_size the number of bytes of src_buf.
 * @return
 *  - IOTC_BSP_TLS_STATE_OK in case of success
 *  - IOTC_BSP_CRYPTO_ERROR this value or any of the other
 * iotc_bsp_crypto_state_t values, when applicable.
 */
iotc_bsp_crypto_state_t iotc_bsp_ecc(
    const iotc_crypto_key_data_t* private_key_pem, uint8_t* dst_buf,
    size_t dst_buf_size, size_t* bytes_written, const uint8_t* src_buf,
    size_t src_buf_size);

#ifdef __cplusplus
}
#endif

#endif /* __IOTC_BSP_CRYPTO_H__ */
