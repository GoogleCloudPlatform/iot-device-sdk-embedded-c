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

#ifndef IOTC_BACKOFF_STATUS_API_H
#define IOTC_BACKOFF_STATUS_API_H

#include <stdint.h>

#include "iotc_event_dispatcher_api.h"

#include <iotc_error.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
  IOTC_BACKOFF_CLASS_NONE = 0,
  IOTC_BACKOFF_CLASS_RECOVERABLE,
  IOTC_BACKOFF_CLASS_TERMINAL
} iotc_backoff_class_t;

typedef iotc_vector_index_type_t iotc_backoff_lut_index_t;

typedef struct iotc_backoff_status_s {
  iotc_time_event_handle_t next_update;
  iotc_vector_t* backoff_lut;
  iotc_vector_t* decay_lut;
  iotc_backoff_class_t backoff_class;
  iotc_backoff_lut_index_t backoff_lut_i;
} iotc_backoff_status_t;

extern void iotc_inc_backoff_penalty();

extern void iotc_dec_backoff_penalty();

extern uint32_t iotc_get_backoff_penalty();

extern void iotc_cancel_backoff_event();

#ifdef IOTC_BACKOFF_RESET
extern void iotc_reset_backoff_penalty();
#endif

extern iotc_state_t iotc_backoff_configure_using_data(
    iotc_vector_elem_t* backoff_lut, iotc_vector_elem_t* decay_lut, size_t len,
    iotc_memory_type_t memory_type);

extern void iotc_backoff_release();

extern iotc_backoff_class_t iotc_backoff_classify_state(
    const iotc_state_t state);

extern iotc_backoff_class_t iotc_update_backoff_penalty(
    const iotc_state_t state);

extern iotc_state_t iotc_restart_update_time();

#ifdef __cplusplus
}
#endif

#endif /* IOTC_BACKOFF_STATUS_API_H */
