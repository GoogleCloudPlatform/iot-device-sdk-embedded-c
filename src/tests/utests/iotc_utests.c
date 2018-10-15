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

#include "tinytest.h"
#include "tinytest_macros.h"

/* This block of code below must not be clang-formatted */
// clang-format off

/* Define a single bit for each test group. */
#define IOTC_TT_CORE                              ( 1 )
#define IOTC_TT_CONNECT                           ( IOTC_TT_CORE << 1 )
#define IOTC_TT_EVENT_DISPATCHER                  ( IOTC_TT_CONNECT << 1 )
#define IOTC_TT_EVENT_DISPATCHER_TIMED            ( IOTC_TT_EVENT_DISPATCHER << 1 )
#define IOTC_TT_DATASTRUCTURES                    ( IOTC_TT_EVENT_DISPATCHER_TIMED << 1 )
#define IOTC_TT_DATA_DESC                         ( IOTC_TT_DATASTRUCTURES << 1 )
#define IOTC_TT_BACKOFF                           ( IOTC_TT_DATA_DESC << 1 )
#define IOTC_TT_MQTT_CTORS_DTORS                  ( IOTC_TT_BACKOFF << 1 )
#define IOTC_TT_MQTT_PARSER                       ( IOTC_TT_MQTT_CTORS_DTORS << 1 )
#define IOTC_TT_MQTT_LOGIC_LAYER_SUBSCRIBE        ( IOTC_TT_MQTT_PARSER << 1 )
#define IOTC_TT_CONTROL_TOPIC                     ( IOTC_TT_MQTT_LOGIC_LAYER_SUBSCRIBE << 1 )
#define IOTC_TT_MQTT_SERIALIZER                   ( IOTC_TT_CONTROL_TOPIC << 1 )
#define IOTC_TT_MEMORY_LIMITER                    ( IOTC_TT_MQTT_SERIALIZER << 1 )
#define IOTC_TT_THREAD                            ( IOTC_TT_MEMORY_LIMITER << 1 )
#define IOTC_TT_THREAD_WORKERTHREAD               ( IOTC_TT_THREAD << 1 )
#define IOTC_TT_THREAD_THREADPOOL                 ( IOTC_TT_THREAD_WORKERTHREAD << 1 )
#define IOTC_TT_HELPERS                           ( IOTC_TT_THREAD_THREADPOOL << 1 )
#define IOTC_TT_MQTT_CODEC_LAYER_DATA             ( IOTC_TT_HELPERS << 1 )
#define IOTC_TT_PUBLISH                           ( IOTC_TT_MQTT_CODEC_LAYER_DATA << 1 )
#define IOTC_TT_FS                                ( IOTC_TT_PUBLISH << 1 )
#define IOTC_TT_RESOURCE_MANAGER                  ( IOTC_TT_FS << 1 )
#define IOTC_TT_IO_LAYER                          ( IOTC_TT_RESOURCE_MANAGER << 1 )
#define IOTC_TT_TIME_EVENT                        ( IOTC_TT_IO_LAYER << 1 )

// clang-format on

/* If no test set is defined, run all tests */
#ifndef IOTC_TT_TEST_SET
#define IOTC_TT_TEST_SET (~0)
#endif

#define IOTC_TT_TESTCASE_PREDECLARATION(testgroupname) \
  extern struct testcase_t testgroupname[]

// test groups
IOTC_TT_TESTCASE_PREDECLARATION(utest_core);
IOTC_TT_TESTCASE_PREDECLARATION(utest_connect);
IOTC_TT_TESTCASE_PREDECLARATION(utest_event_dispatcher);
IOTC_TT_TESTCASE_PREDECLARATION(utest_event_dispatcher_timed);
IOTC_TT_TESTCASE_PREDECLARATION(utest_datastructures);
IOTC_TT_TESTCASE_PREDECLARATION(utest_list);
IOTC_TT_TESTCASE_PREDECLARATION(utest_data_desc);
IOTC_TT_TESTCASE_PREDECLARATION(utest_backoff);
IOTC_TT_TESTCASE_PREDECLARATION(utest_mqtt_ctors_dtors);
IOTC_TT_TESTCASE_PREDECLARATION(utest_mqtt_parser);
IOTC_TT_TESTCASE_PREDECLARATION(utest_mqtt_logic_layer_subscribe);
IOTC_TT_TESTCASE_PREDECLARATION(utest_mqtt_codec_layer_data);
IOTC_TT_TESTCASE_PREDECLARATION(utest_publish);
IOTC_TT_TESTCASE_PREDECLARATION(utest_helpers);
IOTC_TT_TESTCASE_PREDECLARATION(utest_helper_functions);
IOTC_TT_TESTCASE_PREDECLARATION(utest_mqtt_serializer);
IOTC_TT_TESTCASE_PREDECLARATION(utest_handle);
IOTC_TT_TESTCASE_PREDECLARATION(utest_timed_task);

