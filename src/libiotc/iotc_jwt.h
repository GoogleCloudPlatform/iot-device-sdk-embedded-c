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
#include <stdint.h>

#define IOTC_JWT_HEADER_BUF_SIZE 40
#define IOTC_JWT_HEADER_BUF_SIZE_BASE64 (IOTC_JWT_HEADER_BUF_SIZE * 4 / 3)

#define IOTC_JWT_PAYLOAD_BUF_SIZE 256
#define IOTC_JWT_PAYLOAD_BUF_SIZE_BASE64 (IOTC_JWT_PAYLOAD_BUF_SIZE * 4 / 3)

#define IOTC_JWT_MAX_SIGNATURE_SIZE 132
#define IOTC_JWT_MAX_SIGNATURE_SIZE_BASE64 (IOTC_JWT_MAX_SIGNATURE_SIZE * 4 / 3)

#define IOTC_JWT_SIZE                                                       \
  (IOTC_JWT_HEADER_BUF_SIZE_BASE64 + 1 + IOTC_JWT_PAYLOAD_BUF_SIZE_BASE64 + \
   1 + IOTC_JWT_MAX_SIGNATURE_SIZE_BASE64)

iotc_state_t iotc_create_jwt_es256(
    const char* project_id, uint32_t expiration_period_sec,
    const iotc_crypto_private_key_data_t* private_key_data,
    unsigned char* dst_jwt_buf, size_t dst_jwt_buf_len, size_t* bytes_written);

#endif /* __IOTC_JWT_H__ */
