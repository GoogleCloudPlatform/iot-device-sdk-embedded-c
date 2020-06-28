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
#include "atca_test.h"
#include "basic/atca_basic.h"
#include "host/atca_host.h"
#include "test/atca_tests.h"
#include "atca_execution.h"

static const uint8_t nist_hash_msg1[] = "abc";
static const uint8_t nist_hash_msg2[] = "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq";

TEST(atca_cmd_unit_test, sha)
{
    ATCA_STATUS status;
    ATCAPacket packet;
    uint8_t sha_success = 0x00;
    uint8_t sha_digest_out[ATCA_SHA_DIGEST_SIZE];
    ATCACommand ca_cmd = _gDevice->mCommands;

    // initialize SHA calculation engine, initializes TempKey
    packet.param1 = SHA_MODE_SHA256_START;
    packet.param2 = 0x0000;

    status = atSHA(ca_cmd, &packet, 0);
    TEST_ASSERT_EQUAL(ATCA_SUCCESS, status);
    status = atca_execute_command(&packet, _gDevice);
    TEST_ASSERT_EQUAL(ATCA_SUCCESS, status);
    TEST_ASSERT_EQUAL(SHA_RSP_SIZE_SHORT, packet.data[ATCA_COUNT_IDX]);

    // check the response, if error then TempKey not initialized
    TEST_ASSERT_EQUAL_INT8(sha_success, packet.data[ATCA_RSP_DATA_IDX]);

    // Compute the SHA 256 digest if TempKey is loaded correctly
    packet.param1 = SHA_MODE_SHA256_END;
    packet.param2 = 0x0000;

    status = atSHA(ca_cmd, &packet, 0);
    TEST_ASSERT_EQUAL(ATCA_SUCCESS, status);
    status = atca_execute_command(&packet, _gDevice);
    TEST_ASSERT_EQUAL(ATCA_SUCCESS, status);
    TEST_ASSERT_EQUAL(SHA_RSP_SIZE_LONG, packet.data[ATCA_COUNT_IDX]);

    // Copy the response into digest_out
    memcpy(&sha_digest_out[0], &packet.data[ATCA_RSP_DATA_IDX], ATCA_SHA_DIGEST_SIZE);
}

TEST(atca_cmd_basic_test, sha)
{
    ATCA_STATUS status = ATCA_GEN_FAIL;
    uint8_t message[ATCA_SHA256_BLOCK_SIZE];
    uint8_t digest[ATCA_SHA_DIGEST_SIZE];
    uint8_t rightAnswer[] = { 0x1A, 0x3A, 0xA5, 0x45, 0x04, 0x94, 0x53, 0xAF,
                              0xDF, 0x17, 0xE9, 0x89, 0xA4, 0x1F, 0xA0, 0x97,
                              0x94, 0xA5, 0x1B, 0xD5, 0xDB, 0x91, 0x36, 0x37,
                              0x67, 0x55, 0x0C, 0x0F, 0x0A, 0xF3, 0x27, 0xD4 };

    memset(message, 0xBC, sizeof(message));


    status = atcab_sha(sizeof(message), message, digest);

    TEST_ASSERT_EQUAL(ATCA_SUCCESS, status);
    TEST_ASSERT_EQUAL_MEMORY(rightAnswer, digest, ATCA_SHA_DIGEST_SIZE);

    memset(message, 0x5A, sizeof(message));
    status = atcab_sha_start();
    TEST_ASSERT_EQUAL(ATCA_SUCCESS, status);

    status = atcab_sha_update(message);
    TEST_ASSERT_EQUAL(ATCA_SUCCESS, status);

    status = atcab_sha_update(message);
    TEST_ASSERT_EQUAL(ATCA_SUCCESS, status);

    status = atcab_sha_update(message);
    TEST_ASSERT_EQUAL(ATCA_SUCCESS, status);

    status = atcab_sha_end(digest, 0, NULL);
    TEST_ASSERT_EQUAL(ATCA_SUCCESS, status);
}

