/******************************************************************************
 * Copyright 2020 Google
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *****************************************************************************/
#include "include.h"

static void initialize_sntp(void) {
    ESP_LOGI(TAG, "Initializing SNTP");
    sntp_setoperatingmode(SNTP_OPMODE_POLL);
    sntp_setservername(0, "time.google.com");
    sntp_init();
}

static void obtain_time(void) {
    initialize_sntp();
    time_t now = 0;
    struct tm timeinfo = {0};
    while (timeinfo.tm_year < (2016 - 1900)) {
        ESP_LOGI(TAG, "Waiting for system time to be set...");
        vTaskDelay(2000 / portTICK_PERIOD_MS);
        time(&now);
        localtime_r(&now, &timeinfo);
    }
    ESP_LOGI(TAG, "Time is set...");
}

extern void example_ble_mesh_send_vendor_message(bool resend, char *message);

void iotc_mqttlogic_subscribe_callback(iotc_context_handle_t in_context_handle, iotc_sub_call_type_t call_type, const iotc_sub_call_params_t *const params, iotc_state_t state, void *user_data) {
    IOTC_UNUSED(in_context_handle);
    IOTC_UNUSED(call_type);
    IOTC_UNUSED(state);
    IOTC_UNUSED(user_data);
    if (params != NULL && params->message.topic != NULL) {
        ESP_LOGI(TAG, "Subscription Topic: %s\n", params->message.topic);
        char *sub_message = (char *)malloc(params->message.temporary_payload_data_length + 1);
        if (sub_message == NULL) {
            ESP_LOGE(TAG, "Failed to allocate memory");
            return;
        }
        memcpy(sub_message, params->message.temporary_payload_data, params->message.temporary_payload_data_length);
        sub_message[params->message.temporary_payload_data_length] = '\0';
        ESP_LOGI(TAG, "Message Payload: %s \n", sub_message);

        if (strcmp(sub_message, "") != 0) {
            char *message = "";
            char *temp;
            asprintf(&temp, "%s", params->message.topic);
            temp = strtok(temp, "/");
            device = strtok(NULL, "/");

            for (int i = 0; i < sizeof(delegateDevices) / sizeof(delegateDevices[0]); i++) {
                if (strcmp(delegateDevices[i], device) == 0) {
                    server_address = i + 5;
                }
            }

            printf("Server Address : %d\n", server_address);

            asprintf(&telemetryDevice, "%s", device);

            asprintf(&message, "%s/%s", device, sub_message);

            asprintf(&command, "%s", sub_message);

            example_ble_mesh_send_vendor_message(false, message);
        }

        if (strcmp(subscribe_topic_command, params->message.topic) == 0) {
            int value;
            sscanf(sub_message, "{\"outlet\": %d}", &value);
            ESP_LOGI(TAG, "value: %d\n", value);
            if (value == 1) {
                gpio_set_level(OUTPUT_GPIO, true);
            } else if (value == 0) {
                gpio_set_level(OUTPUT_GPIO, false);
            }
        }
        free(sub_message);
    }
}

void publish_delegate_telemetry_event(iotc_context_handle_t context_handle, iotc_timed_task_handle_t timed_task, void *user_data) {
    char *publish_topic = NULL;
    char *event = "";

    if (strcmp(bleRecievedMessage, "") != 0) {
        if (strcmp(command, "Get Temp") == 0) {
            event = "events";
        } else if (strcmp(command, "Get State") == 0) {
            event = "state";
        }

        asprintf(&publish_topic, PUBLISH_TOPIC, telemetryDevice, event);
        ESP_LOGI(TAG, "publishing msg \"%s\" to topic: \"%s\"\n", bleRecievedMessage, publish_topic);
        iotc_publish(context_handle, publish_topic, bleRecievedMessage, iotc_example_qos, iotc_mqttlogic_subscribe_callback, /*user_data=*/NULL);
    }
    free(publish_topic);
    command = "";
    device = "";
    bleRecievedMessage = "";
}
void subscribeCommand(iotc_context_handle_t in_context_handle) {
    for (int i = 0; i < 2; i++) {
        asprintf(&subscribe_topic_command, SUBSCRIBE_TOPIC_COMMAND, delegateDevices[i]);
        iotc_state_t subSt = iotc_subscribe(in_context_handle, subscribe_topic_command, 0, iotc_mqttlogic_subscribe_callback, /*user_data=*/NULL);
        delayed_publish_task = iotc_schedule_timed_task(in_context_handle, publish_delegate_telemetry_event, 6, 1, /*user_data=*/NULL);
    }
}

