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

#ifndef __IOTC_FACTORY_CONF_H__
#define __IOTC_FACTORY_CONF_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "iotc_layer.h"

#define SIZE_SUFFIX _SIZE

#define IOTC_DECLARE_LAYER_CHAIN_SCHEME(name, args) \
  static iotc_layer_type_id_t name[] = {args};      \
  static size_t name##SIZE_SUFFIX = sizeof(name) / sizeof(iotc_layer_type_id_t)

#define IOTC_LAYER_CHAIN_SCHEME_LENGTH(name) name##SIZE_SUFFIX

#define IOTC_DECLARE_LAYER_TYPES_BEGIN(name) static iotc_layer_type_t name[] = {
#define IOTC_LAYER_TYPES_ADD(type_id, push, pull, close, close_externally, \
                             init, connect, post_connect)                  \
  {                                                                        \
    type_id, {                                                             \
      push, pull, close, close_externally, init, connect, post_connect     \
    }                                                                      \
  }

#define IOTC_DECLARE_LAYER_TYPES_END() \
  }                                    \
  ;

#ifdef __cplusplus
}
#endif

#endif /* __IOTC_FACTORY_CONF_H__ */