/** \brief test HW SHA with a long message > SHA block size and not an exact SHA block-size increment
 *
 */
TEST(atca_cmd_basic_test, sha_long)
{
    ATCA_STATUS status = ATCA_GEN_FAIL;
    uint8_t message[ATCA_SHA256_BLOCK_SIZE + 63];  // just short of two blocks
    uint8_t digest[ATCA_SHA_DIGEST_SIZE];
    uint8_t rightAnswer[] = { 0xA9, 0x22, 0x18, 0x56, 0x43, 0x70, 0xA0, 0x57,
                              0x27, 0x3F, 0xF4, 0x85, 0xA8, 0x07, 0x3F, 0x32,
                              0xFC, 0x1F, 0x14, 0x12, 0xEC, 0xA2, 0xE3, 0x0B,
                              0x81, 0xA8, 0x87, 0x76, 0x0B, 0x61, 0x31, 0x72 };

    memset(message, 0xBC, sizeof(message));
    memset(digest, 0x00, ATCA_SHA_DIGEST_SIZE);

    status = atcab_sha(sizeof(message), message, digest);

    TEST_ASSERT_EQUAL(ATCA_SUCCESS, status);
    TEST_ASSERT_EQUAL_MEMORY(rightAnswer, digest, ATCA_SHA_DIGEST_SIZE);
}


/** \brief test HW SHA with a short message < SHA block size and not an exact SHA block-size increment
 *
 */
TEST(atca_cmd_basic_test, sha_short)
{
    ATCA_STATUS status = ATCA_GEN_FAIL;
    uint8_t message[10];  // a short message to sha
    uint8_t digest[ATCA_SHA_DIGEST_SIZE];
    uint8_t rightAnswer[] = { 0x30, 0x3f, 0xf8, 0xba, 0x40, 0xa2, 0x06, 0xe7,
                              0xa9, 0x50, 0x02, 0x1e, 0xf5, 0x10, 0x66, 0xd4,
                              0xa0, 0x01, 0x54, 0x75, 0x32, 0x3e, 0xe9, 0xf2,
                              0x4a, 0xc8, 0xc9, 0x63, 0x29, 0x8f, 0x34, 0xce };

    memset(message, 0xBC, sizeof(message));
    memset(digest, 0x00, ATCA_SHA_DIGEST_SIZE);

    status = atcab_sha(sizeof(message), message, digest);

    TEST_ASSERT_EQUAL(ATCA_SUCCESS, status);
    TEST_ASSERT_EQUAL_MEMORY(rightAnswer, digest, ATCA_SHA_DIGEST_SIZE);
}


TEST(atca_cmd_basic_test, sha2_256_nist1)
{
    const uint8_t digest_ref[] = {
        0xBA, 0x78, 0x16, 0xBF, 0x8F, 0x01, 0xCF, 0xEA, 0x41, 0x41, 0x40, 0xDE, 0x5D, 0xAE, 0x22, 0x23,
        0xB0, 0x03, 0x61, 0xA3, 0x96, 0x17, 0x7A, 0x9C, 0xB4, 0x10, 0xFF, 0x61, 0xF2, 0x00, 0x15, 0xAD
    };
    uint8_t digest[ATCA_SHA2_256_DIGEST_SIZE];
    ATCA_STATUS status;

    TEST_ASSERT_EQUAL(ATCA_SHA2_256_DIGEST_SIZE, sizeof(digest_ref));

    status = atcab_sha(sizeof(nist_hash_msg1) - 1, nist_hash_msg1, digest);

    TEST_ASSERT_EQUAL(ATCA_SUCCESS, status);
    TEST_ASSERT_EQUAL_MEMORY(digest_ref, digest, sizeof(digest_ref));
}