void subscribeConfig(iotc_context_handle_t in_context_handle) {
    for (int i = 0; i < 2; i++) {
        asprintf(&subscribe_topic_config, SUBSCRIBE_TOPIC_CONFIG, delegateDevices[i]);
        iotc_state_t attachSt = iotc_subscribe(in_context_handle, subscribe_topic_config, 1, subscribeCommand, /*user_data=*/NULL);
    }
}

void attachAndSubscribe(iotc_context_handle_t in_context_handle) {
    for (int i = 0; i < 2; i++) {
        asprintf(&publish_topic_attach, PUBLISH_TOPIC_ATTACH, delegateDevices[i]);
        iotc_state_t st = iotc_publish(in_context_handle, publish_topic_attach, "{}", 1, subscribeConfig, /*user_data=*/NULL);
    }
}

void on_connection_state_changed(iotc_context_handle_t in_context_handle, void *data, iotc_state_t state) {
    iotc_connection_data_t *conn_data = (iotc_connection_data_t *)data;

    switch (conn_data->connection_state) {
        case IOTC_CONNECTION_STATE_OPENED:
            printf("connected!\n");

            asprintf(&subscribe_topic_command, SUBSCRIBE_TOPIC_COMMAND, CONFIG_GIOT_DEVICE_ID);
            printf("subscribe to topic: \"%s\"\n", subscribe_topic_command);
            iotc_subscribe(in_context_handle, subscribe_topic_command, IOTC_MQTT_QOS_AT_LEAST_ONCE, &iotc_mqttlogic_subscribe_callback, /*user_data=*/NULL);

            asprintf(&subscribe_topic_config, SUBSCRIBE_TOPIC_CONFIG, CONFIG_GIOT_DEVICE_ID);
            printf("subscribe to topic: \"%s\"\n", subscribe_topic_config);
            iotc_subscribe(in_context_handle, subscribe_topic_config, IOTC_MQTT_QOS_AT_LEAST_ONCE, &iotc_mqttlogic_subscribe_callback, /*user_data=*/NULL);

            /* Create a timed task to publish every 10 seconds. */
            delayed_publish_task = iotc_schedule_timed_task(in_context_handle, attachAndSubscribe, 20, 0, /*user_data=*/NULL);
            break;

        case IOTC_CONNECTION_STATE_OPEN_FAILED:
            printf("ERROR!\tConnection has failed reason %d\n\n", state);
            iotc_events_stop();
            break;

        case IOTC_CONNECTION_STATE_CLOSED:
            free(subscribe_topic_command);
            free(subscribe_topic_config);

            if (IOTC_INVALID_TIMED_TASK_HANDLE != delayed_publish_task) {
                iotc_cancel_timed_task(delayed_publish_task);
                delayed_publish_task = IOTC_INVALID_TIMED_TASK_HANDLE;
            }

            if (state == IOTC_STATE_OK) {
                iotc_events_stop();
            } else {
                printf("connection closed - reason %d!\n", state);
                iotc_connect(in_context_handle, conn_data->username, conn_data->password, conn_data->client_id, conn_data->connection_timeout, conn_data->keepalive_timeout, &on_connection_state_changed);
            }
            break;

        default:
            printf("wrong value\n");
            break;
    }
}

static esp_err_t wifi_event_handler(void *ctx, system_event_t *event) {
    switch (event->event_id) {
        case SYSTEM_EVENT_STA_START:
            esp_wifi_connect();
            break;
        case SYSTEM_EVENT_STA_GOT_IP:
            xEventGroupSetBits(wifi_event_group, CONNECTED_BIT);
            break;
        case SYSTEM_EVENT_STA_DISCONNECTED:
            esp_wifi_connect();
            xEventGroupClearBits(wifi_event_group, CONNECTED_BIT);
            break;
        default:
            break;
    }
    return ESP_OK;
}

static void wifi_init(void) {
    tcpip_adapter_init();
    wifi_event_group = xEventGroupCreate();
    ESP_ERROR_CHECK(esp_event_loop_init(wifi_event_handler, NULL));
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
    wifi_config_t wifi_config = {
        .sta = {
            .ssid = CONFIG_ESP_WIFI_SSID,
            .password = CONFIG_ESP_WIFI_PASSWORD,
        },
    };
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));
    ESP_LOGI(TAG, "start the WIFI SSID:[%s]", CONFIG_ESP_WIFI_SSID);
    ESP_ERROR_CHECK(esp_wifi_start());
    ESP_LOGI(TAG, "Waiting for wifi");
    xEventGroupWaitBits(wifi_event_group, CONNECTED_BIT, false, true, portMAX_DELAY);
}

