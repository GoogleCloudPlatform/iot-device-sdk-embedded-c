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
#include "gtest.h"

#include <netinet/in.h>
#include <stdio.h>
#include <thread>

namespace iotctest {
namespace {
using namespace std;
const uint16_t kTimeoutSeconds = 1;
class ServerTest : public ::testing::Test {
public:
  int sock_type, protocol_type, recv_len, out_written_count, state;
  const uint16_t kTestPort = 2000;
  char* listening_addr;
  const char* test_msg = "hello\n";
  char recv_buf[1024];
  iotc_bsp_socket_events_t socket_evts[1];
  iotc_bsp_socket_t test_socket;

  iotc_bsp_io_net_state_t create_client();
  iotc_bsp_io_net_state_t write_client();
  iotc_bsp_io_net_state_t read_client();
  iotc_bsp_io_net_state_t close_client();
};

class TestingServer {
public:
  TestingServer(uint16_t sockType, uint16_t protocolType, uint16_t testPort,
                const char* listeningAddr) {
    sock_type = sockType;
    protocol_type = protocolType;
    test_port = testPort;
    listening_addr = const_cast<char*>(listeningAddr);
  }
  uint16_t test_port, sock_type, protocol_type;
  int recv_len;
  char recv_buf[1024];
  char* listening_addr;
  bool runnable = true;

  char* get_recv_buf();
  void stop_server();
  uint16_t create_server();
  uint16_t run_server();

private:
  int server_sock, client_sock, rc;
  uint16_t server_addr_size;
  unsigned int client_addr_size;
  struct sockaddr_in server_addr, client_addr;
  struct sockaddr_in6 server_addr6, client_addr6;
  struct sockaddr *server_addr_ptr, *client_addr_ptr;
  struct timeval tv;

  uint16_t run_tcp_server() {
    if (protocol_type == AF_INET) {
      client_addr_size = sizeof(client_addr);
      client_addr_ptr = (struct sockaddr*)&client_addr;
    } else if (protocol_type == AF_INET6) {
      client_addr_size = sizeof(client_addr6);
      client_addr_ptr = (struct sockaddr*)&client_addr6;
    }

    /* Accept and read message from client. */
    while (runnable) {
      client_sock = accept(server_sock, client_addr_ptr, &client_addr_size);
      if (client_sock < 0) {
        close(client_sock);
        close(server_sock);
        return 1;
      }
      recv_len = read(client_sock, recv_buf, 1024);
      recv_buf[recv_len] = '\0';
      write(client_sock, recv_buf, recv_len);
      close(client_sock);
    }

    close(server_sock);
    return 0;
  };

  uint16_t run_udp_server() {
    if (protocol_type == AF_INET) {
      client_addr_size = sizeof(client_addr);
      client_addr_ptr = (struct sockaddr*)&client_addr;
    } else if (protocol_type == AF_INET6) {
      client_addr_size = sizeof(client_addr6);
      client_addr_ptr = (struct sockaddr*)&client_addr6;
    }

    /* Receive message from client. */
    while (runnable) {
      if ((recv_len = recvfrom(server_sock, recv_buf, 1024, 0, client_addr_ptr,
                               &client_addr_size)) < 0) {
        close(server_sock);
        return 1;
      }
      recv_buf[recv_len] = '\0';
      if (sendto(server_sock, recv_buf, recv_len, 0, client_addr_ptr,
                 client_addr_size) != recv_len) {
        perror("[Server] Error sending to client.");
        close(server_sock);
        return 1;
      }
    }

    close(server_sock);
    return 0;
  };
};

char* TestingServer::get_recv_buf() { return recv_buf; }
void TestingServer::stop_server() {
  runnable = false;
  return;
}
uint16_t TestingServer::create_server() {
  /* Create socket. */
  if ((server_sock = socket(protocol_type, sock_type, 0)) < 0) {
    perror("[Server] Error creating server socket.");
    return 1;
  }
  if (setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR, listening_addr,
                 sizeof(listening_addr)) < 0) {
    perror("[Server] Error setting socket address resusability option");
    close(server_sock);
    return 1;
  }

  /* Initialize and set server address and bind socket. */
  if (protocol_type == AF_INET) {
    memset(&server_addr, 0x00, sizeof(server_addr));
    server_addr.sin_family = protocol_type;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(test_port);
    server_addr_ptr = (struct sockaddr*)&server_addr;
    server_addr_size = sizeof(server_addr);
  } else if (protocol_type == AF_INET6) {
    memset(&server_addr6, 0x00, sizeof(server_addr6));
    server_addr6.sin6_family = protocol_type;
    server_addr6.sin6_addr = in6addr_any;
    server_addr6.sin6_port = htons(test_port);
    server_addr_ptr = (struct sockaddr*)&server_addr6;
    server_addr_size = sizeof(server_addr6);
  }

  /* Bind socket. */
  if (bind(server_sock, server_addr_ptr, server_addr_size) < 0) {
    perror("[Server] Error binding socket");
    return 1;
  }

  tv.tv_sec = kTimeoutSeconds;
  if (setsockopt(server_sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
    perror("[Server] Error setting socket timeout option");
    close(server_sock);
    return 1;
  }

  if (sock_type == SOCK_STREAM) {
    if (listen(server_sock, 5) < 0) {
      perror("[Server] Error listening connection");
      return 1;
    }
  }

  return 0;
}

uint16_t TestingServer::run_server() {
  if (sock_type == SOCK_STREAM) {
    if (run_tcp_server() != 0) {
      return 1;
    }
  } else if (sock_type == SOCK_DGRAM) {
    if (run_udp_server() != 0) {
      return 1;
    }
  }
  return 0;
}

iotc_bsp_io_net_state_t ServerTest::create_client() {
  if (iotc_bsp_io_net_socket_connect(&test_socket, listening_addr, kTestPort) !=
      IOTC_BSP_IO_NET_STATE_OK) {
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
  memset(socket_evts, 0, sizeof(iotc_bsp_socket_events_t) * 1);
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
  memset(socket_evts, 0, sizeof(iotc_bsp_socket_events_t) * 1);
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
  TestingServer* test_server =
      new TestingServer(SOCK_STREAM, AF_INET, kTestPort, listening_addr);
  test_server->create_server();
  create_client();

  std::thread server_thread(&TestingServer::run_server, test_server);
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
  TestingServer* test_server =
      new TestingServer(SOCK_DGRAM, AF_INET, kTestPort, listening_addr);
  test_server->create_server();
  create_client();

  std::thread server_thread(&TestingServer::run_server, test_server);
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
  TestingServer* test_server =
      new TestingServer(SOCK_STREAM, AF_INET6, kTestPort, listening_addr);
  test_server->create_server();
  create_client();

  std::thread server_thread(&TestingServer::run_server, test_server);
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
  TestingServer* test_server =
      new TestingServer(SOCK_DGRAM, AF_INET6, kTestPort, listening_addr);
  test_server->create_server();
  create_client();

  std::thread server_thread(&TestingServer::run_server, test_server);
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