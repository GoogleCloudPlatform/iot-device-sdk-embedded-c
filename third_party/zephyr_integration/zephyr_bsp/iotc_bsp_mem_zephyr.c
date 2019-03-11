/* Copyright 2019 Google LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <iotc_bsp_mem.h>
#include <kernel.h>

// https://docs.zephyrproject.org/1.10.0/kernel/memory/pools.html
K_MEM_POOL_DEFINE(iotc_mem_pool, 128, 32768, 6, 4);

void* iotc_bsp_mem_alloc(size_t byte_count) {
  void* ret = (void*)k_mem_pool_malloc(&iotc_mem_pool, byte_count);
  return ret;
}

void* iotc_bsp_mem_realloc(void* ptr, size_t byte_count) {
  (void)ptr;
  (void)byte_count;
  /* not implemented */
  return NULL;
}

void iotc_bsp_mem_free(void* ptr) { k_free(ptr); }
