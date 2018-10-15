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

#ifndef __IOTC_BACKOFF_LUT_CONFIG_H__
#define __IOTC_BACKOFF_LUT_CONFIG_H__

#include "iotc_vector.h"

#ifdef __cplusplus
extern "C" {
#endif

static const iotc_vector_elem_t IOTC_BACKOFF_LUT[] = {
    IOTC_VEC_ELEM(IOTC_VEC_VALUE_UI32(0)),
    IOTC_VEC_ELEM(IOTC_VEC_VALUE_UI32(2)),
    IOTC_VEC_ELEM(IOTC_VEC_VALUE_UI32(4)),
    IOTC_VEC_ELEM(IOTC_VEC_VALUE_UI32(8)),
    IOTC_VEC_ELEM(IOTC_VEC_VALUE_UI32(16)),
    IOTC_VEC_ELEM(IOTC_VEC_VALUE_UI32(32)),
    IOTC_VEC_ELEM(IOTC_VEC_VALUE_UI32(64)),
    IOTC_VEC_ELEM(IOTC_VEC_VALUE_UI32(128)),
    IOTC_VEC_ELEM(IOTC_VEC_VALUE_UI32(256)),
    IOTC_VEC_ELEM(IOTC_VEC_VALUE_UI32(512))};

static const iotc_vector_elem_t IOTC_DECAY_LUT[] = {
    IOTC_VEC_ELEM(IOTC_VEC_VALUE_UI32(4)),
    IOTC_VEC_ELEM(IOTC_VEC_VALUE_UI32(4)),
    IOTC_VEC_ELEM(IOTC_VEC_VALUE_UI32(8)),
    IOTC_VEC_ELEM(IOTC_VEC_VALUE_UI32(16)),
    IOTC_VEC_ELEM(IOTC_VEC_VALUE_UI32(30)),
    IOTC_VEC_ELEM(IOTC_VEC_VALUE_UI32(30)),
    IOTC_VEC_ELEM(IOTC_VEC_VALUE_UI32(30)),
    IOTC_VEC_ELEM(IOTC_VEC_VALUE_UI32(30)),
    IOTC_VEC_ELEM(IOTC_VEC_VALUE_UI32(30)),
    IOTC_VEC_ELEM(IOTC_VEC_VALUE_UI32(30))};

#ifdef __cplusplus
}
#endif

#endif /* __IOTC_BACKOFF_LUT_CONFIG_H__ */
