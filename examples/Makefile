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

all:

iot_core_mqtt_client: iot_core_mqtt_client/iot_core_mqtt_client
freertos_linux_gcp_iot: freertos_linux/Linux_gcc_gcp_iot/Linux_gcc_gcp_iot
zephyr_native_posix: zephyr_native_posix/build/zephyr/zephyr.exe
.PHONY: iot_core_mqtt_client freertos_linux zephyr_native_posix

iot_core_mqtt_client/iot_core_mqtt_client:
	$(MAKE) -C .. clean_all
	$(MAKE) -C ..
	$(MAKE) -C ./iot_core_mqtt_client

freertos_linux/Linux_gcc_gcp_iot/Linux_gcc_gcp_iot:
	$(MAKE) -C .. clean_all
	$(MAKE) -C .. PRESET=FREERTOS_POSIX_REL
	$(MAKE) -C ./freertos_linux/Linux_gcc_gcp_iot

zephyr_native_posix/build/zephyr/zephyr.exe:
	$(MAKE) -C .. clean_all
	$(MAKE) -C .. PRESET=ZEPHYR
	$(MAKE) -C ./zephyr_native_posix/build

all: iot_core_mqtt_client/iot_core_mqtt_client \
	freertos_linux/Linux_gcc_gcp_iot/Linux_gcc_gcp_iot \
	zephyr_native_posix/build/zephyr/zephyr.exe

.PHONY : clean
clean:
	$(MAKE) -C ./iot_core_mqtt_client clean
	$(MAKE) -C ./freertos_linux/Linux_gcc_gcp_iot clean
	$(MAKE) -C ./zephyr_native_posix/build clean
	# The above clean command doesn't remove the zephyr.exe binary.
	rm -rf ./zephyr_native_posix/build/zephyr/zephyr.exe
