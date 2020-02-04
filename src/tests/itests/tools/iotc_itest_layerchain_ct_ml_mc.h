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

#ifndef __IOTC_ITEST_LAYERCHAIN_CT_ML_MC_H__
#define __IOTC_ITEST_LAYERCHAIN_CT_ML_MC_H__

#include "iotc_layer_macros.h"

#include "iotc_control_topic_layer.h"
#include "iotc_itest_mock_broker_layer.h"
#include "iotc_layer_default_functions.h"
#include "iotc_mock_layer_tls_prev.h"
#include "iotc_mqtt_codec_layer.h"
#include "iotc_mqtt_logic_layer.h"

enum iotc_ct_ml_mc_layer_stack_order_e {
  IOTC_LAYER_TYPE_MOCK_IO = 0,
  IOTC_LAYER_TYPE_MOCK_BROKER,
  IOTC_LAYER_TYPE_MQTT_CODEC_SUT,
  IOTC_LAYER_TYPE_MQTT_LOGIC_SUT,
  IOTC_LAYER_TYPE_CONTROL_TOPIC_SUT
};

#define IOTC_CT_ML_MC_LAYER_CHAIN                                \
  IOTC_LAYER_TYPE_MOCK_IO                                        \
  , IOTC_LAYER_TYPE_MOCK_BROKER, IOTC_LAYER_TYPE_MQTT_CODEC_SUT, \
      IOTC_LAYER_TYPE_MQTT_LOGIC_SUT, IOTC_LAYER_TYPE_CONTROL_TOPIC_SUT

IOTC_DECLARE_LAYER_TYPES_BEGIN(itest_ct_ml_mc_layer_chain)
IOTC_LAYER_TYPES_ADD(IOTC_LAYER_TYPE_MOCK_IO, iotc_mock_layer_tls_prev_push,
                     iotc_mock_layer_tls_prev_pull,
                     iotc_mock_layer_tls_prev_close,
                     iotc_mock_layer_tls_prev_close_externally,
                     iotc_mock_layer_tls_prev_init,
                     iotc_mock_layer_tls_prev_connect,
                     iotc_layer_default_post_connect)
,
    IOTC_LAYER_TYPES_ADD(
        IOTC_LAYER_TYPE_MOCK_BROKER, iotc_mock_broker_layer_push,
        iotc_mock_broker_layer_pull, iotc_mock_broker_layer_close,
        iotc_mock_broker_layer_close_externally, iotc_mock_broker_layer_init,
        iotc_mock_broker_layer_connect, iotc_layer_default_post_connect),
    IOTC_LAYER_TYPES_ADD(IOTC_LAYER_TYPE_MQTT_CODEC_SUT,
                         iotc_mqtt_codec_layer_push, iotc_mqtt_codec_layer_pull,
                         iotc_mqtt_codec_layer_close,
                         iotc_mqtt_codec_layer_close_externally,
                         iotc_mqtt_codec_layer_init,
                         iotc_mqtt_codec_layer_connect,
                         iotc_layer_default_post_connect),
    IOTC_LAYER_TYPES_ADD(IOTC_LAYER_TYPE_MQTT_LOGIC_SUT,
                         iotc_mqtt_logic_layer_push, iotc_mqtt_logic_layer_pull,
                         iotc_mqtt_logic_layer_close,
                         iotc_mqtt_logic_layer_close_externally,
                         iotc_mqtt_logic_layer_init,
                         iotc_mqtt_logic_layer_connect,
                         iotc_layer_default_post_connect),
    IOTC_LAYER_TYPES_ADD(IOTC_LAYER_TYPE_CONTROL_TOPIC_SUT,
                         iotc_control_topic_layer_push,
                         iotc_control_topic_layer_pull,
                         iotc_control_topic_layer_close,
                         iotc_control_topic_layer_close_externally,
                         iotc_control_topic_layer_init,
                         iotc_control_topic_layer_connect,
                         iotc_layer_default_post_connect)
        IOTC_DECLARE_LAYER_TYPES_END()

            IOTC_DECLARE_LAYER_CHAIN_SCHEME(IOTC_LAYER_CHAIN_CT_ML_MC,
                                            IOTC_CT_ML_MC_LAYER_CHAIN);

#endif /* __IOTC_ITEST_LAYERCHAIN_CT_ML_MC_H__ */
