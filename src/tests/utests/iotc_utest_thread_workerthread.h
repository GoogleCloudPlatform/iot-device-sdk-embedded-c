/* Copyright 2018-2019 Google LLC
 *
 * This is part of the Google Cloud IoT Device SDK for Embedded C.
 * It is licensed under the BSD 3-Clause license; you may not use this file
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
#include "tinytest.h"
#include "tinytest_macros.h"

#include "iotc_thread_workerthread.h"

#ifndef IOTC_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN

#endif  // IOTC_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN

IOTC_TT_TESTGROUP_BEGIN(utest_thread_workerthread)

#include "iotc_utest_platform_thread_workerthread.h"

IOTC_TT_TESTCASE(
    utest__iotc_workerthread_create_and_destroy_instance__no_input__not_null_output,
    {
      struct iotc_workerthread_s* new_workerthread_instance =
          iotc_workerthread_create_instance(NULL);

      tt_ptr_op(NULL, !=, new_workerthread_instance);

      iotc_workerthread_destroy_instance(&new_workerthread_instance);

      tt_ptr_op(NULL, ==, new_workerthread_instance);

    end:
      return;
    })

IOTC_TT_TESTGROUP_END

#ifndef IOTC_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#define IOTC_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#include __FILE__
#undef IOTC_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#endif  // IOTC_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
