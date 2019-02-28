#include "iotc_test_echoserver.h"

#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <thread>
#include <unistd.h>

namespace iotctest {

EchoTestServer::EchoTestServer(uint16_t socket_type, uint16_t port)
    : socket_type_(socket_type), test_port_(port) {
  CreateServer();
}

EchoTestServer::~EchoTestServer() { server_thread_.join(); }

void EchoTestServer::Run() {
  server_thread_ = std::thread(&EchoTestServer::RunServer, this);
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
    write(client_socket_, recv_buf_, recv_len_);
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
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = socket_type_;
  hints.ai_flags = AI_PASSIVE;
  status = getaddrinfo(NULL, port_s, &hints, &result);
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
    perror("Error binding");
    return ServerError::kError;
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

EchoTestServer::ServerError EchoTestServer::RunServer() {
  if (socket_type_ == SOCK_STREAM) {
    return RunTcpServer();
  } else if (socket_type_ == SOCK_DGRAM) {
    return RunUdpServer();
  }
  return ServerError::kSuccess;
}

void EchoTestServer::StopServer() {
  runnable_ = false;
  return;
}
} // namespace iotctest