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

IOTC_EXAMPLE_DIR = $(LIBIOTC)/src/examples
IOTC_EXAMPLE_OBJDIR = $(IOTC_OBJDIR)/examples
IOTC_EXAMPLE_BINDIR = $(IOTC_BINDIR)/examples

IOTC_EXAMPLE_SOURCES = internal/iotc_coroutine.c

ifdef IOTC_TLS_BSP
	IOTC_EXAMPLE_SOURCES += mqtt_logic_example_tls_bsp.c
endif

IOTC_INTERNAL_EXAMPLES = $(addprefix $(IOTC_EXAMPLE_BINDIR)/,$(IOTC_EXAMPLE_SOURCES:.c=))

IOTC_EXAMPLE_INCLUDE_FLAGS := $(foreach d, $(LIBIOTC_INTERFACE_INCLUDE_DIRS), -I$d)
