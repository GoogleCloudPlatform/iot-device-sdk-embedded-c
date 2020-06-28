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

#ifndef __IOTC_TYPES_INTERNAL_H__
#define __IOTC_TYPES_INTERNAL_H__

#include <iotc_types.h>
#include "iotc_connection_data.h"
#include "iotc_event_dispatcher_api.h"
#include "iotc_layer_chain.h"
#include "iotc_vector.h"

#ifdef __cplusplus
extern "C" {
#endif

/* -----------------------------------------------------------------------
 * TYPES AND STRUCTURES
 * ----------------------------------------------------------------------- */

typedef enum iotc_protocol_e {
  IOTC_MQTT,
} iotc_protocol_t;

typedef enum iotc_shutdown_state_e {
  IOTC_SHUTDOWN_UNINITIALISED,
  IOTC_SHUTDOWN_STARTED,
} iotc_shutdown_state_t;

/**
 * @brief holds context sensitive data
 *
 *
 **/
typedef struct iotc_context_data_s {
/* At the moment this switch contains only the MQTT data
 * but it may happen that we will add more protocols.
 * The idea is to keep each protocol part here and
 * enable/disable it via flags or defines. Bu for now,
 * let's use that simplified form. */
#if 1 /* MQTT context part. */
  iotc_vector_t* copy_of_handlers_for_topics;
  void* copy_of_q12_unacked_messages_queue; /* We have to use void* because we
                                               don't want to create mqtt logic
                                               layer dependency. */
  void (*copy_of_q12_unacked_messages_queue_dtor_ptr)(
      void**); /* This is dstr for unacked messages. */
  uint16_t
      copy_of_last_msg_id; /* Value of the msg_id for continious session. */
#endif
  /* this is the common part */
  iotc_time_event_handle_t connect_handler;
  /* vector or a list of timeouts */
  iotc_vector_t* io_timeouts;
  iotc_connection_data_t* connection_data;
  iotc_evtd_instance_t* evtd_instance;
  iotc_event_handle_t connection_callback;
  iotc_shutdown_state_t shutdown_state;

  char** updateable_files;
  uint16_t updateable_files_count;
} iotc_context_data_t;

typedef struct iotc_context_s {
  iotc_protocol_t protocol;
  iotc_layer_chain_t layer_chain;
  iotc_context_data_t context_data;
} iotc_context_t;

#ifdef __cplusplus
}
#endif

#endif /* __IOTC_TYPES_INTERNAL_H__ */
