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

LIBIOTC_SRC := $(LIBIOTC)/src/
LIBIOTC_SOURCE_DIR := $(LIBIOTC)/src/libiotc
LIBIOTC_INTERFACE_INCLUDE_DIRS := $(LIBIOTC)/include

IOTC_LIB_FLAGS += -liotc

ifneq (,$(findstring tls_bsp,$(CONFIG)))
	include make/mt-config/mt-tls.mk
	include make/mt-config/mt-crypto.mk
endif

include make/mt-config/tests/mt-gtest.mk # For IOTC_GTEST_SOURCES

IOTC_UNIT_TEST_TARGET ?= native

IOTC_OBJDIR_BASE ?= $(LIBIOTC)/obj
IOTC_BINDIR_BASE ?= $(LIBIOTC)/bin
IOTC_PROTOBUF_GENERATED ?= $(LIBIOTC)/third_party/protobuf-c/protobuf-generated

IOTC_PROTO_DIR ?= $(LIBIOTC)/src/protofiles

# TARGET: Debug Output Options
#
ifneq (,$(findstring release,$(TARGET)))
	IOTC_DEBUG_OUTPUT ?= 0
	IOTC_DEBUG_ASSERT ?= 0
	IOTC_DEBUG_EXTRA_INFO ?=0
endif

IOTC_DEBUG_PRINTF ?=

ifneq (,$(findstring shared,$(TARGET)))
	IOTC_SHARED=1
endif

ifneq (,$(findstring backoff_reset,$(CONFIG)))
	IOTC_CONFIG_FLAGS += -DIOTC_BACKOFF_RESET
endif

ifneq (,$(findstring expose_fs,$(CONFIG)))
	IOTC_CONFIG_FLAGS += -DIOTC_EXPOSE_FS
endif

ifneq (,$(findstring debug,$(TARGET)))
	IOTC_DEBUG_OUTPUT ?= 1
	IOTC_DEBUG_ASSERT ?= 1
	IOTC_DEBUG_EXTRA_INFO ?= 1
endif

# Settings that will work only on linux and only against clang-4.0 and greater
ifneq (,$(findstring fuzz_test,$(CONFIG)))
    IOTC_CONFIG_FLAGS += -fsanitize=address -fomit-frame-pointer -fsanitize-coverage=inline-8bit-counters -g
endif

IOTC_COMMON_COMPILER_FLAGS += -Wall -Werror
IOTC_C_FLAGS +=
IOTC_CXX_FLAGS += -Wextra -std=c++11

# TEMPORARILY disable warnings until the code gets changed
# For all compilers:
IOTC_COMMON_COMPILER_FLAGS += -Wno-pointer-arith
ifeq "$(CC)" "clang"
	# For CLANG
	IOTC_COMMON_COMPILER_FLAGS += -Wno-gnu-zero-variadic-macro-arguments
else
	# For no CLANG compilers
	IOTC_COMMON_COMPILER_FLAGS += -Wno-format
endif

IOTC_CONFIG_FLAGS += -DIOTC_DEBUG_OUTPUT=$(IOTC_DEBUG_OUTPUT)
IOTC_CONFIG_FLAGS += -DIOTC_DEBUG_ASSERT=$(IOTC_DEBUG_ASSERT)
IOTC_CONFIG_FLAGS += -DIOTC_DEBUG_EXTRA_INFO=$(IOTC_DEBUG_EXTRA_INFO)

ifneq (,$(IOTC_DEBUG_PRINTF))
    IOTC_CONFIG_FLAGS += -DIOTC_DEBUG_PRINTF=$(IOTC_DEBUG_PRINTF)
endif

# CONFIG: mqtt_localhost
#
ifneq (,$(findstring mqtt_localhost,$(CONFIG)))
	IOTC_CONFIG_FLAGS += -DIOTC_MQTT_HOST='{ "localhost", IOTC_MQTT_PORT }'
endif

# CONFIG: no_certverify
#
ifneq (,$(findstring no_certverify,$(CONFIG)))
	IOTC_CONFIG_FLAGS += -DIOTC_DISABLE_CERTVERIFY
endif

# CONFIG: filesystem
ifneq (,$(findstring dummy_fs,$(CONFIG)))
	IOTC_CONFIG_FLAGS += -DIOTC_FS_DUMMY
	IOTC_SRCDIRS += $(LIBIOTC_SOURCE_DIR)/io/fs/dummy