#ifndef IOTC_NO_TLS_LAYER
IOTC_TT_TESTCASE_PREDECLARATION(utest_bsp_crypto_base64);
IOTC_TT_TESTCASE_PREDECLARATION(utest_bsp_crypto_sha256);
IOTC_TT_TESTCASE_PREDECLARATION(utest_bsp_crypto_ecc);
IOTC_TT_TESTCASE_PREDECLARATION(utest_jwt);
#ifdef IOTC_LIBCRYPTO_AVAILABLE
IOTC_TT_TESTCASE_PREDECLARATION(utest_jwt_openssl_validation);
#endif
#endif

#ifdef IOTC_MEMORY_LIMITER_ENABLED
IOTC_TT_TESTCASE_PREDECLARATION(utest_memory_limiter);
#endif

IOTC_TT_TESTCASE_PREDECLARATION(utest_rng);

#ifdef IOTC_MODULE_THREAD_ENABLED
#include "iotc_utest_thread.h"
#include "iotc_utest_thread_workerthread.h"
#include "iotc_utest_thread_threadpool.h"
#endif

IOTC_TT_TESTCASE_PREDECLARATION(utest_resource_manager);

IOTC_TT_TESTCASE_PREDECLARATION(utest_fs);

#ifdef IOTC_FS_MEMORY
IOTC_TT_TESTCASE_PREDECLARATION(utest_fs_memory);
#endif

#ifdef IOTC_FS_DUMMY
IOTC_TT_TESTCASE_PREDECLARATION(utest_fs_dummy);
#endif

#ifdef IOTC_FS_POSIX
IOTC_TT_TESTCASE_PREDECLARATION(utest_fs_posix);
#endif

IOTC_TT_TESTCASE_PREDECLARATION(utest_time_event);

#include "iotc_test_utils.h"
#include "iotc_lamp_communication.h"

/* Make an array of testgroups. This is mandatory. Unlike more
   heavy-duty testing frameworks, groups can't nest. */
struct testgroup_t groups[] = {
/* Every group has a 'prefix', and an array of tests.  That's it. */

#if (IOTC_TT_TEST_SET & IOTC_TT_CORE)
    {"utest_core - ", utest_core},
#endif

#if (IOTC_TT_TEST_SET & IOTC_TT_CONNECT)
    {"utest_connect - ", utest_connect},
#endif

#if (IOTC_TT_TEST_SET & IOTC_TT_DATASTRUCTURES)
    {"utest_datastructures - ", utest_datastructures},
    {"utest_list - ", utest_list},
#endif

#if (IOTC_TT_TEST_SET & IOTC_TT_EVENT_DISPATCHER)
    {"utest_event_dispatcher - ", utest_event_dispatcher},
#endif

#if (IOTC_TT_TEST_SET & IOTC_TT_EVENT_DISPATCHER_TIMED)
    {"utest_event_dispatcher_timed - ", utest_event_dispatcher_timed},
#endif

#if (IOTC_TT_TEST_SET & IOTC_TT_IO_FILE)
    {"utest_io_file - ", utest_io_file},
#endif

#if (IOTC_TT_TEST_SET & IOTC_TT_DEVICE_CREDENTIALS)
    {"utest_device_credentials - ", utest_device_credentials},
#endif

#if (IOTC_TT_TEST_SET & IOTC_TT_DATA_DESC)
    {"utest_data_desc - ", utest_data_desc},
#endif

#if (IOTC_TT_TEST_SET & IOTC_TT_BACKOFF)
    {"utest_backoff - ", utest_backoff},
#endif

#if (IOTC_TT_TEST_SET & IOTC_TT_MQTT_CTORS_DTORS)
    {"utest_mqtt_ctors_dtors - ", utest_mqtt_ctors_dtors},
#endif

#if (IOTC_TT_TEST_SET & IOTC_TT_MQTT_PARSER)
    {"utest_mqtt_parser - ", utest_mqtt_parser},
#endif

#if (IOTC_TT_TEST_SET & IOTC_TT_MQTT_LOGIC_LAYER_SUBSCRIBE)
    {"utest_mqtt_logic_layer_subscribe - ", utest_mqtt_logic_layer_subscribe},
#endif

#if (IOTC_TT_TEST_SET & IOTC_TT_PUBLISH)
    {"utest_publish - ", utest_publish},
#endif

#if (IOTC_TT_TEST_SET & IOTC_TT_MQTT_SERIALIZER)
    {"utest_mqtt_serializer - ", utest_mqtt_serializer},
#endif

#ifdef IOTC_MEMORY_LIMITER_ENABLED
#if (IOTC_TT_TEST_SET & IOTC_TT_MEMORY_LIMITER)
    {"utest_memory_limiter - ", utest_memory_limiter},
#endif
#endif

#ifdef IOTC_MODULE_THREAD_ENABLED
#if (IOTC_TT_TEST_SET & IOTC_TT_THREAD)
    {"utest_thread - ", utest_thread},
#endif

#if (IOTC_TT_TEST_SET & IOTC_TT_THREAD_WORKERTHREAD)
    {"utest_thread_workerthread - ", utest_thread_workerthread},
#endif

#if (IOTC_TT_TEST_SET & IOTC_TT_THREAD_THREADPOOL)
    {"utest_thread_threadpool - ", utest_thread_threadpool},
#endif
#endif

#if (IOTC_TT_TEST_SET & IOTC_TT_HELPERS)
    {"utest_helpers - ", utest_helpers},
    {"utest_helper_functions - ", utest_helper_functions},
#endif

