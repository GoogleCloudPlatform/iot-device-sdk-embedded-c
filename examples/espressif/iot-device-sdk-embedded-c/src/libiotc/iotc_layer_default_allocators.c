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

#include "iotc_allocator.h"
#include "iotc_layer.h"
#include "iotc_macros.h"

#ifdef __cplusplus
extern "C" {
#endif

iotc_layer_t* default_layer_heap_alloc(iotc_layer_type_t* type) {
  iotc_layer_t* ret = (iotc_layer_t*)iotc_alloc(sizeof(iotc_layer_t));
  iotc_state_t state = IOTC_STATE_OK;

  IOTC_CHECK_MEMORY(ret, state);

  memset(ret, 0, sizeof(iotc_layer_t));

  ret->layer_funcs = &type->layer_interface;
  ret->layer_type_id = type->layer_type_id;
  ret->layer_connection.self = ret;

  return ret;

err_handling:
  return 0;
}

void default_layer_heap_free(iotc_layer_type_t* type, iotc_layer_t* layer) {
  IOTC_UNUSED(type);
  iotc_free(layer);
}

#ifdef __cplusplus
}
#endif
