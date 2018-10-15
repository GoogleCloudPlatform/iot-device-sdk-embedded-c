ifeq ($(IOTC_HOST_PLATFORM),Linux)
	# linux cross-compilation assumes tools downloaded and are on PATH

	IOTC_GCC_ARM_NONE_EABI_DOWNLOAD_FILE = ~/Downloads/gcc-arm-none-eabi-5_4-2016q2-20160622-linux.tar.bz2
	IOTC_GCC_ARM_NONE_EABI_PATH = ~/Downloads/gcc-arm-none-eabi-5_4-2016q2

	CC = $(IOTC_GCC_ARM_NONE_EABI_PATH)/bin/arm-none-eabi-gcc
	AR = $(IOTC_GCC_ARM_NONE_EABI_PATH)/bin/arm-none-eabi-ar

	IOTC_GCC_ARM_TOOLCHAIN_URL := https://launchpad.net/gcc-arm-embedded/5.0/5-2016-q2-update/+download/gcc-arm-none-eabi-5_4-2016q2-20160622-linux.tar.bz2

	#export PATH=$PATH:$(IOTC_GCC_ARM_NONE_EABI_PATH)/bin

	IOTC_BUILD_PRECONDITIONS := $(CC)

else ifeq ($(IOTC_HOST_PLATFORM),Darwin)
	# osx cross-compilation downloads arm-gcc

	IOTC_GCC_ARM_NONE_EABI_DOWNLOAD_FILE = ~/Downloads/gcc-arm-none-eabi-5_4-2016q2-20160622-mac.tar.bz2
	IOTC_GCC_ARM_NONE_EABI_PATH = ~/Downloads/gcc-arm-none-eabi-5_4-2016q2

	CC = $(IOTC_GCC_ARM_NONE_EABI_PATH)/bin/arm-none-eabi-gcc
	AR = $(IOTC_GCC_ARM_NONE_EABI_PATH)/bin/arm-none-eabi-ar

	IOTC_GCC_ARM_TOOLCHAIN_URL := https://launchpad.net/gcc-arm-embedded/5.0/5-2016-q2-update/+download/gcc-arm-none-eabi-5_4-2016q2-20160622-mac.tar.bz2

	IOTC_BUILD_PRECONDITIONS := $(CC)

else ifeq ($(IOTC_HOST_PLATFORM),Windows_NT)
	CC = arm-none-eabi-gcc
	AR = arm-none-eabi-ar
endif

$(IOTC_GCC_ARM_NONE_EABI_DOWNLOAD_FILE):
	@echo "XI ARM-GCC BUILD: downloading arm-gcc toolchain to file $(IOTC_GCC_ARM_NONE_EABI_DOWNLOAD_FILE)"
	@-mkdir -p $(dir $@)
	@curl -L -o $(IOTC_GCC_ARM_NONE_EABI_DOWNLOAD_FILE) $(IOTC_GCC_ARM_TOOLCHAIN_URL)

$(CC): $(IOTC_GCC_ARM_NONE_EABI_DOWNLOAD_FILE)
	@echo "XI ARM-GCC BUILD: extracting arm-gcc toolchain"
	@tar -xf $(IOTC_GCC_ARM_NONE_EABI_DOWNLOAD_FILE) -C ~/Downloads
	@touch $@
	$@ --version
