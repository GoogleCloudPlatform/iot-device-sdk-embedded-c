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

#include <iotc_bsp_mem.h>
// #include <stdlib.h>
// #include <stdio.h>

#include <kernel.h>

// https://docs.zephyrproject.org/1.10.0/kernel/memory/pools.html
K_MEM_POOL_DEFINE(iotc_mem_pool, 128, 32768, 16, 4);

void* iotc_bsp_mem_alloc(size_t byte_count) {
  void* ret = (void*)k_mem_pool_malloc(&iotc_mem_pool, byte_count);
  // printf("%s, ret: %p, size: %u\n", __FUNCTION__, ret, byte_count);
  return ret;
}

void* iotc_bsp_mem_realloc(void* ptr, size_t byte_count) {
  // return (void*)realloc(ptr, byte_count);
  (void)ptr;
  (void)byte_count;
  return NULL;
}

void iotc_bsp_mem_free(void* ptr) {
  // printf("[%s, ptr: %p\n", __FUNCTION__, ptr);
  k_free(ptr);
  // printf("]%s\n", __FUNCTION__);
}
