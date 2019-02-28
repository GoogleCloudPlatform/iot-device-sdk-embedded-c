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
CXX ?= g++
AR ?= ar

IOTC_COMMON_COMPILER_FLAGS += -fPIC
IOTC_LIB_FLAGS += $(IOTC_TLS_LIBFLAGS) -lpthread -lm -lcrypto

include make/mt-os/mt-os-common.mk

ifdef IOTC_SHARED
  AR = gcc
  XI = $(IOTC_BINDIR)/libiotc.so
  IOTC_ARFLAGS += -fPIC -DIOTC_SHARED -shared -o $(XI)
else
  IOTC_ARFLAGS += -rs -c $(XI)
endif

# Temporarily disable these warnings until the code gets changed.
IOTC_COMMON_COMPILER_FLAGS += -Wno-format

IOTC_CONFIG_FLAGS += -DIOTC_MULTI_LEVEL_DIRECTORY_STRUCTURE
IOTC_CONFIG_FLAGS += -DIOTC_LIBCRYPTO_AVAILABLE
IOTC_LIBCRYPTO_AVAILABLE := 1
