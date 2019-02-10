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
#include "iotc_allocator.h"
#include "iotc_bsp_mem.h"

extern void* memset(void* ptr, int value, size_t num);

void* __iotc_alloc(size_t byte_count) { return iotc_bsp_mem_alloc(byte_count); }

void* __iotc_calloc(size_t num, size_t byte_count) {
  const size_t size_to_allocate = num * byte_count;
  void* ret = iotc_bsp_mem_alloc(size_to_allocate);

  /* It's unspecified if memset works with NULL pointer. */
  if (NULL != ret) {
    memset(ret, 0, size_to_allocate);
  }

  return ret;
}

void* __iotc_realloc(void* ptr, size_t byte_count) {
  return iotc_bsp_mem_realloc(ptr, byte_count);
}

void __iotc_free(void* ptr) { iotc_bsp_mem_free(ptr); }
