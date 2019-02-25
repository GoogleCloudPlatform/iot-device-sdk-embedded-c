#include "iotc_test_echoserver.h"

#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

uint16_t EchoTestServer::create_server() {
  struct timeval tv;
  struct addrinfo hints;
  struct addrinfo *result, *rp;
  uint8_t status;
  char port_s[10];
  sprintf(port_s, "%d", test_port);

  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = sock_type;
  hints.ai_flags = AI_PASSIVE;
  status = getaddrinfo(NULL, port_s, &hints, &result);
  if (0 != status) {
    return 1;
  }

  for (rp = result; rp != NULL; rp = rp->ai_next) {
    if ((server_sock =
             socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol)) < 0) {
      continue;
    }
    printf("protocol type: %d\n", rp->ai_family);
    int reuseAddress = 1;
    if (setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR, &reuseAddress,
                   sizeof(reuseAddress))) {
      perror("[Server] Error setting socket reuseaddr option");
      close(server_sock);
      return 1;
    }
    if (bind(server_sock, rp->ai_addr, rp->ai_addrlen) == 0)
      break;
    else
      close(server_sock);
  }

  if (rp == NULL) {
    perror("Error binding");
    return 1;
  }
  freeaddrinfo(result);

  tv.tv_sec = kTimeoutSeconds;
  tv.tv_usec = 0;
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

uint16_t EchoTestServer::run_server() {
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

uint16_t EchoTestServer::run_tcp_server() {
  struct sockaddr_storage client_addr;
  socklen_t client_addr_size = sizeof(struct sockaddr_storage);

  /* Accept and read message from client. */
  while (runnable) {
    client_sock =
        accept(server_sock, (struct sockaddr*)&client_addr, &client_addr_size);
    if (client_sock < 0) {
      close(client_sock);
      close(server_sock);
      return 1;
    }
    recv_len = read(client_sock, recv_buf, kBufferSize);
    recv_buf[recv_len] = '\0';
    write(client_sock, recv_buf, recv_len);
    close(client_sock);
  }

  close(server_sock);
  return 0;
}

uint16_t EchoTestServer::run_udp_server() {
  struct sockaddr_storage client_addr;
  socklen_t client_addr_size = sizeof(struct sockaddr_storage);

  /* Receive message from client. */
  while (runnable) {
    if ((recv_len =
             recvfrom(server_sock, recv_buf, kBufferSize, 0,
                      (struct sockaddr*)&client_addr, &client_addr_size)) < 0) {
      close(server_sock);
      return 1;
    }
    recv_buf[recv_len] = '\0';
    if (sendto(server_sock, recv_buf, recv_len, 0,
               (struct sockaddr*)&client_addr, client_addr_size) != recv_len) {
      perror("[Server] Error sending to client");
      close(server_sock);
      return 1;
    }
  }

  close(server_sock);
  return 0;
}

char* EchoTestServer::get_recv_buf() { return recv_buf; }

void EchoTestServer::stop_server() {
  runnable = false;
  return;
}