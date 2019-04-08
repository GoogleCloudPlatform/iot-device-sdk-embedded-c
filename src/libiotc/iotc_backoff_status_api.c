/* Copyright 2018-2019 Google LLC
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

#include "iotc_backoff_status_api.h"
#include "iotc_bsp_rng.h"
#include "iotc_globals.h"

#ifdef __cplusplus
extern "C" {
#endif

/* local functions */
static iotc_state_t iotc_apply_cooldown(void);

void iotc_inc_backoff_penalty() {
  iotc_globals.backoff_status.backoff_lut_i =
      IOTC_MIN(iotc_globals.backoff_status.backoff_lut_i + 1,
               iotc_globals.backoff_status.backoff_lut->elem_no - 1);

  iotc_restart_update_time();
}

void iotc_dec_backoff_penalty() {
  iotc_globals.backoff_status.backoff_lut_i =
      IOTC_MAX(iotc_globals.backoff_status.backoff_lut_i - 1, 0);
}

uint32_t iotc_get_backoff_penalty() {
  const iotc_backoff_lut_index_t prev_backoff_index =
      IOTC_MAX(iotc_globals.backoff_status.backoff_lut_i - 1, 0);

  const int32_t prev_backoff_base_value =
      iotc_globals.backoff_status.backoff_lut->array[prev_backoff_index]
          .selector_t.ui32_value;

  /* full_range = previous backoff value | 0 if index == 0 */
  const int32_t full_range = (iotc_globals.backoff_status.backoff_lut_i == 0)
                                 ? 0
                                 : prev_backoff_base_value;

  /* base_value = current backoff value */
  const int32_t base_value =
      iotc_globals.backoff_status.backoff_lut
          ->array[iotc_globals.backoff_status.backoff_lut_i]
          .selector_t.ui32_value;

  /* half_range = max( previous backoff value * 0.5, 1 ) */
  const int32_t half_range = IOTC_MAX(full_range / 2, 1);

  /* rand_value = random( 0, full_range ) */
  const int32_t rand_value = iotc_bsp_rng_get() % (full_range + 1);

  /* backoff_value =
   *      base_value + random( -0.5 * prev_backoff_base_value
   *                          , 0.5 * prev_backoff_base_value )
   * because if:
   *
   *      rand_value == 0:
   *
   *          then: backoff_value = base_value +
   *              0 - prev_backoff_base_value * 0.5
   *
   *      rand_value == prev_backoff_base_value:
   *
   *          then: backoff_value = base_value +
   *              prev_backoff_base_value - prev_backoff_base_value * 0.5
   *          so: backoff_value = base_value + prev_backoff_base_value * 0.5
   */
  const int32_t backoff_value = base_value + rand_value - half_range;

  /*
   * Clamp the ret_value so that it's not lesser than backoff_lut[ 0 ]
   */
  const int32_t ret_value = IOTC_MAX(
      backoff_value, (int32_t)iotc_globals.backoff_status.backoff_lut->array[0]
                         .selector_t.ui32_value);

  return ret_value;
}

void iotc_cancel_backoff_event() {
  if (NULL != iotc_globals.backoff_status.next_update.ptr_to_position) {
    iotc_evtd_cancel(iotc_globals.evtd_instance,
                     &iotc_globals.backoff_status.next_update);
  }
}

#ifdef IOTC_BACKOFF_RESET
void iotc_reset_backoff_penalty() {
  iotc_globals.backoff_status.backoff_lut_i = 0;

  iotc_cancel_backoff_event();
}
#endif

static void iotc_backoff_release_luts(void) {
  if (iotc_globals.backoff_status.backoff_lut != NULL) {
    iotc_globals.backoff_status.backoff_lut =
        iotc_vector_destroy(iotc_globals.backoff_status.backoff_lut);
  }

  if (iotc_globals.backoff_status.decay_lut != NULL) {
    iotc_globals.backoff_status.decay_lut =
        iotc_vector_destroy(iotc_globals.backoff_status.decay_lut);
  }
}

