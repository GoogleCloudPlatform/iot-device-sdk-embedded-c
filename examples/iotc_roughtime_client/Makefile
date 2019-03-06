IOTC_EXAMPLE_NAME := iotc_roughtime_client

include ../common/rules.mk
include ../common/targets.mk

include $(IOTC_CLIENT_PATH)/make/mt-config/mt-roughtime.mk

IOTC_FLAGS_LINKER += -lstdc++
IOTC_FLAGS_LINKER += -L$(ROUGHTIME_DIR) -lroughtime
