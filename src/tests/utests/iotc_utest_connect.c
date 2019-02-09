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

#include "iotc_tt_testcase_management.h"
#include "iotc_utest_basic_testcase_frame.h"
#include "tinytest.h"
#include "tinytest_macros.h"

#include "iotc.h"

#include <stdio.h>

#ifndef IOTC_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN

extern void iotc_default_client_callback(
    iotc_context_handle_t in_context_handle, void* data, iotc_state_t state);

#endif

IOTC_TT_TESTGROUP_BEGIN(utest_connect)

IOTC_TT_TESTCASE_WITH_SETUP(test_INVALID_connect_context,
                            iotc_utest_setup_basic, iotc_utest_teardown_basic,
                            NULL, {
                              iotc_state_t state;

                              /* Call iotc_connect_with_callback before creating
                               * a context. */
                              iotc_context_handle_t iotc_invalid_context = -1;

                              state = iotc_connect(
                                  iotc_invalid_context, "test_username",
                                  "test_password", "test_client_id",
                                  /*connection_timeout=*/10,
                                  /*keepalive_timeout=*/20,
                                  &iotc_default_client_callback);

                              tt_assert(IOTC_NULL_CONTEXT == state);
                            end:;
                            })

IOTC_TT_TESTCASE_WITH_SETUP(test_VALID_connect_context, iotc_utest_setup_basic,
                            iotc_utest_teardown_basic, NULL, {
                              iotc_state_t state;

                              /* Call iotc_connect_with_callback after creating
                               * a context. */
                              iotc_context_handle_t iotc_context =
                                  iotc_create_context();
                              tt_assert(0 <= iotc_context);

                              state = iotc_connect(
                                  iotc_context, "test_username",
                                  "test_password", "test_client_id",
                                  /*connection_timeout=*/10,
                                  /*keepalive_timeout=*/20,
                                  &iotc_default_client_callback);

                              tt_assert(IOTC_STATE_OK == state);
                              iotc_delete_context(iotc_context);
                            end:;
                            })

IOTC_TT_TESTCASE_WITH_SETUP(test_connect_to__valid_host, iotc_utest_setup_basic,
                            iotc_utest_teardown_basic, NULL, {
                              iotc_state_t state;

                              /* Call iotc_connect_with_callback after creating
                               * a context. */
                              iotc_context_handle_t iotc_context =
                                  iotc_create_context();
                              tt_assert(0 <= iotc_context);

                              state = iotc_connect_to(
                                  iotc_context, /*host=*/"random.host.com",
                                  /*port=*/12345, "test_username",
                                  "test_password", "test_client_id",
                                  /*connection_timeout=*/10,
                                  /*keepalive_timeout=*/20,
                                  &iotc_default_client_callback);

                              tt_assert(IOTC_STATE_OK == state);
                              iotc_delete_context(iotc_context);
                            end:;
                            })

IOTC_TT_TESTCASE_WITH_SETUP(
    test_connect_to__invalid_host, iotc_utest_setup_basic,
    iotc_utest_teardown_basic, NULL, {
      iotc_state_t state;

      /* Call iotc_connect_with_callback after creating a context. */
      iotc_context_handle_t iotc_context = iotc_create_context();
      tt_assert(0 <= iotc_context);

      state = iotc_connect_to(
          iotc_context, /*host=*/NULL, /*port=*/1234, "test_username",
          "test_password", "test_client_id", /*connection_timeout=*/10,
          /*keepalive_timeout=*/20, &iotc_default_client_callback);

      tt_assert(IOTC_NULL_HOST == state);
      iotc_delete_context(iotc_context);
    end:;
    })

IOTC_TT_TESTCASE_WITH_SETUP(test_INVALID_is_context_connected_context,
                            iotc_utest_setup_basic, iotc_utest_teardown_basic,
                            NULL, {
                              uint8_t is_connected = 0;

                              /* Call iotc_is_context_connected before creating
                               * a context. */
                              iotc_context_handle_t iotc_invalid_context = -1;

                              is_connected = iotc_is_context_connected(
                                  iotc_invalid_context);

                              tt_assert(0 == is_connected);
                            end:;
                            })

IOTC_TT_TESTCASE_WITH_SETUP(
    test_VALID_is_context_connected_context, iotc_utest_setup_basic,
    iotc_utest_teardown_basic, NULL, {
      uint8_t is_connected = 0;

      /* Call iotc_is_context_connected after creating a context. */
      iotc_context_handle_t iotc_context = iotc_create_context();
      tt_assert(IOTC_INVALID_CONTEXT_HANDLE < iotc_context);

      is_connected = iotc_is_context_connected(iotc_context);

      tt_assert(0 == is_connected);
      iotc_delete_context(iotc_context);
    end:;
    })

