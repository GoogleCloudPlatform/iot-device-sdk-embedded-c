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

#ifndef __IOTC_TLS_LAYER_STATE_H__
#define __IOTC_TLS_LAYER_STATE_H__

#include <iotc_bsp_tls.h>
#include <iotc_resource_manager.h>

typedef enum iotc_tls_layer_data_write_state_e {
  IOTC_TLS_LAYER_DATA_NONE = 0,
  IOTC_TLS_LAYER_DATA_WRITING,
  IOTC_TLS_LAYER_DATA_WRITTEN,
} iotc_tls_layer_data_write_state_t;

typedef struct iotc_tls_layer_state_s {
  iotc_bsp_tls_context_t* tls_context;

  iotc_data_desc_t* raw_buffer;
  iotc_data_desc_t* decoded_buffer;
  iotc_data_desc_t* to_write_buffer;

  iotc_event_handle_func_argc3_ptr tls_layer_logic_recv_handler;
  iotc_event_handle_func_argc3_ptr tls_layer_logic_send_handler;

  iotc_resource_manager_context_t* rm_context;

  uint16_t tls_lib_handler_sending_cs;
  uint16_t tls_layer_conn_cs;
  uint16_t tls_layer_send_cs;
  uint16_t tls_layer_recv_cs;

  iotc_tls_layer_data_write_state_t tls_layer_write_state;

} iotc_tls_layer_state_t;

#endif /* __IOTC_TLS_LAYER_STATE_H__ */
