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

#ifndef __IOTC_DRIVER_CODEC_PROTOBUF_LAYER_DATA_H__
#define __IOTC_DRIVER_CODEC_PROTOBUF_LAYER_DATA_H__

typedef struct iotc_driver_codec_protobuf_layer_data_s {
  iotc_data_desc_t* encoded_protobuf_accumulated;
} iotc_driver_codec_protobuf_layer_data_t;

#endif /* __IOTC_DRIVER_CODEC_PROTOBUF_LAYER_DATA_H__ */
