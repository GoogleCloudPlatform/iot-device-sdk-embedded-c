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
#include "gmock.h"
#include "gtest.h"

#include "iotc_event_dispatcher_api.h"
#include "iotc_event_handle.h"

namespace iotctest {
namespace {

using ::testing::Eq;
using ::testing::Return;

class FreeFunctions {
 public:
  virtual ~FreeFunctions() = default;
  virtual iotc_state_t H0() const = 0;
  virtual iotc_state_t H1(iotc_event_handle_arg1_t a1) const = 0;
  virtual iotc_state_t H2(iotc_event_handle_arg1_t a1,
                          iotc_event_handle_arg2_t a2) const = 0;
  virtual iotc_state_t H3(iotc_event_handle_arg1_t a1,
                          iotc_event_handle_arg2_t a2,
                          iotc_event_handle_arg3_t a3) const = 0;
  virtual iotc_state_t H4(iotc_event_handle_arg1_t a1,
                          iotc_event_handle_arg2_t a2,
                          iotc_event_handle_arg3_t a3,
                          iotc_event_handle_arg4_t a4) const = 0;
  virtual iotc_state_t H5(iotc_event_handle_arg1_t a1,
                          iotc_event_handle_arg2_t a2,
                          iotc_event_handle_arg3_t a3,
                          iotc_event_handle_arg4_t a4,
                          iotc_event_handle_arg5_t a5) const = 0;
  virtual iotc_state_t H6(iotc_event_handle_arg1_t a1,
                          iotc_event_handle_arg2_t a2,
                          iotc_event_handle_arg3_t a3,
                          iotc_event_handle_arg4_t a4,
                          iotc_event_handle_arg5_t a5,
                          iotc_event_handle_arg6_t a6) const = 0;
};

class FreeFunctionsMock : public FreeFunctions {
 public:
  MOCK_CONST_METHOD0(H0, iotc_state_t());
  MOCK_CONST_METHOD1(H1, iotc_state_t(iotc_event_handle_arg1_t a1));
  MOCK_CONST_METHOD2(H2, iotc_state_t(iotc_event_handle_arg1_t a1,
                                      iotc_event_handle_arg2_t a2));
  MOCK_CONST_METHOD3(H3, iotc_state_t(iotc_event_handle_arg1_t a1,
                                      iotc_event_handle_arg2_t a2,
                                      iotc_event_handle_arg3_t a3));
  MOCK_CONST_METHOD4(H4, iotc_state_t(iotc_event_handle_arg1_t a1,
                                      iotc_event_handle_arg2_t a2,
                                      iotc_event_handle_arg3_t a3,
                                      iotc_event_handle_arg4_t a4));
  MOCK_CONST_METHOD5(H5, iotc_state_t(iotc_event_handle_arg1_t a1,
                                      iotc_event_handle_arg2_t a2,
                                      iotc_event_handle_arg3_t a3,
                                      iotc_event_handle_arg4_t a4,
                                      iotc_event_handle_arg5_t a5));
  MOCK_CONST_METHOD6(H6, iotc_state_t(iotc_event_handle_arg1_t a1,
                                      iotc_event_handle_arg2_t a2,
                                      iotc_event_handle_arg3_t a3,
                                      iotc_event_handle_arg4_t a4,
                                      iotc_event_handle_arg5_t a5,
                                      iotc_event_handle_arg6_t a6));
};

class IotcEvtdExecuteHandleTest : public ::testing::Test {
 public:
  static void SetUpTestCase() {
    mock_fns_ = new ::testing::StrictMock<FreeFunctionsMock>();
  }

  static void TearDownTestCase() { delete mock_fns_; }