    {"utest_handle - ", utest_handle},

    {"utest_timed_task - ", utest_timed_task},

#if (IOTC_TT_TEST_SET & IOTC_TT_MQTT_CODEC_LAYER_DATA)
    {"utest_mqtt_codec_layer_data - ", utest_mqtt_codec_layer_data},
#endif

#if (IOTC_TT_TEST_SET & IOTC_TT_FS)
    {"utest_fs - ", utest_fs},
#ifdef IOTC_FS_DUMMY
    {"utest_fs_dummy - ", utest_fs_dummy},
#endif
#ifdef IOTC_FS_MEMORY
    {"utest_fs_memory - ", utest_fs_memory},
#endif
#ifdef IOTC_FS_POSIX
    {"utest_fs_posix - ", utest_fs_posix},
#endif
#endif

#if (IOTC_TT_TEST_SET & IOTC_TT_RESOURCE_MANAGER)
    {"utest_resource_manager - ", utest_resource_manager},
#endif

#if (IOTC_TT_TEST_SET & IOTC_TT_TIME_EVENT)
    {"utest_time_event - ", utest_time_event},
#endif

    {"utest_rng - ", utest_rng},

#ifndef IOTC_NO_TLS_LAYER
    {"utest_bsp_crypto_base64 - ", utest_bsp_crypto_base64},
    {"utest_bsp_crypto_sha256 - ", utest_bsp_crypto_sha256},
    {"utest_bsp_crypto_ecc - ", utest_bsp_crypto_ecc},
    {"utest_jwt - ", utest_jwt},
#ifdef IOTC_LIBCRYPTO_AVAILABLE
    {"utest_jwt_openssl_validation - ", utest_jwt_openssl_validation},
#endif
#endif

    END_OF_GROUPS};

#ifndef IOTC_EMBEDDED_TESTS
int main(int argc, char const* argv[])
#else
int iotc_utests_main(int argc, char const* argv[])
#endif
{
  iotc_test_init(argc, argv);

  // report test start
  iotc_test_report_result(
      iotc_test_load_level ? "iotc_utest_id_l1" : "iotc_utest_id_l0",
      iotc_test_load_level ? "iotcu1" : "iotcu", 1, 0);

  // run all tests
  const int number_of_failures = tinytest_main(argc, argv, groups);
  // printf( "tinttest_main return value = %i\n", number_of_failures );

  // report test finish + result
  iotc_test_report_result(
      iotc_test_load_level ? "iotc_utest_id_l1" : "iotc_utest_id_l0",
      iotc_test_load_level ? "iotcu1" : "iotcu", 0, number_of_failures);

  return number_of_failures;
}
