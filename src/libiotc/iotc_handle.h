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

#ifndef __IOTC_HANDLE_H__
#define __IOTC_HANDLE_H__

#include "iotc_handle.h"
#include "iotc_vector.h"

/*-----------------------------------------------------------------------
 *  TYPEDEFS
 * ----------------------------------------------------------------------- */

typedef int32_t iotc_handle_t;

/*-----------------------------------------------------------------------
 *  PUBLIC FUNCTIONS
 * ----------------------------------------------------------------------- */

void* iotc_object_for_handle(iotc_vector_t* vector, iotc_handle_t handle);
iotc_state_t iotc_find_handle_for_object(iotc_vector_t* vector,
                                         const void* object,
                                         iotc_handle_t* handle);
iotc_state_t iotc_delete_handle_for_object(iotc_vector_t* vector,
                                           const void* object);
iotc_state_t iotc_register_handle_for_object(iotc_vector_t* vector,
                                             const int32_t max_object_cnt,
                                             const void* object);

#endif /* __IOTC_HANDLE_H__ */
