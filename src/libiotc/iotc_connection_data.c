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
    const char* host, uint16_t port, const char* username,
    const char* password, const char* client_id, uint16_t connection_timeout,
    uint16_t keepalive_timeout, iotc_session_type_t session_type) {
  return iotc_alloc_connection_data_lastwill(
      host, port, username, password, client_id, connection_timeout,
      keepalive_timeout, session_type, NULL, NULL, (iotc_mqtt_qos_t)0,
      (iotc_mqtt_retain_t)0);
}

iotc_connection_data_t* iotc_alloc_connection_data_lastwill(
    const char* host, uint16_t port, const char* username,
    const char* password, const char* client_id,
    uint16_t connection_timeout, uint16_t keepalive_timeout,
    iotc_session_type_t session_type, const char* will_topic,
    const char* will_message, iotc_mqtt_qos_t will_qos,
    iotc_mqtt_retain_t will_retain) {
  iotc_state_t state = IOTC_STATE_OK;

  IOTC_ALLOC(iotc_connection_data_t, ret, state);

  if (host) {
    ret->host = iotc_str_dup(host);
    IOTC_CHECK_MEMORY(ret->host, state);
  }

  if (username) {
    ret->username = iotc_str_dup(username);
    IOTC_CHECK_MEMORY(ret->username, state);
  }

  if (password) {
    ret->password = iotc_str_dup(password);
    IOTC_CHECK_MEMORY(ret->password, state);
  }

  if (client_id) {
    ret->client_id = iotc_str_dup(client_id);
    IOTC_CHECK_MEMORY(ret->client_id, state);
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
  ret->connection_timeout = connection_timeout;
  ret->keepalive_timeout = keepalive_timeout;
  ret->session_type = session_type;
  ret->will_qos = will_qos;
  ret->will_retain = will_retain;

  return ret;

err_handling:
  if (ret) {
    IOTC_SAFE_FREE(ret->host);
    IOTC_SAFE_FREE(ret->username);
    IOTC_SAFE_FREE(ret->password);
    IOTC_SAFE_FREE(ret->client_id);
    IOTC_SAFE_FREE(ret->will_topic);
    IOTC_SAFE_FREE(ret->will_message);
  }
  IOTC_SAFE_FREE(ret);
  return 0;
}

iotc_state_t iotc_connection_data_update(
    iotc_connection_data_t* conn_data, const char* host, uint16_t port,
    const char* username, const char* password, const char* client_id,
    uint16_t connection_timeout, uint16_t keepalive_timeout,
    iotc_session_type_t session_type) {
  return iotc_connection_data_update_lastwill(
      conn_data, host, port, username, password, client_id, connection_timeout,
      keepalive_timeout, session_type, NULL, NULL, (iotc_mqtt_qos_t)0,
      (iotc_mqtt_retain_t)0);
}

iotc_state_t iotc_connection_data_update_lastwill(
    iotc_connection_data_t* conn_data, const char* host, uint16_t port,
    const char* username, const char* password, const char* client_id,
    uint16_t connection_timeout, uint16_t keepalive_timeout,
    iotc_session_type_t session_type, const char* will_topic,
    const char* will_message, iotc_mqtt_qos_t will_qos,
    iotc_mqtt_retain_t will_retain) {
  iotc_state_t local_state = IOTC_STATE_OK;

  if (NULL != host && strcmp(host, conn_data->host) != 0) {
    IOTC_SAFE_FREE(conn_data->host);
    conn_data->host = iotc_str_dup(host);
    IOTC_CHECK_MEMORY(conn_data->host, local_state);
  }

  if (NULL != username && strcmp(username, conn_data->username) != 0) {
    IOTC_SAFE_FREE(conn_data->username);
    conn_data->username = iotc_str_dup(username);
    IOTC_CHECK_MEMORY(conn_data->username, local_state);
  }

  if (NULL != password && strcmp(password, conn_data->password) != 0) {
    IOTC_SAFE_FREE(conn_data->password);
    conn_data->password = iotc_str_dup(password);
    IOTC_CHECK_MEMORY(conn_data->password, local_state);
  }

  if (NULL != client_id && strcmp(client_id, conn_data->client_id) != 0) {
    IOTC_SAFE_FREE(conn_data->client_id);
    conn_data->client_id = iotc_str_dup(client_id);
    IOTC_CHECK_MEMORY(conn_data->client_id, local_state);
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
    IOTC_SAFE_FREE((*data)->username);
    IOTC_SAFE_FREE((*data)->password);
    IOTC_SAFE_FREE((*data)->client_id);
    IOTC_SAFE_FREE((*data)->will_topic);
    IOTC_SAFE_FREE((*data)->will_message);
  }

  IOTC_SAFE_FREE(*data);
}

