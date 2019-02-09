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

#ifndef __IOTC_ITEST_MOCK_BROKER_LAYER_H__
#define __IOTC_ITEST_MOCK_BROKER_LAYER_H__

#include "iotc_data_desc.h"
#include "iotc_layer.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum iotc_mock_broker_control_init_e {
  CONTROL_CONTINUE,
  CONTROL_ERROR,
  CONTROL_SKIP_CHECK_EXPECTED,
  CONTROL_PULL_PINGREQ_SUPPRESS_RESPONSE
} iotc_mock_broker_control_t;

typedef struct iotc_mock_broker_data_s {
  const char* control_topic_name_broker_in;
  const char* control_topic_name_broker_out;

  iotc_data_desc_t* outgoing_publish_content;
} iotc_mock_broker_data_t;

/**
 * @name    mock broker primary and secondary layers
 * @brief   mock broker is used in integration tests for monitoring and driving
 * libiotc's behavior
 *
 * These layers are part of a mock broker layer chain.
 * (e.g. mock broker primary layer <-> mqtt codec layer <-> mock broker
 * secondary layer) Primary layer push function receives emitted messages from
 * libiotc's layers and does two things with that: forwards it to mock io (which
 * behaves like libiotc's default io layer except of networking) and forward it
 * for decoding to codec layer inside mock broker layer chain.
 * P
 */

/************************************************************************************
 * mock broker primary layer
 *********************************************************
 ************************************************************************************/
/**
 * @name    iotc_mock_broker_layer_push
 * @brief   receives mqtt encoded messages from libiotc's layers
 *
 * Receives emitted messages from libiotc's layers and does two things with
 * that: forwards it to mock io (which behaves like libiotc's default io layer
 * except of networking) and forwards it for decoding to codec layer inside mock
 * broker layer chain. Instrumented function: can be monitored and driven by
 * test case.
 */
iotc_state_t iotc_mock_broker_layer_push(void* context, void* data,
                                         iotc_state_t state);

/**
 * @name    iotc_mock_broker_layer_pull
 * @brief   contains mock broker behavior, receives mqtt decoded messages from
 * mock broker layer chain
 *
 * Receives decoded messages which were previously sent for decode by push
 * function. Implements the mock broker behavior, can be driven by preset values
 * by test case, constructs broker reactions on incoming messages, send replies
 * for encoding to mqtt codec layer inside the mock broker layer chain.
 * Instrumented function: can be monitored and driven by test case.
 */
iotc_state_t iotc_mock_broker_layer_pull(void* context, void* data,
                                         iotc_state_t state);

iotc_state_t iotc_mock_broker_layer_close(void* context, void* data,
                                          iotc_state_t state);

iotc_state_t iotc_mock_broker_layer_close_externally(void* context, void* data,
                                                     iotc_state_t state);

/**
 * @name    iotc_mock_broker_layer_init
 * @brief   mock instrumented function to mimic different behaviours
 */
iotc_state_t iotc_mock_broker_layer_init(void* context, void* data,
                                         iotc_state_t state);

/**
 * @name    iotc_mock_broker_layer_connect
 * @brief   mock instrumented function to mimic different behaviours
 */
iotc_state_t iotc_mock_broker_layer_connect(void* context, void* data,
                                            iotc_state_t state);

/************************************************************************************
 * mock broker secondary layer
 *******************************************************
 ************************************************************************************/
/**
 * @name    iotc_mock_broker_secondary_layer_push
 * @brief   receives mqtt encoded messages from mock broker layer chain
 *
 * Receives emitted messages from mock broker layer chain and forwards it to
 * libiotc's layer chain.
 * Instrumented function: can be monitored and driven by test case.
 */
iotc_state_t iotc_mock_broker_secondary_layer_push(void* context, void* data,
                                                   iotc_state_t state);

iotc_state_t iotc_mock_broker_secondary_layer_pull(void* context, void* data,
                                                   iotc_state_t in_out_state);

iotc_state_t iotc_mock_broker_secondary_layer_close(void* context, void* data,
                                                    iotc_state_t in_out_state);

iotc_state_t iotc_mock_broker_secondary_layer_close_externally(
    void* context, void* data, iotc_state_t in_out_state);

iotc_state_t iotc_mock_broker_secondary_layer_init(void* context, void* data,
                                                   iotc_state_t in_out_state);

iotc_state_t iotc_mock_broker_secondary_layer_connect(
    void* context, void* data, iotc_state_t in_out_state);

#ifdef __cplusplus
}
#endif

#endif /* __IOTC_ITEST_MOCK_BROKER_LAYER_H__ */