TEST(atca_cmd_basic_test, sha2_256_nist2)
{
    const uint8_t digest_ref[] = {
        0x24, 0x8D, 0x6A, 0x61, 0xD2, 0x06, 0x38, 0xB8, 0xE5, 0xC0, 0x26, 0x93, 0x0C, 0x3E, 0x60, 0x39,
        0xA3, 0x3C, 0xE4, 0x59, 0x64, 0xFF, 0x21, 0x67, 0xF6, 0xEC, 0xED, 0xD4, 0x19, 0xDB, 0x06, 0xC1
    };
    uint8_t digest[ATCA_SHA2_256_DIGEST_SIZE];
    ATCA_STATUS status;

    TEST_ASSERT_EQUAL(ATCA_SHA2_256_DIGEST_SIZE, sizeof(digest_ref));

    status = atcab_sha(sizeof(nist_hash_msg2) - 1, nist_hash_msg2, digest);

    TEST_ASSERT_EQUAL(ATCA_SUCCESS, status);
    TEST_ASSERT_EQUAL_MEMORY(digest_ref, digest, sizeof(digest_ref));
}
#ifdef _WIN32
static void hex_to_uint8(const char hex_str[2], uint8_t* num)
{
    *num = 0;

    if (hex_str[0] >= '0' && hex_str[0] <= '9')
    {
        *num += (hex_str[0] - '0') << 4;
    }
    else if (hex_str[0] >= 'A' && hex_str[0] <= 'F')
    {
        *num += (hex_str[0] - 'A' + 10) << 4;
    }
    else if (hex_str[0] >= 'a' && hex_str[0] <= 'f')
    {
        *num += (hex_str[0] - 'a' + 10) << 4;
    }
    else
    {
        TEST_FAIL_MESSAGE("Not a hex digit.");
    }

    if (hex_str[1] >= '0' && hex_str[1] <= '9')
    {
        *num += (hex_str[1] - '0');
    }
    else if (hex_str[1] >= 'A' && hex_str[1] <= 'F')
    {
        *num += (hex_str[1] - 'A' + 10);
    }
    else if (hex_str[1] >= 'a' && hex_str[1] <= 'f')
    {
        *num += (hex_str[1] - 'a' + 10);
    }
    else
    {
        TEST_FAIL_MESSAGE("Not a hex digit.");
    }
}
void hex_to_data(const char* hex_str, uint8_t* data, size_t data_size)
{
    size_t i = 0;

    TEST_ASSERT_EQUAL_MESSAGE(data_size * 2, strlen(hex_str) - 1, "Hex string unexpected length.");

    for (i = 0; i < data_size; i++)
    {
        hex_to_uint8(&hex_str[i * 2], &data[i]);
    }
}
static int read_rsp_hex_value(FILE* file, const char* name, uint8_t* data, size_t data_size)
{
    char line[16384];
    char* str = NULL;
    size_t name_size = strlen(name);

    do
    {
        str = fgets(line, sizeof(line), file);
        if (str == NULL)
        {
            continue;
        }

        if (memcmp(line, name, name_size) == 0)
        {
            str = &line[name_size];
        }
        else
        {
            str = NULL;
        }
    }
    while (str == NULL && !feof(file));
    if (str == NULL)
    {
        return ATCA_GEN_FAIL;
    }
    hex_to_data(str, data, data_size);

    return ATCA_SUCCESS;
}
static int read_rsp_int_value(FILE* file, const char* name, int* value)
{
    char line[2048];
    char* str = NULL;
    size_t name_size = strlen(name);

    do
    {
        str = fgets(line, sizeof(line), file);
        if (str == NULL)
        {
            continue;
        }

        if (memcmp(line, name, name_size) == 0)
        {
            str = &line[name_size];
        }
        else
        {
            str = NULL;
        }
    }
    while (str == NULL && !feof(file));
    if (str == NULL)
    {
        return ATCA_GEN_FAIL;
    }
    *value = atoi(str);

    return ATCA_SUCCESS;
}
#endif
static void test_basic_hw_sha2_256_nist_simple(const char* filename)
{
    #ifndef _WIN32
    TEST_IGNORE_MESSAGE("Test only available under windows.");
    #else
    FILE* rsp_file = NULL;
    uint8_t md_ref[ATCA_SHA2_256_DIGEST_SIZE];
    uint8_t md[sizeof(md_ref)];
    int len_bits = 0;
    uint8_t* msg = NULL;
    size_t count = 0;
    ATCA_STATUS status;

    rsp_file = fopen(filename, "r");
    TEST_ASSERT_NOT_NULL_MESSAGE(rsp_file, "Failed to  open file");

    do
    {
        status = read_rsp_int_value(rsp_file, "Len = ", &len_bits);
        if (status != ATCA_SUCCESS)
        {
            continue;
        }

        msg = unity_malloc(len_bits == 0 ? 1 : len_bits / 8);
        TEST_ASSERT_NOT_NULL_MESSAGE(msg, "malloc failed");

        status = read_rsp_hex_value(rsp_file, "Msg = ", msg, len_bits == 0 ? 1 : len_bits / 8);
        TEST_ASSERT_EQUAL(ATCA_SUCCESS, status);

        status = read_rsp_hex_value(rsp_file, "MD = ", md_ref, sizeof(md_ref));
        TEST_ASSERT_EQUAL(ATCA_SUCCESS, status);

        status = atcab_sha(len_bits / 8, msg, md);
        TEST_ASSERT_EQUAL(ATCA_SUCCESS, status);
        TEST_ASSERT_EQUAL_MEMORY(md_ref, md, sizeof(md_ref));

        unity_free(msg);
        msg = NULL;
        count++;
    }
    while (status == ATCA_SUCCESS);
    TEST_ASSERT_MESSAGE(count > 0, "No long tests found in file.");
    #endif
}

