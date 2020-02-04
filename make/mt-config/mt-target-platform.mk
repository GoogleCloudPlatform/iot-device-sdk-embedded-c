# Copyright 2018-2020 Google LLC
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

# Detect the host platform, it can be overriden
ifeq ($(OS),Windows_NT)
IOTC_HOST_PLATFORM ?= Windows_NT
else
	UNAME_S := $(shell uname -s)
	ifeq ($(UNAME_S),Linux)
		IOTC_HOST_PLATFORM ?= Linux
		IOTC_PROTO_COMPILER := $(LIBIOTC)/third_party/protobuf-c/compiler/protoc-c-linux
	endif
	ifeq ($(UNAME_S),Darwin)
		IOTC_HOST_PLATFORM ?= Darwin
		IOTC_PROTO_COMPILER := $(LIBIOTC)/third_party/protobuf-c/compiler/protoc-c-macos
	endif
endif

#if nothing has been set or detected we could try with Unknown
IOTC_HOST_PLATFORM ?= Unknown

# Translate host platform to target platform - name differences
ifeq ($(IOTC_HOST_PLATFORM),Linux)
	IOTC_TARGET_PLATFORM ?= linux
else ifeq ($(IOTC_HOST_PLATFORM),Darwin)
	IOTC_TARGET_PLATFORM ?= osx
endif

# If there is no platform
IOTC_TARGET_PLATFORM ?= Unknown
