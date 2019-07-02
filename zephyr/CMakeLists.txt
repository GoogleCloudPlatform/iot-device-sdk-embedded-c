if(CONFIG_GOOGLE_IOT_DEVICE_SDK)

zephyr_interface_library_named(google_iot_device_sdk)

target_include_directories(google_iot_device_sdk INTERFACE
	../include
	../include/bsp
	)

file(GLOB_RECURSE all_dirs_src LIST_DIRECTORIES true ../src/only_dir_match.xxx)
#  message(${all_dirs_third_party})

target_include_directories(google_iot_device_sdk INTERFACE
	${all_dirs_src}
	../third_party/mqtt-protocol-c
	)

add_compile_definitions(IOTC_FS_MEMORY)

#  ERROR: TODO(atigyi): cert verify MUST be turned back on, TODO(b/136539357)
add_compile_definitions(IOTC_DISABLE_CERTVERIFY)

add_compile_definitions(IOTC_DEBUG_OUTPUT)

zephyr_library()
zephyr_library_sources(
../src/bsp/crypto/mbedtls/iotc_bsp_crypto_mbedtls.c
#  ../src/bsp/platform/zephyr/iotc_bsp_io_fs_zephyr.c
../src/bsp/platform/zephyr/iotc_bsp_io_net_zephyr.c
../src/bsp/platform/zephyr/iotc_bsp_mem_zephyr.c
../src/bsp/platform/zephyr/iotc_bsp_rng_zephyr.c
../src/bsp/platform/zephyr/iotc_bsp_time_zephyr.c
../src/bsp/tls/mbedtls/iotc_bsp_tls_mbedtls.c
../src/libiotc/control_topic/iotc_control_topic_layer.c
../src/libiotc/datastructures/iotc_vector.c
../src/libiotc/event_dispatcher/iotc_event_dispatcher.c
../src/libiotc/event_dispatcher/iotc_event_handle.c
../src/libiotc/event_dispatcher/iotc_event_thread_dispatcher.c
../src/libiotc/event_dispatcher/iotc_time_event.c
../src/libiotc/event_loop/iotc_event_loop.c
../src/libiotc/io/fs/iotc_fs_bsp_to_iotc_mapping.c
../src/libiotc/io/fs/iotc_fs_filenames.c
../src/libiotc/io/fs/iotc_resource_manager.c
../src/libiotc/io/fs/memory/iotc_fs_memory.c
../src/libiotc/io/net/iotc_io_net_layer.c
../src/libiotc/iotc.c
../src/libiotc/iotc_backoff_status_api.c
../src/libiotc/iotc_connection_data.c
../src/libiotc/iotc_data_desc.c
../src/libiotc/iotc_debug.c
../src/libiotc/iotc_error.c
../src/libiotc/iotc_globals.c
../src/libiotc/iotc_handle.c
../src/libiotc/iotc_helpers.c
../src/libiotc/iotc_internals.c
../src/libiotc/iotc_io_timeouts.c
../src/libiotc/iotc_jwt.c
../src/libiotc/iotc_layer_api.c
../src/libiotc/iotc_layer_default_allocators.c
../src/libiotc/iotc_layer_default_functions.c
../src/libiotc/iotc_timed_task.c
../src/libiotc/iotc_tuples.c
../src/libiotc/iotc_user_sub_call_wrapper.c
../src/libiotc/memory/iotc_allocator.c
../src/libiotc/mqtt/codec/iotc_mqtt_codec_layer.c
../src/libiotc/mqtt/codec/iotc_mqtt_codec_layer_data.c
../src/libiotc/mqtt/logic/iotc_mqtt_logic_layer.c
../src/libiotc/mqtt/logic/iotc_mqtt_logic_layer_data.c
../src/libiotc/mqtt/logic/iotc_mqtt_logic_layer_keepalive_handler.c
../src/libiotc/mqtt/logic/iotc_mqtt_logic_layer_task_helpers.c
../src/libiotc/tls/certs/iotc_RootCA_list.c
../src/libiotc/tls/iotc_tls_layer.c
../third_party/mqtt-protocol-c/iotc_debug_data_desc_dump.c
../third_party/mqtt-protocol-c/iotc_mqtt_message.c
../third_party/mqtt-protocol-c/iotc_mqtt_parser.c
../third_party/mqtt-protocol-c/iotc_mqtt_serialiser.c
)

zephyr_library_link_libraries(google_iot_device_sdk)

target_link_libraries(google_iot_device_sdk INTERFACE zephyr_interface)
target_link_libraries(google_iot_device_sdk INTERFACE mbedTLS)

endif()