endif
ifneq (,$(findstring memory_fs,$(CONFIG)))
	IOTC_CONFIG_FLAGS += -DIOTC_FS_MEMORY
	IOTC_SRCDIRS += $(LIBIOTC_SOURCE_DIR)/io/fs/memory
endif
ifneq (,$(findstring posix_fs,$(CONFIG)))
	IOTC_CONFIG_FLAGS += -DIOTC_FS_POSIX
	IOTC_SRCDIRS += $(LIBIOTC_SOURCE_DIR)/io/fs/posix
endif

# DEBUG_EXTENSION: choose debug extensions
ifneq (,$(findstring memory_limiter,$(CONFIG)))
	IOTC_CONFIG_FLAGS += -DIOTC_MEMORY_LIMITER_APPLICATION_MEMORY_LIMIT=524288 # 512 KB
	IOTC_CONFIG_FLAGS += -DIOTC_MEMORY_LIMITER_SYSTEM_MEMORY_LIMIT=2024 # 2 KB
	IOTC_CONFIG_FLAGS += -DIOTC_MEMORY_LIMITER_ENABLED
	IOTC_MEMORY_LIMITER_ENABLED := 1
	IOTC_SRCDIRS += $(LIBIOTC_SOURCE_DIR)/debug_extensions/memory_limiter
endif

# CONFIG: modules here we are going to check each defined module

IOTC_PLATFORM_MODULES ?= iotc_thread

# CONFIG: enable threading
ifneq (,$(findstring threading,$(CONFIG)))
	IOTC_CONFIG_FLAGS += -DIOTC_MODULE_THREAD_ENABLED
	IOTC_PLATFORM_MODULES_ENABLED += iotc_thread
endif

# CONFIG: choose modules platform
ifneq (,$(findstring posix_platform,$(CONFIG)))
	IOTC_PLATFORM_BASE = posix
	IOTC_CONFIG_FLAGS += -DIOTC_PLATFORM_BASE_POSIX
else ifneq (,$(findstring wmsdk_platform,$(CONFIG)))
	IOTC_PLATFORM_BASE = wmsdk
	IOTC_CONFIG_FLAGS += -DIOTC_PLATFORM_BASE_WMSDK
else
	IOTC_PLATFORM_BASE = dummy
	IOTC_CONFIG_FLAGS += -DIOTC_PLATFORM_BASE_DUMMY
endif

# CONFIG: BSP related include and source configuration
# enumerate existing platform bsps
BSP_PLATFORM_DIR_EXIST := $(shell if [ ! -d $(IOTC_BSP_DIR)/platform/$(IOTC_BSP_PLATFORM) ]; then echo 0; else echo 1; fi; )

