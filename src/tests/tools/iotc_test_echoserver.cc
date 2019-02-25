#include "iotc_test_echoserver.h"

#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

uint16_t EchoTestServer::CreateServer() {
  struct timeval tv;
  struct addrinfo hints;
  struct addrinfo *result, *rp;
  uint8_t status;
  char port_s[10];
  sprintf(port_s, "%d", test_port_);

  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = sock_type_;
  hints.ai_flags = AI_PASSIVE;
  status = getaddrinfo(NULL, port_s, &hints, &result);
  if (0 != status) {
    return 1;
  }

  for (rp = result; rp != NULL; rp = rp->ai_next) {
    if ((server_sock_ =
             socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol)) < 0) {
      continue;
    }
    printf("protocol type: %d\n", rp->ai_family);
    int reuseAddress = 1;
    if (setsockopt(server_sock_, SOL_SOCKET, SO_REUSEADDR, &reuseAddress,
                   sizeof(reuseAddress))) {
      perror("[Server] Error setting socket reuseaddr option");
      close(server_sock_);
      return 1;
    }
    if (bind(server_sock_, rp->ai_addr, rp->ai_addrlen) == 0)
      break;
    else
      close(server_sock_);
  }

  if (rp == NULL) {
    perror("Error binding");
    return 1;
  }
  freeaddrinfo(result);

  tv.tv_sec = kTimeoutSeconds;
  tv.tv_usec = 0;
  if (setsockopt(server_sock_, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
    perror("[Server] Error setting socket timeout option");
    close(server_sock_);
    return 1;
  }
  if (sock_type_ == SOCK_STREAM) {
    if (listen(server_sock_, 5) < 0) {
      perror("[Server] Error listening connection");
      return 1;
    }
  }

  return 0;
}

uint16_t EchoTestServer::RunServer() {
  if (sock_type_ == SOCK_STREAM) {
    if (RunTcpServer() != 0) {
      return 1;
    }
  } else if (sock_type_ == SOCK_DGRAM) {
    if (RunTdpServer() != 0) {
      return 1;
    }
  }
  return 0;
}

uint16_t EchoTestServer::RunTcpServer() {
  struct sockaddr_storage client_addr;
  socklen_t client_addr_size = sizeof(struct sockaddr_storage);

  /* Accept and read message from client. */
  while (runnable_) {
    client_sock_ =
        accept(server_sock_, (struct sockaddr*)&client_addr, &client_addr_size);
    if (client_sock_ < 0) {
      close(client_sock_);
      close(server_sock_);
      return 1;
    }
    recv_len_ = read(client_sock_, recv_buf_, kBufferSize);
    recv_buf_[recv_len_] = '\0';
    write(client_sock_, recv_buf_, recv_len_);
    close(client_sock_);
  }

  close(server_sock_);
  return 0;
}

uint16_t EchoTestServer::RunTdpServer() {
  struct sockaddr_storage client_addr;
  socklen_t client_addr_size = sizeof(struct sockaddr_storage);

  /* Receive message from client. */
  while (runnable_) {
    if ((recv_len_ =
             recvfrom(server_sock_, recv_buf_, kBufferSize, 0,
                      (struct sockaddr*)&client_addr, &client_addr_size)) < 0) {
      close(server_sock_);
      return 1;
    }
    recv_buf_[recv_len_] = '\0';
    if (sendto(server_sock_, recv_buf_, recv_len_, 0,
               (struct sockaddr*)&client_addr, client_addr_size) != recv_len_) {
      perror("[Server] Error sending to client");
      close(server_sock_);
      return 1;
    }
  }

  close(server_sock_);
  return 0;
}

char* EchoTestServer::get_recv_buf() { return recv_buf_; }

void EchoTestServer::stop_server() {
  runnable_ = false;
  return;
}