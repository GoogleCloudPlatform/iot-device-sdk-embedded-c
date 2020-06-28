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

IOTC_TLS_LIB_INC_DIR ?= ./third_party/tls/mbedtls/include/
IOTC_TLS_LIB_SRC_DIR ?= ./third_party/tls/mbedtls
IOTC_TLS_LIB_DIR ?= ./third_party/tls/mbedtls/library/
IOTC_TLS_LIB_NAME ?= mbedtls mbedx509 mbedcrypto

# Will be used to check if we should start a TLS library build:
IOTC_TLS_LIB_DEP ?= $(IOTC_TLS_LIB_DIR)libmbedtls.a

IOTC_CONFIG_FLAGS += -DIOTC_TLS_LIB_MBEDTLS
IOTC_CONFIG_FLAGS += -DMBEDTLS_PLATFORM_MEMORY