ifeq ($(BSP_PLATFORM_DIR_EXIST),0)
	$(error The platform with BSP implementation - [$(IOTC_BSP_PLATFORM)] couldn't be found. Please check your $(IOTC_BSP_DIR)/platform/ directory.)
endif

IOTC_SRCDIRS += $(IOTC_BSP_DIR)

# platform specific BSP implementations
IOTC_SRCDIRS += $(IOTC_BSP_DIR)/platform/$(IOTC_BSP_PLATFORM)

IOTC_INCLUDE_FLAGS += -I$(LIBIOTC)/include/bsp

# platform independent BSP drivers
IOTC_SRCDIRS += $(LIBIOTC_SOURCE_DIR)/io/net
IOTC_SRCDIRS += $(LIBIOTC_SOURCE_DIR)/memory
IOTC_SRCDIRS += $(LIBIOTC_SOURCE_DIR)/event_loop
IOTC_SRCDIRS += $(LIBIOTC_SOURCE_DIR)/time

# if no tls_bsp then set proper flag
ifeq (,$(findstring tls_bsp,$(CONFIG)))
	IOTC_CONFIG_FLAGS += -DIOTC_NO_TLS_LAYER
	IOTC_NO_TLS_LAYER := 1

	ifneq (,$(findstring tls_socket,$(CONFIG)))
		IOTC_CONFIG_FLAGS += -DIOTC_BSP_IO_NET_TLS_SOCKET
	else
		IOTC_CONFIG_FLAGS += -DIOTC_MQTT_PORT=1883
	endif

else
	IOTC_SRCDIRS += $(LIBIOTC_SOURCE_DIR)/tls/certs
	IOTC_SRCDIRS += $(LIBIOTC_SOURCE_DIR)/tls
	IOTC_SRCDIRS += $(IOTC_BSP_DIR)/tls/$(IOTC_BSP_TLS)
endif

#
# SOURCE CONFIGURATIONS
#
IOTC_SRCDIRS += $(LIBIOTC_SOURCE_DIR)
IOTC_SRCDIRS += $(LIBIOTC_SOURCE_DIR)/io
IOTC_SRCDIRS += $(LIBIOTC_SOURCE_DIR)/io/fs
IOTC_SRCDIRS += $(LIBIOTC_SOURCE_DIR)/event_dispatcher
IOTC_SRCDIRS += $(LIBIOTC_SOURCE_DIR)/datastructures
IOTC_SRCDIRS += $(LIBIOTC_SOURCE_DIR)/mqtt/codec
IOTC_SRCDIRS += $(LIBIOTC_SOURCE_DIR)/mqtt/logic
IOTC_SRCDIRS += $(LIBIOTC_SOURCE_DIR)/control_topic

#
# INCLUDE IOTC LIBRARY THIRD PARTY SOURCE DEPENENCIES
#
IOTC_SRCDIRS += $(LIBIOTC)/third_party/mqtt-protocol-c


# MODULES SRCDIRS
IOTC_SRCDIRS += $(foreach platformdep,$(IOTC_PLATFORM_MODULES_ENABLED) \
			,$(LIBIOTC_SOURCE_DIR)/platform/$(IOTC_PLATFORM_BASE)/$(platformdep))

IOTC_INCLUDE_FLAGS += $(foreach platformdep,$(IOTC_PLATFORM_MODULES) \
			,-I$(LIBIOTC_SOURCE_DIR)/platform/$(platformdep))

IOTC_INCLUDE_FLAGS += $(foreach d, $(LIBIOTC_INTERFACE_INCLUDE_DIRS), -I$d)

IOTC_INCLUDE_FLAGS += -I$(LIBIOTC_SOURCE_DIR)

IOTC_INCLUDE_FLAGS += $(foreach d, $(IOTC_SRCDIRS), -I$d)

IOTC_INCLUDE_FLAGS += -I$(LIBIOTC_SOURCE_DIR)/platform/$(IOTC_PLATFORM_BASE)

IOTC_INCLUDE_FLAGS += $(foreach d,\
			$(wildcard $(LIBIOTC_SOURCE_DIR)/platform/$(IOTC_PLATFORM_BASE)/*),-I$d)

IOTC_SOURCES += $(wildcard ./src/*.c)

IOTC_SOURCES += $(foreach layerdir,$(IOTC_SRCDIRS),\
	$(wildcard $(layerdir)/*.c))

# C++ source files
IOTC_SOURCES_CXX := $(wildcard ./src/*.cc)
IOTC_SOURCES_CXX += $(foreach layerdir,$(IOTC_SRCDIRS),\
	$(wildcard $(layerdir)/*.cc))
IOTC_SOURCES_CXX := $(filter-out $(IOTC_GTEST_SOURCES), $(IOTC_SOURCES_CXX)) # Filter out tests

ifeq ($(IOTC_DEBUG_OUTPUT),0)
IOTC_SOURCES := $(filter-out $(LIBIOTC_SOURCE_DIR)/iotc_debug.c, $(IOTC_SOURCES) )
IOTC_SOURCES := $(filter-out $(LIBIOTC)/third_party/mqtt-protocol-c/iotc_debug_data_desc_dump.c, $(IOTC_SOURCES) )
endif

ifdef MAKEFILE_DEBUG
$(info --mt-config-- Using [$(IOTC_BSP_PLATFORM)] BSP configuration)
$(info --mt-config-- event_loop=$(IOTC_EVENT_LOOP))
$(info --mt-config-- $$IOTC_PLATFORM_BASE is [${IOTC_PLATFORM_BASE}])
$(info --mt-config-- $$IOTC_PLATFORM_MODULES is [${IOTC_PLATFORM_MODULES}])
$(info --mt-config-- $$LIBIOTC_SOURCE_DIR is [${LIBIOTC_SOURCE_DIR}])
$(info --mt-config-- $$IOTC_SOURCES is [${IOTC_SOURCES}])
$(info --mt-config-- $$IOTC_INCLUDE_FLAGS is [${IOTC_INCLUDE_FLAGS}])
$(info --mt-config-- $$IOTC_OBJDIR_BASE is [${IOTC_OBJDIR_BASE}])
$(info --mt-config-- $$IOTC_BINDIR_BASE is [${IOTC_BINDIR_BASE}])
$(info --mt-config-- $$IOTC_SRCDIRS is [${IOTC_SRCDIRS}])
endif
