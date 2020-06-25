# Automatically generated build file. Do not edit.
COMPONENT_INCLUDES += $(IDF_PATH)/components/bt/include $(IDF_PATH)/components/bt/host/bluedroid/api/include/api $(IDF_PATH)/components/bt/common/osi/include $(IDF_PATH)/components/bt/esp_ble_mesh/mesh_common/include $(IDF_PATH)/components/bt/esp_ble_mesh/mesh_core $(IDF_PATH)/components/bt/esp_ble_mesh/mesh_core/include $(IDF_PATH)/components/bt/esp_ble_mesh/mesh_core/storage $(IDF_PATH)/components/bt/esp_ble_mesh/btc/include $(IDF_PATH)/components/bt/esp_ble_mesh/mesh_models/common/include $(IDF_PATH)/components/bt/esp_ble_mesh/mesh_models/client/include $(IDF_PATH)/components/bt/esp_ble_mesh/mesh_models/server/include $(IDF_PATH)/components/bt/esp_ble_mesh/api/core/include $(IDF_PATH)/components/bt/esp_ble_mesh/api/models/include $(IDF_PATH)/components/bt/esp_ble_mesh/api
COMPONENT_LDFLAGS += -L$(BUILD_DIR_BASE)/bt -lbt -L $(IDF_PATH)/components/bt/controller/lib -lbtdm_app
COMPONENT_LINKER_DEPS += $(IDF_PATH)/components/bt/controller/lib/libbtdm_app.a
COMPONENT_SUBMODULES += $(IDF_PATH)/components/bt/controller/lib
COMPONENT_LIBRARIES += bt
COMPONENT_LDFRAGMENTS += 
component-bt-build: 
