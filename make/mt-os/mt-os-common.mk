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

IOTC_COMMON_COMPILER_FLAGS += -fstrict-aliasing

XI ?= $(IOTC_BINDIR)/libiotc.a

ifneq (,$(findstring release,$(TARGET)))
    IOTC_COMMON_COMPILER_FLAGS += -Os
endif

ifneq (,$(findstring debug,$(TARGET)))
    IOTC_COMMON_COMPILER_FLAGS += -O0 -g
endif

# warning level
IOTC_COMMON_COMPILER_FLAGS += -Wall -Wextra

# Object files from C and C++ sources
IOTC_OBJS := $(filter-out $(IOTC_SOURCES), $(IOTC_SOURCES:.c=.o))
IOTC_OBJS += $(filter-out $(IOTC_SOURCES_CXX), $(IOTC_SOURCES_CXX:.cc=.o))

IOTC_OBJS := $(subst $(LIBIOTC)/src,$(IOTC_OBJDIR),$(IOTC_OBJS))
IOTC_OBJS := $(subst $(IOTC_BSP_DIR),$(IOTC_OBJDIR)/bsp/,$(IOTC_OBJS))
IOTC_OBJS := $(subst $(LIBIOTC)/third_party,$(IOTC_OBJDIR)/third_party,$(IOTC_OBJS))
IOTC_OBJS := $(sort $(IOTC_OBJS)) # sort for determinism and ease of debug

# UNIT TESTS
IOTC_TEST_OBJS := $(filter-out $(IOTC_UTEST_SOURCES), $(IOTC_UTEST_SOURCES:.c=.o))
IOTC_TEST_OBJS := $(subst $(IOTC_UTEST_SOURCE_DIR),$(IOTC_TEST_OBJDIR),$(IOTC_TEST_OBJS))

IOTC_POST_COMPILE_ACTION_CC  = $(MD) $(CC)  $(IOTC_CONFIG_FLAGS) $(IOTC_COMMON_COMPILER_FLAGS) $(IOTC_C_FLAGS) $(IOTC_INCLUDE_FLAGS) -MM $< -MT $@ -MF $(@:.o=.d)
IOTC_POST_COMPILE_ACTION_CXX = $(MD) $(CXX) $(IOTC_CONFIG_FLAGS) $(IOTC_COMMON_COMPILER_FLAGS) $(IOTC_CXX_FLAGS) $(IOTC_INCLUDE_FLAGS) -MM $< -MT $@ -MF $(@:.o=.d)
