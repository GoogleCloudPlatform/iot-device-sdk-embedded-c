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

/**
 * Microchip's Cryptoauthlib implementation of crypto BSP.
 * To be used for ATECC* secure elements.
 */

#include "iotc_bsp_crypto.h"
#include "iotc_macros.h"
#include "iotc_helpers.h"

#include "cryptoauthlib.h"

#define IOTC_CHECK_DEBUG_FORMAT( cnd, fmt, ... )                                         \
    if ( ( cnd ) )                                                                       \
    {                                                                                    \
        iotc_debug_format( fmt, __VA_ARGS__ );                                           \
        goto err_handling;                                                               \
    }

iotc_bsp_crypto_state_t
iotc_bsp_base64_encode( unsigned char* dst_string, size_t dst_string_size, size_t* bytes_written,
                      const uint8_t* src_buf, size_t src_buf_size )
{
    size_t size = dst_string_size; // used for dst size as input & bytes written as output
    ATCA_STATUS status = atcab_base64encode(src_buf, src_buf_size, (char *) dst_string, &size);

    if (status == ATCA_SUCCESS) {
        *bytes_written = size;
        return IOTC_BSP_CRYPTO_STATE_OK;
    } else if (status == ATCA_SMALL_BUFFER) {
        return IOTC_BSP_CRYPTO_BUFFER_TOO_SMALL_ERROR;
    } else {
        return IOTC_BSP_CRYPTO_ERROR;
    };
}

iotc_bsp_crypto_state_t
iotc_bsp_base64_encode_urlsafe( unsigned char* dst_string, size_t dst_string_size,
                                size_t* bytes_written, const uint8_t* src_buf, size_t src_buf_size )
{
    size_t size = dst_string_size; // used for dst size as input & bytes written as output
    ATCA_STATUS status = atcab_base64encode_(src_buf, src_buf_size, (char *) dst_string, &size,
                                             atcab_b64rules_urlsafe);

    if (status == ATCA_SUCCESS) {
        *bytes_written = size;
        return IOTC_BSP_CRYPTO_STATE_OK;
    } else if (status == ATCA_SMALL_BUFFER) {
        return IOTC_BSP_CRYPTO_BUFFER_TOO_SMALL_ERROR;
    } else {
        return IOTC_BSP_CRYPTO_ERROR;
    };
}

iotc_bsp_crypto_state_t
iotc_bsp_sha256( uint8_t* dst_buf_32_bytes, const uint8_t* src_buf, uint32_t src_buf_size )
{
    if (NULL == dst_buf_32_bytes) {
        return IOTC_BSP_CRYPTO_INVALID_INPUT_PARAMETER_ERROR;
    }

    int ret = atcac_sw_sha2_256(src_buf, src_buf_size, dst_buf_32_bytes);

    if (ret == ATCA_SUCCESS)
    {
        return IOTC_BSP_CRYPTO_STATE_OK;
    }
    else
    {
        return IOTC_BSP_CRYPTO_ERROR;
    }
}

iotc_bsp_crypto_state_t
iotc_bsp_ecc( const iotc_crypto_key_data_t* private_key_data, uint8_t* dst_buf,
              size_t dst_buf_size, size_t* bytes_written, const uint8_t* src_buf,
              size_t src_buf_size )
{
    if (private_key_data->crypto_key_union_type != IOTC_CRYPTO_KEY_UNION_TYPE_SLOT_ID) {
        iotc_debug_format(
            "Cryptoauthlib impl of iotc_bsp_ecc() only supports slot ID keys. "
            "Got key type %d", private_key_data->crypto_key_union_type);

        return IOTC_BSP_CRYPTO_ERROR;
    }

    uint8_t slot_id = private_key_data->crypto_key_union.key_slot.slot_id;

    IOTC_CHECK_DEBUG_FORMAT( dst_buf_size < 64,
        "dst_buf_size must be >= %zu: was %zu", 64, dst_buf_size );

    IOTC_CHECK_DEBUG_FORMAT( src_buf_size != 32,
        "src_buf_size must be %zu: was %zu", 32, src_buf_size );

    // input message is 32 bytes, output is 64 bytes
    int ret = atcab_sign(slot_id, src_buf, dst_buf);

    IOTC_CHECK_DEBUG_FORMAT( ret != ATCA_SUCCESS,
        "atcab_sign returned %d", ret);

    *bytes_written = 64;
    return IOTC_BSP_CRYPTO_STATE_OK;

err_handling:
    return IOTC_BSP_CRYPTO_ERROR;
}
