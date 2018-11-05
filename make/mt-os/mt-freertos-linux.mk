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

IOTC_COMPILER_FLAGS += -fPIC
IOTC_LIB_FLAGS += $(IOTC_TLS_LIBFLAGS)

include make/mt-os/mt-os-common.mk

#  IOTC_FREERTOS_DIR_PATH = /usr/local/google/home/atigyi/Downloads/FreeRTOSv10.1.1/FreeRTOS
#  IOTC_INCLUDE_FLAGS += -I$(IOTC_FREERTOS_DIR_PATH)/Source/include
#  IOTC_INCLUDE_FLAGS += -I$(IOTC_FREERTOS_DIR_PATH)/Demo/portable/GCC/Linux

IOTC_ARFLAGS += -rs -c $(XI)

# Temporarily disable these warnings until the code gets changed.
IOTC_COMPILER_FLAGS += -Wno-format

