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

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "iotc_allocator.h"
#include "iotc_common.h"
#include "iotc_debug.h"
#include "iotc_io_dummy_layer.h"
#include "iotc_io_dummy_layer_state.h"
#include "iotc_layer_api.h"
#include "iotc_macros.h"

#include <iotc_error.h>

iotc_state_t iotc_io_dummy_layer_push(void* context, void* data,
                                      iotc_state_t in_out_state) {
  IOTC_UNUSED(context);
  IOTC_UNUSED(data);
  IOTC_UNUSED(in_out_state);

  return IOTC_STATE_OK;
}

iotc_state_t iotc_io_dummy_layer_pull(void* context, void* data,
                                      iotc_state_t in_out_state) {
  IOTC_UNUSED(context);
  IOTC_UNUSED(data);
  IOTC_UNUSED(in_out_state);

  return IOTC_STATE_OK;
}

iotc_state_t iotc_io_dummy_layer_close(void* context, void* data,
                                       iotc_state_t in_out_state) {
  IOTC_UNUSED(context);
  IOTC_UNUSED(data);
  IOTC_UNUSED(in_out_state);

  return IOTC_STATE_OK;
}

iotc_state_t iotc_io_dummy_layer_close_externally(void* context, void* data,
                                                  iotc_state_t in_out_state) {
  IOTC_UNUSED(context);
  IOTC_UNUSED(data);
  IOTC_UNUSED(in_out_state);

  return IOTC_PROCESS_CLOSE_EXTERNALLY_ON_NEXT_LAYER(context, data,
                                                     in_out_state);
}

iotc_state_t iotc_io_dummy_layer_init(void* context, void* data,
                                      iotc_state_t in_out_state) {
  IOTC_UNUSED(context);
  IOTC_UNUSED(data);
  IOTC_UNUSED(in_out_state);

  return IOTC_STATE_OK;
}

iotc_state_t iotc_io_dummy_layer_connect(void* context, void* data,
                                         iotc_state_t in_out_state) {
  IOTC_UNUSED(context);
  IOTC_UNUSED(data);
  IOTC_UNUSED(in_out_state);

  return IOTC_STATE_OK;
}
