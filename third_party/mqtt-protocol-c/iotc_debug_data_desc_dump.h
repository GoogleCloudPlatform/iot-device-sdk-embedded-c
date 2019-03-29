/* 2018-2019 Google LLC
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

#include "iotc_data_desc.h"

#ifndef __IOTC_DEBUG_DATA_DESC_DUMP_H__
#define __IOTC_DEBUG_DATA_DESC_DUMP_H__


#if IOTC_DEBUG_OUTPUT
extern void iotc_debug_data_desc_dump(const iotc_data_desc_t* buffer);
extern void iotc_debug_data_desc_dump_ascii(const iotc_data_desc_t* buffer);
extern void iotc_debug_data_desc_dump_hex(const iotc_data_desc_t* buffer);
#else
#define iotc_debug_data_desc_dump(...)
#define iotc_debug_data_desc_dump_ascii(...)
#define iotc_debug_data_desc_dump_hex(...)
#endif

#endif /* __IOTC_DEBUG_DATA_DESC_DUMP_H__ */