TEST(atca_cmd_basic_test, sha2_256_nist_short)
{
    test_basic_hw_sha2_256_nist_simple("cryptoauthlib/test/sha-byte-test-vectors/SHA256ShortMsg.rsp");
}

TEST(atca_cmd_basic_test, sha2_256_nist_long)
{
    test_basic_hw_sha2_256_nist_simple("cryptoauthlib/test/sha-byte-test-vectors/SHA256LongMsg.rsp");
}

TEST(atca_cmd_basic_test, sha2_256_nist_monte)
{
    #ifndef _WIN32
    TEST_IGNORE_MESSAGE("Test only available under windows.");
    #else
    FILE* rsp_file = NULL;
    uint8_t seed[ATCA_SHA2_256_DIGEST_SIZE];
    uint8_t md[4][sizeof(seed)];
    int i, j;
    uint8_t m[sizeof(seed) * 3];
    uint8_t md_ref[sizeof(seed)];
    ATCA_STATUS status;

    rsp_file = fopen("cryptoauthlib/test/sha-byte-test-vectors/SHA256Monte.rsp", "r");
    TEST_ASSERT_NOT_EQUAL_MESSAGE(NULL, rsp_file, "Failed to  open sha-byte-test-vectors/SHA256Monte.rsp");

    // Find the seed value
    status = read_rsp_hex_value(rsp_file, "Seed = ", seed, sizeof(seed));
    TEST_ASSERT_EQUAL_MESSAGE(ATCA_SUCCESS, status, "Failed to find Seed value in file.");

    for (j = 0; j < 100; j++)
    {
        memcpy(&md[0], seed, sizeof(seed));
        memcpy(&md[1], seed, sizeof(seed));
        memcpy(&md[2], seed, sizeof(seed));
        for (i = 0; i < 1000; i++)
        {
            memcpy(m, md, sizeof(m));
            status = atcab_sha(sizeof(m), m, &md[3][0]);
            TEST_ASSERT_EQUAL_MESSAGE(ATCA_SUCCESS, status, "atcac_sw_sha1 failed");
            memmove(&md[0], &md[1], sizeof(seed) * 3);
        }
        status = read_rsp_hex_value(rsp_file, "MD = ", md_ref, sizeof(md_ref));
        TEST_ASSERT_EQUAL_MESSAGE(ATCA_SUCCESS, status, "Failed to find MD value in file.");
        TEST_ASSERT_EQUAL_MEMORY(md_ref, &md[2], sizeof(md_ref));
        memcpy(seed, &md[2], sizeof(seed));
    }
    #endif
}

