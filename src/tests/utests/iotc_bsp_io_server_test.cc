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

#include "iotc.h"
#include "iotc_bsp_crypto.h"
#include "iotc_heapcheck_test.h"
#include "iotc_helpers.h"
#include "iotc_jwt.h"
#include "iotc_macros.h"
#include "iotc_types.h"

#include <iostream>
#include <thread>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

namespace iotctest {
namespace {
using namespace std;
class ServerTest : public IotcHeapCheckTest {
public:
  ServerTest() { iotc_initialize(); }
  ~ServerTest() { iotc_shutdown(); }
  int out_written_count;
  const char* test_msg = "hello\n";
  iotc_bsp_socket_t test_socket;

  iotc_bsp_io_net_state_t open_server();
  iotc_bsp_io_net_state_t run_client();
  void setSockType(int sockType);
  void setProtocolType(int protocolType);
  void setTestPort(uint16_t port);
  void setListeningAddr(const char* addr);
  void setRun(bool run);
  char* getRecvBuf();
  /*
    sock_type: TCP(1), UDP(2)
    protocol_type: IPv4(2), IPv6(10)
  */
  int sock_type, protocol_type, recv_len;
  uint16_t test_port;
  char* listening_addr;
  char recv_buf[1024];
  bool runnable = true;
};

void ServerTest::setSockType(int sockType) { sock_type = sockType; }
void ServerTest::setProtocolType(int protocolType) {
  protocol_type = protocolType;
}
void ServerTest::setTestPort(uint16_t testPort) { test_port = testPort; }
void ServerTest::setListeningAddr(const char* addr) {
  listening_addr = const_cast<char*>(addr);
}
void ServerTest::setRun(bool run) { runnable = run; }
char* ServerTest::getRecvBuf() { return recv_buf; }

iotc_bsp_io_net_state_t ServerTest::open_server() {
  int sock, client_sock, rc;
  unsigned int addr_len;
  struct sockaddr_in server_addr, client_addr;
  struct sockaddr_in6 server_addr6, client_addr6;
  // Create socket.
  if ((sock = socket(protocol_type, sock_type, 0)) < 0) {
    perror("Error creating server socket.");
    return IOTC_BSP_IO_NET_STATE_ERROR;
  }
  rc = setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, listening_addr,
                  sizeof(listening_addr));
  if (rc < 0) {
    perror("Error setting socket option: ");
    close(sock);
    return IOTC_BSP_IO_NET_STATE_ERROR;
  }

