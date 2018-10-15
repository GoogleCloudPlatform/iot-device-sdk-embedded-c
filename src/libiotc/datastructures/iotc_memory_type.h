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

#ifndef __IOTC_MEMORY_TYPE_H__
#define __IOTC_MEMORY_TYPE_H__

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \enum iotc_memory_type_t
 * \brief describes the memory type so that datastructures know
 * if it should manage ( release / clean ) memory by itself or leave
 * it as it is.
 *
 * IOTC_MEMORY_TYPE_UNKNOWN - should be used as a guard, whenever the status
 * is unset this is mostly for default argument assigment protection
 *
 * IOTC_MEMORY_TYPE_MANAGED - buffer memory will be freed whenever destroy is
 * called.
 *
 * IOTC_MEMORY_TYPE_UNMANAGED - buffer memory is not managed by the entity.
 * Therefore the buffer will not be freed whenever destroy is called.
 **/
typedef enum {
  IOTC_MEMORY_TYPE_UNKNOWN,
  IOTC_MEMORY_TYPE_MANAGED,
  IOTC_MEMORY_TYPE_UNMANAGED
} iotc_memory_type_t;

#ifdef __cplusplus
}
#endif

#endif /* __IOTC_MEMORY_TYPE_H_ */
