/* Copyright 2018 Google LLC
 *
 * This is part of the Google Cloud IoT Edge Embedded C Client,
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

#ifndef __IOTC_TUPLES_H__
#define __IOTC_TUPLES_H__

#include "iotc_mqtt_message.h"
#include "iotc_tuple.h"

/** \tuple iotc_even_handle_and_void_t may not be used for now. */
/* IOTC_DEF_TUPLE_TYPE( iotc_even_handle_and_void_t, iotc_event_handle_t, void*
 * ) */

IOTC_DEF_TUPLE_TYPE(iotc_mqtt_written_data_t, uint16_t, iotc_mqtt_type_t)

/**
 *  \tuple iotc_uint32_and_void_t will be used to differantiate between
 * different io calls so that the callers of io layer know which call has been
 *          finished.
 */
/* IOTC_DEF_TUPLE_TYPE( iotc_uint16_and_void_t, uint16_t, void* ) */

/**
 * \tuple iotc_uint16_and_uint16_t is used via the sending system to be able
 *          to process sending of msgs related to the given
 *          msg id via coroutines.
 */
/* IOTC_DEF_TUPLE_TYPE( iotc_uint16_and_uint16_t, uint16_t, uint16_t ) */

#endif /* __IOTC_TUPLES_H__ */
