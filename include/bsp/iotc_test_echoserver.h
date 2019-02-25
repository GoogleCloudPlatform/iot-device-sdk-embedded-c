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
  EchoTestServer(uint16_t sockType, uint16_t port, const char* listeningAddr)
      : sock_type(sockType), test_port(port), listening_addr(listeningAddr) {}

  int recv_len;
  char recv_buf[kBufferSize];
  bool runnable = true;

  uint16_t create_server();
  uint16_t run_server();
  uint16_t run_tcp_server();
  uint16_t run_udp_server();
  char* get_recv_buf();
  void stop_server();

private:
  uint16_t sock_type, test_port, server_addr_size;
  int server_sock, client_sock, rc;
  const char* listening_addr;
};
};
#endif