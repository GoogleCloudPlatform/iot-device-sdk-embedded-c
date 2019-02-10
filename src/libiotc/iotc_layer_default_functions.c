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

#include "iotc_layer_default_functions.h"

iotc_state_t iotc_layer_default_post_connect(void* context, void* data,
                                             iotc_state_t in_out_state) {
  IOTC_LAYER_FUNCTION_PRINT_FUNCTION_DIGEST();

  void* layer_data = (void*)IOTC_THIS_LAYER(context)->user_data;

  if (NULL == layer_data) {
    return in_out_state;
  }

  IOTC_PROCESS_POST_CONNECT_ON_PREV_LAYER(context, data, in_out_state);

  return in_out_state;
}
