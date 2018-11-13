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

# Set up googletest.
GTEST_DIR = $(LIBIOTC)/third_party/googletest/googletest
GTEST_CPPFLAGS += -isystem $(GTEST_DIR)/include
GTEST_CXXFLAGS += -g -Wall -Wextra -pthread -std=c++11
GTEST_HEADERS = $(GTEST_DIR)/include/gtest/*.h \
                $(GTEST_DIR)/include/gtest/internal/*.h
GTEST_SOURCES = $(GTEST_DIR)/src/*.cc $(GTEST_DIR)/src/*.h $(GTEST_HEADERS)
GTEST_OBJDIR := $(IOTC_OBJDIR)/third_party/googletest

$(GTEST_OBJDIR)/gtest.o: $(GTEST_SOURCES)
	$(info [$(CXX)] $@)
	@-mkdir -p $(GTEST_OBJDIR)
	$(MD) $(CXX) $(GTEST_CPPFLAGS) -I$(GTEST_DIR) $(GTEST_CXXFLAGS) -c \
            $(GTEST_DIR)/src/gtest-all.cc $(IOTC_COMPILER_OUTPUT)

$(GTEST_OBJDIR)/gtest_main.o: $(GTEST_SOURCES)
	$(info [$(CXX)] $@)
	@-mkdir -p $(GTEST_OBJDIR)
	$(MD) $(CXX) $(GTEST_CPPFLAGS) -I$(GTEST_DIR) $(GTEST_CXXFLAGS) -c \
            $(GTEST_DIR)/src/gtest_main.cc $(IOTC_COMPILER_OUTPUT)

GTEST_OBJS := $(GTEST_OBJDIR)/gtest.o $(GTEST_OBJDIR)/gtest_main.o

# Set up iotc googletest tests.
IOTC_GTEST_SUITE ?= iotc_gtests
IOTC_GTEST_OBJDIR := $(IOTC_TEST_OBJDIR)/gtests
IOTC_GTEST_SOURCES = $(shell find $(LIBIOTC)/src -name "*.cc")

IOTC_GTEST_OBJS := $(filter-out $(IOTC_GTEST_SOURCES), $(IOTC_GTEST_SOURCES:.cc=.o))
IOTC_GTEST_OBJS := $(subst $(LIBIOTC_SRC), $(IOTC_GTEST_OBJDIR)/, $(IOTC_GTEST_OBJS))

IOTC_GTESTS = $(IOTC_TEST_BINDIR)/$(IOTC_GTEST_SUITE)

IOTC_GTEST_CXX_FLAGS := $(GTEST_CXXFLAGS)
IOTC_GTEST_CXX_FLAGS += $(IOTC_INCLUDE_FLAGS)
IOTC_GTEST_CXX_FLAGS += -I$(GTEST_DIR)/include -I$(GTEST_DIR)/include/gtest
IOTC_GTEST_CXX_FLAGS += -I$(LIBIOTC)/src/tests
IOTC_GTEST_CXX_FLAGS += $(foreach platformdep,$(IOTC_PLATFORM_MODULES) \
            ,-I$(IOTC_GTEST_SOURCE_DIR)/platform/$(IOTC_PLATFORM_BASE)/$(platformdep))

IOTC_GTEST_CONFIG_FLAGS = $(IOTC_CONFIG_FLAGS) $(IOTC_COMPILER_FLAGS)
IOTC_GTEST_CONFIG_FLAGS += -DNO_FORKING
