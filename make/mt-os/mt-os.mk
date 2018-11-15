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
IOTC_CONST_PLATFORM_FREERTOS_LINUX := freertos-linux

ifneq (,$(findstring $(IOTC_CONST_PLATFORM_ARM),$(TARGET)))
  IOTC_CONST_PLATFORM_CURRENT := $(IOTC_CONST_PLATFORM_ARM)
	IOTC_PLATFORM_IS_ARM=1
else
ifneq (,$(findstring $(IOTC_CONST_PLATFORM_LINUX),$(TARGET)))
  IOTC_CONST_PLATFORM_CURRENT := $(IOTC_CONST_PLATFORM_LINUX)
	IOTC_PLATFORM_IS_LINUX=1
endif
endif

ifneq (,$(findstring $(IOTC_CONST_PLATFORM_OSX),$(TARGET)))
  IOTC_CONST_PLATFORM_CURRENT := $(IOTC_CONST_PLATFORM_OSX)
  IOTC_PLATFORM_IS_OSX=1
endif

ifneq (,$(findstring $(IOTC_CONST_PLATFORM_FREERTOS_LINUX),$(TARGET)))
  IOTC_CONST_PLATFORM_CURRENT := $(IOTC_CONST_PLATFORM_FREERTOS_LINUX)
	IOTC_PLATFORM_IS_FREERTOS=1
endif

IOTC_BINDIR := $(IOTC_BINDIR_BASE)/$(IOTC_CONST_PLATFORM_CURRENT)
IOTC_OBJDIR := $(IOTC_OBJDIR_BASE)/$(IOTC_CONST_PLATFORM_CURRENT)
