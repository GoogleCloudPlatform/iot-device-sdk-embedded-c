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

#include <iotc_bsp_mem.h>
#include <stdio.h>

#include <projdefs.h>

#include <portable.h>

void* iotc_bsp_mem_alloc(size_t byte_count) {
  // workaround: FreeRTOS's pvPortMalloc crashes if 0 size
  // block is tried to be allocated.
  if (0 == byte_count) {
    byte_count = 1;
  }

  void* ptr = (void*)pvPortMalloc(byte_count);

  if (NULL == ptr) {
    printf("Failed to allocate %d\n", byte_count);
  }

  return ptr;
}

void* iotc_bsp_mem_realloc(void* ptr, size_t byte_count) {
  (void)ptr;
  void* new_ptr = NULL;

  // iotc_bsp_mem_free(ptr);
  // new_ptr = iotc_bsp_mem_alloc(byte_count);

  if (NULL == new_ptr) {
    printf("Failed to reallocate %d\n", byte_count);
    fflush(stdout);
  }

  return new_ptr;
}

void iotc_bsp_mem_free(void* ptr) { vPortFree(ptr); }