 protected:
  static iotc_state_t h0() { return mock_fns_->H0(); }
  static iotc_state_t h1(iotc_event_handle_arg1_t a1) {
    return mock_fns_->H1(a1);
  }
  static iotc_state_t h2(iotc_event_handle_arg1_t a1,
                         iotc_event_handle_arg2_t a2) {
    return mock_fns_->H2(a1, a2);
  }
  static iotc_state_t h3(iotc_event_handle_arg1_t a1,
                         iotc_event_handle_arg2_t a2,
                         iotc_event_handle_arg3_t a3) {
    return mock_fns_->H3(a1, a2, a3);
  }
  static iotc_state_t h4(iotc_event_handle_arg1_t a1,
                         iotc_event_handle_arg2_t a2,
                         iotc_event_handle_arg3_t a3,
                         iotc_event_handle_arg4_t a4) {
    return mock_fns_->H4(a1, a2, a3, a4);
  }
  static iotc_state_t h5(iotc_event_handle_arg1_t a1,
                         iotc_event_handle_arg2_t a2,
                         iotc_event_handle_arg3_t a3,
                         iotc_event_handle_arg4_t a4,
                         iotc_event_handle_arg5_t a5) {
    return mock_fns_->H5(a1, a2, a3, a4, a5);
  }
  static iotc_state_t h6(iotc_event_handle_arg1_t a1,
                         iotc_event_handle_arg2_t a2,
                         iotc_event_handle_arg3_t a3,
                         iotc_event_handle_arg4_t a4,
                         iotc_event_handle_arg5_t a5,
                         iotc_event_handle_arg6_t a6) {
    return mock_fns_->H6(a1, a2, a3, a4, a5, a6);
  }
  static FreeFunctionsMock* mock_fns_;
};

FreeFunctionsMock* IotcEvtdExecuteHandleTest::mock_fns_ = nullptr;

TEST_F(IotcEvtdExecuteHandleTest, ExecutesHandleWithZeroArgument) {
  iotc_event_handle_t eh = iotc_make_handle(IotcEvtdExecuteHandleTest::h0);

  EXPECT_CALL(*IotcEvtdExecuteHandleTest::mock_fns_, H0())
      .Times(1)
      .WillOnce(Return(IOTC_STATE_OK));
  EXPECT_EQ(iotc_evtd_execute_handle(&eh), IOTC_STATE_OK);
}

TEST_F(IotcEvtdExecuteHandleTest, ExecutesHandleWithOneArgument) {
  int arg1 = 1;
  iotc_event_handle_t eh =
      iotc_make_handle(IotcEvtdExecuteHandleTest::h1, &arg1);

  EXPECT_CALL(*IotcEvtdExecuteHandleTest::mock_fns_, H1(Eq(&arg1)))
      .Times(1)
      .WillOnce(Return(IOTC_STATE_OK));
  EXPECT_EQ(iotc_evtd_execute_handle(&eh), IOTC_STATE_OK);
}

TEST_F(IotcEvtdExecuteHandleTest, ExecutesHandleWithTwoArguments) {
  int arg1 = 1;
  int arg2 = 2;
  iotc_event_handle_t eh =
      iotc_make_handle(IotcEvtdExecuteHandleTest::h2, &arg1, &arg2);

  EXPECT_CALL(*IotcEvtdExecuteHandleTest::mock_fns_, H2(Eq(&arg1), Eq(&arg2)))
      .Times(1)
      .WillOnce(Return(IOTC_STATE_OK));
  EXPECT_EQ(iotc_evtd_execute_handle(&eh), IOTC_STATE_OK);
}

TEST_F(IotcEvtdExecuteHandleTest, ExecutesHandleWithThreeArguments) {
  int arg1 = 1;
  int arg2 = 2;
  iotc_state_t arg3 = IOTC_STATE_TIMEOUT;
  iotc_event_handle_t eh =
      iotc_make_handle(IotcEvtdExecuteHandleTest::h3, &arg1, &arg2, arg3);

  EXPECT_CALL(*IotcEvtdExecuteHandleTest::mock_fns_,
              H3(Eq(&arg1), Eq(&arg2), Eq(arg3)))
      .Times(1)
      .WillOnce(Return(IOTC_STATE_OK));
  EXPECT_EQ(iotc_evtd_execute_handle(&eh), IOTC_STATE_OK);
}

TEST_F(IotcEvtdExecuteHandleTest, ExecutesHandleWithFourArguments) {
  int arg1 = 1;
  int arg2 = 2;
  iotc_state_t arg3 = IOTC_STATE_TIMEOUT;
  int arg4 = 4;
  iotc_event_handle_t eh = iotc_make_handle(IotcEvtdExecuteHandleTest::h4,
                                            &arg1, &arg2, arg3, &arg4);

  EXPECT_CALL(*IotcEvtdExecuteHandleTest::mock_fns_,
              H4(Eq(&arg1), Eq(&arg2), Eq(arg3), Eq(&arg4)))
      .Times(1)
      .WillOnce(Return(IOTC_STATE_OK));
  EXPECT_EQ(iotc_evtd_execute_handle(&eh), IOTC_STATE_OK);
}

TEST_F(IotcEvtdExecuteHandleTest, ExecutesHandleWithFiveArguments) {
  int arg1 = 1;
  int arg2 = 2;
  iotc_state_t arg3 = IOTC_STATE_TIMEOUT;
  int arg4 = 4;
  int arg5 = 5;
  iotc_event_handle_t eh = iotc_make_handle(IotcEvtdExecuteHandleTest::h5,
                                            &arg1, &arg2, arg3, &arg4, &arg5);

  EXPECT_CALL(*IotcEvtdExecuteHandleTest::mock_fns_,
              H5(Eq(&arg1), Eq(&arg2), Eq(arg3), Eq(&arg4), Eq(&arg5)))
      .Times(1)
      .WillOnce(Return(IOTC_STATE_OK));
  EXPECT_EQ(iotc_evtd_execute_handle(&eh), IOTC_STATE_OK);
}

TEST_F(IotcEvtdExecuteHandleTest, ExecutesHandleWithSixArguments) {
  int arg1 = 1;
  int arg2 = 2;
  iotc_state_t arg3 = IOTC_STATE_TIMEOUT;
  int arg4 = 4;
  int arg5 = 5;
  int arg6 = 6;
  iotc_event_handle_t eh = iotc_make_handle(
      IotcEvtdExecuteHandleTest::h6, &arg1, &arg2, arg3, &arg4, &arg5, &arg6);

  EXPECT_CALL(
      *IotcEvtdExecuteHandleTest::mock_fns_,
      H6(Eq(&arg1), Eq(&arg2), Eq(arg3), Eq(&arg4), Eq(&arg5), Eq(&arg6)))
      .Times(1)
      .WillOnce(Return(IOTC_STATE_OK));
  EXPECT_EQ(iotc_evtd_execute_handle(&eh), IOTC_STATE_OK);
}

}  // namespace
}  // namespace iotctest
