/* Copyright 2018-2019 Google LLC
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

#ifndef __IOTC_DRIVER_CONTROL_CHANNEL_LAYERCHAIN_H__
#define __IOTC_DRIVER_CONTROL_CHANNEL_LAYERCHAIN_H__

#include "iotc_driver_codec_protobuf_layer.h"
#include "iotc_driver_logic_layer.h"
#include "iotc_layer_default_functions.h"
#include "iotc_layer_macros.h"

enum iotc_driver_control_channel_layerchain_e {
  IOTC_LAYER_TYPE_DRIVER_IO = 0,
  IOTC_LAYER_TYPE_DRIVER_CODEC,
  IOTC_LAYER_TYPE_DRIVER_LOGIC
};

// io layer selection
#include "iotc_io_net_layer.h"

#define IOTC_DRIVER_CONTROL_CHANNEL_LAYER_CHAIN \
  IOTC_LAYER_TYPE_DRIVER_IO                     \
  , IOTC_LAYER_TYPE_DRIVER_CODEC, IOTC_LAYER_TYPE_DRIVER_LOGIC

IOTC_DECLARE_LAYER_TYPES_BEGIN(iotc_driver_control_channel_layerchain)
IOTC_LAYER_TYPES_ADD(IOTC_LAYER_TYPE_DRIVER_IO, iotc_io_net_layer_push,
                     iotc_io_net_layer_pull, iotc_io_net_layer_close,
                     iotc_io_net_layer_close_externally, iotc_io_net_layer_init,
                     iotc_io_net_layer_connect,
                     &iotc_layer_default_post_connect)
,
    IOTC_LAYER_TYPES_ADD(IOTC_LAYER_TYPE_DRIVER_CODEC,
                         iotc_driver_codec_protobuf_layer_push,
                         iotc_driver_codec_protobuf_layer_pull,
                         iotc_driver_codec_protobuf_layer_close,
                         iotc_driver_codec_protobuf_layer_close_externally,
                         iotc_driver_codec_protobuf_layer_init,
                         iotc_driver_codec_protobuf_layer_connect,
                         &iotc_layer_default_post_connect),
    IOTC_LAYER_TYPES_ADD(
        IOTC_LAYER_TYPE_DRIVER_LOGIC, iotc_driver_logic_layer_push,
        iotc_driver_logic_layer_pull, iotc_driver_logic_layer_close,
        iotc_driver_logic_layer_close_externally, iotc_driver_logic_layer_init,
        iotc_driver_logic_layer_connect, &iotc_layer_default_post_connect)
        IOTC_DECLARE_LAYER_TYPES_END()

            IOTC_DECLARE_LAYER_CHAIN_SCHEME(
                IOTC_LAYER_CHAIN_DRIVER_CONTROL_CHANNEL,
                IOTC_DRIVER_CONTROL_CHANNEL_LAYER_CHAIN);

#endif /* __IOTC_DRIVER_CONTROL_CHANNEL_LAYERCHAIN_H__ */
