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

#ifndef __IOTC_IO_NET_LAYER_H__
#define __IOTC_IO_NET_LAYER_H__

#include "iotc_error.h"

iotc_state_t iotc_io_net_layer_init(void* context, void* data,
                                    iotc_state_t in_out_state);

iotc_state_t iotc_io_net_layer_connect(void* context, void* data,
                                       iotc_state_t in_out_state);

iotc_state_t iotc_io_net_layer_push(void* context, void* data,
                                    iotc_state_t in_out_state);

iotc_state_t iotc_io_net_layer_pull(void* context, void* data,
                                    iotc_state_t in_out_state);

iotc_state_t iotc_io_net_layer_close(void* context, void* data,
                                     iotc_state_t in_out_state);

iotc_state_t iotc_io_net_layer_close_externally(void* context, void* data,
                                                iotc_state_t in_out_state);

#endif /* __IOTC_IO_NET_LAYER_H__ */
