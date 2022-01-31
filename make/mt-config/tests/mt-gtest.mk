# Copyright 2018-2020 Google LLC
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

include make/mt-config/tests/mt-tests.mk

# Set up googletest.
GTEST_DIR = $(LIBIOTC)/third_party/googletest/googletest
GMOCK_DIR = $(LIBIOTC)/third_party/googletest/googlemock
GTEST_OBJDIR := $(IOTC_OBJDIR)/third_party/googletest

GTEST_INCLUDE_FLAGS += -isystem $(GTEST_DIR)/include -isystem $(GMOCK_DIR)/include
GTEST_CXXFLAGS += -g -Wall -Wextra -pthread -std=c++11

GTEST_HEADERS = $(GTEST_DIR)/include/gtest/*.h \
                $(GTEST_DIR)/include/gtest/internal/*.h
GMOCK_HEADERS = $(GMOCK_DIR)/include/gmock/*.h \
                $(GMOCK_DIR)/include/gmock/internal/*.h $(GTEST_HEADERS)
GTEST_SOURCES = $(GTEST_DIR)/src/*.cc $(GTEST_DIR)/src/*.h $(GTEST_HEADERS)
GMOCK_SOURCES = $(GMOCK_DIR)/src/*.cc $(GMOCK_HEADERS)

$(GTEST_OBJDIR)/gtest-all.o: $(GTEST_SOURCES)
	$(info [$(CXX)] $@)
	@-mkdir -p $(GTEST_OBJDIR)
	$(MD) $(CXX) $(GTEST_INCLUDE_FLAGS) -I$(GTEST_DIR) $(GTEST_CXXFLAGS) -c \
            $(GTEST_DIR)/src/gtest-all.cc $(IOTC_COMPILER_OUTPUT)

$(GTEST_OBJDIR)/gmock-all.o: $(GMOCK_SOURCES)
	$(info [$(CXX)] $@)
	@-mkdir -p $(GTEST_OBJDIR)
	$(MD) $(CXX) $(GTEST_INCLUDE_FLAGS) -I$(GTEST_DIR) -I$(GMOCK_DIR) \
	          $(GTEST_CXXFLAGS) -c \
            $(GMOCK_DIR)/src/gmock-all.cc $(IOTC_COMPILER_OUTPUT)

$(GTEST_OBJDIR)/gmock_main.o: $(GMOCK_SOURCES)
	$(info [$(CXX)] $@)
	@-mkdir -p $(GTEST_OBJDIR)
	$(MD) $(CXX) $(GTEST_INCLUDE_FLAGS) -I$(GTEST_DIR) -I$(GMOCK_DIR) \
	          $(GTEST_CXXFLAGS) -c \
            $(GMOCK_DIR)/src/gmock_main.cc $(IOTC_COMPILER_OUTPUT)

GTEST_OBJS := $(GTEST_OBJDIR)/gtest-all.o \
              $(GTEST_OBJDIR)/gmock-all.o \
              $(GTEST_OBJDIR)/gmock_main.o

# Set up iotc googletest tests.
IOTC_GTEST_SUITE ?= iotc_gtests
IOTC_GTEST_OBJDIR := $(IOTC_TEST_OBJDIR)/gtests

# Test cases: files ending in _test.cc
IOTC_GTEST_SOURCES := $(shell find $(LIBIOTC)/src -name "*_test.cc")
# Test utils: .cc files in src/tests/
IOTC_GTEST_SOURCES += $(shell find $(LIBIOTC)/src/tests -name "*.cc")

IOTC_GTEST_OBJS := $(filter-out $(IOTC_GTEST_SOURCES), $(IOTC_GTEST_SOURCES:.cc=.o))
IOTC_GTEST_OBJS := $(subst $(LIBIOTC_SRC), $(IOTC_GTEST_OBJDIR)/, $(IOTC_GTEST_OBJS))

IOTC_GTESTS = $(IOTC_TEST_BINDIR)/$(IOTC_GTEST_SUITE)

IOTC_GTEST_CXX_FLAGS := $(GTEST_CXXFLAGS)
IOTC_GTEST_CXX_FLAGS += $(IOTC_INCLUDE_FLAGS)
IOTC_GTEST_CXX_FLAGS += -I$(GTEST_DIR)/include -I$(GTEST_DIR)/include/gtest
IOTC_GTEST_CXX_FLAGS += -I$(GMOCK_DIR)/include -I$(GMOCK_DIR)/include/gmock
IOTC_GTEST_CXX_FLAGS += -I$(LIBIOTC)/src/tests
IOTC_GTEST_CXX_FLAGS += -I$(LIBIOTC)/src/tests/tools
IOTC_GTEST_CXX_FLAGS += -fpic
IOTC_GTEST_CXX_FLAGS += $(foreach platformdep,$(IOTC_PLATFORM_MODULES) \
            ,-I$(IOTC_GTEST_SOURCE_DIR)/platform/$(IOTC_PLATFORM_BASE)/$(platformdep))

IOTC_GTEST_CONFIG_FLAGS = $(IOTC_CONFIG_FLAGS) $(IOTC_COMMON_COMPILER_FLAGS)
IOTC_GTEST_CONFIG_FLAGS += -DNO_FORKING
