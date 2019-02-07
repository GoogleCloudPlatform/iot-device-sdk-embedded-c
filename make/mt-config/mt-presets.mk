# Copyright 2019 Google LLC
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

include make/mt-config/mt-target-platform.mk

# CONFIG for POSIX presets
CONFIG_POSIX_MAX           =posix_fs-posix_platform-tls_bsp-memory_limiter
CONFIG_POSIX_MAX_THREADING =posix_fs-posix_platform-tls_bsp-threading-memory_limiter
CONFIG_POSIX_MIN           =posix_fs-posix_platform-tls_bsp
CONFIG_POSIX_MIN_UNSECURE  =posix_fs-posix_platform

# CONFIG for ZEPHYR presets
CONFIG_ZEPHYR_MAX          =memory_fs-posix_platform-tls_bsp-memory_limiter

# CONFIG for ARM
CONFIG_DUMMY_MAX           =memory_fs-memory_limiter
CONFIG_DUMMY_MIN           =memory_fs

# TARGET presets
TARGET_STATIC_DEV          =-static-debug
TARGET_STATIC_REL          =-static-release

PRESET ?= POSIX_REL

# -------------------------------------------------------
# POSIX DEV
ifeq ($(PRESET), POSIX_DEV_MIN)
    CONFIG = $(CONFIG_POSIX_MIN)
    TARGET = $(TARGET_STATIC_DEV)
    IOTC_BSP_PLATFORM = posix
else ifeq ($(PRESET), POSIX_DEV)
    CONFIG = $(CONFIG_POSIX_MAX)
    TARGET = $(TARGET_STATIC_DEV)
    IOTC_BSP_PLATFORM = posix

# POSIX REL
else ifeq ($(PRESET), POSIX_REL_MIN)
    CONFIG = $(CONFIG_POSIX_MIN)
    TARGET = $(TARGET_STATIC_REL)
    IOTC_BSP_PLATFORM = posix
else ifeq ($(PRESET), POSIX_REL)
    CONFIG = $(CONFIG_POSIX_MAX)
    TARGET = $(TARGET_STATIC_REL)
    IOTC_BSP_PLATFORM = posix

# -------------------------------------------------------
# UNSECURE
else ifeq ($(PRESET), POSIX_UNSECURE_REL)
    CONFIG = $(CONFIG_POSIX_MIN_UNSECURE)
    TARGET = $(TARGET_STATIC_REL)
    IOTC_BSP_PLATFORM = posix
    IOTC_BSP_TLS =

# + THREADING
else ifeq ($(PRESET), POSIX_THREADING_REL)
    CONFIG = $(CONFIG_POSIX_MAX_THREADING)
    TARGET = $(TARGET_STATIC_REL)
    IOTC_BSP_PLATFORM = posix

# -------------------------------------------------------
# FREERTOS POSIX DEV
else ifeq ($(PRESET), FREERTOS_POSIX_REL)
    CONFIG = $(CONFIG_POSIX_MIN)
    TARGET = $(TARGET_STATIC_REL)
    IOTC_BSP_PLATFORM = freertos-posix
    IOTC_TARGET_PLATFORM = freertos-linux

else ifeq ($(PRESET), FREERTOS_POSIX_DEV)
    CONFIG = $(CONFIG_POSIX_MIN)
    TARGET = $(TARGET_STATIC_DEV)
    IOTC_BSP_PLATFORM = freertos-posix
    IOTC_TARGET_PLATFORM = freertos-linux

# -------------------------------------------------------
# ZEPHYR
else ifeq ($(PRESET), ZEPHYR)
    CONFIG = $(CONFIG_ZEPHYR_MAX)
    TARGET = $(TARGET_STATIC_DEV)
    IOTC_BSP_PLATFORM = zephyr
    IOTC_TARGET_PLATFORM = zephyr

# -------------------------------------------------------
# ARM
else ifeq ($(PRESET), ARM_REL_MIN)
    CONFIG = $(CONFIG_DUMMY_MIN)
    TARGET = $(TARGET_STATIC_REL)
    IOTC_BSP_PLATFORM = dummy
    IOTC_TARGET_PLATFORM = arm-linux
else ifeq ($(PRESET), ARM_REL)
    CONFIG = $(CONFIG_DUMMY_MAX)
    TARGET = $(TARGET_STATIC_REL)
    IOTC_BSP_PLATFORM = dummy
    IOTC_TARGET_PLATFORM = arm-linux

# -------------------------------------------------------
# Fuzz Tests
else ifeq ($(PRESET), FUZZ_TESTS)
    CONFIG = $(CONFIG_POSIX_MIN_UNSECURE)_fuzz_test
    TARGET = $(TARGET_STATIC_REL)
    IOTC_BSP_PLATFORM = posix
    IOTC_BSP_TLS =

# -------------------------------------------------------
# DEFAULT
else
    ifndef PRESET
    # default settings in case of undefined
    CONFIG ?= $(CONFIG_POSIX_MIN)
    TARGET ?= $(TARGET_STATIC_REL)
  	    $(info INFO: '$(PRESET)' not detected, using default CONFIG: [$(CONFIG)] and TARGET: [$(TARGET)])
    else
    # error in case of unrecognised PRESET
    $(error Invalid PRESET: ${PRESET}, see valid presets in make/mt-config/mt-presets.mk)
    endif
endif

TARGET := $(addprefix $(IOTC_TARGET_PLATFORM), $(TARGET))

.PHONY: preset_output
preset_output:
    $(info # Using build PRESET "$(PRESET)" to set CONFIG and TARGET variables:)
    $(info .    IOTC_BSP_PLATFORM: [$(IOTC_BSP_PLATFORM)] )
    $(info .    IOTC_BSP_TLS:      [$(IOTC_BSP_TLS)] )
