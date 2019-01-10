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

#include "bsp/iotc_bsp_io_net.h"
#include "gmock.h"
#include "gtest.h"

#include <iostream>

namespace iotctest {
namespace {
using namespace std;

iotc_bsp_socket_t test_socket;
// const char *host_ipv6 = "localhost";
const char *host_ipv4 = "127.0.0.1";
const char msg[] = "hello\n";
uint16_t port = 2000;
char port_str[10];
int out_written_count;

TEST(NetTest, SocketCreation) {
  sprintf(port_str, "%d", port);

  iotc_bsp_io_net_state_t ret =
      iotc_bsp_io_net_socket_connect(&test_socket, host_ipv4, port_str);
  cerr << "create : " << ret << endl;
  EXPECT_EQ(ret, IOTC_BSP_IO_NET_STATE_OK);

  ret = iotc_bsp_io_net_connection_check(test_socket, host_ipv4, port);
  cerr << "conn check: " << ret << endl;
  EXPECT_EQ(ret, IOTC_BSP_IO_NET_STATE_OK);

  ret = iotc_bsp_io_net_write(test_socket, &out_written_count, (uint8_t *)msg,
                              strlen(msg));
  cerr << "write : " << ret << endl;
  cerr << "bytes written: " << out_written_count << endl;
  EXPECT_EQ(ret, IOTC_BSP_IO_NET_STATE_OK);
  EXPECT_EQ(out_written_count, (int)strlen(msg));

  ret = iotc_bsp_io_net_close_socket(&test_socket);
  cerr << "close : " << ret << endl;
  EXPECT_EQ(ret, IOTC_BSP_IO_NET_STATE_OK);
}

} // namespace
} // namespace iotctest