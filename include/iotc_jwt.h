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

#ifndef __IOTC_JWT_H__
#define __IOTC_JWT_H__

#include <iotc_types.h>

#ifdef __cplusplus
extern "C" {
#endif

#define IOTC_JWT_HEADER_BUF_SIZE 40
#define IOTC_JWT_HEADER_BUF_SIZE_BASE64 (IOTC_JWT_HEADER_BUF_SIZE * 4 / 3)

#define IOTC_JWT_PAYLOAD_BUF_SIZE 256
#define IOTC_JWT_PAYLOAD_BUF_SIZE_BASE64 (IOTC_JWT_PAYLOAD_BUF_SIZE * 4 / 3)

#define IOTC_JWT_MAX_SIGNATURE_SIZE 132
#define IOTC_JWT_MAX_SIGNATURE_SIZE_BASE64 (IOTC_JWT_MAX_SIGNATURE_SIZE * 4 / 3)

#define IOTC_JWT_SIZE                                                       \
  (IOTC_JWT_HEADER_BUF_SIZE_BASE64 + 1 + IOTC_JWT_PAYLOAD_BUF_SIZE_BASE64 + \
   1 + IOTC_JWT_MAX_SIGNATURE_SIZE_BASE64)


/**
 * @function
 * @brief Creates a JWT which will be used to connect to the IoT Core service.
 * The JWT should be used in the MQTT Connect password field.
 *
 * @param [in] expiration_period_sec the length of time (in seconds) before
 * this JWT will expire.
 * @param [in] project_id the project id the device belongs to in the GCP
 * IoT organization.
 * @param [in] private_key_data data identifying a private key to be used
 * for client signatures. For more information on how to generate a
 * private-public key pair for your device, please see:
 * https://cloud.google.com/iot/docs/how-tos/credentials/keys.
 * @param [in/out] a pointer to a buffer to hold the formatted and signed
 * JWT.
 * @param [in] dst_jwt_buf_len the length of the dst_jwt_buf buffer, in
 * bytes.
 * application will use these credentials to connect to IoT Core.
 * @param [out] bytes_written upon return, will contain the number of bytes
 * that were written to the provided dst_jwt_buf.
 */

iotc_state_t iotc_create_iotcore_jwt(
    const char* project_id, uint32_t expiration_period_sec,
    const iotc_crypto_key_data_t* private_key_data,
    unsigned char* dst_jwt_buf, size_t dst_jwt_buf_len, size_t*
    bytes_written);


#ifdef __cplusplus
}
#endif

#endif /* __IOTC_JWT_H__ */