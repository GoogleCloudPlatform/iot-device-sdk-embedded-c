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

#include <iotc_bsp_io_net.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef MAX
#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#endif

iotc_bsp_io_net_state_t iotc_bsp_io_net_create_socket(
    iotc_bsp_socket_t* iotc_socket, iotc_bsp_protocol_t iotc_protocol) {
  /* TCP with IPv4 */
  if (IOTC_BSP_PROTOCOL_TCP == iotc_protocol){
    (void)iotc_socket;

    *iotc_socket = 0;
  }
  return IOTC_BSP_IO_NET_STATE_OK;
}

iotc_bsp_io_net_state_t iotc_bsp_io_net_connect(iotc_bsp_socket_t* iotc_socket,
                                                const char* host,
                                                uint16_t port,
                                                iotc_bsp_protocol_t iotc_protocol) {
  /* TCP with IPv4 */
  if (IOTC_BSP_PROTOCOL_TCP == iotc_protocol){
  (void)iotc_socket;
  (void)host;
  (void)port;
  }

  return IOTC_BSP_IO_NET_STATE_OK;
}

iotc_bsp_io_net_state_t iotc_bsp_io_net_connection_check(
    iotc_bsp_socket_t iotc_socket, const char* host, uint16_t port) {
  (void)iotc_socket;
  (void)host;
  (void)port;

  return IOTC_BSP_IO_NET_STATE_OK;
}

iotc_bsp_io_net_state_t iotc_bsp_io_net_write(iotc_bsp_socket_t iotc_socket,
                                              int* out_written_count,
                                              const uint8_t* buf,
                                              size_t count) {
  (void)iotc_socket;
  (void)buf;
  *out_written_count = count;
  return IOTC_BSP_IO_NET_STATE_OK;
}

iotc_bsp_io_net_state_t iotc_bsp_io_net_read(iotc_bsp_socket_t iotc_socket,
                                             int* out_read_count, uint8_t* buf,
                                             size_t count) {
  (void)iotc_socket;
  (void)buf;
  (void)count;
  *out_read_count = 0;
  return IOTC_BSP_IO_NET_STATE_OK;
}

iotc_bsp_io_net_state_t iotc_bsp_io_net_close_socket(
    iotc_bsp_socket_t* iotc_socket) {
  *iotc_socket = 0;
  return IOTC_BSP_IO_NET_STATE_OK;
}

iotc_bsp_io_net_state_t iotc_bsp_io_net_select(
    iotc_bsp_socket_events_t* socket_events_array,
    size_t socket_events_array_size, long timeout_sec) {
  (void)socket_events_array;
  (void)socket_events_array_size;
  (void)timeout_sec;

  return IOTC_BSP_IO_NET_STATE_OK;
}

#ifdef __cplusplus
}
#endif