  // Initialize and set server address and bind socket.
  if (protocol_type == AF_INET) {
    memset(&server_addr, 0x00, sizeof(server_addr));
    server_addr.sin_family = protocol_type;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(test_port);
    // Bind socket.
    if (bind(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
      perror("Error binding: ");
      return IOTC_BSP_IO_NET_STATE_ERROR;
    }
  } else if (protocol_type == AF_INET6) {
    memset(&server_addr6, 0x00, sizeof(server_addr6));
    server_addr6.sin6_family = protocol_type;
    server_addr6.sin6_addr = in6addr_any;
    server_addr6.sin6_port = htons(test_port);
    // Bind socket.
    if (bind(sock, (struct sockaddr*)&server_addr6, sizeof(server_addr6)) < 0) {
      perror("Error binding: ");
      return IOTC_BSP_IO_NET_STATE_ERROR;
    }
  }

  if (sock_type == SOCK_STREAM) {
    if (listen(sock, 5) < 0) {
      perror("Error listening connect");
      return IOTC_BSP_IO_NET_STATE_ERROR;
    }
    if (protocol_type == AF_INET) {
      addr_len = sizeof(client_addr);
      while (runnable) {
        client_sock = accept(sock, (struct sockaddr*)&client_addr, &addr_len);
        if (client_sock < 0) {
          perror("Error accepting");
          return IOTC_BSP_IO_NET_STATE_ERROR;
        }
        recv_len = read(client_sock, recv_buf, 1024);
        close(client_sock);
      }
    } else if (protocol_type == AF_INET6) {
      addr_len = sizeof(client_addr6);
      while (runnable) {
        client_sock = accept(sock, (struct sockaddr*)&client_addr6, &addr_len);
        if (client_sock < 0) {
          perror("Error accepting");
          return IOTC_BSP_IO_NET_STATE_ERROR;
        }
        recv_len = read(client_sock, recv_buf, 1024);
        close(client_sock);
      }
    }
  } else if (sock_type == SOCK_DGRAM) {
    if (protocol_type == AF_INET) {
      addr_len = sizeof(client_addr);
      while (runnable) {
        if ((recv_len = recvfrom(sock, recv_buf, 1024, 0,
                                 (struct sockaddr*)&client_addr, &addr_len)) <
            0) {
          perror("Error recvfrom");
          return IOTC_BSP_IO_NET_STATE_ERROR;
        }
        recv_buf[recv_len] = '\0';
      }
    } else if (protocol_type == AF_INET6) {
      addr_len = sizeof(client_addr6);
      while (runnable) {
        if ((recv_len = recvfrom(sock, recv_buf, 1024, 0,
                                 (struct sockaddr*)&client_addr6, &addr_len)) <
            0) {
          perror("Error recvfrom");
          return IOTC_BSP_IO_NET_STATE_ERROR;
        }
        recv_buf[recv_len] = '\0';
      }
    }
  }

  close(sock);
  return IOTC_BSP_IO_NET_STATE_OK;
}
iotc_bsp_io_net_state_t ServerTest::run_client() {
  iotc_bsp_io_net_socket_connect(&test_socket, listening_addr, test_port);
  iotc_bsp_io_net_connection_check(test_socket, listening_addr, test_port);
  iotc_bsp_io_net_write(test_socket, &out_written_count, (uint8_t*)test_msg,
                        strlen(test_msg));
  setRun(false);
  return IOTC_BSP_IO_NET_STATE_OK;
}

TEST_F(ServerTest, TcpIpv4EndToEndCommunicationWorks) {
  setSockType(1);
  setProtocolType(2);
  setTestPort(2000);
  setListeningAddr("127.0.0.1");
  setRun(true);

  std::thread server_thread(&ServerTest::open_server, this);
  std::thread client_thread(&ServerTest::run_client, this);

  client_thread.join();
  server_thread.join();
  iotc_bsp_io_net_close_socket(&test_socket);

  EXPECT_STREQ(getRecvBuf(), test_msg);
}
TEST_F(ServerTest, TcpIpv6EndToEndCommunicationWorks) {
  setSockType(1);
  setProtocolType(10);
  setTestPort(2000);
  setListeningAddr("::1");
  setRun(true);

  std::thread server_thread(&ServerTest::open_server, this);
  std::thread client_thread(&ServerTest::run_client, this);

  client_thread.join();
  server_thread.join();
  iotc_bsp_io_net_close_socket(&test_socket);

  EXPECT_STREQ(getRecvBuf(), test_msg);
}
TEST_F(ServerTest, UdpIpv4EndToEndCommunicationWorks) {
  setSockType(2);
  setProtocolType(2);
  setTestPort(2000);
  setListeningAddr("127.0.0.1");
  setRun(true);

  std::thread server_thread(&ServerTest::open_server, this);
  std::thread client_thread(&ServerTest::run_client, this);

  client_thread.join();
  server_thread.join();
  iotc_bsp_io_net_close_socket(&test_socket);

  EXPECT_STREQ(getRecvBuf(), test_msg);
}
TEST_F(ServerTest, UdpIpv6EndToEndCommunicationWorks) {
  setSockType(2);
  setProtocolType(10);
  setTestPort(2000);
  setListeningAddr("::1");
  setRun(true);

  std::thread server_thread(&ServerTest::open_server, this);
  std::thread client_thread(&ServerTest::run_client, this);

  server_thread.join();
  client_thread.join();
  iotc_bsp_io_net_close_socket(&test_socket);

  EXPECT_STREQ(getRecvBuf(), test_msg);
}

} // namespace
} // namespace iotctest