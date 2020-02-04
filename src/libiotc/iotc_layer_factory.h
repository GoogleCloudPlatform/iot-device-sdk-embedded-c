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

#ifndef __IOTC_LAYER_FACTORY_H__
#define __IOTC_LAYER_FACTORY_H__

#include "iotc_layer.h"
#include "iotc_layer_default_allocators.h"

#ifdef __cplusplus
extern "C" {
#endif

/* simpliest layer initialiser that does simply nothing, but returns the pointer
 * to the initialized layer */
inline static iotc_layer_t* iotc_layer_placement_create(
    iotc_layer_t* layer, void* user_data,
    struct iotc_context_data_s* context_data) {
  layer->user_data = user_data;
  layer->context_data = context_data;

  return layer;
}

/* simpliest layer delete function that does simply nothing, but returns the
 * pointer to given layer which can free()'ed */
inline static iotc_layer_t* iotc_layer_placement_delete(iotc_layer_t* layer) {
  return layer;
}

static inline iotc_layer_t* iotc_layer_alloc(iotc_layer_type_id_t layer_type_id,
                                             iotc_layer_type_t layer_types[]) {
  iotc_layer_type_t* layer_type = &layer_types[layer_type_id];
  return default_layer_heap_alloc(layer_type);
}

static inline void iotc_layer_free(iotc_layer_t* layer,
                                   iotc_layer_type_t layer_types[]) {
  iotc_layer_type_t* layer_type = &layer_types[layer->layer_type_id];
  default_layer_heap_free(layer_type, layer);
}

static inline iotc_layer_t* iotc_layer_place(
    iotc_layer_t* layer, void* user_data,
    struct iotc_context_data_s* context_data) {
  /* PRECONDITION */
  assert(layer != 0);

  return iotc_layer_placement_create(layer, user_data, context_data);
}

static inline void iotc_layer_destroy(iotc_layer_t* layer) {
  /* PRECONDITION */
  assert(layer != 0);

  iotc_layer_placement_delete(layer);
}

static inline iotc_layer_t* iotc_layer_create(
    const iotc_layer_type_id_t layer_type_id, void* user_data,
    struct iotc_context_data_s* context_data, iotc_layer_type_t layer_types[]) {
  iotc_state_t state = IOTC_STATE_OK;
  iotc_layer_t* ret = iotc_layer_alloc(layer_type_id, layer_types);

  IOTC_CHECK_MEMORY(ret, state);

  return iotc_layer_place(ret, user_data, context_data);

err_handling:
  return 0;
}

static inline void iotc_layer_delete(iotc_layer_t* layer,
                                     iotc_layer_type_t layer_types[]) {
  iotc_layer_destroy(layer);
  iotc_layer_free(layer, layer_types);
}

#ifdef __cplusplus
}
#endif

#endif /* __IOTC_LAYER_FACTORY_H__ */
