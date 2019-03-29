/* Copyright 2018-2019 Google LLC
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
#include <cassert>

#include "gtest.h"
#include "iotc_memory_checks.h"

namespace iotctest {
namespace {

class IotcHeapCheckTest : public ::testing::Test {
 public:
  IotcHeapCheckTest() { iotc_memory_limiter_tearup(); }
  ~IotcHeapCheckTest() { assert(iotc_memory_limiter_teardown() == 1); }
};

}  // namespace
}  // namespace iotctest