TEST(atca_cmd_basic_test, sha_context)
{
    ATCA_STATUS status;
    uint16_t data_out_size = 0;
    uint16_t context_size;
    uint8_t context[SHA_CONTEXT_MAX_SIZE];
    uint8_t digest[ATCA_SHA_DIGEST_SIZE];
    uint8_t digest1[ATCA_SHA_DIGEST_SIZE];
    uint8_t digest2[ATCA_SHA_DIGEST_SIZE];
    uint8_t message[ATCA_SHA256_BLOCK_SIZE];

    uint8_t data_input[] = {
        0x01, 0x02, 0x03, 0x04, 0x05
    };

    /*	uint8_t expected_read_context_Data[] = {
       0x05, 0x00, 0x00, 0x00, 0x67, 0xE6, 0x09, 0x6A, 0X85, 0xAE, 0x67, 0xBB, 0x72, 0xF3, 0x6E, 0x3C,
       0x3A, 0xF5, 0x4F, 0xA5, 0x7F, 0x52, 0x0E, 0x51, 0X8C, 0x68, 0x05, 0x9B, 0xAB, 0xD9, 0x83, 0x1F,
       0x19, 0xCD, 0xE0, 0x5B, 0x01, 0x02, 0x03, 0x04, 0X05

       };

       uint8_t expected_digest[] = {
       0xE6, 0xD3, 0x21, 0x95, 0x02, 0x33, 0x65, 0xAD, 0X50, 0x3A, 0xB7, 0xE6, 0x70, 0xBE, 0x31, 0x4B,
       0x87, 0x22, 0xF6, 0x72, 0xA2, 0x61, 0x30, 0x03, 0X06, 0x8C, 0x28, 0xBA, 0x86, 0x47, 0x1E, 0x04
       };
     */
    //Calculating the digest for message data_input and reading the context
    status = atcab_sha_base(SHA_MODE_SHA256_START, 0, NULL, NULL, NULL);
    TEST_ASSERT_EQUAL(ATCA_SUCCESS, status);

    status = atcab_sha_base(SHA_MODE_SHA256_UPDATE, sizeof(data_input), data_input, NULL, NULL);
    TEST_ASSERT_EQUAL(ATCA_SUCCESS, status);

    context_size = sizeof(context);
    status = atcab_sha_read_context(context, &context_size); //Reading the context to use it later
    TEST_ASSERT_EQUAL(ATCA_SUCCESS, status);

    status = atcab_sha_base(SHA_MODE_SHA256_UPDATE, sizeof(data_input), data_input, NULL, NULL);
    TEST_ASSERT_EQUAL(ATCA_SUCCESS, status);

    data_out_size = sizeof(digest1);
    status = atcab_sha_base(SHA_MODE_SHA256_END | SHA_MODE_TARGET_OUT_ONLY, 0, NULL, digest1, &data_out_size);
    TEST_ASSERT_EQUAL(ATCA_SUCCESS, status);



    //Calculating the digest for another message
    memset(message, 0x5A, sizeof(message));
    status = atcab_sha_base(SHA_MODE_SHA256_START, 0, NULL, NULL, NULL);
    TEST_ASSERT_EQUAL(ATCA_SUCCESS, status);

    status = atcab_sha_base(SHA_MODE_SHA256_UPDATE, 64, message, NULL, NULL);
    TEST_ASSERT_EQUAL(ATCA_SUCCESS, status);

    status = atcab_sha_base(SHA_MODE_SHA256_UPDATE, 64, message, NULL, NULL);
    TEST_ASSERT_EQUAL(ATCA_SUCCESS, status);

    status = atcab_sha_base(SHA_MODE_SHA256_UPDATE, 64, message, NULL, NULL);
    TEST_ASSERT_EQUAL(ATCA_SUCCESS, status);

    data_out_size = sizeof(digest);
    status = atcab_sha_base(SHA_MODE_SHA256_END | SHA_MODE_TARGET_OUT_ONLY, 0, NULL, digest, &data_out_size);
    TEST_ASSERT_EQUAL(ATCA_SUCCESS, status);


    //Calculating the digest using the write context

    status = atcab_sha_write_context(context, context_size); //Write context the data, read from read context and comparing both the digest.
    TEST_ASSERT_EQUAL(ATCA_SUCCESS, status);

    status = atcab_sha_base(SHA_MODE_SHA256_UPDATE, sizeof(data_input), data_input, NULL, NULL);
    TEST_ASSERT_EQUAL(ATCA_SUCCESS, status);

    data_out_size = sizeof(digest2);
    status = atcab_sha_base(SHA_MODE_SHA256_END | SHA_MODE_TARGET_OUT_ONLY, 0, NULL, digest2, &data_out_size);
    TEST_ASSERT_EQUAL(ATCA_SUCCESS, status);

    TEST_ASSERT_EQUAL_MEMORY(digest1, digest2, ATCA_SHA_DIGEST_SIZE);

}


