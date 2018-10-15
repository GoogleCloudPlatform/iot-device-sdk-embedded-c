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

IOTC_ITESTS_OBJDIR := $(IOTC_TEST_OBJDIR)/itests
IOTC_ITESTS_CFLAGS = $(IOTC_CONFIG_FLAGS) $(IOTC_COMPILER_FLAGS)

# CMOCKA LIBRARY SECTION
CMOCKA_DIR := $(LIBIOTC)/third_party/cmocka
CMOCKA_BUILD_DIR := $(CMOCKA_DIR)/cmocka_build/$(IOTC_CONST_PLATFORM_CURRENT)
CMOCKA_MAKEFILE := $(CMOCKA_BUILD_DIR)/Makefile
CMOCKA_LIBRARY := $(CMOCKA_BUILD_DIR)/src/libcmocka.a
CMOCKA_LIBRARY_DEPS := $(CMOCKA_LIBRARY)
CMOCKA_INCLUDE_DIR := $(CMOCKA_DIR)/include/

# add cmocka library
IOTC_MOCK_LIB_DIR := $(CMOCKA_BUILD_DIR)/src

$(CMOCKA_BUILD_DIR):
	@mkdir -p $(CMOCKA_BUILD_DIR)

ifeq ($(IOTC_CONST_PLATFORM_CURRENT),$(IOTC_CONST_PLATFORM_ARM))
    CMAKE_FLAGS=-DCMAKE_TOOLCHAIN_FILE=$(LIBIOTC)/$(IOTC_CONST_PLATFORM_ARM).cmake
endif

$(CMOCKA_MAKEFILE): | $(CMOCKA_BUILD_DIR)
	   cmake -B$(CMOCKA_BUILD_DIR) -H$(CMOCKA_DIR) $(CMAKE_FLAGS) -DWITH_STATIC_LIB=ON

$(CMOCKA_LIBRARY): | $(CMOCKA_MAKEFILE)
	@make -C $(CMOCKA_BUILD_DIR)

# MOCK TEST SECTION
IOTC_ITESTS_SUITE := iotc_itests
IOTC_ITESTS_SOURCE_DIR := $(IOTC_TEST_DIR)/itests
IOTC_ITESTS_SOURCES := $(IOTC_ITESTS_SOURCE_DIR)/$(IOTC_ITESTS_SUITE).c
IOTC_ITESTS := $(IOTC_TEST_BINDIR)/$(IOTC_ITESTS_SUITE)

# ADD INTEGRATION TEST CASE FILES
IOTC_ITESTS_SOURCES += $(wildcard $(IOTC_ITESTS_SOURCE_DIR)/iotc_itest_*.c)
IOTC_ITESTS_SOURCES += $(wildcard $(IOTC_ITESTS_SOURCE_DIR)/tools/iotc_*.c)
IOTC_ITESTS_SOURCES += $(wildcard $(IOTC_ITESTS_SOURCE_DIR)/tools/dummy/*.c)

# ADD INTEGRATION TEST TOOLS AND COMMON FILES
IOTC_ITESTS_SOURCES += $(wildcard $(IOTC_TEST_DIR)/*.c)

# ADD dummy io layer
IOTC_ITESTS_SOURCES += $(wildcard $(LIBIOTC)/src/libiotc/io/dummy/*.c)

# removing TLS layer related tests in case TLS is turned off from compilation
ifeq (,$(findstring tls_bsp,$(CONFIG)))
    IOTC_ITESTS_SOURCES := $(filter-out $(IOTC_ITESTS_SOURCE_DIR)/iotc_itest_tls_layer.c, $(IOTC_ITESTS_SOURCES))
endif

IOTC_ITEST_OBJS := $(filter-out $(IOTC_ITESTS_SOURCES), $(IOTC_ITESTS_SOURCES:.c=.o))
IOTC_ITEST_OBJS := $(subst $(IOTC_ITESTS_SOURCE_DIR), $(IOTC_ITESTS_OBJDIR), $(IOTC_ITEST_OBJS))
IOTC_ITEST_OBJS := $(subst $(LIBIOTC)/src, $(IOTC_OBJDIR), $(IOTC_ITEST_OBJS))

IOTC_ITESTS_INCLUDE_FLAGS += -I$(CMOCKA_INCLUDE_DIR)
IOTC_ITESTS_INCLUDE_FLAGS += -I$(IOTC_TEST_DIR)
IOTC_ITESTS_INCLUDE_FLAGS += -I$(IOTC_TEST_DIR)/tools
IOTC_ITESTS_INCLUDE_FLAGS += -I$(IOTC_TEST_DIR)/itests
IOTC_ITESTS_INCLUDE_FLAGS += -I$(IOTC_TEST_DIR)/itests/tools
IOTC_ITESTS_INCLUDE_FLAGS += -I$(IOTC_TEST_DIR)/itests/tools/dummy
IOTC_ITESTS_INCLUDE_FLAGS += -I$(LIBIOTC)/src/libiotc
IOTC_ITESTS_INCLUDE_FLAGS += $(foreach platformdep,$(IOTC_PLATFORM_MODULES) \
	,-I$(IOTC_ITESTS_SOURCE_DIR)/platform/$(IOTC_PLATFORM_BASE)/$(platformdep))

IOTC_INCLUDE_FLAGS += $(IOTC_ITESTS_INCLUDE_FLAGS)

ifdef MAKEFILE_DEBUG
$(info "--mt-tests-intergration-- Platform base: ${IOTC_PLATFORM_BASE})
endif
