/* Copyright 2018-2020 Google LLC
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

#include <stddef.h>

/**
 * IOTC TinyTest extension macros to ease usage of TinyTest framework.
 * Twofolded precompiler run is used to define the test functions (test cases)
 * and the TT test group structure.
 */
#ifdef IOTC_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN  // TT test group
                                                            // structure
                                                            // generation mode

#undef IOTC_TT_TESTGROUP_BEGIN
#define IOTC_TT_TESTGROUP_BEGIN(testgroupname) \
  struct testcase_t testgroupname[] = {
#undef IOTC_TT_TESTGROUP_END
#define IOTC_TT_TESTGROUP_END \
  END_OF_TESTCASES            \
  }                           \
  ;

#undef IOTC_TT_TESTCASE
#define IOTC_TT_TESTCASE(testcasename, ...) \
  {#testcasename, testcasename, TT_ENABLED_, 0, 0},

#undef IOTC_TT_TESTCASE_WITH_SETUP
#define IOTC_TT_TESTCASE_WITH_SETUP(testcasename, setupfnname, cleanfnname, \
                                    setup_data, ...)                        \
  {#testcasename, testcasename, TT_ENABLED_,                                \
   &testcasename##setupfnname##cleanfnname, setup_data},

#undef SKIP_IOTC_TT_TESTCASE
#define SKIP_IOTC_TT_TESTCASE(testcasename, ...) \
  {#testcasename, testcasename, TT_SKIP, 0, 0},

#else  // IOTC_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN, TT test case
       // definition mode

#undef IOTC_TT_TESTGROUP_BEGIN
#define IOTC_TT_TESTGROUP_BEGIN(testgroupname)

#undef IOTC_TT_TESTGROUP_END
#define IOTC_TT_TESTGROUP_END

#undef IOTC_TT_TESTCASE
#define IOTC_TT_TESTCASE(testcasename, ...) void testcasename() __VA_ARGS__

#undef IOTC_TT_TESTCASE_WITH_SETUP
#define IOTC_TT_TESTCASE_WITH_SETUP(testcasename, setupfnname, cleanfnname, \
                                    setup_data, ...)                        \
                                                                            \
  void testcasename() __VA_ARGS__;                                          \
                                                                            \
  const struct testcase_setup_t testcasename##setupfnname##cleanfnname = {  \
      setupfnname, cleanfnname};

#undef SKIP_IOTC_TT_TESTCASE
#define SKIP_IOTC_TT_TESTCASE(testcasename, ...) void testcasename() __VA_ARGS__

#endif  // IOTC_TT_TESTCASE_ENUMERATION

extern char iotc_test_load_level;
