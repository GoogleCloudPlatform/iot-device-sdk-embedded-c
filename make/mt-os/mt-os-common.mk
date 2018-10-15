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

IOTC_COMPILER_FLAGS += -fstrict-aliasing

XI ?= $(IOTC_BINDIR)/libiotc.a

ifneq (,$(findstring release,$(TARGET)))
    IOTC_COMPILER_FLAGS += -Os
endif

ifneq (,$(findstring debug,$(TARGET)))
    IOTC_COMPILER_FLAGS += -O0 -g
endif

# warning level
IOTC_COMPILER_FLAGS += -Wall -Wextra

IOTC_OBJS := $(filter-out $(IOTC_SOURCES), $(IOTC_SOURCES:.c=.o))
IOTC_OBJS := $(subst $(LIBIOTC)/src,$(IOTC_OBJDIR),$(IOTC_OBJS))
IOTC_OBJS := $(subst $(IOTC_BSP_DIR),$(IOTC_OBJDIR)/bsp/,$(IOTC_OBJS))
IOTC_OBJS := $(subst $(LIBIOTC)/third_party/mqtt-protocol-c,$(IOTC_OBJDIR)/third_party/mqtt-protocol-c,$(IOTC_OBJS))

# UNIT TESTS
IOTC_TEST_OBJS := $(filter-out $(IOTC_UTEST_SOURCES), $(IOTC_UTEST_SOURCES:.c=.o))
IOTC_TEST_OBJS := $(subst $(IOTC_UTEST_SOURCE_DIR),$(IOTC_TEST_OBJDIR),$(IOTC_TEST_OBJS))

IOTC_POST_COMPILE_ACTION = @$(CC) $(IOTC_CONFIG_FLAGS) $(IOTC_COMPILER_FLAGS) $(IOTC_INCLUDE_FLAGS) -MM $< -MT $@ -MF $(@:.o=.d)
