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

IOTC_CONFIG_FLAGS += -DIOTC_PLATFORM_IS_OSX
IOTC_LIB_FLAGS += $(IOTC_TLS_LIBFLAGS) -lpthread -lm -lcrypto

include make/mt-os/mt-os-common.mk

ifdef IOTC_SHARED
  XI = $(IOTC_BINDIR)/libiotc.dylib
  IOTC_ARFLAGS := -shared -o $(XI) $(IOTC_TLS_LIBFLAGS)
  AR = gcc
  IOTC_COMPILER_FLAGS += -fPIC
  IOTC_CONFIG_FLAGS += -DIOTC_SHARED
else
  IOTC_ARFLAGS += -rs -c $(XI)
endif

IOTC_CONFIG_FLAGS += -DIOTC_MULTI_LEVEL_DIRECTORY_STRUCTURE
IOTC_CONFIG_FLAGS += -DIOTC_LIBCRYPTO_AVAILABLE
IOTC_LIBCRYPTO_AVAILABLE := 1
