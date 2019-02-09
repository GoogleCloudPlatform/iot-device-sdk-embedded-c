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

#ifndef __IOTC_ITEST_LAYERCHAIN_MQTTLOGIC_LAYER_H__
#define __IOTC_ITEST_LAYERCHAIN_MQTTLOGIC_LAYER_H__

#include "iotc_layer_macros.h"

#include "iotc_layer_default_functions.h"
#include "iotc_mock_layer_mqttlogic_next.h"
#include "iotc_mock_layer_mqttlogic_prev.h"
#include "iotc_mqtt_logic_layer.h"

enum iotc_mqttlogic_layer_stack_order_e {
  IOTC_LAYER_TYPE_MOCK_MQTTLOGIC_PREV = 0,
  IOTC_LAYER_TYPE_SUT_MQTTLOGIC,
  IOTC_LAYER_TYPE_MOCK_MQTTLOGIC_NEXT
};

#define IOTC_MQTTLOGIC_LAYER_CHAIN    \
  IOTC_LAYER_TYPE_MOCK_MQTTLOGIC_PREV \
  , IOTC_LAYER_TYPE_SUT_MQTTLOGIC, IOTC_LAYER_TYPE_MOCK_MQTTLOGIC_NEXT

IOTC_DECLARE_LAYER_TYPES_BEGIN(iotc_itest_layer_chain_mqttlogic)
IOTC_LAYER_TYPES_ADD(IOTC_LAYER_TYPE_MOCK_MQTTLOGIC_PREV,
                     iotc_mock_layer_mqttlogic_prev_push,
                     iotc_mock_layer_mqttlogic_prev_pull,
                     iotc_mock_layer_mqttlogic_prev_close,
                     iotc_mock_layer_mqttlogic_prev_close_externally,
                     iotc_mock_layer_mqttlogic_prev_init,
                     iotc_mock_layer_mqttlogic_prev_connect,
                     iotc_layer_default_post_connect)
,
    IOTC_LAYER_TYPES_ADD(IOTC_LAYER_TYPE_SUT_MQTTLOGIC,
                         iotc_mqtt_logic_layer_push, iotc_mqtt_logic_layer_pull,
                         iotc_mqtt_logic_layer_close,
                         iotc_mqtt_logic_layer_close_externally,
                         iotc_mqtt_logic_layer_init,
                         iotc_mqtt_logic_layer_connect,
                         iotc_mqtt_logic_layer_post_connect),
    IOTC_LAYER_TYPES_ADD(IOTC_LAYER_TYPE_MOCK_MQTTLOGIC_NEXT,
                         iotc_mock_layer_mqttlogic_next_push,
                         iotc_mock_layer_mqttlogic_next_pull,
                         iotc_mock_layer_mqttlogic_next_close,
                         iotc_mock_layer_mqttlogic_next_close_externally,
                         iotc_mock_layer_mqttlogic_next_init,
                         iotc_mock_layer_mqttlogic_next_connect,
                         iotc_layer_default_post_connect)
        IOTC_DECLARE_LAYER_TYPES_END()

            IOTC_DECLARE_LAYER_CHAIN_SCHEME(IOTC_LAYER_CHAIN_MQTTLOGIC,
                                            IOTC_MQTTLOGIC_LAYER_CHAIN);

#endif /* __IOTC_ITEST_LAYERCHAIN_MQTTLOGIC_LAYER_H__ */