TEST(atca_cmd_basic_test, sha_hmac)
{
    ATCA_STATUS status = ATCA_GEN_FAIL;
    atca_hmac_sha256_ctx_t ctx;
    uint8_t hmac[ATCA_SHA_DIGEST_SIZE];
    uint8_t data_input[] = {
        0x01, 0x02, 0x03, 0x04, 0x05
    };
    const uint8_t hmac_ref[ATCA_SHA_DIGEST_SIZE] = {
        0xef, 0x3a, 0xcc, 0x04, 0xb9, 0x90, 0x36, 0x2f, 0x2e, 0x48, 0xac, 0x62, 0xd5, 0x4d, 0xf0, 0xbe,
        0x42, 0xe7, 0xd6, 0xd3, 0x86, 0x13, 0x91, 0x67, 0x73, 0x6e, 0x0a, 0xbe, 0x04, 0x48, 0x49, 0x11
    };

    uint16_t key_id = 4;

    test_assert_data_is_locked();


    //Calculating HMAC using the key in slot4
    status = atcab_sha_hmac_init(&ctx, key_id);
    TEST_ASSERT_EQUAL(ATCA_SUCCESS, status);

    status = atcab_sha_hmac_update(&ctx, data_input, sizeof(data_input));
    TEST_ASSERT_EQUAL(ATCA_SUCCESS, status);

    status = atcab_sha_hmac_finish(&ctx, hmac, SHA_MODE_TARGET_TEMPKEY);
    TEST_ASSERT_EQUAL(ATCA_SUCCESS, status);


    TEST_ASSERT_EQUAL_MEMORY(hmac_ref, hmac, ATCA_SHA_DIGEST_SIZE);
}

TEST(atca_cmd_basic_test, sha_hmac_tempkey)
{
    ATCA_STATUS status = ATCA_GEN_FAIL;
    atca_hmac_sha256_ctx_t ctx;
    uint8_t hmac[ATCA_SHA_DIGEST_SIZE];
    uint8_t data_input[] = {
        0x01, 0x02, 0x03, 0x04, 0x05
    };
    const uint8_t hmac_ref[ATCA_SHA_DIGEST_SIZE] = {
        0xef, 0x3a, 0xcc, 0x04, 0xb9, 0x90, 0x36, 0x2f, 0x2e, 0x48, 0xac, 0x62, 0xd5, 0x4d, 0xf0, 0xbe,
        0x42, 0xe7, 0xd6, 0xd3, 0x86, 0x13, 0x91, 0x67, 0x73, 0x6e, 0x0a, 0xbe, 0x04, 0x48, 0x49, 0x11
    };

    uint16_t key_id = ATCA_TEMPKEY_KEYID;

    test_assert_data_is_locked();

    // Load key into TempKey
    status = atcab_nonce_load(NONCE_MODE_TARGET_TEMPKEY, g_slot4_key, 32);
    TEST_ASSERT_EQUAL(ATCA_SUCCESS, status);

    //Calculating HMAC using the key in TempKey
    status = atcab_sha_hmac_init(&ctx, key_id);
    TEST_ASSERT_EQUAL(ATCA_SUCCESS, status);

    status = atcab_sha_hmac_update(&ctx, data_input, sizeof(data_input));
    TEST_ASSERT_EQUAL(ATCA_SUCCESS, status);

    status = atcab_sha_hmac_finish(&ctx, hmac, SHA_MODE_TARGET_OUT_ONLY);
    TEST_ASSERT_EQUAL(ATCA_SUCCESS, status);


    TEST_ASSERT_EQUAL_MEMORY(hmac_ref, hmac, ATCA_SHA_DIGEST_SIZE);
}