static void example_ble_mesh_set_msg_common(esp_ble_mesh_client_common_param_t *common, esp_ble_mesh_node_t *node, esp_ble_mesh_model_t *model, uint32_t opcode) {
    common->opcode = opcode;
    common->model = model;
    common->ctx.net_idx = prov_key.net_idx;
    common->ctx.app_idx = prov_key.app_idx;
    common->ctx.addr = node->unicast_addr;
    common->ctx.send_ttl = MSG_SEND_TTL;
    common->ctx.send_rel = MSG_SEND_REL;
    common->msg_timeout = MSG_TIMEOUT;
    common->msg_role = MSG_ROLE;
}

static esp_err_t prov_complete(uint16_t node_index, const esp_ble_mesh_octet16_t uuid, uint16_t primary_addr, uint8_t element_num, uint16_t net_idx)
{
    esp_ble_mesh_client_common_param_t common = {0};
    esp_ble_mesh_cfg_client_get_state_t get = {0};
    esp_ble_mesh_node_t *node = NULL;
    char name[10] = {'\0'};
    esp_err_t err;

    ESP_LOGI(TAG, "node_index %u, primary_addr 0x%04x, element_num %u, net_idx 0x%03x", node_index, primary_addr, element_num, net_idx);
    ESP_LOG_BUFFER_HEX("uuid", uuid, ESP_BLE_MESH_OCTET16_LEN);

    server_address = primary_addr;

    sprintf(name, "%s%02x", "NODE-", node_index);
    err = esp_ble_mesh_provisioner_set_node_name(node_index, name);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set node name");
        return ESP_FAIL;
    }

    node = esp_ble_mesh_provisioner_get_node_with_addr(primary_addr);
    if (node == NULL) {
        ESP_LOGE(TAG, "Failed to get node 0x%04x info", primary_addr);
        return ESP_FAIL;
    }

    example_ble_mesh_set_msg_common(&common, node, config_client.model, ESP_BLE_MESH_MODEL_OP_COMPOSITION_DATA_GET);
    get.comp_data_get.page = COMP_DATA_PAGE_0;
    err = esp_ble_mesh_config_client_get_state(&common, &get);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to send Config Composition Data Get");
        return ESP_FAIL;
    }

    return ESP_OK;
}

static void recv_unprov_adv_pkt(uint8_t dev_uuid[ESP_BLE_MESH_OCTET16_LEN], uint8_t addr[BD_ADDR_LEN], esp_ble_mesh_addr_type_t addr_type, uint16_t oob_info, uint8_t adv_type, esp_ble_mesh_prov_bearer_t bearer) {
    esp_ble_mesh_unprov_dev_add_t add_dev = {0};
    esp_err_t err;

    ESP_LOG_BUFFER_HEX("Device address", addr, BD_ADDR_LEN);
    ESP_LOGI(TAG, "Address type 0x%02x, adv type 0x%02x", addr_type, adv_type);
    ESP_LOG_BUFFER_HEX("Device UUID", dev_uuid, ESP_BLE_MESH_OCTET16_LEN);
    ESP_LOGI(TAG, "oob info 0x%04x, bearer %s", oob_info, (bearer & ESP_BLE_MESH_PROV_ADV) ? "PB-ADV" : "PB-GATT");

    memcpy(add_dev.addr, addr, BD_ADDR_LEN);
    add_dev.addr_type = (uint8_t)addr_type;
    memcpy(add_dev.uuid, dev_uuid, ESP_BLE_MESH_OCTET16_LEN);
    add_dev.oob_info = oob_info;
    add_dev.bearer = (uint8_t)bearer;

    err = esp_ble_mesh_provisioner_add_unprov_dev(&add_dev, ADD_DEV_RM_AFTER_PROV_FLAG | ADD_DEV_START_PROV_NOW_FLAG | ADD_DEV_FLUSHABLE_DEV_FLAG);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to start provisioning device");
    }
}

