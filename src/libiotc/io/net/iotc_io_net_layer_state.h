/* Copyright 2018 - 2019 Google LLC
 *
 * This is part of the Google Cloud IoT Device SDK for Embedded C,
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

#ifndef __IOTC_IO_NET_LAYER_STATE_H__
#define __IOTC_IO_NET_LAYER_STATE_H__

#include <stdint.h>
#include "iotc_bsp_io_net.h"

typedef struct iotc_io_net_layer_state_s {
  iotc_bsp_socket_t socket;

  uint16_t layer_connect_cs;
} iotc_io_net_layer_state_t;

#endif /* __IOTC_IO_NET_LAYER_STATE_H__ */
