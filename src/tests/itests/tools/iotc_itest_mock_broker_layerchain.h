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

#ifndef __IOTC_ITEST_MOCKBROKER_LAYERCHAIN_H__
#define __IOTC_ITEST_MOCKBROKER_LAYERCHAIN_H__

#include "iotc_itest_mock_broker_layer.h"
#include "iotc_layer_default_functions.h"
#include "iotc_layer_macros.h"
#include "iotc_mqtt_codec_layer.h"

enum iotc_mock_broker_codec_layer_stack_order_e {
  IOTC_LAYER_TYPE_MOCKBROKER_BOTTOM = 0,
  IOTC_LAYER_TYPE_MOCKBROKER_MQTT_CODEC,
  IOTC_LAYER_TYPE_MOCKBROKER_TOP
};

#define IOTC_MOCK_BROKER_CODEC_LAYER_CHAIN \
  IOTC_LAYER_TYPE_MOCKBROKER_BOTTOM        \
  , IOTC_LAYER_TYPE_MOCKBROKER_MQTT_CODEC, IOTC_LAYER_TYPE_MOCKBROKER_TOP

IOTC_DECLARE_LAYER_TYPES_BEGIN(itest_mock_broker_codec_layer_chain)
IOTC_LAYER_TYPES_ADD(IOTC_LAYER_TYPE_MOCKBROKER_BOTTOM,
                     iotc_mock_broker_secondary_layer_push,
                     iotc_mock_broker_secondary_layer_pull,
                     iotc_mock_broker_secondary_layer_close,
                     iotc_mock_broker_secondary_layer_close_externally,
                     iotc_mock_broker_secondary_layer_init,
                     iotc_mock_broker_secondary_layer_connect,
                     iotc_layer_default_post_connect)
,
    IOTC_LAYER_TYPES_ADD(IOTC_LAYER_TYPE_MOCKBROKER_MQTT_CODEC,
                         iotc_mqtt_codec_layer_push, iotc_mqtt_codec_layer_pull,
                         iotc_mqtt_codec_layer_close,
                         iotc_mqtt_codec_layer_close_externally,
                         iotc_mqtt_codec_layer_init,
                         iotc_mqtt_codec_layer_connect,
                         iotc_layer_default_post_connect),
    IOTC_LAYER_TYPES_ADD(
        IOTC_LAYER_TYPE_MOCKBROKER_TOP, iotc_mock_broker_layer_push,
        iotc_mock_broker_layer_pull, iotc_mock_broker_layer_close,
        iotc_mock_broker_layer_close_externally, iotc_mock_broker_layer_init,
        iotc_mock_broker_layer_connect, iotc_layer_default_post_connect)
        IOTC_DECLARE_LAYER_TYPES_END()

            IOTC_DECLARE_LAYER_CHAIN_SCHEME(IOTC_LAYER_CHAIN_MOCK_BROKER_CODEC,
                                            IOTC_MOCK_BROKER_CODEC_LAYER_CHAIN);

#endif /* __IOTC_ITEST_MOCKBROKER_LAYERCHAIN_H__ */
