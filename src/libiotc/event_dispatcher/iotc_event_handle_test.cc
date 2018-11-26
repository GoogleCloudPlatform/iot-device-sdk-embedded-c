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
#include <memory>

#include "gmock.h"
#include "gtest.h"

#include "iotc_event_handle.h"

namespace iotctest {
namespace {

using ::testing::Eq;
using ::testing::Pointee;

iotc_state_t h0() { return IOTC_STATE_OK; }
iotc_state_t h1(iotc_event_handle_arg1_t) { return IOTC_STATE_OK; }
iotc_state_t h2(iotc_event_handle_arg1_t, iotc_event_handle_arg2_t) {
  return IOTC_STATE_OK;
}
iotc_state_t h3(iotc_event_handle_arg1_t, iotc_event_handle_arg2_t,
                iotc_event_handle_arg3_t) {
  return IOTC_STATE_OK;
}
iotc_state_t h4(iotc_event_handle_arg1_t, iotc_event_handle_arg2_t,
                iotc_event_handle_arg3_t, iotc_event_handle_arg4_t) {
  return IOTC_STATE_OK;
}
iotc_state_t h5(iotc_event_handle_arg1_t, iotc_event_handle_arg2_t,
                iotc_event_handle_arg3_t, iotc_event_handle_arg4_t,
                iotc_event_handle_arg5_t) {
  return IOTC_STATE_OK;
}
iotc_state_t h6(iotc_event_handle_arg1_t, iotc_event_handle_arg2_t,
                iotc_event_handle_arg3_t, iotc_event_handle_arg4_t,
                iotc_event_handle_arg5_t, iotc_event_handle_arg6_t) {
  return IOTC_STATE_OK;
}

TEST(IotcMakeHandle, CreatesCorrectHandleWithZeroArgument) {
  iotc_event_handle_t eh = iotc_make_handle(h0);

  EXPECT_EQ(eh.handle_type, IOTC_EVENT_HANDLE_ARGC0);
  EXPECT_EQ(eh.handlers.h0.fn_argc0, h0);
}

TEST(IotcMakeHandle, CreatesCorrectHandleWithOneArgument) {
  int a1 = 1;
  iotc_event_handle_t eh = iotc_make_handle(h1, &a1);

  EXPECT_EQ(eh.handle_type, IOTC_EVENT_HANDLE_ARGC1);
  EXPECT_EQ(eh.handlers.h1.fn_argc1, h1);
  EXPECT_THAT((int*)eh.handlers.h1.a1, Pointee(Eq(1)));
}

TEST(IotcMakeHandle, CreatesCorrectHandleWithTwoArguments) {
  int a1 = 1;
  int a2 = 2;
  iotc_event_handle_t eh = iotc_make_handle(h2, &a1, &a2);

  EXPECT_EQ(eh.handle_type, IOTC_EVENT_HANDLE_ARGC2);
  EXPECT_EQ(eh.handlers.h2.fn_argc2, h2);
  EXPECT_THAT((int*)eh.handlers.h2.a1, Pointee(Eq(1)));
  EXPECT_THAT((int*)eh.handlers.h2.a2, Pointee(Eq(2)));
}

TEST(IotcMakeHandle, CreatesCorrectHandleWithThreeArguments) {
  int a1 = 1;
  int a2 = 2;
  iotc_state_t a3 = IOTC_STATE_TIMEOUT;
  iotc_event_handle_t eh = iotc_make_handle(h3, &a1, &a2, a3);

  EXPECT_EQ(eh.handle_type, IOTC_EVENT_HANDLE_ARGC3);
  EXPECT_EQ(eh.handlers.h3.fn_argc3, h3);
  EXPECT_THAT((int*)eh.handlers.h3.a1, Pointee(Eq(1)));
  EXPECT_THAT((int*)eh.handlers.h3.a2, Pointee(Eq(2)));
  EXPECT_EQ(eh.handlers.h3.a3, IOTC_STATE_TIMEOUT);
}

TEST(IotcMakeHandle, CreatesCorrectHandleWithFourArguments) {
  int a1 = 1;
  int a2 = 2;
  iotc_state_t a3 = IOTC_STATE_TIMEOUT;
  int a4 = 4;
  iotc_event_handle_t eh = iotc_make_handle(h4, &a1, &a2, a3, &a4);

  EXPECT_EQ(eh.handle_type, IOTC_EVENT_HANDLE_ARGC4);
  EXPECT_EQ(eh.handlers.h4.fn_argc4, h4);
  EXPECT_THAT((int*)eh.handlers.h4.a1, Pointee(Eq(1)));
  EXPECT_THAT((int*)eh.handlers.h4.a2, Pointee(Eq(2)));
  EXPECT_EQ(eh.handlers.h4.a3, IOTC_STATE_TIMEOUT);
  EXPECT_THAT((int*)eh.handlers.h4.a4, Pointee(Eq(4)));
}

TEST(IotcMakeHandle, CreatesCorrectHandleWithFiveArguments) {
  int a1 = 1;
  int a2 = 2;
  iotc_state_t a3 = IOTC_STATE_TIMEOUT;
  int a4 = 4;
  int a5 = 5;
  iotc_event_handle_t eh = iotc_make_handle(h5, &a1, &a2, a3, &a4, &a5);

  EXPECT_EQ(eh.handle_type, IOTC_EVENT_HANDLE_ARGC5);
  EXPECT_EQ(eh.handlers.h5.fn_argc5, h5);
  EXPECT_THAT((int*)eh.handlers.h5.a1, Pointee(Eq(1)));
  EXPECT_THAT((int*)eh.handlers.h5.a2, Pointee(Eq(2)));
  EXPECT_EQ(eh.handlers.h5.a3, IOTC_STATE_TIMEOUT);
  EXPECT_THAT((int*)eh.handlers.h5.a4, Pointee(Eq(4)));
  EXPECT_THAT((int*)eh.handlers.h5.a5, Pointee(Eq(5)));
}

TEST(IotcMakeHandle, CreatesCorrectHandleWithSixArguments) {
  int a1 = 1;
  int a2 = 2;
  iotc_state_t a3 = IOTC_STATE_TIMEOUT;
  int a4 = 4;
  int a5 = 5;
  int a6 = 6;
  iotc_event_handle_t eh = iotc_make_handle(h6, &a1, &a2, a3, &a4, &a5, &a6);

  EXPECT_EQ(eh.handle_type, IOTC_EVENT_HANDLE_ARGC6);
  EXPECT_EQ(eh.handlers.h6.fn_argc6, h6);
  EXPECT_THAT((int*)eh.handlers.h6.a1, Pointee(Eq(1)));
  EXPECT_THAT((int*)eh.handlers.h6.a2, Pointee(Eq(2)));
  EXPECT_EQ(eh.handlers.h6.a3, IOTC_STATE_TIMEOUT);
  EXPECT_THAT((int*)eh.handlers.h6.a4, Pointee(Eq(4)));
  EXPECT_THAT((int*)eh.handlers.h6.a5, Pointee(Eq(5)));
  EXPECT_THAT((int*)eh.handlers.h6.a6, Pointee(Eq(6)));
}

}  // namespace
}  // namespace iotctest