IOTC_TT_TESTCASE_WITH_SETUP(
    test_connect__null_username, iotc_utest_setup_basic,
    iotc_utest_teardown_basic, NULL, {
      iotc_state_t state;

      /* Call iotc_is_context_connected after creating a context. */
      iotc_context_handle_t iotc_context = iotc_create_context();
      tt_assert(IOTC_INVALID_CONTEXT_HANDLE < iotc_context);
      state = iotc_connect(
          iotc_context,  /*username=*/NULL, "test_password", "test_client_id",
          /*connection_timeout=*/10, /*keepalive_timeout=*/20,
          &iotc_default_client_callback);
      tt_assert(IOTC_STATE_OK == state);
      iotc_delete_context(iotc_context);
    end:;
    })

IOTC_TT_TESTCASE_WITH_SETUP(
    test_connect__null_password, iotc_utest_setup_basic,
    iotc_utest_teardown_basic, NULL, {
      iotc_state_t state;

      /* Call iotc_is_context_connected after creating a context. */
      iotc_context_handle_t iotc_context = iotc_create_context();
      tt_assert(IOTC_INVALID_CONTEXT_HANDLE < iotc_context);
      state = iotc_connect(
          iotc_context, "test_username", /*password=*/NULL, "test_client_id",
          /*connection_timeout=*/10, /*keepalive_timeout=*/20,
          &iotc_default_client_callback);
      tt_assert(IOTC_STATE_OK == state);
      iotc_delete_context(iotc_context);
    end:;
    })

IOTC_TT_TESTCASE_WITH_SETUP(
    test_connect_to__null_username, iotc_utest_setup_basic,
    iotc_utest_teardown_basic, NULL, {
      iotc_state_t state;

      /* Call iotc_is_context_connected after creating a context. */
      iotc_context_handle_t iotc_context = iotc_create_context();
      tt_assert(IOTC_INVALID_CONTEXT_HANDLE < iotc_context);
      state = iotc_connect_to(
          iotc_context, /*host=*/"random.host.com", /*port=*/12345,
          /*username=*/NULL, "test_password", "test_client_id",
          /*connection_timeout=*/10, /*keepalive_timeout=*/20,
          &iotc_default_client_callback);
      tt_assert(IOTC_STATE_OK == state);
      iotc_delete_context(iotc_context);
    end:;
    })

IOTC_TT_TESTCASE_WITH_SETUP(
    test_connect_to__null_password, iotc_utest_setup_basic,
    iotc_utest_teardown_basic, NULL, {
      iotc_state_t state;

      /* Call iotc_is_context_connected after creating a context. */
      iotc_context_handle_t iotc_context = iotc_create_context();
      tt_assert(IOTC_INVALID_CONTEXT_HANDLE < iotc_context);
      state = iotc_connect_to(
          iotc_context, /*host=*/"random.host.com", /*port=*/12345,
          "test_username", /*password=*/NULL, "test_client_id",
          /*connection_timeout=*/10, /*keepalive_timeout=*/20,
          &iotc_default_client_callback);
      tt_assert(IOTC_STATE_OK == state);
      iotc_delete_context(iotc_context);
    end:;
    })

IOTC_TT_TESTCASE_WITH_SETUP(
    test_INVALID_connect__null_client_id,
    iotc_utest_setup_basic,
    iotc_utest_teardown_basic, NULL, {
        iotc_state_t state;

        /* Call iotc_connect_with_callback after creating
        * a context. */
        iotc_context_handle_t iotc_context =
            iotc_create_context();
        tt_assert(0 <= iotc_context);

        state = iotc_connect(
            iotc_context, "test_username", "test_password", /*client_id=*/NULL,
            /*connection_timeout=*/10, /*keepalive_timeout=*/20,
            &iotc_default_client_callback);

        tt_assert(IOTC_NULL_CLIENT_ID_ERROR == state);
        iotc_delete_context(iotc_context);
    end:;
    })

IOTC_TT_TESTCASE_WITH_SETUP(
    test_INVALID_connect_to__null_client_id,
    iotc_utest_setup_basic,
    iotc_utest_teardown_basic, NULL, {
        iotc_state_t state;

        /* Call iotc_connect_with_callback after creating
        * a context. */
        iotc_context_handle_t iotc_context =
            iotc_create_context();
        tt_assert(0 <= iotc_context);

        state = iotc_connect_to(
            iotc_context, /*host=*/"random.host.com", /*port=*/12345,
            "test_username", "test_password", /*client_id=*/NULL,
            /*connection_timeout=*/10, /*keepalive_timeout=*/20,
            &iotc_default_client_callback);

        tt_assert(IOTC_NULL_CLIENT_ID_ERROR == state);
        iotc_delete_context(iotc_context);
    end:;
    })

IOTC_TT_TESTGROUP_END

#ifndef IOTC_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#define IOTC_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#include __FILE__
#undef IOTC_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#endif
