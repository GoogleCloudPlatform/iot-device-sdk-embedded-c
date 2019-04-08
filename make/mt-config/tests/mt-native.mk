# Copyright 2018-2019 Google LLC
#
# This is part of the Google Cloud IoT Device SDK for Embedded C.
# It is licensed under the BSD 3-Clause license; you may not use this file
# except in compliance with the License.
#
# You may obtain a copy of the License at:
#  https://opensource.org/licenses/BSD-3-Clause
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

IOTC_FTEST_MAX_TOTAL_TIME ?= 10
IOTC_FTEST_MAX_LEN ?= 256

#IOTC_RUN_UTESTS = $(IOTC_UTESTS) -l0 --terse
IOTC_RUN_UTESTS := (cd $(dir $(IOTC_UTESTS)) && LD_LIBRARY_PATH=$(dir $(XI)):$$LD_LIBRARY_PATH exec $(IOTC_UTESTS) -l0)
IOTC_RUN_ITESTS := (cd $(dir $(IOTC_ITESTS)) && LD_LIBRARY_PATH=$(dir $(XI)):$$LD_LIBRARY_PATH exec $(IOTC_ITESTS))
IOTC_RUN_FUZZ_TEST = (cd $(IOTC_FUZZ_TESTS_BINDIR) && $(1) $(IOTC_FUZZ_TESTS_CORPUS_DIR)/$(notdir $(1))/ -max_total_time=$(IOTC_FTEST_MAX_TOTAL_TIME) -max_len=$(IOTC_FTEST_MAX_LEN));
IOTC_RUN_GTESTS := (cd $(dir $(IOTC_ITESTS)) && LD_LIBRARY_PATH=$(dir $(XI)):$$LD_LIBRARY_PATH exec $(IOTC_GTESTS))
