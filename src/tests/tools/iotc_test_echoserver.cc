/* Copyright 2018-2019 Google LLC
 *
 * This is part of the Google Cloud IoT Device SDK for Embedded C,
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

#include "iotc_test_echoserver.h"

#include <netinet/in.h>
#include <string.h>
#include <thread>

namespace iotctest {

EchoTestServer::EchoTestServer(std::string host, uint16_t port,
                               uint16_t socket_type, uint16_t protocol_type)
    : host_(host), test_port_(port), socket_type_(socket_type),
      protocol_type_(protocol_type) {}
EchoTestServer::~EchoTestServer() {}

std::unique_ptr<EchoTestServer> EchoTestServer::Create(std::string host,
                                                       uint16_t port,
                                                       uint16_t socket_type,
                                                       uint16_t protocol_type) {
  std::unique_ptr<EchoTestServer> echo_test_server(
      new EchoTestServer(host, port, socket_type, protocol_type));
  echo_test_server->CreateServer();
  return echo_test_server;
}

void EchoTestServer::Run() {
  if (server_thread_) {
    Stop();
  }
  runnable_ = true;
  if (socket_type_ == SOCK_STREAM) {
    // TODO(b/127770330)
    // server_thread_ =
    //     std::make_unique<std::thread>(&EchoTestServer::RunTcpServer, this);
    server_thread_ = std::unique_ptr<std::thread>(
        new std::thread(&EchoTestServer::RunTcpServer, this));
  } else if (socket_type_ == SOCK_DGRAM) {
    // TODO(b/127770330)
    // server_thread_ =
    //     std::make_unique<std::thread>(&EchoTestServer::RunUdpServer, this);
    server_thread_ = std::unique_ptr<std::thread>(
        new std::thread(&EchoTestServer::RunUdpServer, this));
  }
  return;
}

EchoTestServer::ServerError EchoTestServer::RunTcpServer() {
  struct sockaddr_storage client_addr;
  socklen_t client_addr_size = sizeof(struct sockaddr_storage);

  // Accept and read message from client.
  while (runnable_) {
    client_socket_ = accept(server_socket_, (struct sockaddr*)&client_addr,
                            &client_addr_size);
    if (client_socket_ < 0) {
      close(client_socket_);
      close(server_socket_);
      return ServerError::kFailedAccept;
    }
    recv_len_ = read(client_socket_, recv_buf_, kBufferSize);
    recv_buf_[recv_len_] = '\0';
    if (write(client_socket_, recv_buf_, recv_len_) != recv_len_)
      return ServerError::kInternalError;
    close(client_socket_);
  }

  close(server_socket_);
  return ServerError::kSuccess;
}

EchoTestServer::ServerError EchoTestServer::RunUdpServer() {
  struct sockaddr_storage client_addr;
  socklen_t client_addr_size = sizeof(struct sockaddr_storage);

  // Receive message from client.
  while (runnable_) {
    if ((recv_len_ =
             recvfrom(server_socket_, recv_buf_, kBufferSize, 0,
                      (struct sockaddr*)&client_addr, &client_addr_size)) < 0) {
      close(server_socket_);
      return ServerError::kFailedRecvFrom;
    }
    recv_buf_[recv_len_] = '\0';
    if (sendto(server_socket_, recv_buf_, recv_len_, 0,
               (struct sockaddr*)&client_addr, client_addr_size) != recv_len_) {
      close(server_socket_);
      return ServerError::kFailedSendTo;
    }
  }

  close(server_socket_);
  return ServerError::kSuccess;
}

EchoTestServer::ServerError EchoTestServer::CreateServer() {
  struct timeval tv;
  struct addrinfo hints;
  struct addrinfo *result, *rp;
  uint8_t status;
  char port_s[10];
  sprintf(port_s, "%d", test_port_);

  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_family = protocol_type_;
  hints.ai_socktype = socket_type_;
  hints.ai_flags = AI_PASSIVE;
  status = getaddrinfo(host_.c_str(), port_s, &hints, &result);
  if (0 != status) {
    return ServerError::kFailedGetAddrInfo;
  }

  for (rp = result; rp != NULL; rp = rp->ai_next) {
    if ((server_socket_ =
             socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol)) < 0) {
      continue;
    }
    int reuseAddress = 1;
    if (setsockopt(server_socket_, SOL_SOCKET, SO_REUSEADDR, &reuseAddress,
                   sizeof(reuseAddress))) {
      close(server_socket_);
      return ServerError::kFailedSetSockOpt;
    }
    if (bind(server_socket_, rp->ai_addr, rp->ai_addrlen) == 0)
      break;
    else
      close(server_socket_);
  }

  if (rp == NULL) {
    return ServerError::kInternalError;
  }
  freeaddrinfo(result);

  tv.tv_sec = kTimeoutSeconds;
  tv.tv_usec = 0;
  if (setsockopt(server_socket_, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) <
      0) {
    close(server_socket_);
    return ServerError::kFailedSetSockOpt;
  }
  if (socket_type_ == SOCK_STREAM) {
    if (listen(server_socket_, 5) < 0) {
      return ServerError::kFailedListen;
    }
  }

  return ServerError::kSuccess;
}

void EchoTestServer::Stop() {
  runnable_ = false;
  server_thread_->join();
  delete server_thread_.release();
  return;
}
} // namespace iotctest
