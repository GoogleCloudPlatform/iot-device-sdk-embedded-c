/* Copyright 2018-2020 Google LLC
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

#ifndef __IOTC_DRIVER_CODEC_PROTOBUF_LAYER_H__
#define __IOTC_DRIVER_CODEC_PROTOBUF_LAYER_H__

#include "iotc_layer.h"

/**
 * driver codec layer:
 * Main purpose is two folded: first is to encode C struct data
 * to binary format to make an IO layer able to forward message
 * to the other side. Second is to decode incoming binary data
 * to C struct to notify driver logic layer about incoming messages.
 *
 * Google's protobuf solution is applied in this version as
 * an encoder / decoder.
 */

iotc_state_t iotc_driver_codec_protobuf_layer_push(void* context, void* data,
                                                   iotc_state_t state);

iotc_state_t iotc_driver_codec_protobuf_layer_pull(void* context, void* data,
                                                   iotc_state_t state);

iotc_state_t iotc_driver_codec_protobuf_layer_close(void* context, void* data,
                                                    iotc_state_t state);

iotc_state_t iotc_driver_codec_protobuf_layer_close_externally(
    void* context, void* data, iotc_state_t state);

iotc_state_t iotc_driver_codec_protobuf_layer_init(void* context, void* data,
                                                   iotc_state_t state);

iotc_state_t iotc_driver_codec_protobuf_layer_connect(void* context, void* data,
                                                      iotc_state_t state);

#endif /* __IOTC_DRIVER_CODEC_PROTOBUF_LAYER_H__ */
