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

IOTC_CLIENT_PATH ?= $(CURDIR)/../../

include $(IOTC_CLIENT_PATH)/make/mt-config/mt-target-platform.mk

CC ?= cc
AR ?= ar

MD ?= @

IOTC_EXAMPLE_SRCDIR := $(CURDIR)/src
IOTC_EXAMPLE_OBJDIR := $(CURDIR)/obj
IOTC_EXAMPLE_BINDIR ?= $(CURDIR)/bin

IOTC_EXAMPLE_SRCS += common/commandline.c
IOTC_EXAMPLE_SRCS += common/example_utils.c
IOTC_EXAMPLE_SRCS += $(IOTC_EXAMPLE_NAME).c

IOTC_EXAMPLE_DEPS := $(subst $(IOTC_EXAMPLE_SRCDIR)/,,$(IOTC_EXAMPLE_SRCS:.c=.d))
IOTC_EXAMPLE_OBJS := $(subst $(IOTC_EXAMPLE_SRCDIR)/,,$(IOTC_EXAMPLE_SRCS:.c=.o))

IOTC_EXAMPLE_DEPS := $(addprefix $(IOTC_EXAMPLE_OBJDIR)/,$(IOTC_EXAMPLE_DEPS))
IOTC_EXAMPLE_OBJS := $(addprefix $(IOTC_EXAMPLE_OBJDIR)/,$(IOTC_EXAMPLE_OBJS))

IOTC_EXAMPLE_BIN := $(IOTC_EXAMPLE_BINDIR)/$(IOTC_EXAMPLE_NAME)

IOTC_CLIENT_INC_PATH += $(IOTC_CLIENT_PATH)/include
IOTC_CLIENT_INC_PATH += $(IOTC_CLIENT_PATH)/include/bsp
IOTC_CLIENT_LIB_PATH ?= $(IOTC_CLIENT_PATH)/bin/$(IOTC_TARGET_PLATFORM)

IOTC_CLIENT_ROOTCA_LIST := $(IOTC_CLIENT_PATH)/res/trusted_RootCA_certs/roots.pem

IOTC_FLAGS_INCLUDE += $(foreach i,$(IOTC_CLIENT_INC_PATH),-I$i)

IOTC_FLAGS_COMPILER ?= -Wall -Werror -Wno-pointer-arith -Wno-format -fstrict-aliasing -Os -Wextra

# -lm is only needed by linux
# -lpthread only if both linux and multithreading is enabled in the
# Google Cloud IoT Device SDK Client at compile time
IOTC_FLAGS_LINKER := -L$(IOTC_CLIENT_LIB_PATH) -liotc -lm -lpthread

# TLS BSP related configuration
IOTC_BSP_TLS ?= mbedtls

ifneq ("$(IOTC_BSP_TLS)", "")
  # pick the proper configuration file for TLS library
  IOTC_TLS_LIB_CONFIG_FNAME ?= $(IOTC_CLIENT_PATH)/make/mt-config/mt-tls-$(IOTC_BSP_TLS).mk
  include $(IOTC_TLS_LIB_CONFIG_FNAME)

  TLS_LIB_CONFIG_FLAGS := -L$(addprefix $(IOTC_CLIENT_PATH),$(IOTC_TLS_LIB_DIR))
  TLS_LIB_CONFIG_FLAGS += $(foreach d, $(IOTC_TLS_LIB_NAME), -l$d)
  IOTC_FLAGS_LINKER += $(TLS_LIB_CONFIG_FLAGS)
endif

# Crypto BSP related configuration
IOTC_BSP_CRYPTO ?= mbedtls

ifeq ("$(IOTC_BSP_CRYPTO)", "cryptoauthlib")
  IOTC_FLAGS_INCLUDE += -I$(addprefix $(IOTC_CLIENT_PATH), third_party/cryptoauthlib/lib)
  CRYPTO_LIB_CONFIG_FLAGS := -L$(addprefix $(IOTC_CLIENT_PATH), third_party/cryptoauthlib/build)
  CRYPTO_LIB_CONFIG_FLAGS += -lcryptoauth
  IOTC_FLAGS_LINKER += $(CRYPTO_LIB_CONFIG_FLAGS)

  # For a secure element connected via USB
  IOTC_FLAGS_LINKER += -ludev -lusb -lusb-1.0

  # For selectively enabling Cryptoauthlib in the examples
  IOTC_FLAGS_COMPILER += -DENABLE_CRYPTOAUTHLIB=1
endif
