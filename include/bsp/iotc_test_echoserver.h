#ifndef __IOTC_TEST_ECHOSERVER_H__
#define __IOTC_TEST_ECHOSERVER_H__

#include <netdb.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>

#ifdef __cplusplus
extern "C" {
#endif

const uint16_t kTimeoutSeconds = 1;
const uint16_t kBufferSize = 1024;
class EchoTestServer {
public:
  EchoTestServer(uint16_t sockType, uint16_t port)
      : sock_type_(sockType), test_port_(port) {}

  uint16_t CreateServer();
  uint16_t RunServer();
  void StopServer();
  char* get_recv_buf();

private:
  uint16_t sock_type_, test_port_;
  int server_sock_, client_sock_, recv_len_;
  char recv_buf_[kBufferSize];
  bool runnable_ = true;

  uint16_t RunTcpServer() {
    struct sockaddr_storage client_addr;
    socklen_t client_addr_size = sizeof(struct sockaddr_storage);

    /* Accept and read message from client. */
    while (runnable_) {
      client_sock_ = accept(server_sock_, (struct sockaddr*)&client_addr,
                            &client_addr_size);
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

  uint16_t RunUdpServer() {
    struct sockaddr_storage client_addr;
    socklen_t client_addr_size = sizeof(struct sockaddr_storage);

    /* Receive message from client. */
    while (runnable_) {
      if ((recv_len_ = recvfrom(server_sock_, recv_buf_, kBufferSize, 0,
                                (struct sockaddr*)&client_addr,
                                &client_addr_size)) < 0) {
        close(server_sock_);
        return 1;
      }
      recv_buf_[recv_len_] = '\0';
      if (sendto(server_sock_, recv_buf_, recv_len_, 0,
                 (struct sockaddr*)&client_addr,
                 client_addr_size) != recv_len_) {
        perror("[Server] Error sending to client");
        close(server_sock_);
        return 1;
      }
    }

    close(server_sock_);
    return 0;
  }
};
};
#endif