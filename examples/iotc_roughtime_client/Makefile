IOTC_EXAMPLE_NAME := iotc_roughtime_client

include ../common/rules.mk
include ../common/targets.mk

IOTC_FLAGS_LINKER += -lstdc++
IOTC_FLAGS_LINKER += -lssl -lcrypto
IOTC_FLAGS_LINKER += -lprotobuf
IOTC_FLAGS_LINKER += -lwolfssl
IOTC_FLAGS_LINKER += -L$(IOTC_CLIENT_PATH)/third_party/roughtime/bazel-bin -lclient -lprotocol