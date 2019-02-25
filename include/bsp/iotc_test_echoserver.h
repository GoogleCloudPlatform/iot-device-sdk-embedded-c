#ifndef __IOTC_TEST_ECHOSERVER_H__
#define __IOTC_TEST_ECHOSERVER_H__

#include <netdb.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

const uint16_t kTimeoutSeconds = 1;
const uint16_t kBufferSize = 1024;
class EchoTestServer {
public:
  EchoTestServer(uint16_t sockType, uint16_t port)
      : sock_type_(sockType), test_port_(port) {}

  int recv_len_;
  char recv_buf_[kBufferSize];
  bool runnable_ = true;

  uint16_t CreateServer();
  uint16_t RunServer();
  uint16_t RunTcpServer();
  uint16_t RunTdpServer();
  char* get_recv_buf();
  void stop_server();

private:
  uint16_t sock_type_, test_port_;
  int server_sock_, client_sock_;
};
};
#endif