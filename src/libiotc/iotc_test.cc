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

#include "gtest.h"

#include "iotc.h"
#include "iotc_error.h"
#include "iotc_heapcheck_test.h"
#include "iotc_version.h"

namespace iotctest {
namespace {

class IotcCore : public IotcHeapCheckTest {
 public:
  IotcCore() { iotc_initialize(); }
  ~IotcCore() { iotc_shutdown(); }
};

TEST_F(IotcCore, InitShutDownCycleFreesUpAllMemory) {
  // As part of setup, iotc_initialize is called.
  iotc_shutdown();
  iotc_initialize();
  // As part of teardown, iotc_shutdown is called and memory leaks are checked.
}

TEST_F(IotcCore, CreateContextCreatesValidContextHandle) {
  auto ctx_handle = iotc_create_context();
  EXPECT_GT(ctx_handle, IOTC_INVALID_CONTEXT_HANDLE);

  // We need to delete the context, otherwise we leak memory and fail the test.
  iotc_delete_context(ctx_handle);
}

TEST_F(IotcCore, DISABLED_DeleteContextResetsContextHandle) {
  auto ctx_handle = iotc_create_context();
  ASSERT_GT(ctx_handle, IOTC_INVALID_CONTEXT_HANDLE);

  auto state = iotc_delete_context(ctx_handle);
  EXPECT_EQ(state, IOTC_STATE_OK);

  // This does not hold currently, thus the test is disabled.
  EXPECT_EQ(ctx_handle, IOTC_INVALID_CONTEXT_HANDLE);
}

TEST(Iotc, VersionNumbersAreCorrect) {
  EXPECT_EQ(iotc_major, IOTC_MAJOR);
  EXPECT_EQ(iotc_minor, IOTC_MINOR);
  EXPECT_EQ(iotc_revision, IOTC_REVISION);
}

TEST(Iotc, InitializeReturnsOK) {
  EXPECT_EQ(iotc_initialize(), IOTC_STATE_OK);

  // Need to call iotc_shutdown() to release memory.
  iotc_shutdown();
}

}  // namespace
}  // namespace iotctest
