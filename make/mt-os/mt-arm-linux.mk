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

include make/mt-os/mt-os-common.mk

IOTC_COMMON_COMPILER_FLAGS += -fPIC
IOTC_COMMON_COMPILER_FLAGS += -mcpu=cortex-m3 -mthumb
# Temporarily disable these warnings until the code gets changed.
IOTC_COMMON_COMPILER_FLAGS += -Wno-format
IOTC_COMMON_COMPILER_FLAGS += -specs=rdimon.specs
IOTC_LIBS_FLAGS += lrdimon

IOTC_ARFLAGS += -rs -c $(XI)

include make/mt-utils/mt-get-gnu-arm-toolchain.mk
