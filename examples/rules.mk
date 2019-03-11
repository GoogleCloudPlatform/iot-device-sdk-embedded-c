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
# limitations under the License

CC ?= cc
AR ?= ar
MD ?= @

# Assumes the following variables:
# IOTC_EXAMPLE_NAME             - the name of the binary (not path).
# IOTC_EXAMPLE_TARGET_PLATFORM  - the target platform
# IOTC_EXAMPLE_SRCS             - the list of the sources for the example
#
# Optional variables:
# IOTC_EXAMPLE_DIR      - the directory where build artifacts will be written
#                         under subdirectories (bin, obj)
# IOTC_EXAMPLE_SRC_DIR  - the directory where the example sources are
#
IOTC_EXAMPLE_DIR ?= $(CURDIR)
IOTC_EXAMPLE_SRCDIR ?= $(CURDIR)/src
IOTC_ROOT_DIR ?= $(abspath $(IOTC_EXAMPLE_DIR)/../../)

include $(IOTC_ROOT_DIR)/make/mt-config/mt-target-platform.mk

# Output directories.
IOTC_EXAMPLE_OBJDIR := $(IOTC_EXAMPLE_DIR)/obj
IOTC_EXAMPLE_BINDIR := $(IOTC_EXAMPLE_DIR)/bin
IOTC_EXAMPLE_PEMDIR := $(IOTC_EXAMPLE_DIR)/pem

# Add the common utility sources.
IOTC_EXAMPLE_COMMON_SRCDIR := $(IOTC_ROOT_DIR)/examples/common/src
IOTC_EXAMPLE_COMMON_SRCS += $(IOTC_EXAMPLE_COMMON_SRCDIR)/commandline.c
IOTC_EXAMPLE_COMMON_SRCS += $(IOTC_EXAMPLE_COMMON_SRCDIR)/example_utils.c

# Compute all the common dependencies and objects.
IOTC_EXAMPLE_COMMON_DEPS := $(notdir $(IOTC_EXAMPLE_COMMON_SRCS:.c=.d))
IOTC_EXAMPLE_COMMON_OBJS := $(notdir $(IOTC_EXAMPLE_COMMON_SRCS:.c=.o))
IOTC_EXAMPLE_COMMON_DEPS := $(addprefix $(IOTC_EXAMPLE_OBJDIR)/,$(IOTC_EXAMPLE_COMMON_DEPS))
IOTC_EXAMPLE_COMMON_OBJS := $(addprefix $(IOTC_EXAMPLE_OBJDIR)/,$(IOTC_EXAMPLE_COMMON_OBJS))

# The path to the output binary.
IOTC_EXAMPLE_BINARY = $(IOTC_EXAMPLE_BINDIR)/$(IOTC_EXAMPLE_NAME)

# Compute all the source dependencies and objects.
IOTC_EXAMPLE_DEPS := $(notdir $(IOTC_EXAMPLE_SRCS:.c=.d))
IOTC_EXAMPLE_OBJS := $(notdir $(IOTC_EXAMPLE_SRCS:.c=.o))
IOTC_EXAMPLE_DEPS := $(addprefix $(IOTC_EXAMPLE_OBJDIR)/,$(IOTC_EXAMPLE_DEPS))
IOTC_EXAMPLE_OBJS := $(addprefix $(IOTC_EXAMPLE_OBJDIR)/,$(IOTC_EXAMPLE_OBJS))

# Compute compiler flags.
IOTC_CLIENT_INCLUDE_PATH += $(IOTC_ROOT_DIR)include
IOTC_CLIENT_INCLUDE_PATH += $(IOTC_ROOT_DIR)include/bsp
IOTC_CLIENT_INCLUDE_PATH += $(IOTC_EXAMPLE_COMMON_SRCDIR)
IOTC_CLIENT_INCLUDE_PATH += $(IOTC_EXAMPLE_SRCDIR)
IOTC_EXAMPLE_INCLUDE_FLAGS += $(foreach i,$(IOTC_CLIENT_INCLUDE_PATH),-I$i)

IOTC_EXAMPLE_COMPILER_FLAGS += -Wall -Werror -Wno-pointer-arith -Wno-format -fstrict-aliasing -Os -Wextra

# Compute linker flags.
# TLS BSP related configuration.
IOTC_BSP_TLS ?= mbedtls

ifneq ("$(IOTC_BSP_TLS)", "")
  # Pick the proper configuration file for TLS library.
  IOTC_TLS_LIB_CONFIG_FNAME ?= $(IOTC_ROOT_DIR)/make/mt-config/mt-tls-$(IOTC_BSP_TLS).mk
  include $(IOTC_TLS_LIB_CONFIG_FNAME)

  TLS_LIB_CONFIG_FLAGS := -L$(addprefix $(IOTC_ROOT_DIR)/,$(IOTC_TLS_LIB_DIR))
  TLS_LIB_CONFIG_FLAGS += $(foreach d, $(IOTC_TLS_LIB_NAME), -l$d)
endif
# -lm is only needed by linux
# -lpthread only if both linux and multithreading is enabled in the
#           Google Cloud IoT EmbeddedC Client at compile time
IOTC_CLIENT_LIB_PATH ?= $(IOTC_ROOT_DIR)/bin/$(IOTC_EXAMPLE_TARGET_PLATFORM)
IOTC_EXAMPLE_LINKER_FLAGS := -L$(IOTC_CLIENT_LIB_PATH) -liotc -lpthread $(TLS_LIB_CONFIG_FLAGS) -lm