static void example_ble_mesh_provisioning_cb(esp_ble_mesh_prov_cb_event_t event, esp_ble_mesh_prov_cb_param_t *param) {
    switch (event) {
        case ESP_BLE_MESH_PROV_REGISTER_COMP_EVT:
            ESP_LOGI(TAG, "ESP_BLE_MESH_PROV_REGISTER_COMP_EVT, err_code %d", param->prov_register_comp.err_code);
            break;
        case ESP_BLE_MESH_PROVISIONER_PROV_ENABLE_COMP_EVT:
            ESP_LOGI(TAG, "ESP_BLE_MESH_PROVISIONER_PROV_ENABLE_COMP_EVT, err_code %d", param->provisioner_prov_enable_comp.err_code);
            break;
        case ESP_BLE_MESH_PROVISIONER_PROV_DISABLE_COMP_EVT:
            ESP_LOGI(TAG, "ESP_BLE_MESH_PROVISIONER_PROV_DISABLE_COMP_EVT, err_code %d", param->provisioner_prov_disable_comp.err_code);
            break;
        case ESP_BLE_MESH_PROVISIONER_RECV_UNPROV_ADV_PKT_EVT:
            ESP_LOGI(TAG, "ESP_BLE_MESH_PROVISIONER_RECV_UNPROV_ADV_PKT_EVT");
            recv_unprov_adv_pkt(param->provisioner_recv_unprov_adv_pkt.dev_uuid, param->provisioner_recv_unprov_adv_pkt.addr, param->provisioner_recv_unprov_adv_pkt.addr_type, param->provisioner_recv_unprov_adv_pkt.oob_info, param->provisioner_recv_unprov_adv_pkt.adv_type, param->provisioner_recv_unprov_adv_pkt.bearer);
            break;
        case ESP_BLE_MESH_PROVISIONER_PROV_LINK_OPEN_EVT:
            ESP_LOGI(TAG, "ESP_BLE_MESH_PROVISIONER_PROV_LINK_OPEN_EVT, bearer %s",
                    param->provisioner_prov_link_open.bearer == ESP_BLE_MESH_PROV_ADV ? "PB-ADV" : "PB-GATT");
            break;
        case ESP_BLE_MESH_PROVISIONER_PROV_LINK_CLOSE_EVT:
            ESP_LOGI(TAG, "ESP_BLE_MESH_PROVISIONER_PROV_LINK_CLOSE_EVT, bearer %s, reason 0x%02x",
                    param->provisioner_prov_link_close.bearer == ESP_BLE_MESH_PROV_ADV ? "PB-ADV" : "PB-GATT", param->provisioner_prov_link_close.reason);
            break;
        case ESP_BLE_MESH_PROVISIONER_PROV_COMPLETE_EVT:
            prov_complete(param->provisioner_prov_complete.node_idx, param->provisioner_prov_complete.device_uuid, param->provisioner_prov_complete.unicast_addr, param->provisioner_prov_complete.element_num, param->provisioner_prov_complete.netkey_idx);
            break;
        case ESP_BLE_MESH_PROVISIONER_ADD_UNPROV_DEV_COMP_EVT:
            ESP_LOGI(TAG, "ESP_BLE_MESH_PROVISIONER_ADD_UNPROV_DEV_COMP_EVT, err_code %d", param->provisioner_add_unprov_dev_comp.err_code);
            break;
        case ESP_BLE_MESH_PROVISIONER_SET_DEV_UUID_MATCH_COMP_EVT:
            ESP_LOGI(TAG, "ESP_BLE_MESH_PROVISIONER_SET_DEV_UUID_MATCH_COMP_EVT, err_code %d", param->provisioner_set_dev_uuid_match_comp.err_code);
            break;
        case ESP_BLE_MESH_PROVISIONER_SET_NODE_NAME_COMP_EVT:
            ESP_LOGI(TAG, "ESP_BLE_MESH_PROVISIONER_SET_NODE_NAME_COMP_EVT, err_code %d", param->provisioner_set_node_name_comp.err_code);
            if (param->provisioner_set_node_name_comp.err_code == 0) {
                const char *name = esp_ble_mesh_provisioner_get_node_name(param->provisioner_set_node_name_comp.node_index);
                if (name) {
                    ESP_LOGI(TAG, "Node %d name %s", param->provisioner_set_node_name_comp.node_index, name);
                }
            }
            break;
        case ESP_BLE_MESH_PROVISIONER_ADD_LOCAL_APP_KEY_COMP_EVT:
            ESP_LOGI(TAG, "ESP_BLE_MESH_PROVISIONER_ADD_LOCAL_APP_KEY_COMP_EVT, err_code %d", param->provisioner_add_app_key_comp.err_code);
            if (param->provisioner_add_app_key_comp.err_code == 0) {
                prov_key.app_idx = param->provisioner_add_app_key_comp.app_idx;
                esp_err_t err = esp_ble_mesh_provisioner_bind_app_key_to_local_model(PROV_OWN_ADDR, prov_key.app_idx, ESP_BLE_MESH_VND_MODEL_ID_CLIENT, CID_ESP);
                if (err != ESP_OK) {
                    ESP_LOGE(TAG, "Failed to bind AppKey to vendor client");
                }
            }
            break;
        case ESP_BLE_MESH_PROVISIONER_BIND_APP_KEY_TO_MODEL_COMP_EVT:
            ESP_LOGI(TAG, "ESP_BLE_MESH_PROVISIONER_BIND_APP_KEY_TO_MODEL_COMP_EVT, err_code %d", param->provisioner_bind_app_key_to_model_comp.err_code);
            break;
        case ESP_BLE_MESH_PROVISIONER_STORE_NODE_COMP_DATA_COMP_EVT:
            ESP_LOGI(TAG, "ESP_BLE_MESH_PROVISIONER_STORE_NODE_COMP_DATA_COMP_EVT, err_code %d", param->provisioner_store_node_comp_data_comp.err_code);
            break;
        default:
            break;
    }
}

