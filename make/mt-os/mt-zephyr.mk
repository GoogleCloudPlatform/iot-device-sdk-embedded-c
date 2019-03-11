# Copyright 2018-2019 Google LLC
#
# This is part of the Google Cloud IoT Device SDK for Embedded C,
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

CC ?= gcc
AR ?= ar

IOTC_C_FLAGS += -fPIC -m32
IOTC_C_FLAGS += -imacros autoconf.h

IOTC_LIB_FLAGS += $(IOTC_TLS_LIBFLAGS) -lpthread -lm -lcrypto

include make/mt-os/mt-os-common.mk

IOTC_INCLUDE_FLAGS += -I$(ZEPHYR_BASE)/include

IOTC_BSP_TLS_BUILD_ARGS = -m32
IOTC_ZEPHYR_EXAMPLE_PATH = $(LIBIOTC)/third_party/zephyr_integration/zephyr_native_posix

#  sys/types.h
IOTC_INCLUDE_FLAGS += -I$(ZEPHYR_BASE)/include/posix
#  IOTC_INCLUDE_FLAGS += -I$(ZEPHYR_BASE)/lib/libc/minimal/include

IOTC_INCLUDE_FLAGS += -I$(ZEPHYR_BASE)/arch/posix/include
IOTC_INCLUDE_FLAGS += -I$(ZEPHYR_BASE)/soc/posix/inf_clock
IOTC_INCLUDE_FLAGS += -I$(ZEPHYR_BASE)/boards/posix/native_posix

IOTC_INCLUDE_FLAGS += -I$(IOTC_ZEPHYR_EXAMPLE_PATH)/build/zephyr/include/generated

IOTC_ARFLAGS += -rs -c $(XI)

IOTC_C_FLAGS += -Wno-ignored-qualifiers
IOTC_C_FLAGS += -Wno-shift-overflow

# Temporarily disable these warnings until the code gets changed.
IOTC_C_FLAGS += -Wno-format -Wno-unused-parameter

IOTC_CONFIG_FLAGS += -DIOTC_MULTI_LEVEL_DIRECTORY_STRUCTURE
IOTC_CONFIG_FLAGS += -DIOTC_LIBCRYPTO_AVAILABLE

# Zephyr specific macros
IOTC_CONFIG_FLAGS += -DMBEDTLS_PLATFORM_TIME_TYPE_MACRO=long\ long
IOTC_CONFIG_FLAGS += -DCONFIG_POSIX_API

IOTC_LIBCRYPTO_AVAILABLE := 1

IOTC_THIRD_PARTY_DIR = $(LIBIOTC)/third_party
IOTC_ZEPHYR_README_PATH = $(IOTC_THIRD_PARTY_DIR)/zephyr/README.rst
IOTC_ZEPHYR_PREREQUISITE_AUTOCONF = $(IOTC_ZEPHYR_EXAMPLE_PATH)/build/zephyr/include/generated/autoconf.h

#################################################################
# git clone Zephyr repository ###################################
#################################################################
$(IOTC_ZEPHYR_README_PATH):
	@echo "IOTC Zephyr build: git clone Zephyr repository to $(dir $@)"
	@git clone https://github.com/zephyrproject-rtos/zephyr $(dir $@)
	@git -C $(dir $@) checkout 6798a421e1
	@git -C $(dir $@) apply $(IOTC_THIRD_PARTY_DIR)/iotc_zephyr_dtc_version.patch

export ZEPHYR_TOOLCHAIN_VARIANT = zephyr
export ZEPHYR_BASE = $(IOTC_THIRD_PARTY_DIR)/zephyr

IOTC_ZEPHYR_PREREQUISITE_AUTOCONF: $(IOTC_ZEPHYR_README_PATH)
	cd $(IOTC_ZEPHYR_EXAMPLE_PATH); ./prebuild.sh

IOTC_BUILD_PRECONDITIONS := IOTC_ZEPHYR_PREREQUISITE_AUTOCONF

