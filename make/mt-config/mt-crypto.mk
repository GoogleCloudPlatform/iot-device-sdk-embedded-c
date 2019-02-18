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

# Build configuration for Crypto BSP implementation

ifeq ($(IOTC_BSP_CRYPTO), mbedtls)
    ifneq ($(IOTC_BSP_TLS), mbedtls)
        $(error mbedtls as crypto BSP requires IOTC_BSP_TLS=mbedtls, was "$(IOTC_BSP_TLS)")
    endif
    # Nothing else to to, rely on IOTC_BSP_TLS to configure dependencies

else ifeq ($(IOTC_BSP_CRYPTO), wolfssl)
    ifneq ($(IOTC_BSP_TLS), wolfssl)
        $(error wolfssl as crypto BSP requires IOTC_BSP_TLS=wolfssl, was "$(IOTC_BSP_TLS)")
    endif
    # Nothing else to to, rely on IOTC_BSP_TLS to configure dependencies

else ifeq ($(IOTC_BSP_CRYPTO), cryptoauthlib)
CRYPTOAUTHLIB_DIR := ./third_party/cryptoauthlib/lib
CRYPTOAUTHLIB_SUBDIRS := basic crypto hal
CRYPTOAUTHLIB_BUILD_DIR := $(CRYPTOAUTHLIB_DIR)/../build
CRYPTOAUTHLIB_MAKEFILE := $(CRYPTOAUTHLIB_DIR)/../build/Makefile
CRYPTOAUTHLIB_LIBRARY := $(CRYPTOAUTHLIB_BUILD_DIR)/cryptoauthlib.a

$(CRYPTOAUTHLIB_BUILD_DIR):
	mkdir -p $(CRYPTOAUTHLIB_BUILD_DIR)

$(CRYPTOAUTHLIB_MAKEFILE): | $(CRYPTOAUTHLIB_BUILD_DIR)
	cmake -B$(CRYPTOAUTHLIB_BUILD_DIR) -H$(CRYPTOAUTHLIB_DIR) 

$(CRYPTOAUTHLIB_LIBRARY): | $(CRYPTOAUTHLIB_MAKEFILE)
	make -C $(CRYPTOAUTHLIB_BUILD_DIR)

IOTC_INCLUDE_FLAGS += -I$(CRYPTOAUTHLIB_DIR)
IOTC_INCLUDE_FLAGS += $(foreach d, $(CRYPTOAUTHLIB_SUBDIRS), -I$(CRYPTOAUTHLIB_DIR)/$d)
IOTC_CRYPTO_LIB_DEP ?= $(CRYPTOAUTHLIB_LIBRARY)
IOTC_LIB_FLAGS += -L$(CRYPTOAUTHLIB_BUILD_DIR)
IOTC_LIB_FLAGS += -lcryptoauth

else
    $(error unsupported IOTC_BSP_CRYPTO value "$(IOTC_BSP_CRYPTO)")
endif

IOTC_SRCDIRS += $(IOTC_BSP_DIR)/crypto/$(IOTC_BSP_CRYPTO)
