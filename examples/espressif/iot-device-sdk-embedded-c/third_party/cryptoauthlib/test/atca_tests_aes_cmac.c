/**
 * \file
 * \brief Unity tests for the cryptoauthlib Basic API
 *
 * \copyright (c) 2015-2018 Microchip Technology Inc. and its subsidiaries.
 *
 * \page License
 *
 * Subject to your compliance with these terms, you may use Microchip software
 * and any derivatives exclusively with Microchip products. It is your
 * responsibility to comply with third party license terms applicable to your
 * use of third party software (including open source software) that may
 * accompany Microchip software.
 *
 * THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES, WHETHER
 * EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY IMPLIED
 * WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS FOR A
 * PARTICULAR PURPOSE. IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT,
 * SPECIAL, PUNITIVE, INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE
 * OF ANY KIND WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF
 * MICROCHIP HAS BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE
 * FORESEEABLE. TO THE FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL
 * LIABILITY ON ALL CLAIMS IN ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED
 * THE AMOUNT OF FEES, IF ANY, THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR
 * THIS SOFTWARE.
 */
#include <stdlib.h>
#ifdef _WIN32
#include <time.h>
#endif
#include "atca_test.h"
#include "basic/atca_basic.h"
#include "host/atca_host.h"
#include "test/atca_tests.h"

extern const uint8_t g_aes_keys[4][16];
extern const uint8_t g_plaintext[64];

// Message sizes from the g_plaintext array to try when testing the AES128-CMAC
// functions
const uint32_t g_cmac_msg_sizes[] = { 0, 16, 20, 64 };
// Expected CMACs for all keys and message sizes
// The first set of entries is from the the NIST CMAC test vectors
// https://csrc.nist.gov/CSRC/media/Projects/Cryptographic-Standards-and-Guidelines/documents/examples/AES_CMAC.pdf
const uint8_t g_cmacs[4][4][16] = {
    {
        { 0xbb, 0x1d, 0x69, 0x29, 0xe9, 0x59, 0x37, 0x28, 0x7f, 0xa3, 0x7d, 0x12, 0x9b, 0x75, 0x67, 0x46 },
        { 0x07, 0x0a, 0x16, 0xb4, 0x6b, 0x4d, 0x41, 0x44, 0xf7, 0x9b, 0xdd, 0x9d, 0xd0, 0x4a, 0x28, 0x7c },
        { 0x7d, 0x85, 0x44, 0x9e, 0xa6, 0xea, 0x19, 0xc8, 0x23, 0xa7, 0xbf, 0x78, 0x83, 0x7d, 0xfa, 0xde },
        { 0x51, 0xf0, 0xbe, 0xbf, 0x7e, 0x3b, 0x9d, 0x92, 0xfc, 0x49, 0x74, 0x17, 0x79, 0x36, 0x3c, 0xfe },
    },
    {
        { 0xfc, 0xfd, 0x69, 0xfe, 0x15, 0x80, 0xc0, 0x95, 0xa7, 0x83, 0x54, 0x89, 0x31, 0xf3, 0x01, 0x70 },
        { 0x5e, 0x36, 0xdc, 0x8f, 0x6a, 0xd2, 0xbe, 0x40, 0xb3, 0x87, 0xae, 0x78, 0x52, 0x28, 0xfe, 0x56 },
        { 0x03, 0xcc, 0x26, 0x2a, 0xfe, 0x76, 0x91, 0x5f, 0xe5, 0x6a, 0x52, 0xe0, 0x57, 0x98, 0xf2, 0xeb },
        { 0xd5, 0x29, 0x4e, 0xe1, 0x19, 0xb6, 0x70, 0x44, 0xb5, 0x57, 0xbc, 0x94, 0x65, 0x0b, 0x3f, 0xa5 },
    },
    {
        { 0xe2, 0x86, 0xa8, 0x0e, 0xd6, 0xbd, 0x92, 0xa4, 0xaa, 0xaf, 0x20, 0x6e, 0xb3, 0x27, 0x21, 0x29 },
        { 0xac, 0xac, 0x7d, 0x25, 0x9e, 0x9d, 0x26, 0x69, 0x52, 0x66, 0xa6, 0xb8, 0xed, 0x09, 0x76, 0xcf },
        { 0xba, 0x12, 0x9f, 0x42, 0x27, 0x1f, 0x7d, 0x5f, 0x7d, 0x3f, 0x57, 0xf3, 0x08, 0xee, 0x37, 0xb2 },
        { 0x4d, 0xd1, 0x67, 0x53, 0xe3, 0xd3, 0x61, 0xc6, 0x36, 0x3b, 0xf5, 0x16, 0x04, 0x45, 0x2e, 0x85 },
    },
    {
        { 0xbf, 0xb7, 0x21, 0xb5, 0x2c, 0xc2, 0x13, 0x66, 0x99, 0xca, 0x97, 0x8b, 0x50, 0xb9, 0xa0, 0xa3 },
        { 0x82, 0x84, 0xbe, 0x95, 0x59, 0xcd, 0x27, 0xf2, 0x4d, 0x9b, 0x07, 0x33, 0x93, 0x56, 0xc2, 0x3e },
        { 0xb9, 0xc7, 0xa3, 0x5f, 0xc5, 0x83, 0xf1, 0x3f, 0x4d, 0x0f, 0x8c, 0x79, 0x8d, 0xac, 0xc8, 0xc5 },
        { 0x91, 0xed, 0x39, 0x68, 0xff, 0x64, 0xbe, 0x68, 0x8f, 0x43, 0x6e, 0xbc, 0xeb, 0x57, 0x72, 0xe7 },
    }
};


TEST(atca_cmd_basic_test, aes_cmac)
{
    ATCA_STATUS status;
    uint8_t key_block;
    size_t msg_index;
    atca_aes_cmac_ctx_t ctx;
    uint8_t cmac[AES_DATA_SIZE];

    check_config_aes_enable();

    // Load AES keys into TempKey
    status = atcab_nonce_load(NONCE_MODE_TARGET_TEMPKEY, g_aes_keys[0], 64);
    TEST_ASSERT_EQUAL(ATCA_SUCCESS, status);

    for (key_block = 0; key_block < 4; key_block++)
    {
        for (msg_index = 0; msg_index < sizeof(g_cmac_msg_sizes) / sizeof(g_cmac_msg_sizes[0]); msg_index++)
        {
            status = atcab_aes_cmac_init(&ctx, ATCA_TEMPKEY_KEYID, key_block);
            TEST_ASSERT_EQUAL(ATCA_SUCCESS, status);

            status = atcab_aes_cmac_update(&ctx, g_plaintext, g_cmac_msg_sizes[msg_index]);
            TEST_ASSERT_EQUAL(ATCA_SUCCESS, status);

            status = atcab_aes_cmac_finish(&ctx, cmac, sizeof(cmac));
            TEST_ASSERT_EQUAL(ATCA_SUCCESS, status);
            TEST_ASSERT_EQUAL_MEMORY(g_cmacs[key_block][msg_index], cmac, sizeof(cmac));
        }
    }
}

// *INDENT-OFF* - Preserve formatting
t_test_case_info aes_cmac_basic_test_info[] =
{
    { REGISTER_TEST_CASE(atca_cmd_basic_test, aes_cmac),                         DEVICE_MASK(ATECC608A) },
    { (fp_test_case)NULL,                     (uint8_t)0 },             /* Array Termination element*/
};

// *INDENT-ON*