static void example_ble_mesh_parse_node_comp_data(const uint8_t *data, uint16_t length) {
    uint16_t cid, pid, vid, crpl, feat;
    uint16_t loc, model_id, company_id;
    uint8_t nums, numv;
    uint16_t offset;
    int i;

    cid = COMP_DATA_2_OCTET(data, 0);
    pid = COMP_DATA_2_OCTET(data, 2);
    vid = COMP_DATA_2_OCTET(data, 4);
    crpl = COMP_DATA_2_OCTET(data, 6);
    feat = COMP_DATA_2_OCTET(data, 8);
    offset = 10;

    ESP_LOGI(TAG, "********************** Composition Data Start **********************");
    ESP_LOGI(TAG, "* CID 0x%04x, PID 0x%04x, VID 0x%04x, CRPL 0x%04x, Features 0x%04x *", cid, pid, vid, crpl, feat);
    for (; offset < length;) {
        loc = COMP_DATA_2_OCTET(data, offset);
        nums = COMP_DATA_1_OCTET(data, offset + 2);
        numv = COMP_DATA_1_OCTET(data, offset + 3);
        offset += 4;
        ESP_LOGI(TAG, "* Loc 0x%04x, NumS 0x%02x, NumV 0x%02x *", loc, nums, numv);
        for (i = 0; i < nums; i++) {
            model_id = COMP_DATA_2_OCTET(data, offset);
            ESP_LOGI(TAG, "* SIG Model ID 0x%04x *", model_id);
            offset += 2;
        }
        for (i = 0; i < numv; i++) {
            company_id = COMP_DATA_2_OCTET(data, offset);
            model_id = COMP_DATA_2_OCTET(data, offset + 2);
            ESP_LOGI(TAG, "* Vendor Model ID 0x%04x, Company ID 0x%04x *", model_id, company_id);
            offset += 4;
        }
    }
    ESP_LOGI(TAG, "*********************** Composition Data End ***********************");
}

void example_ble_mesh_send_vendor_message(bool resend, char *message) {
    esp_ble_mesh_msg_ctx_t ctx = {0};
    uint32_t opcode;
    esp_err_t err;

    ctx.net_idx = prov_key.net_idx;
    ctx.app_idx = prov_key.app_idx;
    ctx.addr = server_address;
    ctx.send_ttl = MSG_SEND_TTL;
    ctx.send_rel = MSG_SEND_REL;
    opcode = ESP_BLE_MESH_VND_MODEL_OP_SEND;

    uint8_t notify_data[20];

    char *msg1 = message;

    size_t length = strlen(msg1) + 1;

    const char *beg = msg1;
    const char *end = msg1 + length;

    size_t i = 0;
    for (; beg != end; ++beg, ++i) {
        notify_data[i] = (uint8_t)(*beg);
    }

    err = esp_ble_mesh_client_model_send_msg(vendor_client.model, &ctx, opcode, sizeof(notify_data), notify_data, MSG_TIMEOUT, true, MSG_ROLE);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to send vendor message 0x%06x", opcode);
    }
}

