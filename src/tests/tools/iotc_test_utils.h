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

#ifndef __IOTC_TEST_UTILS_H__
#define __IOTC_TEST_UTILS_H__

char iotc_test_load_level = 0;

#ifndef IOTC_EMBEDDED_TESTS
/* this is platform specific code */
#include <unistd.h>
#endif /* not IOTC_EMBEDDED_TESTS */

void iotc_test_init(int argc, char const* argv[]) {
#ifndef IOTC_EMBEDDED_TESTS
  int result_getopt = 0;

  /* this is platform specific code */
  opterr = 0;
  while ((result_getopt = getopt(argc, (char* const*)argv, "l:")) != -1) {
    switch (result_getopt) {
      case 'l':
        iotc_test_load_level = *optarg - '0';
        break;
    }
  }

// printf("*** iotc_test_load_level = %d\n", iotc_test_load_level);
#endif /* not IOTC_EMBEDDED_TESTS */
}

#endif /* __IOTC_TEST_UTILS_H__ */
