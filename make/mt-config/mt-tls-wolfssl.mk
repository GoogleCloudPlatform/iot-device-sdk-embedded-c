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

IOTC_TLS_LIB_INC_DIR ?= ./third_party/tls/wolfssl/
IOTC_TLS_LIB_SRC_DIR ?= ./third_party/tls/wolfssl/
IOTC_TLS_LIB_DIR ?= ./third_party/tls/wolfssl/src/.libs/
IOTC_TLS_LIB_NAME ?= wolfssl

# Will be used to check if we should start a TLS library build:
IOTC_TLS_LIB_DEP ?= $(IOTC_TLS_LIB_DIR)libwolfssl.a

# wolfssl API
IOTC_CONFIG_FLAGS += -DHAVE_SNI
IOTC_CONFIG_FLAGS += -DHAVE_CERTIFICATE_STATUS_REQUEST
IOTC_CONFIG_FLAGS += -DHAVE_ECC
IOTC_CONFIG_FLAGS += -DTFM_TIMING_RESISTANT -DECC_TIMING_RESISTANT -DWC_RSA_BLINDING
IOTC_CONFIG_FLAGS += -DUSE_FAST_MATH
IOTC_CONFIG_FLAGS += -DWOLFSSL_X86_64_BUILD
#TODO(frec): define the SHA512 and ed5139 stuff here ?

# libiotc OCSP stapling feature switch
IOTC_CONFIG_FLAGS += -DIOTC_TLS_OCSP_STAPLING

# libiotc OCSP feature switch
# IOTC_CONFIG_FLAGS += -DIOTC_TLS_OCSP

IOTC_CONFIG_FLAGS += -DIOTC_TLS_LIB_WOLFSSL

