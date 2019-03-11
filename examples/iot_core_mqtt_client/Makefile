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

IOTC_EXAMPLE_NAME := iot_core_mqtt_client
IOTC_EXAMPLE_TARGET_PLATFORM := linux
IOTC_EXAMPLE_SRCS = src/iot_core_mqtt_client.c

include ../rules.mk
include ../targets.mk

default: $(IOTC_EXAMPLE_BINARY)

clean:
	$(info [clean] $(IOTC_EXAMPLE_NAME) )
	@rm -rf $(IOTC_EXAMPLE_OBJDIR)
	@rm -rf $(IOTC_EXAMPLE_BINDIR)
	@rm -rf $(IOTC_EXAMPLE_PEMDIR)
