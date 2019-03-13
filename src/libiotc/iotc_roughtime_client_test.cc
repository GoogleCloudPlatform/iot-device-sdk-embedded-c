#include <iotc_roughtime_client.h>

#include "gmock.h"
#include "gtest.h"

namespace iotctest {
namespace {

class IotcRoughtimeClientTest : public testing::Test {
  protected:
    const char* public_key_ = "gD63hSj3ScS+wuOeGrubXlq35N1c5Lby/S+T7MNTjxo=";
    const char* server_address_ = "roughtime.cloudflare.com:2002";
    iotc_roughtime_timedata_t time_data_;
};

TEST_F(IotcRoughtimeClientTest, ReportErrorWithNullPublicKey) {
  EXPECT_EQ(
      iotc_roughtime_getcurrenttime(nullptr, server_address_, &time_data_),
      IOTC_ROUGHTIME_ERROR);
}

TEST_F(IotcRoughtimeClientTest, ReportErrorWithNullServerAddress) {
  EXPECT_EQ(iotc_roughtime_getcurrenttime(public_key_, nullptr, &time_data_),
            IOTC_ROUGHTIME_ERROR);
}

TEST_F(IotcRoughtimeClientTest, ReportErrorWithNullTimeData) {
  EXPECT_EQ(
      iotc_roughtime_getcurrenttime(public_key_, server_address_, nullptr),
      IOTC_ROUGHTIME_ERROR);
}

} // namespace
} // namespace iotctest