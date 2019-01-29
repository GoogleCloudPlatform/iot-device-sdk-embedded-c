/* Copyright 2018 Google LLC
 *
 * This is part of the Google Cloud IoT Edge Embedded C Client,
 * it is licensed under the BSD 3-Clause license; you may not use this file
 * except in compliance with the License.
 *
 * You may obtain a copy of the License at:
 *  https://opensource.org/licenses/BSD-3-Clause
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "iotc_config.h"
#include "iotc_connection_data_internal.h"
#include "iotc_event_dispatcher_api.h"
#include "iotc_helpers.h"

iotc_connection_data_t* iotc_alloc_connection_data(
    const char* host, uint16_t port, const char* project_id,
    const char* device_path,
    const iotc_crypto_key_data_t* private_key_data,
    uint32_t jwt_expiration_period_sec, uint16_t connection_timeout,
    uint16_t keepalive_timeout, iotc_session_type_t session_type) {
  return iotc_alloc_connection_data_lastwill(
      host, port, project_id, device_path, private_key_data,
      jwt_expiration_period_sec, connection_timeout, keepalive_timeout,
      session_type, NULL, NULL, (iotc_mqtt_qos_t)0, (iotc_mqtt_retain_t)0);
}

iotc_connection_data_t* iotc_alloc_connection_data_lastwill(
    const char* host, uint16_t port, const char* project_id,
    const char* device_path,
    const iotc_crypto_key_data_t* private_key_data,
    uint32_t jwt_expiration_period_sec, uint16_t connection_timeout,
    uint16_t keepalive_timeout, iotc_session_type_t session_type,
    const char* will_topic, const char* will_message, iotc_mqtt_qos_t will_qos,
    iotc_mqtt_retain_t will_retain) {
  iotc_state_t state = IOTC_STATE_OK;

  IOTC_ALLOC(iotc_connection_data_t, ret, state);

  if (host) {
    ret->host = iotc_str_dup(host);
    IOTC_CHECK_MEMORY(ret->host, state);
  }

  if (project_id) {
    ret->project_id = iotc_str_dup(project_id);
    IOTC_CHECK_MEMORY(ret->project_id, state);
  }

  if (device_path) {
    ret->device_path = iotc_str_dup(device_path);
    IOTC_CHECK_MEMORY(ret->device_path, state);
  }

  if (private_key_data) {
    ret->private_key_data = iotc_crypto_private_key_data_dup(private_key_data);
    IOTC_CHECK_MEMORY(ret->private_key_data, state);
  }

  if (will_topic) {
    ret->will_topic = iotc_str_dup(will_topic);
    IOTC_CHECK_MEMORY(ret->will_topic, state);
  }

  if (will_message) {
    ret->will_message = iotc_str_dup(will_message);
    IOTC_CHECK_MEMORY(ret->will_message, state);
  }

  ret->port = port;
  ret->jwt_expiration_period_sec = jwt_expiration_period_sec;
  ret->connection_timeout = connection_timeout;
  ret->keepalive_timeout = keepalive_timeout;
  ret->session_type = session_type;
  ret->will_qos = will_qos;
  ret->will_retain = will_retain;

  return ret;

err_handling:
  if (ret) {
    IOTC_SAFE_FREE(ret->host);
    IOTC_SAFE_FREE(ret->project_id);
    IOTC_SAFE_FREE(ret->device_path);
    iotc_crypto_private_key_data_free(ret->private_key_data);
    IOTC_SAFE_FREE(ret->will_topic);
    IOTC_SAFE_FREE(ret->will_message);
  }
  IOTC_SAFE_FREE(ret);
  return 0;
}

iotc_state_t iotc_connection_data_update(
    iotc_connection_data_t* conn_data, const char* host, uint16_t port,
    const char* project_id, const char* device_path,
    const iotc_crypto_key_data_t* private_key_data,
    uint32_t jwt_expiration_period_sec, uint16_t connection_timeout,
    uint16_t keepalive_timeout, iotc_session_type_t session_type) {
  return iotc_connection_data_update_lastwill(
      conn_data, host, port, project_id, device_path, private_key_data,
      jwt_expiration_period_sec, connection_timeout, keepalive_timeout,
      session_type, NULL, NULL, (iotc_mqtt_qos_t)0, (iotc_mqtt_retain_t)0);
}

iotc_state_t iotc_connection_data_update_lastwill(
    iotc_connection_data_t* conn_data, const char* host, uint16_t port,
    const char* project_id, const char* device_path,
    const iotc_crypto_key_data_t* private_key_data,
    uint32_t jwt_expiration_period_sec, uint16_t connection_timeout,
    uint16_t keepalive_timeout, iotc_session_type_t session_type,
    const char* will_topic, const char* will_message, iotc_mqtt_qos_t will_qos,
    iotc_mqtt_retain_t will_retain) {
  iotc_state_t local_state = IOTC_STATE_OK;

  if (NULL != host && strcmp(host, conn_data->host) != 0) {
    IOTC_SAFE_FREE(conn_data->host);
    conn_data->host = iotc_str_dup(host);
    IOTC_CHECK_MEMORY(conn_data->host, local_state);
  }

  if (NULL != project_id && strcmp(project_id, conn_data->project_id) != 0) {
    IOTC_SAFE_FREE(conn_data->project_id);
    conn_data->project_id = iotc_str_dup(project_id);
    IOTC_CHECK_MEMORY(conn_data->project_id, local_state);
  }

  if (NULL != device_path && strcmp(device_path, conn_data->device_path) != 0) {
    IOTC_SAFE_FREE(conn_data->device_path);
    conn_data->device_path = iotc_str_dup(device_path);
    IOTC_CHECK_MEMORY(conn_data->device_path, local_state);
  }

  if (NULL != private_key_data &&
      iotc_crypto_private_key_data_cmp(private_key_data,
                                       conn_data->private_key_data) != 0) {
    iotc_crypto_private_key_data_free(conn_data->private_key_data);
    conn_data->private_key_data =
        iotc_crypto_private_key_data_dup(private_key_data);
    IOTC_CHECK_MEMORY(conn_data->private_key_data, local_state);
  }

  if (NULL != will_topic && strcmp(will_topic, conn_data->will_topic) != 0) {
    IOTC_SAFE_FREE(conn_data->will_topic);
    conn_data->will_topic = iotc_str_dup(will_topic);
    IOTC_CHECK_MEMORY(conn_data->will_topic, local_state);
  }

  if (NULL != will_message &&
      strcmp(will_message, conn_data->will_message) != 0) {
    IOTC_SAFE_FREE(conn_data->will_message);
    conn_data->will_message = iotc_str_dup(will_message);
    IOTC_CHECK_MEMORY(conn_data->will_message, local_state);
  }

  conn_data->port = port;
  conn_data->jwt_expiration_period_sec = jwt_expiration_period_sec;
  conn_data->connection_timeout = connection_timeout;
  conn_data->keepalive_timeout = keepalive_timeout;
  conn_data->session_type = session_type;
  conn_data->will_qos = will_qos;
  conn_data->will_retain = will_retain;

err_handling:
  return local_state;
}

void iotc_free_connection_data(iotc_connection_data_t** data) {
  if (*data) {
    IOTC_SAFE_FREE((*data)->host);
    IOTC_SAFE_FREE((*data)->project_id);
    IOTC_SAFE_FREE((*data)->device_path);
    iotc_crypto_private_key_data_free((*data)->private_key_data);
    IOTC_SAFE_FREE((*data)->will_topic);
    IOTC_SAFE_FREE((*data)->will_message);
  }

  IOTC_SAFE_FREE(*data);
}

int iotc_crypto_private_key_data_cmp(
    const iotc_crypto_key_data_t* key1,
    const iotc_crypto_key_data_t* key2) {
  if (key1 == NULL && key2 == NULL) {
    return 0;
  }

  if (key1 == NULL || key2 == NULL) {
    return -1;
  }

  if (key1->crypto_key_signature_algorithm !=
      key2->crypto_key_signature_algorithm) {
    return -1;
  }

  if (key1->crypto_key_union_type != key2->crypto_key_union_type) {
    return -1;
  }

  switch (key1->crypto_key_union_type) {
    case IOTC_CRYPTO_KEY_UNION_TYPE_PEM:
      if (key1->crypto_key_union.key_pem.key == NULL &&
          key2->crypto_key_union.key_pem.key == NULL) {
        return 0;
      } else if (key1->crypto_key_union.key_pem.key == NULL ||
                 key2->crypto_key_union.key_pem.key == NULL) {
        return -1;
      } else {
        return strcmp(key1->crypto_key_union.key_pem.key,
                      key2->crypto_key_union.key_pem.key);
      }
    case IOTC_CRYPTO_KEY_UNION_TYPE_SLOT_ID:
      if (key1->crypto_key_union.key_slot.slot_id ==
          key2->crypto_key_union.key_slot.slot_id) {
        return 0;
      } else {
        return -1;
      }
    case IOTC_CRYPTO_KEY_UNION_TYPE_CUSTOM:
      if (key1->crypto_key_union.key_custom.data_size !=
          key2->crypto_key_union.key_custom.data_size) {
        return -1;
      } else {
        return memcmp(key1->crypto_key_union.key_custom.data,
                      key2->crypto_key_union.key_custom.data,
                      key1->crypto_key_union.key_custom.data_size);
      }
  }

  return -1;  // Should never happen
}

iotc_crypto_key_data_t* iotc_crypto_private_key_data_dup(
    const iotc_crypto_key_data_t* src_key) {
  if (src_key == NULL) {
    return NULL;
  }

  iotc_state_t state = IOTC_STATE_OK;
  IOTC_ALLOC(iotc_crypto_key_data_t, dup, state);

  dup->crypto_key_signature_algorithm =
      src_key->crypto_key_signature_algorithm;
  dup->crypto_key_union_type = src_key->crypto_key_union_type;

  switch (src_key->crypto_key_union_type) {
    case IOTC_CRYPTO_KEY_UNION_TYPE_PEM:
      dup->crypto_key_union.key_pem.key =
          iotc_str_dup(src_key->crypto_key_union.key_pem.key);
      break;
    case IOTC_CRYPTO_KEY_UNION_TYPE_SLOT_ID:
      dup->crypto_key_union.key_slot.slot_id =
          src_key->crypto_key_union.key_slot.slot_id;
      break;
    case IOTC_CRYPTO_KEY_UNION_TYPE_CUSTOM:
      dup->crypto_key_union.key_custom.data =
          iotc_alloc(src_key->crypto_key_union.key_custom.data_size);
      memcpy(dup->crypto_key_union.key_custom.data,
             src_key->crypto_key_union.key_custom.data,
             src_key->crypto_key_union.key_custom.data_size);
      dup->crypto_key_union.key_custom.data_size =
          src_key->crypto_key_union.key_custom.data_size;
      break;
  }

  return dup;

err_handling:
  iotc_crypto_private_key_data_free(dup);
  return NULL;
}

void iotc_crypto_private_key_data_free(iotc_crypto_key_data_t* key) {
  if (key != NULL) {
    switch (key->crypto_key_union_type) {
      case IOTC_CRYPTO_KEY_UNION_TYPE_PEM:
        IOTC_SAFE_FREE(key->crypto_key_union.key_pem.key);
        break;
      case IOTC_CRYPTO_KEY_UNION_TYPE_SLOT_ID:
        break;
      case IOTC_CRYPTO_KEY_UNION_TYPE_CUSTOM:
        IOTC_SAFE_FREE(key->crypto_key_union.key_custom.data);
        break;
    }
  }

  IOTC_SAFE_FREE(key);
}
