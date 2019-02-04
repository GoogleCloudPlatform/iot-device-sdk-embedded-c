/* Copyright 2018-2019 Google LLC
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

#ifndef __IOTC_JWT_H__
#define __IOTC_JWT_H__

#include <iotc_types.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @function
 * @brief Creates a JWT which will be used to connect to the IoT Core service.
 * The JWT should be used in the MQTT Connect password field when connecting
 * to IoT Core.
 *
 * The function currently supports ES256 key types only, as its intended
 * for small devices where ECC algorithims should be used to their smaller
 * key footprint requirements.
 *
 * Note: This function invokes the Crypto BSP functions
 * iotc_bsp_sha256(), iotc_bsp_ecc(), and iotc_bsp_base64_encode_urlsafe()
 * to fullfill string encoding and signature functionality.
 *
 * @param [in] expiration_period_sec the length of time (in seconds) before
 * this JWT will expire.
 * @param [in] project_id the project id the device belongs to in the GCP
 * IoT organization.
 * @param [in] private_key_data data identifying a private key to be used
 * to sign the JWT. For more information on how to generate a
 * private-public key pair for your device, please see:
 * https://cloud.google.com/iot/docs/how-tos/credentials/keys.
 * @param [in/out] a pointer to a buffer to hold the formatted and signed
 * JWT.
 * @param [in] dst_jwt_buf_len the length of the dst_jwt_buf buffer, in
 * bytes.
 * @param [out] bytes_written will contain the number of bytes
 * that were written to the provided dst_jwt_buf.
 *
 * @returns IOTC_STATE_OK if jwt generation was successful.
 * @returns IOTC_INVALID_PARAMETER if the project_id, private_key_data or
 * dst_jwt_buf parameters are NULL, or if either of the crypto bsp
 * functions returns IOTC_BSP_CRYPTO_INVALID_INPUT_PARAMETER_ERROR.
 * @returns IOTC_ALG_NOT_SUPPORTED_ERROR if the
 * private_key_data->crypto_key_signature_algorithm is not of type
 * IOTC_CRYPTO_KEY_SIGNATURE_ALGORITHM_ES256.
 * @returns IOTC_NULL_KEY_DATA_ERROR if the private_key_data is of type
 * IOTC_CRYPTO_KEY_UNION_TYPE_PEM and the
 * private_key_data->crypto_key_union.key_pem.key pointer is NULL.
 * @returns IOTC_NOT_IMPLEMENTED if the private_key_data->crypto_key_union
 * is of an uknown type.
 * @returns IOTC_BUFFER_TOO_SMALL_ERROR if the crypto BSP returns
 */

iotc_state_t iotc_create_iotcore_jwt(
    const char* project_id, uint32_t expiration_period_sec,
    const iotc_crypto_key_data_t* private_key_data, unsigned char* dst_jwt_buf,
    size_t dst_jwt_buf_len, size_t* bytes_written);

#ifdef __cplusplus
}
#endif

#endif /* __IOTC_JWT_H__ */
