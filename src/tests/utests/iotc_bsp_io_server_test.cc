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
#include "iotc_bsp_io_net.h"
#include "iotc_test_echoserver.h"

#include <stdio.h>
#include <thread>

namespace iotctest {
namespace {
using namespace std;

class ServerTest : public ::testing::Test {
public:
  int protocol_type, recv_len, out_written_count, state;
  const uint16_t kTestPort = 2000;
  char* listening_addr;
  const char* test_msg = "hello\n";
  char recv_buf[kBufferSize];
  iotc_bsp_socket_events_t socket_evts[1];
  iotc_bsp_socket_t test_socket;

  iotc_bsp_io_net_state_t create_client(uint16_t sock_type);
  iotc_bsp_io_net_state_t write_client();
  iotc_bsp_io_net_state_t read_client();
  iotc_bsp_io_net_state_t close_client();
};

iotc_bsp_io_net_state_t ServerTest::create_client(uint16_t sock_type) {
  if (iotc_bsp_io_net_socket_connect(&test_socket, listening_addr, kTestPort,
                                     sock_type) != IOTC_BSP_IO_NET_STATE_OK) {
    perror("[Client] Error connecting socket");
    return IOTC_BSP_IO_NET_STATE_ERROR;
  }
  if (iotc_bsp_io_net_connection_check(test_socket, listening_addr,
                                       kTestPort) != IOTC_BSP_IO_NET_STATE_OK) {
    perror("[Client]Error checking connection");
    return IOTC_BSP_IO_NET_STATE_ERROR;
  }

  return IOTC_BSP_IO_NET_STATE_OK;
}

iotc_bsp_io_net_state_t ServerTest::write_client() {
  memset(socket_evts, 0, sizeof(iotc_bsp_socket_events_t));
  socket_evts[0].iotc_socket = test_socket;
  socket_evts[0].in_socket_want_write = 1;
  bool ready_to_write = false;
  while (!ready_to_write) {
    state = iotc_bsp_io_net_select(socket_evts, 1, kTimeoutSeconds);
    switch (state) {
    case IOTC_BSP_IO_NET_STATE_OK:
      if (socket_evts[0].out_socket_can_write == 1) {
        ready_to_write = true;
      }
      break;
    default:
      break;
    }
  }
  return iotc_bsp_io_net_write(test_socket, &out_written_count,
                               (uint8_t*)test_msg, strlen(test_msg));
}

iotc_bsp_io_net_state_t ServerTest::read_client() {
  memset(socket_evts, 0, sizeof(iotc_bsp_socket_events_t));
  socket_evts[0].iotc_socket = test_socket;
  socket_evts[0].in_socket_want_read = 1;
  bool ready_to_read = false;
  while (!ready_to_read) {
    state = iotc_bsp_io_net_select(socket_evts, 1, kTimeoutSeconds);
    switch (state) {
    case IOTC_BSP_IO_NET_STATE_OK:
      if (socket_evts[0].out_socket_can_read == 1) {
        ready_to_read = true;
      }
      break;
    default:
      break;
    }
  }
  return iotc_bsp_io_net_read(test_socket, &recv_len,
                              reinterpret_cast<uint8_t*>(recv_buf),
                              sizeof(recv_buf));
}

iotc_bsp_io_net_state_t ServerTest::close_client() {
  return iotc_bsp_io_net_close_socket(&test_socket);
}

TEST_F(ServerTest, TcpIpv4EndToEndCommunicationWorks) {
  listening_addr = const_cast<char*>("127.0.0.1");
  EchoTestServer* test_server =
      new EchoTestServer(SOCK_STREAM, kTestPort, listening_addr);
  test_server->create_server();
  create_client(SOCK_STREAM);

  std::thread server_thread(&EchoTestServer::run_server, test_server);
  write_client();
  read_client();

  test_server->stop_server();
  close_client();
  server_thread.join();

  EXPECT_STREQ(test_server->get_recv_buf(), test_msg);
  EXPECT_STREQ(recv_buf, test_msg);
}

TEST_F(ServerTest, UdpIpv4EndToEndCommunicationWorks) {
  listening_addr = const_cast<char*>("127.0.0.1");
  EchoTestServer* test_server =
      new EchoTestServer(SOCK_DGRAM, kTestPort, listening_addr);
  test_server->create_server();
  create_client(SOCK_DGRAM);

  std::thread server_thread(&EchoTestServer::run_server, test_server);
  write_client();
  read_client();

  test_server->stop_server();
  close_client();
  server_thread.join();

  EXPECT_STREQ(test_server->get_recv_buf(), test_msg);
  EXPECT_STREQ(recv_buf, test_msg);
}

TEST_F(ServerTest, TcpIpv6EndToEndCommunicationWorks) {
  listening_addr = const_cast<char*>("::1");
  EchoTestServer* test_server =
      new EchoTestServer(SOCK_STREAM, kTestPort, listening_addr);
  test_server->create_server();
  create_client(SOCK_STREAM);

  std::thread server_thread(&EchoTestServer::run_server, test_server);
  write_client();
  read_client();

  test_server->stop_server();
  close_client();
  server_thread.join();

  EXPECT_STREQ(test_server->get_recv_buf(), test_msg);
  EXPECT_STREQ(recv_buf, test_msg);
}

TEST_F(ServerTest, UdpIpv6EndToEndCommunicationWorks) {
  listening_addr = const_cast<char*>("::1");
  EchoTestServer* test_server =
      new EchoTestServer(SOCK_DGRAM, kTestPort, listening_addr);
  test_server->create_server();
  create_client(SOCK_DGRAM);

  std::thread server_thread(&EchoTestServer::run_server, test_server);
  write_client();
  read_client();

  test_server->stop_server();
  close_client();
  server_thread.join();

  EXPECT_STREQ(test_server->get_recv_buf(), test_msg);
  EXPECT_STREQ(recv_buf, test_msg);
}

} // namespace
} // namespace iotctest