static void mqtt_task(void *pvParameters) {
    iotc_crypto_key_data_t iotc_connect_private_key_data;
    iotc_connect_private_key_data.crypto_key_signature_algorithm = IOTC_CRYPTO_KEY_SIGNATURE_ALGORITHM_ES256;
    iotc_connect_private_key_data.crypto_key_union_type = IOTC_CRYPTO_KEY_UNION_TYPE_PEM;
    iotc_connect_private_key_data.crypto_key_union.key_pem.key = (char *)ec_pv_key_start;

    const iotc_state_t error_init = iotc_initialize();

    if (IOTC_STATE_OK != error_init) {
        printf(" iotc failed to initialize, error: %d\n", error_init);
        vTaskDelete(NULL);
    }

    iotc_context = iotc_create_context();
    if (IOTC_INVALID_CONTEXT_HANDLE >= iotc_context) {
        printf(" iotc failed to create context, error: %d\n", -iotc_context);
        vTaskDelete(NULL);
    }

    const uint16_t connection_timeout = 0;
    const uint16_t keepalive_timeout = 20;

    char jwt[IOTC_JWT_SIZE] = {0};
    size_t bytes_written = 0;
    iotc_state_t state = iotc_create_iotcore_jwt(CONFIG_GIOT_PROJECT_ID, 3600, &iotc_connect_private_key_data, jwt, IOTC_JWT_SIZE, &bytes_written);

    if (IOTC_STATE_OK != state) {
        printf("iotc_create_iotcore_jwt returned with error: %ul", state);
        vTaskDelete(NULL);
    }

    char *device_path = NULL;
    asprintf(&device_path, DEVICE_PATH, CONFIG_GIOT_PROJECT_ID, CONFIG_GIOT_LOCATION, CONFIG_GIOT_REGISTRY_ID, CONFIG_GIOT_DEVICE_ID);
    iotc_connect(iotc_context, NULL, jwt, device_path, connection_timeout, keepalive_timeout, &on_connection_state_changed);

    free(device_path);

    iotc_events_process_blocking();

    iotc_delete_context(iotc_context);

    iotc_shutdown();

    vTaskDelete(NULL);
}