// *INDENT-OFF* - Preserve formatting
t_test_case_info sha_basic_test_info[] =
{
    { REGISTER_TEST_CASE(atca_cmd_basic_test, sha),                 DEVICE_MASK(ATSHA204A) | DEVICE_MASK(ATECC108A) | DEVICE_MASK(ATECC508A) | DEVICE_MASK(ATECC608A) },
    { REGISTER_TEST_CASE(atca_cmd_basic_test, sha_long),            DEVICE_MASK(ATSHA204A) | DEVICE_MASK(ATECC108A) | DEVICE_MASK(ATECC508A) | DEVICE_MASK(ATECC608A) },
    { REGISTER_TEST_CASE(atca_cmd_basic_test, sha_short),           DEVICE_MASK(ATSHA204A) | DEVICE_MASK(ATECC108A) | DEVICE_MASK(ATECC508A) | DEVICE_MASK(ATECC608A) },
    { REGISTER_TEST_CASE(atca_cmd_basic_test, sha2_256_nist1),      DEVICE_MASK(ATSHA204A) | DEVICE_MASK(ATECC108A) | DEVICE_MASK(ATECC508A) | DEVICE_MASK(ATECC608A) },
    { REGISTER_TEST_CASE(atca_cmd_basic_test, sha2_256_nist2),      DEVICE_MASK(ATSHA204A) | DEVICE_MASK(ATECC108A) | DEVICE_MASK(ATECC508A) | DEVICE_MASK(ATECC608A) },
    { REGISTER_TEST_CASE(atca_cmd_basic_test, sha2_256_nist_short), DEVICE_MASK(ATSHA204A) | DEVICE_MASK(ATECC108A) | DEVICE_MASK(ATECC508A) | DEVICE_MASK(ATECC608A) },
    //{ REGISTER_TEST_CASE(atca_cmd_basic_test, sha2_256_nist_long),  DEVICE_MASK(ATSHA204A) | DEVICE_MASK(ATECC108A) | DEVICE_MASK(ATECC508A) | DEVICE_MASK(ATECC608A) },
    //{ REGISTER_TEST_CASE(atca_cmd_basic_test, sha2_256_nist_monte), DEVICE_MASK(ATSHA204A) | DEVICE_MASK(ATECC108A) | DEVICE_MASK(ATECC508A) | DEVICE_MASK(ATECC608A) },
    { REGISTER_TEST_CASE(atca_cmd_basic_test, sha_context),                                                                                    DEVICE_MASK(ATECC608A) },
    { REGISTER_TEST_CASE(atca_cmd_basic_test, sha_hmac),                                                                                       DEVICE_MASK(ATECC608A) },
    { REGISTER_TEST_CASE(atca_cmd_basic_test, sha_hmac_tempkey),                                                                               DEVICE_MASK(ATECC608A) },
    { (fp_test_case)NULL,                     (uint8_t)0 },         /* Array Termination element*/
};

t_test_case_info sha_unit_test_info[] =
{
    { REGISTER_TEST_CASE(atca_cmd_unit_test, sha), DEVICE_MASK(ATSHA204A) | DEVICE_MASK(ATECC108A) | DEVICE_MASK(ATECC508A) | DEVICE_MASK(ATECC608A) },
    { (fp_test_case)NULL,                    (uint8_t)0 },/* Array Termination element*/
};
// *INDENT-ON*

