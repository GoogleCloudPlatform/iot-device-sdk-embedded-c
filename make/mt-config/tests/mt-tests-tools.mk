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

include make/mt-config/tests/mt-tests.mk

IOTC_TEST_TOOLS_SRCDIR := $(IOTC_TEST_DIR)/tools
IOTC_TEST_TOOLS_OBJDIR := $(IOTC_TEST_OBJDIR)/tools
IOTC_TEST_TOOLS_BINDIR := $(IOTC_TEST_BINDIR)/tools

IOTC_TEST_TOOLS_LIBIOTC_DRIVER_FILENAME = iotc_libiotc_driver
IOTC_TEST_TOOLS_LIBIOTC_DRIVER = $(IOTC_TEST_TOOLS_BINDIR)/$(IOTC_TEST_TOOLS_LIBIOTC_DRIVER_FILENAME)

IOTC_TEST_TOOLS_SOURCES_WITH_MAIN_FUNCTION := \
	$(IOTC_TEST_TOOLS_SRCDIR)/$(IOTC_TEST_TOOLS_LIBIOTC_DRIVER_FILENAME)/$(IOTC_TEST_TOOLS_LIBIOTC_DRIVER_FILENAME).c

IOTC_TEST_TOOLS = $(addprefix $(IOTC_TEST_TOOLS_BINDIR)/,$(notdir $(IOTC_TEST_TOOLS_SOURCES_WITH_MAIN_FUNCTION:.c=)))

IOTC_TEST_TOOLS_SOURCES_TMP :=  $(wildcard $(IOTC_TEST_TOOLS_SRCDIR)/iotc_libiotc_driver/*.c)
IOTC_TEST_TOOLS_SOURCES_TMP += $(wildcard $(LIBIOTC)/third_party/protobuf-c/library/*.c)
IOTC_TEST_TOOLS_SOURCES :=  $(patsubst $(IOTC_TEST_TOOLS_SOURCES_WITH_MAIN_FUNCTION), , $(IOTC_TEST_TOOLS_SOURCES_TMP))

IOTC_TEST_TOOLS_OBJS = $(filter-out $(IOTC_TEST_TOOLS_SOURCES), $(IOTC_TEST_TOOLS_SOURCES:.c=.o))
IOTC_TEST_TOOLS_OBJS := $(subst $(IOTC_TEST_TOOLS_SRCDIR), $(IOTC_TEST_TOOLS_OBJDIR), $(IOTC_TEST_TOOLS_OBJS))
IOTC_TEST_TOOLS_OBJS := $(subst $(LIBIOTC)/src, $(IOTC_OBJDIR), $(IOTC_TEST_TOOLS_OBJS))

IOTC_TEST_TOOLS_EXT_PROTOBUF_OBJS := $(LIBIOTC)/third_party/protobuf-c/library/protobuf-c.o

IOTC_TEST_TOOLS_INCLUDE_FLAGS := -I$(LIBIOTC)/third_party/protobuf-c/library

