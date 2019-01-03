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

#include "/usr/include/gmock/gmock.h"
#include "/usr/include/gtest/gtest.h"

extern "C" {
#include "/usr/local/google/home/sungju/Desktop/sungju/iot-edge-sdk-embedded-c/include/bsp/iotc_bsp_io_net.h"
}

namespace iotctest{
namespace {

iotc_bsp_socket_t test_socket;
const char* test_host = "localhost";
uint16_t test_port = 2000;

TEST(NetTest, SocketCreation){
    const iotc_bsp_io_net_state_t ret = iotc_bsp_io_net_create_socket(&test_socket);
    EXPECT_EQ(ret, IOTC_BSP_IO_NET_STATE_ERROR);
}

TEST(NetTest, SocketConnection){
    const iotc_bsp_io_net_state_t ret = iotc_bsp_io_net_connect(&test_socket, test_host, test_port);
    EXPECT_EQ(ret, IOTC_BSP_IO_NET_STATE_ERROR);
}



} // namespace
} // anmespace iotctest