static void example_ble_mesh_config_client_cb(esp_ble_mesh_cfg_client_cb_event_t event, esp_ble_mesh_cfg_client_cb_param_t *param) {
    esp_ble_mesh_client_common_param_t common = {0};
    esp_ble_mesh_cfg_client_set_state_t set = {0};
    esp_ble_mesh_node_t *node = NULL;
    esp_err_t err;

    ESP_LOGI(TAG, "Config client, err_code %d, event %u, addr 0x%04x, opcode 0x%04x", param->error_code, event, param->params->ctx.addr, param->params->opcode);

    if (param->error_code) {
        ESP_LOGE(TAG, "Send config client message failed, opcode 0x%04x", param->params->opcode);
        return;
    }

    node = esp_ble_mesh_provisioner_get_node_with_addr(param->params->ctx.addr);
    if (!node) {
        ESP_LOGE(TAG, "Failed to get node 0x%04x info", param->params->ctx.addr);
        return;
    }

    switch (event) {
        case ESP_BLE_MESH_CFG_CLIENT_GET_STATE_EVT:
            if (param->params->opcode == ESP_BLE_MESH_MODEL_OP_COMPOSITION_DATA_GET) {
                ESP_LOG_BUFFER_HEX("Composition data", param->status_cb.comp_data_status.composition_data->data, param->status_cb.comp_data_status.composition_data->len);
                example_ble_mesh_parse_node_comp_data(param->status_cb.comp_data_status.composition_data->data, param->status_cb.comp_data_status.composition_data->len);
                err = esp_ble_mesh_provisioner_store_node_comp_data(param->params->ctx.addr, param->status_cb.comp_data_status.composition_data->data, param->status_cb.comp_data_status.composition_data->len);
                if (err != ESP_OK) {
                    ESP_LOGE(TAG, "Failed to store node composition data");
                    break;
                }

                example_ble_mesh_set_msg_common(&common, node, config_client.model, ESP_BLE_MESH_MODEL_OP_APP_KEY_ADD);
                set.app_key_add.net_idx = prov_key.net_idx;
                set.app_key_add.app_idx = prov_key.app_idx;
                memcpy(set.app_key_add.app_key, prov_key.app_key, ESP_BLE_MESH_OCTET16_LEN);
                err = esp_ble_mesh_config_client_set_state(&common, &set);
                if (err != ESP_OK) {
                    ESP_LOGE(TAG, "Failed to send Config AppKey Add");
                }
            }
            break;
        case ESP_BLE_MESH_CFG_CLIENT_SET_STATE_EVT:
            if (param->params->opcode == ESP_BLE_MESH_MODEL_OP_APP_KEY_ADD) {
                example_ble_mesh_set_msg_common(&common, node, config_client.model, ESP_BLE_MESH_MODEL_OP_MODEL_APP_BIND);
                set.model_app_bind.element_addr = node->unicast_addr;
                set.model_app_bind.model_app_idx = prov_key.app_idx;
                set.model_app_bind.model_id = ESP_BLE_MESH_VND_MODEL_ID_SERVER;
                set.model_app_bind.company_id = CID_ESP;
                err = esp_ble_mesh_config_client_set_state(&common, &set);
                if (err != ESP_OK) {
                    ESP_LOGE(TAG, "Failed to send Config Model App Bind");
                }
            } else if (param->params->opcode == ESP_BLE_MESH_MODEL_OP_MODEL_APP_BIND) {
                ESP_LOGW(TAG, "%s, Provision and config successfully", __func__);
                count += 1;

                if (passed == false && count >= (sizeof(delegateDevices)/sizeof(delegateDevices[0]))) {
                    passed = true;
                    esp_err_t ret = nvs_flash_init();
                    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
                    {
                        ESP_ERROR_CHECK(nvs_flash_erase());
                        ret = nvs_flash_init();
                    }
                    ESP_ERROR_CHECK(ret);
                    ESP_LOGI(TAG, "ESP_WIFI_MODE_STA");
                    wifi_init();
                    obtain_time();
                    xTaskCreate(&mqtt_task, "mqtt_task", 8192, NULL, 5, NULL);
                }
            }
            break;
        case ESP_BLE_MESH_CFG_CLIENT_PUBLISH_EVT:
            if (param->params->opcode == ESP_BLE_MESH_MODEL_OP_COMPOSITION_DATA_STATUS) {
                ESP_LOG_BUFFER_HEX("Composition data", param->status_cb.comp_data_status.composition_data->data, param->status_cb.comp_data_status.composition_data->len);
            }
            break;
        case ESP_BLE_MESH_CFG_CLIENT_TIMEOUT_EVT:
            switch (param->params->opcode) {
                case ESP_BLE_MESH_MODEL_OP_COMPOSITION_DATA_GET: {
                    esp_ble_mesh_cfg_client_get_state_t get = {0};
                    example_ble_mesh_set_msg_common(&common, node, config_client.model, ESP_BLE_MESH_MODEL_OP_COMPOSITION_DATA_GET);
                    get.comp_data_get.page = COMP_DATA_PAGE_0;
                    err = esp_ble_mesh_config_client_get_state(&common, &get);
                    if (err != ESP_OK) {
                        ESP_LOGE(TAG, "Failed to send Config Composition Data Get");
                    }
                    break;
                }
                case ESP_BLE_MESH_MODEL_OP_APP_KEY_ADD:
                    example_ble_mesh_set_msg_common(&common, node, config_client.model, ESP_BLE_MESH_MODEL_OP_APP_KEY_ADD);
                    set.app_key_add.net_idx = prov_key.net_idx;
                    set.app_key_add.app_idx = prov_key.app_idx;
                    memcpy(set.app_key_add.app_key, prov_key.app_key, ESP_BLE_MESH_OCTET16_LEN);
                    err = esp_ble_mesh_config_client_set_state(&common, &set);
                    if (err != ESP_OK) {
                        ESP_LOGE(TAG, "Failed to send Config AppKey Add");
                    }
                    break;
                case ESP_BLE_MESH_MODEL_OP_MODEL_APP_BIND:
                    example_ble_mesh_set_msg_common(&common, node, config_client.model, ESP_BLE_MESH_MODEL_OP_MODEL_APP_BIND);
                    set.model_app_bind.element_addr = node->unicast_addr;
                    set.model_app_bind.model_app_idx = prov_key.app_idx;
                    set.model_app_bind.model_id = ESP_BLE_MESH_VND_MODEL_ID_SERVER;
                    set.model_app_bind.company_id = CID_ESP;
                    err = esp_ble_mesh_config_client_set_state(&common, &set);
                    if (err != ESP_OK) {
                        ESP_LOGE(TAG, "Failed to send Config Model App Bind");
                    }
                    break;
                default:
                    break;
                }
            break;
        default:
            ESP_LOGE(TAG, "Invalid config client event %u", event);
            break;
    }
}