iotc_state_t iotc_backoff_configure_using_data(iotc_vector_elem_t* backoff_lut,
                                               iotc_vector_elem_t* decay_lut,
                                               size_t len,
                                               iotc_memory_type_t memory_type) {
  if (backoff_lut == NULL || decay_lut == NULL || len == 0) {
    return IOTC_INVALID_PARAMETER;
  }

  iotc_state_t local_state = IOTC_STATE_OK;

  iotc_backoff_release_luts();

  iotc_globals.backoff_status.backoff_lut =
      iotc_vector_create_from(backoff_lut, len, memory_type);

  IOTC_CHECK_MEMORY(iotc_globals.backoff_status.backoff_lut, local_state);

  iotc_globals.backoff_status.decay_lut =
      iotc_vector_create_from(decay_lut, len, memory_type);

  IOTC_CHECK_MEMORY(iotc_globals.backoff_status.decay_lut, local_state);

err_handling:
  return local_state;
}

extern void iotc_backoff_release() { iotc_backoff_release_luts(); }

iotc_backoff_class_t iotc_backoff_classify_state(const iotc_state_t state) {
  switch (state) {
    case IOTC_CONNECTION_RESET_BY_PEER_ERROR:
    case IOTC_MQTT_UNACCEPTABLE_PROTOCOL_VERSION:
    case IOTC_MQTT_IDENTIFIER_REJECTED:
    case IOTC_MQTT_BAD_USERNAME_OR_PASSWORD:
    case IOTC_MQTT_NOT_AUTHORIZED:
      return IOTC_BACKOFF_CLASS_TERMINAL;
    case IOTC_STATE_OK:
    case IOTC_STATE_WRITTEN:
      return IOTC_BACKOFF_CLASS_NONE;
    default:
      return IOTC_BACKOFF_CLASS_RECOVERABLE;
  }
}

iotc_backoff_class_t iotc_update_backoff_penalty(const iotc_state_t state) {
  iotc_backoff_class_t backoff_class = iotc_backoff_classify_state(state);

  iotc_globals.backoff_status.backoff_class = backoff_class;

  switch (backoff_class) {
    case IOTC_BACKOFF_CLASS_TERMINAL:
    case IOTC_BACKOFF_CLASS_RECOVERABLE:
      iotc_inc_backoff_penalty();
      iotc_debug_format("inc backoff index: %d",
                        iotc_globals.backoff_status.backoff_lut_i);
      break;
    case IOTC_BACKOFF_CLASS_NONE:
      break;
    default:
      iotc_debug_logger("invalid backoff class!");
      assert(0);
  }

  return backoff_class;
}

iotc_state_t iotc_restart_update_time() {
  iotc_state_t local_state = IOTC_STATE_OK;
  iotc_evtd_instance_t* event_dispatcher = iotc_globals.evtd_instance;

  if (NULL != iotc_globals.backoff_status.next_update.ptr_to_position) {
    local_state = iotc_evtd_restart(
        event_dispatcher, &iotc_globals.backoff_status.next_update,
        iotc_globals.backoff_status.decay_lut
            ->array[iotc_globals.backoff_status.backoff_lut_i]
            .selector_t.ui32_value);
  } else {
    local_state = iotc_evtd_execute_in(
        event_dispatcher, iotc_make_handle(&iotc_apply_cooldown),
        iotc_globals.backoff_status.decay_lut
            ->array[iotc_globals.backoff_status.backoff_lut_i]
            .selector_t.ui32_value,
        &iotc_globals.backoff_status.next_update);
  }

  return local_state;
}

static iotc_state_t iotc_apply_cooldown(void) {
  /* clearing the event pointer is the first thing to do */
  assert(NULL == iotc_globals.backoff_status.next_update.ptr_to_position);

  if (iotc_globals.backoff_status.backoff_class == IOTC_BACKOFF_CLASS_NONE) {
    iotc_dec_backoff_penalty();
    iotc_debug_format("dec backoff index: %d",
                      iotc_globals.backoff_status.backoff_lut_i);
  }

  /* if the backoff lut index is greater than 0 */
  if (iotc_globals.backoff_status.backoff_lut_i > 0) {
    return iotc_restart_update_time();
  }

  return IOTC_STATE_OK;
}

#ifdef __cplusplus
}
#endif
