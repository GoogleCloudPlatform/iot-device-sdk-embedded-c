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

# Build configuration for Roughtime client support

ifdef IOTC_ENABLE_ROUGHTIME

ifneq ($(IOTC_BSP_TLS), wolfssl)
$(error Roughtime feature needs wolfssl as crypto BSP; was IOTC_BSP_TLS="$(IOTC_BSP_TLS)")
endif

LIBIOTC ?= $(CURDIR)/../..

ROUGHTIME_DIR := $(LIBIOTC)/third_party/roughtime
ROUGHTIME_FAKE_INCLUDE_DIR := $(ROUGHTIME_DIR)/roughtime
ROUGHTIME_LIBRARY := $(ROUGHTIME_DIR)/libroughtime.a

ROUGHTIME_SOURCES := $(ROUGHTIME_DIR)/client.cc \
                     $(ROUGHTIME_DIR)/protocol.cc

ROUGHTIME_OBJS := $(filter-out $(ROUGHTIME_SOURCES), $(ROUGHTIME_SOURCES:.cc=.o))
IOTC_INCLUDE_FLAGS += -I$(ROUGHTIME_DIR)

$(ROUGHTIME_DIR)/%.o: $(ROUGHTIME_DIR)/%.cc
	$(info [$(CXX)] $@)
	$(MD) $(CXX) $(IOTC_INCLUDE_FLAGS) $(IOTC_COMMON_COMPILER_FLAGS) $(IOTC_CXX_FLAGS) \
		-c $< $(IOTC_COMPILER_OUTPUT)

$(ROUGHTIME_LIBRARY): $(ROUGHTIME_OBJS)
	$(info [$(AR)] $@ )
	$(MD) $(AR) -rs -c $@ $^

IOTC_ROUGHTIME_LIB_DEP ?= $(ROUGHTIME_LIBRARY)

endif # IOTC_ENABLE_ROUGHTIME
