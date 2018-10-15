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

IOTC_TEST_SIM := qemu-system-arm -machine integratorcp -cpu cortex-m3 -nographic -monitor null -serial null -semihosting -kernel

ifeq ($(IOTC_HOST_PLATFORM),Linux)
    IOTC_RUN_UTESTS := $(IOTC_TEST_SIM) $(IOTC_UTESTS) -append "-l0"
    IOTC_RUN_ITESTS := $(IOTC_TEST_SIM) $(IOTC_ITESTS)
else ifeq ($(IOTC_HOST_PLATFORM),Darwin)
    IOTC_RUN_UTESTS := @echo "WARNING: no qemu on OSX yet, utests are not executed"
    IOTC_RUN_ITESTS := @echo "WARNING: no qemu on OSX yet, itests are not executed"
endif
