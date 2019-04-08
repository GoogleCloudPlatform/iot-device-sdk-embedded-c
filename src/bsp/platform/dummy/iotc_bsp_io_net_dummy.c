/* Copyright 2018-2019 Google LLC
 *
 * This is part of the Google Cloud IoT Device SDK for Embedded C.
 * It is licensed under the BSD 3-Clause license; you may not use this file
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
#include <string.h>
#include "iotc_macros.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef MAX
#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#endif

iotc_bsp_io_net_state_t iotc_bsp_io_net_socket_connect(
    iotc_bsp_socket_t* iotc_socket, const char* host, uint16_t port,
    iotc_bsp_socket_type_t socket_type) {
  IOTC_UNUSED(iotc_socket);
  IOTC_UNUSED(host);
  IOTC_UNUSED(port);
  IOTC_UNUSED(socket_type);
  return IOTC_BSP_IO_NET_STATE_OK;
}

iotc_bsp_io_net_state_t iotc_bsp_io_net_connection_check(
    iotc_bsp_socket_t iotc_socket, const char* host, uint16_t port) {
  IOTC_UNUSED(iotc_socket);
  IOTC_UNUSED(host);
  IOTC_UNUSED(port);

  return IOTC_BSP_IO_NET_STATE_OK;
}

iotc_bsp_io_net_state_t iotc_bsp_io_net_write(iotc_bsp_socket_t iotc_socket,
                                              int* out_written_count,
                                              const uint8_t* buf,
                                              size_t count) {
  IOTC_UNUSED(iotc_socket);
  IOTC_UNUSED(buf);
  *out_written_count = count;
  return IOTC_BSP_IO_NET_STATE_OK;
}

iotc_bsp_io_net_state_t iotc_bsp_io_net_read(iotc_bsp_socket_t iotc_socket,
                                             int* out_read_count, uint8_t* buf,
                                             size_t count) {
  IOTC_UNUSED(iotc_socket);
  IOTC_UNUSED(buf);
  IOTC_UNUSED(count);
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
  IOTC_UNUSED(socket_events_array);
  IOTC_UNUSED(socket_events_array_size);
  IOTC_UNUSED(timeout_sec);

  return IOTC_BSP_IO_NET_STATE_OK;
}

#ifdef __cplusplus
}
#endif