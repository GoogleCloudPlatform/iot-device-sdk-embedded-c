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

#define IOTC_MOCK_TEST_PREPROCESSOR_RUN
#include "iotc_itest_clean_session.h"
#include "iotc_itest_connect_error.h"
#include "iotc_itest_tls_error.h"
#ifndef IOTC_NO_TLS_LAYER
#include "iotc_itest_tls_layer.h"
#endif
#include "iotc_itest_mqtt_keepalive.h"
#include "iotc_itest_mqttlogic_layer.h"
#undef IOTC_MOCK_TEST_PREPROCESSOR_RUN

#include "iotc_lamp_communication.h"
#include "iotc_test_utils.h"

struct CMGroupTest groups[] = {cmocka_test_group(iotc_itests_clean_session),
                               cmocka_test_group(iotc_itests_tls_error),
#ifndef IOTC_NO_TLS_LAYER
                               cmocka_test_group(iotc_itests_tls_layer),
#endif
                               cmocka_test_group(iotc_itests_mqttlogic_layer),
                               cmocka_test_group(iotc_itests_connect_error),
                               cmocka_test_group(iotc_itests_mqtt_keepalive),
                               cmocka_test_group_end};

int8_t iotc_cm_strict_mock = 0;

#ifndef IOTC_EMBEDDED_TESTS
int main(int argc, char const* argv[])
#else
int iotc_itests_main(int argc, char const* argv[])
#endif
{
  iotc_test_init(argc, argv);

  int i;
  char host_name[2048];
	uint16_t port_val = 0;

  iotc_test_init(argc, argv);

  	for (i=1; i<argc; ++i) {
		if (argv[i][0] == '-') {
			if (!strcmp(argv[i], "--hostURL")) {
				i++;
				memcpy(host_name, argv[i], strlen(argv[i]));
			} else if (!strcmp(argv[i], "--hostPort")) {
				i++;
				port_val = atoi(argv[i]);
			}
		}
	}
	if(port_val > 0){
		/* caller supplied custom hostURL and port number  */
		setHostNameAndPort(host_name, port_val);
	}
  
    // report test start
  iotc_test_report_result(
      iotc_test_load_level ? "iotc_itest_id_l1" : "iotc_itest_id_l0",
      iotc_test_load_level ? "xi1" : "xi", 1, 0);

  const int number_of_failures = cmocka_run_test_groups(groups);

  // report test start
  iotc_test_report_result(
      iotc_test_load_level ? "iotc_itest_id_l1" : "iotc_itest_id_l0",
      iotc_test_load_level ? "xi1" : "xi", 0, number_of_failures);

  return number_of_failures;
}