static void example_ble_mesh_custom_model_cb(esp_ble_mesh_model_cb_event_t event, esp_ble_mesh_model_cb_param_t *param) {
    switch (event) {
        case ESP_BLE_MESH_MODEL_OPERATION_EVT:
            if (param->model_operation.opcode == ESP_BLE_MESH_VND_MODEL_OP_STATUS) {
                int64_t end_time = esp_timer_get_time();
                ESP_LOGI(TAG, "Recv 0x%06x, message %s ,time %lldus", param->model_operation.opcode, (char *)param->model_operation.msg, end_time - start_time);
                asprintf(&bleRecievedMessage, TEMPERATURE_DATA, (char *)param->model_operation.msg);
                printf(bleRecievedMessage);
            }
            break;
        case ESP_BLE_MESH_MODEL_SEND_COMP_EVT:
            if (param->model_send_comp.err_code) {
                ESP_LOGE(TAG, "Failed to send message 0x%06x", param->model_send_comp.opcode);
                break;
            }
            start_time = esp_timer_get_time();
            ESP_LOGI(TAG, "Send Command\n");
            break;
        case ESP_BLE_MESH_CLIENT_MODEL_RECV_PUBLISH_MSG_EVT:
        case ESP_BLE_MESH_CLIENT_MODEL_SEND_TIMEOUT_EVT:
        default:
            break;
    }
}

static esp_err_t ble_mesh_init() {
    uint8_t match[2] = {0x32, 0x10};

    esp_err_t err;

    prov_key.net_idx = ESP_BLE_MESH_KEY_PRIMARY;
    prov_key.app_idx = APP_KEY_IDX;
    memset(prov_key.app_key, APP_KEY_OCTET, sizeof(prov_key.app_key));

    esp_ble_mesh_register_prov_callback(example_ble_mesh_provisioning_cb);
    esp_ble_mesh_register_config_client_callback(example_ble_mesh_config_client_cb);
    esp_ble_mesh_register_custom_model_callback(example_ble_mesh_custom_model_cb);

    err = esp_ble_mesh_init(&provision, &composition);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize mesh stack");
        return err;
    }

    err = esp_ble_mesh_client_model_init(&vnd_models[0]);
    if (err) {
        ESP_LOGE(TAG, "Failed to initialize vendor client");
        return err;
    }

    err = esp_ble_mesh_provisioner_set_dev_uuid_match(match, sizeof(match), 0x0, false);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set matching device uuid");
        return err;
    }

    err = esp_ble_mesh_provisioner_prov_enable(ESP_BLE_MESH_PROV_ADV | ESP_BLE_MESH_PROV_GATT);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to enable mesh provisioner");
        return err;
    }

    err = esp_ble_mesh_provisioner_add_local_app_key(prov_key.app_key, prov_key.net_idx, prov_key.app_idx);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to add local AppKey");
        return err;
    }

    ESP_LOGI(TAG, "ESP BLE Mesh Provisioner initialized");
    gpio_set_level(BLINK_GPIO, 0);
    return ESP_OK;
}

void app_main() {
    // Turn on LED to visualize init
    gpio_set_level(BLINK_GPIO, 1);

    esp_err_t err;

    ESP_LOGI(TAG, "Initializing...");

    err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);

    err = bluetooth_init();
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "esp32_bluetooth_init failed (err %d)", err);
        return;
    }

    ble_mesh_get_dev_uuid(dev_uuid);

    /* Initialize the Bluetooth Mesh Subsystem */
    err = ble_mesh_init();
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Bluetooth mesh init failed (err %d)", err);
    }

    // Start message loop, signal this is happen with blinky wonder
    gpio_set_direction(BLINK_GPIO, GPIO_MODE_OUTPUT);
    for (int i = 0; i < 10; i++) {
        /* Blink off (output low) */
        gpio_set_level(BLINK_GPIO, 0);
        vTaskDelay(10);
        gpio_set_level(BLINK_GPIO, 1);
        vTaskDelay(10);
    }
    gpio_set_level(BLINK_GPIO, 0);
}
