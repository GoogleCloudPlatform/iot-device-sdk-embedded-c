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

IOTC_CONST_PLATFORM_LINUX := linux
IOTC_CONST_PLATFORM_OSX := osx
IOTC_CONST_PLATFORM_ARM := arm-linux
IOTC_CONST_PLATFORM_ESP32 := esp32

ifneq (,$(findstring $(IOTC_CONST_PLATFORM_ARM),$(TARGET)))
  IOTC_CONST_PLATFORM_CURRENT := $(IOTC_CONST_PLATFORM_ARM)
else
ifneq (,$(findstring $(IOTC_CONST_PLATFORM_LINUX),$(TARGET)))
  IOTC_CONST_PLATFORM_CURRENT := $(IOTC_CONST_PLATFORM_LINUX)
else
ifneq (,$(findstring $(IOTC_CONST_PLATFORM_ESP32),$(TARGET)))
  IOTC_CONST_PLATFORM_CURRENT := $(IOTC_CONST_PLATFORM_ESP32)
endif
endif
endif

ifneq (,$(findstring $(IOTC_CONST_PLATFORM_OSX),$(TARGET)))
  IOTC_CONST_PLATFORM_CURRENT := $(IOTC_CONST_PLATFORM_OSX)
  IOTC_BUILD_OSX=1
endif

IOTC_BINDIR := $(IOTC_BINDIR_BASE)/$(IOTC_CONST_PLATFORM_CURRENT)
IOTC_OBJDIR := $(IOTC_OBJDIR_BASE)/$(IOTC_CONST_PLATFORM_CURRENT)
