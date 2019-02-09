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

#ifndef __IOTC_LAYERS_IDS_H__
#define __IOTC_LAYERS_IDS_H__

#ifdef __cplusplus
extern "C" {
#endif

enum iotc_layer_stack_order_e {
  IOTC_LAYER_TYPE_IO = 0
#ifndef IOTC_NO_TLS_LAYER
  ,
  IOTC_LAYER_TYPE_TLS
#endif
  ,
  IOTC_LAYER_TYPE_MQTT_CODEC,
  IOTC_LAYER_TYPE_MQTT_LOGIC,
  IOTC_LAYER_TYPE_CONTROL_TOPIC
};

#ifdef __cplusplus
}
#endif

#endif /* __IOTC_LAYERS_IDS_H__ */
