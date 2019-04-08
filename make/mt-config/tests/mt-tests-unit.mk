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

include make/mt-config/tests/mt-tests.mk

IOTC_UTEST_SUITE ?= iotc_utests
IOTC_UTEST_FIXTURE ?= iotc_utest_basic_testcase_frame
IOTC_UTEST_SOURCE_DIR ?= $(IOTC_TEST_DIR)/utests
IOTC_UTEST_OBJDIR := $(IOTC_TEST_OBJDIR)/utests

IOTC_UTEST_EXCLUDED := $(IOTC_UTEST_SUITE).c

IOTC_UTEST_SUITE_SOURCE := $(IOTC_UTEST_SOURCE_DIR)/$(IOTC_UTEST_SUITE).c

ifndef IOTC_MEMORY_LIMITER_ENABLED
    IOTC_UTEST_EXCLUDED += iotc_utest_memory_limiter.c
endif

ifndef IOTC_LIBCRYPTO_AVAILABLE
    IOTC_UTEST_EXCLUDED += iotc_utest_jwt_openssl_validation.c
endif

IOTC_UTEST_EXCLUDED := $(addprefix $(IOTC_UTEST_SOURCE_DIR)/, $(IOTC_UTEST_EXCLUDED))

IOTC_UTEST_SOURCES += $(wildcard $(IOTC_UTEST_SOURCE_DIR)/*.c)
IOTC_UTEST_SOURCES += $(wildcard $(IOTC_TEST_DIR)/*.c)
IOTC_UTEST_SOURCES := $(filter-out $(IOTC_UTEST_EXCLUDED), $(IOTC_UTEST_SOURCES))

IOTC_UTEST_OBJS := $(filter-out $(IOTC_UTEST_SOURCES), $(IOTC_UTEST_SOURCES:.c=.o))
IOTC_UTEST_OBJS := $(subst $(IOTC_UTEST_SOURCE_DIR), $(IOTC_UTEST_OBJDIR), $(IOTC_UTEST_OBJS))
IOTC_UTEST_OBJS := $(subst $(LIBIOTC)/src, $(IOTC_OBJDIR), $(IOTC_UTEST_OBJS))
IOTC_UTESTS = $(IOTC_TEST_BINDIR)/$(IOTC_UTEST_SUITE)

IOTC_UTEST_UTIL_SOURCES = $(IOTC_TEST_DIR)/iotc_memory_checks.c
ifdef IOTC_PLATFORM_IS_FREERTOS
    IOTC_UTEST_UTIL_SOURCES += $(IOTC_TEST_DIR)/iotc_freertos_impl.c
endif

IOTC_UTEST_UTIL_OBJS := $(filter-out $(IOTC_UTEST_UTIL_SOURCES), $(IOTC_UTEST_UTIL_SOURCES:.c=.o))
IOTC_UTEST_UTIL_OBJS := $(subst $(IOTC_UTEST_SOURCE_DIR), $(IOTC_UTEST_OBJDIR), $(IOTC_UTEST_UTIL_OBJS))
IOTC_UTEST_UTIL_OBJS := $(subst $(LIBIOTC)/src, $(IOTC_OBJDIR), $(IOTC_UTEST_UTIL_OBJS))

TINY_TEST_OBJ := $(IOTC_TEST_OBJDIR)/tinytest.o
TINYTEST_SRCDIR ?= $(LIBIOTC)/third_party/tinytest/

IOTC_UTEST_INCLUDE_FLAGS += $(IOTC_INCLUDE_FLAGS)
IOTC_UTEST_INCLUDE_FLAGS += -I$(TINYTEST_SRCDIR)
IOTC_UTEST_INCLUDE_FLAGS += -I$(IOTC_TEST_DIR)
IOTC_UTEST_INCLUDE_FLAGS += -I$(IOTC_TEST_DIR)/tools
IOTC_UTEST_INCLUDE_FLAGS += $(foreach platformdep,$(IOTC_PLATFORM_MODULES) \
            ,-I$(IOTC_UTEST_SOURCE_DIR)/platform/$(IOTC_PLATFORM_BASE)/$(platformdep))

IOTC_UTEST_CONFIG_FLAGS = $(IOTC_CONFIG_FLAGS) $(IOTC_COMMON_COMPILER_FLAGS) $(IOTC_C_FLAGS)
IOTC_UTEST_CONFIG_FLAGS += -DNO_FORKING

$(TINY_TEST_OBJ): $(TINYTEST_SRCDIR)/tinytest.c $(IOTC_BUILD_PRECONDITIONS)
	@-mkdir -p $(dir $@)
	$(info [$(CC)] $@)
	$(MD) $(CC) $(IOTC_UTEST_CONFIG_FLAGS) $(IOTC_UTEST_INCLUDE_FLAGS) -c $< -o $@
