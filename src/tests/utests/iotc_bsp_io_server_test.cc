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
  const uint16_t kTestPort = 2000;
  const char* kTestMsg = "hello\n";
  int sock_type_, recv_len_, out_written_count_, state_;
  char* listening_addr_;
  char recv_buf_[kBufferSize];
  iotc_bsp_socket_events_t socket_evts_[1];
  iotc_bsp_socket_t test_socket_;

  iotc_bsp_io_net_state_t CreateClient(uint16_t sock_type);
  iotc_bsp_io_net_state_t WriteClient();
  iotc_bsp_io_net_state_t ReadClient();
  iotc_bsp_io_net_state_t CloseClient();
};

iotc_bsp_io_net_state_t ServerTest::CreateClient(uint16_t sock_type) {
  if (iotc_bsp_io_net_socket_connect(&test_socket_, listening_addr_, kTestPort,
                                     sock_type) != IOTC_BSP_IO_NET_STATE_OK) {
    perror("[Client] Error connecting socket");
    return IOTC_BSP_IO_NET_STATE_ERROR;
  }
  if (iotc_bsp_io_net_connection_check(test_socket_, listening_addr_,
                                       kTestPort) != IOTC_BSP_IO_NET_STATE_OK) {
    perror("[Client]Error checking connection");
    return IOTC_BSP_IO_NET_STATE_ERROR;
  }

  return IOTC_BSP_IO_NET_STATE_OK;
}

iotc_bsp_io_net_state_t ServerTest::WriteClient() {
  memset(socket_evts_, 0, sizeof(iotc_bsp_socket_events_t));
  socket_evts_[0].iotc_socket = test_socket_;
  socket_evts_[0].in_socket_want_write = 1;
  bool ready_to_write = false;
  while (!ready_to_write) {
    state_ = iotc_bsp_io_net_select(socket_evts_, 1, kTimeoutSeconds);
    switch (state_) {
    case IOTC_BSP_IO_NET_STATE_OK:
      if (socket_evts_[0].out_socket_can_write == 1) {
        ready_to_write = true;
      }
      break;
    default:
      break;
    }
  }
  return iotc_bsp_io_net_write(test_socket_, &out_written_count_,
                               (uint8_t*)kTestMsg, strlen(kTestMsg));
}

iotc_bsp_io_net_state_t ServerTest::ReadClient() {
  memset(socket_evts_, 0, sizeof(iotc_bsp_socket_events_t));
  socket_evts_[0].iotc_socket = test_socket_;
  socket_evts_[0].in_socket_want_read = 1;
  bool ready_to_read = false;
  while (!ready_to_read) {
    state_ = iotc_bsp_io_net_select(socket_evts_, 1, kTimeoutSeconds);
    switch (state_) {
    case IOTC_BSP_IO_NET_STATE_OK:
      if (socket_evts_[0].out_socket_can_read == 1) {
        ready_to_read = true;
      }
      break;
    default:
      break;
    }
  }
  return iotc_bsp_io_net_read(test_socket_, &recv_len_,
                              reinterpret_cast<uint8_t*>(recv_buf_),
                              sizeof(recv_buf_));
}

iotc_bsp_io_net_state_t ServerTest::CloseClient() {
  return iotc_bsp_io_net_close_socket(&test_socket_);
}

TEST_F(ServerTest, TcpIpv4EndToEndCommunicationWorks) {
  sock_type_ = SOCK_STREAM;
  listening_addr_ = const_cast<char*>("127.0.0.1");
  EchoTestServer* test_server = new EchoTestServer(sock_type_, kTestPort);
  test_server->CreateServer();
  CreateClient(sock_type_);

  std::thread server_thread(&EchoTestServer::RunServer, test_server);
  WriteClient();
  ReadClient();

  test_server->stop_server();
  CloseClient();
  server_thread.join();

  EXPECT_STREQ(test_server->get_recv_buf(), kTestMsg);
  EXPECT_STREQ(recv_buf_, kTestMsg);
}

TEST_F(ServerTest, UdpIpv4EndToEndCommunicationWorks) {
  sock_type_ = SOCK_DGRAM;
  listening_addr_ = const_cast<char*>("127.0.0.1");
  EchoTestServer* test_server = new EchoTestServer(sock_type_, kTestPort);
  test_server->CreateServer();
  CreateClient(sock_type_);

  std::thread server_thread(&EchoTestServer::RunServer, test_server);
  WriteClient();
  ReadClient();

  test_server->stop_server();
  CloseClient();
  server_thread.join();

  EXPECT_STREQ(test_server->get_recv_buf(), kTestMsg);
  EXPECT_STREQ(recv_buf_, kTestMsg);
}

TEST_F(ServerTest, TcpIpv6EndToEndCommunicationWorks) {
  sock_type_ = SOCK_STREAM;
  listening_addr_ = const_cast<char*>("::1");
  EchoTestServer* test_server = new EchoTestServer(sock_type_, kTestPort);
  test_server->CreateServer();
  CreateClient(sock_type_);

  std::thread server_thread(&EchoTestServer::RunServer, test_server);
  WriteClient();
  ReadClient();

  test_server->stop_server();
  CloseClient();
  server_thread.join();

  EXPECT_STREQ(test_server->get_recv_buf(), kTestMsg);
  EXPECT_STREQ(recv_buf_, kTestMsg);
}

TEST_F(ServerTest, UdpIpv6EndToEndCommunicationWorks) {
  sock_type_ = SOCK_DGRAM;
  listening_addr_ = const_cast<char*>("::1");
  EchoTestServer* test_server = new EchoTestServer(sock_type_, kTestPort);
  test_server->CreateServer();
  CreateClient(sock_type_);

  std::thread server_thread(&EchoTestServer::RunServer, test_server);
  WriteClient();
  ReadClient();

  test_server->stop_server();
  CloseClient();
  server_thread.join();

  EXPECT_STREQ(test_server->get_recv_buf(), kTestMsg);
  EXPECT_STREQ(recv_buf_, kTestMsg);
}

} // namespace
} // namespace iotctest