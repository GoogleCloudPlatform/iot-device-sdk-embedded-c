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

#ifndef __IOTC_LAYER_CHAIN_H__
#define __IOTC_LAYER_CHAIN_H__

#include "iotc_debug.h"
#include "iotc_layer.h"
#include "iotc_layer_api.h"
#include "iotc_layer_factory.h"
#include "iotc_macros.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct iotc_layer_chain_s {
  iotc_layer_t* bottom;
  iotc_layer_t* top;
} iotc_layer_chain_t;

static inline iotc_layer_chain_t iotc_layer_chain_connect(
    iotc_layer_t* layers[], const size_t length) {
  IOTC_UNUSED(layers);
  assert(length >= 2 &&
         "you have to connect at least two layers to each other");

  size_t i = 1;
  for (; i < length; ++i) {
    IOTC_LAYERS_CONNECT(layers[i - 1], layers[i]);
  }

  iotc_layer_chain_t ret;

  ret.bottom = layers[0];
  ret.top = layers[length - 1];

  return ret;
}

static inline iotc_layer_chain_t iotc_layer_chain_create(
    const iotc_layer_type_id_t layers_ids[], const size_t length,
    struct iotc_context_data_s* context_data, iotc_layer_type_t layer_types[]) {
  iotc_layer_t* layers[length];
  memset(layers, 0, sizeof(layers));

  size_t i = 0;
  for (; i < length; ++i) {
    void* init_to_null = 0;
    layers[i] = iotc_layer_create(layers_ids[i], init_to_null, context_data,
                                  layer_types);
  }

  return iotc_layer_chain_connect(layers, length);
}

static inline iotc_layer_chain_t iotc_layer_chain_create_with_user_data(
    const iotc_layer_type_id_t layers_ids[], void* user_datas[],
    const size_t length, struct iotc_context_data_s* context_data,
    iotc_layer_type_t layer_types[]) {
  iotc_layer_t* layers[length];
  memset(layers, 0, sizeof(layers));

  size_t i = 0;
  for (; i < length; ++i) {
    layers[i] = iotc_layer_create(layers_ids[i], user_datas[i], context_data,
                                  layer_types);
  }

  return iotc_layer_chain_connect(layers, length);
}

static inline void iotc_layer_chain_delete(iotc_layer_chain_t* chain,
                                           const size_t length,
                                           iotc_layer_type_t layer_types[]) {
  iotc_layer_t* layers[length];
  memset(layers, 0, sizeof(layers));

  assert(chain != 0 && "layer chain must not be 0!");
  assert(chain->bottom->layer_connection.next != 0 &&
         "layer chain must have at least 2 elements!");

  iotc_layer_t* prev = chain->bottom;
  iotc_layer_t* tmp = prev->layer_connection.next;

  unsigned char idx = 0;
  layers[idx] = prev;

  while (tmp) {
    IOTC_LAYERS_DISCONNECT(prev, tmp);

    prev = tmp;
    tmp = tmp->layer_connection.next;

    layers[++idx] = prev;
  }

  size_t i = 0;
  for (; i < length; ++i) {
    iotc_layer_delete(layers[i], layer_types);
  }
}

#ifdef __cplusplus
}
#endif

#endif /* __IOTC_LAYER_CHAIN_H__ */
