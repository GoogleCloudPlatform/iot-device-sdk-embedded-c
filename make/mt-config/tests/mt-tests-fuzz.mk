# Copyright 2018 Google LLC
#
# This is part of the Google Cloud IoT Edge Embedded C Client,
# it is licensed under the BSD 3-Clause license; you may not use this file
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

include make/mt-config/tests/mt-tests.mk

IOTC_FUZZ_TESTS_BINDIR := $(IOTC_TEST_BINDIR)/fuzztests
IOTC_FUZZ_TESTS_OBJDIR := $(IOTC_TEST_OBJDIR)/fuzztests
IOTC_FUZZ_TESTS_CFLAGS := $(IOTC_CONFIG_FLAGS)

IOTC_FUZZ_TESTS_SOURCE_DIR := $(IOTC_TEST_DIR)/fuzztests
IOTC_FUZZ_TESTS_SOURCES := $(wildcard $(IOTC_FUZZ_TESTS_SOURCE_DIR)/*.cpp)
IOTC_FUZZ_TESTS := $(foreach fuzztest,$(IOTC_FUZZ_TESTS_SOURCES),$(notdir $(fuzztest)))
IOTC_FUZZ_TESTS := $(IOTC_FUZZ_TESTS:.cpp=)
IOTC_FUZZ_TESTS_CORPUS_DIR := $(IOTC_FUZZ_TESTS_SOURCE_DIR)/corpuses
IOTC_FUZZ_TESTS_CORPUS_DIRS := $(foreach fuzztest, $(IOTC_FUZZ_TESTS), $(IOTC_FUZZ_TESTS_CORPUS_DIR)/$(fuzztest))
IOTC_FUZZ_TESTS := $(foreach fuzztest, $(IOTC_FUZZ_TESTS), $(IOTC_FUZZ_TESTS_BINDIR)/$(fuzztest))

IOTC_FUZZ_TEST_LIBRARY := -lFuzzer

#### =========================================================

IOTC_CLANG_TOOLS_DIR := $(LIBIOTC)/src/import/clang_tools

IOTC_LIBFUZZER_URL := https://llvm.org/svn/llvm-project/compiler-rt/trunk/lib/fuzzer
IOTC_LIBFUZZER_DOWNLOAD_DIR := $(IOTC_CLANG_TOOLS_DIR)/downloaded_libfuzzer
IOTC_LIBFUZZER := $(IOTC_LIBFUZZER_DOWNLOAD_DIR)/libFuzzer.a

$(IOTC_LIBFUZZER_DOWNLOAD_DIR):
	@-mkdir -p $(IOTC_LIBFUZZER_DOWNLOAD_DIR)
	svn checkout $(IOTC_LIBFUZZER_URL) $(IOTC_LIBFUZZER_DOWNLOAD_DIR)

$(IOTC_LIBFUZZER): $(IOTC_LIBFUZZER_DOWNLOAD_DIR)
	(cd $(IOTC_LIBFUZZER_DOWNLOAD_DIR) && clang++ -c -g -O2 -lstdc++ -std=c++11 *.cpp -IFuzzer && ar ruv libFuzzer.a Fuzzer*.o)

$(IOTC_FUZZ_TESTS_CORPUS_DIRS):
	@-mkdir -p $@
