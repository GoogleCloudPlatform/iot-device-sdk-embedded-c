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

#include "iotc_bsp_crypto.h"
#include "iotc_macros.h"

iotc_bsp_crypto_state_t iotc_bsp_base64_encode_urlsafe(
    unsigned char* dst_string, size_t dst_string_size, size_t* bytes_written,
    const uint8_t* src_buf, size_t src_buf_size) {
  IOTC_UNUSED(dst_string);
  IOTC_UNUSED(dst_string_size);
  IOTC_UNUSED(bytes_written);
  IOTC_UNUSED(src_buf);
  IOTC_UNUSED(src_buf_size);

  return IOTC_BSP_CRYPTO_STATE_OK;
}

iotc_bsp_crypto_state_t iotc_bsp_sha256(uint8_t* dst_buf_32_bytes,
                                        const uint8_t* src_buf,
                                        uint32_t src_buf_size) {
  IOTC_UNUSED(dst_buf_32_bytes);
  IOTC_UNUSED(src_buf);
  IOTC_UNUSED(src_buf_size);

  return IOTC_BSP_CRYPTO_STATE_OK;
}

iotc_bsp_crypto_state_t iotc_bsp_ecc(
    const iotc_crypto_key_data_t* private_key_data, uint8_t* dst_buf,
    size_t dst_buf_size, size_t* bytes_written, const uint8_t* src_buf,
    size_t src_buf_size) {
  IOTC_UNUSED(private_key_data);
  IOTC_UNUSED(dst_buf);
  IOTC_UNUSED(dst_buf_size);
  IOTC_UNUSED(bytes_written);
  IOTC_UNUSED(src_buf);
  IOTC_UNUSED(src_buf_size);

  return IOTC_BSP_CRYPTO_STATE_OK;
}
