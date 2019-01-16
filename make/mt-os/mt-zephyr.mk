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

CC ?= gcc
AR ?= ar

IOTC_COMPILER_FLAGS += -fPIC -m32
IOTC_COMPILER_FLAGS += -Wno-ignored-qualifiers
IOTC_COMPILER_FLAGS += -imacros $(LIBIOTC)/examples/hello_world/build/zephyr/include/generated/autoconf.h
IOTC_LIB_FLAGS += $(IOTC_TLS_LIBFLAGS) -lpthread -lm -lcrypto

include make/mt-os/mt-os-common.mk

IOTC_INCLUDE_FLAGS += -I$(ZEPHYR_BASE)/include

#  sys/types.h
IOTC_INCLUDE_FLAGS += -I$(ZEPHYR_BASE)/include/posix
#  IOTC_INCLUDE_FLAGS += -I$(ZEPHYR_BASE)/lib/libc/minimal/include

IOTC_INCLUDE_FLAGS += -I$(ZEPHYR_BASE)/arch/posix/include
IOTC_INCLUDE_FLAGS += -I$(ZEPHYR_BASE)/soc/posix/inf_clock
IOTC_INCLUDE_FLAGS += -I$(ZEPHYR_BASE)/boards/posix/native_posix

IOTC_INCLUDE_FLAGS += -I$(LIBIOTC)/examples/hello_world/build/zephyr/include/generated

IOTC_ARFLAGS += -rs -c $(XI)

# Temporarily disable these warnings until the code gets changed.
IOTC_COMPILER_FLAGS += -Wno-format -Wno-unused-parameter

IOTC_CONFIG_FLAGS += -DIOTC_MULTI_LEVEL_DIRECTORY_STRUCTURE
IOTC_CONFIG_FLAGS += -DIOTC_LIBCRYPTO_AVAILABLE
IOTC_CONFIG_FLAGS += -DMBEDTLS_PLATFORM_TIME_TYPE_MACRO=long\ long

IOTC_LIBCRYPTO_AVAILABLE := 1
