#include <iotc_bsp_io_net.h>
#include <iotc_roughtime_client.h>

#include <arpa/inet.h>
#include <assert.h>
#include <errno.h>
#include <inttypes.h>
#include <netdb.h>
#include <netinet/udp.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <wolfssl/options.h>
#include <wolfssl/ssl.h>
#include <wolfssl/wolfcrypt/settings.h>

#include "client.cc"
#include "protocol.h"
#include <clock_linux.cc>
#include <stdio.h>
#include <string>

// kTimeoutSeconds is the number of seconds that we will wait for a reply
// from the server.
static const int kTimeoutSeconds = 3;

#ifdef __cplusplus
extern "C" {
#endif

namespace roughtime {
bool iotc_roughtime_create_socket(int* out_socket, const char* server_address) {
  std::string address(server_address);

  const size_t colon_offset = address.rfind(':');
  if (colon_offset == std::string ::npos) {
    fprintf(stderr, "No port number in server address: %s\n", address.c_str());
    return false;
  }
  std::string host(address.substr(0, colon_offset));
  const std::string port_str(address.substr(colon_offset + 1));

  if (IOTC_BSP_IO_NET_STATE_OK ==
      iotc_bsp_io_net_socket_connect(
          reinterpret_cast<iotc_bsp_socket_t*>(out_socket), host.c_str(),
          port_str.c_str())) {
    printf("Client: socket_connect() succeed!\n");
  }

  if (IOTC_BSP_IO_NET_STATE_OK ==
      iotc_bsp_io_net_connection_check(*out_socket, host.c_str(),
                                       port_str.c_str())) {
    printf("Client: connection_check() succeed!\n");
  }

  return true;

  // struct addrinfo hints;
  // memset(&hints, 0, sizeof(hints));
  // hints.ai_socktype = SOCK_DGRAM;
  // hints.ai_protocol = IPPROTO_UDP;
  // hints.ai_flags = AI_NUMERICSERV;

  // if (!host.empty() && host[0] == '[' && host[host.size() - 1] == ']') {
  //   host = host.substr(1, host.size() - 1);
  //   hints.ai_family = AF_INET6;
  //   hints.ai_flags |= AI_NUMERICHOST;
  // }

  // struct addrinfo* addrs;
  // int r = getaddrinfo(host.c_str(), port_str.c_str(), &hints, &addrs);
  // if (r != 0) {
  //   fprintf(stderr, "Failed to resolve %s: %s", address.c_str(),
  //           gai_strerror(r));
  //   return false;
  // }

  // int sock = socket(addrs->ai_family, addrs->ai_socktype,
  // addrs->ai_protocol); if (sock < 0) {
  //   perror("Failed to create UDP socket");
  //   freeaddrinfo(addrs);
  //   return false;
  // }

  // if (connect(sock, addrs->ai_addr, addrs->ai_addrlen)) {
  //   perror("Failed to connect UDP socket");
  //   freeaddrinfo(addrs);
  //   close(sock);
  //   return false;
  // }

  // char dest_str[INET6_ADDRSTRLEN];
  // r = getnameinfo(addrs->ai_addr, addrs->ai_addrlen, dest_str,
  // sizeof(dest_str),
  //                 NULL /* don't want port information */, 0, NI_NUMERICHOST);
  // freeaddrinfo(addrs);

  // if (r != 0) {
  //   fprintf(stderr, "getnameinfo: %s", gai_strerror(r));
  //   close(sock);
  //   return false;
  // }

  // printf("Sending request to %s, port %s.\n", dest_str, port_str.c_str());
  // *out_socket = sock;
  // return true;
} // iotc_roughtime_create_socket

int iotc_roughtime_getcurrenttime(int socket, const char* name,
                                  const char* public_key) {

  std::string server_name(name);
  std::string server_public_key(public_key);
  iotc_bsp_io_net_state_t state;
  uint8_t nonce[roughtime::kNonceLength];
  size_t socket_evts_size = 1;
  iotc_bsp_socket_events_t socket_evts[socket_evts_size];

  printf("\n--------------getcurrent()--------------\n");

  RAND_bytes(nonce, sizeof(nonce));
  const std::string request = roughtime::CreateRequest(nonce);

  /* Write to the socket */
  memset(socket_evts, 0, sizeof(iotc_bsp_socket_events_t) * socket_evts_size);
  socket_evts[0].iotc_socket = socket;
  socket_evts[0].in_socket_want_write = 1;
  bool ready_to_write = false;
  while (!ready_to_write) {
    state =
        iotc_bsp_io_net_select(socket_evts, socket_evts_size, kTimeoutSeconds);
    switch (state) {
    case IOTC_BSP_IO_NET_STATE_OK:
      if (socket_evts[0].out_socket_can_write == 1) {
        printf("Socket is available to write.\n");
        ready_to_write = true;
      }
      break;
    case IOTC_BSP_IO_NET_STATE_TIMEOUT:
      break;
    case IOTC_BSP_IO_NET_STATE_ERROR:
      // todo: err handling
      break;
    default:
      break;
    }
  }

  int bytes_written = 0;
  state = iotc_bsp_io_net_write(
      socket, &bytes_written, reinterpret_cast<const uint8_t*>(request.data()),
      request.size());
  if (IOTC_BSP_IO_NET_STATE_ERROR == state) {
    printf("Error: write()\n");
    // handle other error as well
  }
  const uint64_t start_us = roughtime::MonotonicUs();

  // ssize_t sent_len;
  // do {
  //   sent_len = send(socket, request.data(), request.size(), 0 /* flags
  //   */);
  // } while (sent_len == -1 && errno == EINTR);
  // const uint64_t start_us = roughtime::MonotonicUs();

  // if (sent_len < 0 || static_cast<size_t>(sent_len) != request.size()) {
  //   perror("send on UDP socket");
  //   close(socket);
  //   return kExitNetworkError;
  // }

  // int sent_len;
  // do {
  //   iotc_bsp_io_net_write(socket, &sent_len,
  //                         reinterpret_cast<const uint8_t*>(request.data()),
  //                         request.size());
  // } while (sent_len == -1 && errno == EINTR);
  // const uint64_t start_us = roughtime::MonotonicUs();

  // if (sent_len < 0 || static_cast<size_t>(sent_len) != request.size()) {
  //   perror("send on UDP socket");
  //   iotc_bsp_io_net_close_socket(reinterpret_cast<iotc_bsp_socket_t*>(&socket));
  //   return kExitNetworkError;
  // }

  // uint8_t recv_buf[roughtime::kMinRequestSize];
  // ssize_t buf_len;
  // do {
  //   buf_len = recv(socket, recv_buf, sizeof(recv_buf), 0 /* flags */);
  // } while (buf_len == -1 && errno == EINTR);

  // const uint64_t end_us = roughtime::MonotonicUs();
  // const uint64_t end_realtime_us = roughtime::RealtimeUs();

  // close(socket);

  // if (buf_len == -1) {
  //   if (errno == EINTR) {
  //     fprintf(stderr, "No response from %s with %d seconds.\n",
  //             server_name.c_str(), kTimeoutSeconds);
  //     return kExitTimeout;
  //   }

  //   perror("recv from UDP socket");
  //   return kExitNetworkError;
  // }

  /* Read from the socket */
  memset(socket_evts, 0, sizeof(iotc_bsp_socket_events_t) * socket_evts_size);
  socket_evts[0].iotc_socket = socket;
  socket_evts[0].in_socket_want_read = 1;
  bool ready_to_read = false;
  while (!ready_to_read) {
    state =
        iotc_bsp_io_net_select(socket_evts, socket_evts_size, kTimeoutSeconds);
    switch (state) {
    case IOTC_BSP_IO_NET_STATE_OK:
      if (socket_evts[0].out_socket_can_read == 1) {
        printf("Socket is available to read\n");
        ready_to_read = true;
      }
      break;
    case IOTC_BSP_IO_NET_STATE_TIMEOUT:
      break;
    case IOTC_BSP_IO_NET_STATE_ERROR:
      // todo: err handling
      break;
    default:
      break;
    }
  }

  uint8_t recv_buf[roughtime::kMinRequestSize];
  int buf_len;
  state = iotc_bsp_io_net_read(socket, &buf_len, recv_buf, sizeof(recv_buf));
  if (IOTC_BSP_IO_NET_STATE_ERROR == state) {
    printf("Error: read()\n");
    // handle other error as well
  }
  const uint64_t end_us = roughtime::MonotonicUs();
  const uint64_t end_realtime_us = roughtime::RealtimeUs();

  iotc_bsp_io_net_close_socket(reinterpret_cast<iotc_bsp_socket_t*>(&socket));

  if (buf_len == -1) {
    if (errno == EINTR) {
      fprintf(stderr, "No response from %s with %d seconds.\n",
              server_name.c_str(), kTimeoutSeconds);
      return kExitTimeout;
    }

    perror("recv from UDP socket");
    return kExitNetworkError;
  }
  printf("%s\n", recv_buf);

  roughtime::rough_time_t timestamp;
  uint32_t radius;
  std::string error;
  if (!roughtime::ParseResponse(
          &timestamp, &radius, &error,
          reinterpret_cast<const uint8_t*>(server_public_key.data()), recv_buf,
          buf_len, nonce)) {
    fprintf(stderr, "Response from %s failed verification: %s",
            server_name.c_str(), error.c_str());
    return kExitBadReply;
  }

  // We assume that the path to the Roughtime server is symmetric and thus
  // add half the round-trip time to the server's timestamp to produce our
  // estimate of the current time.
  timestamp += (end_us - start_us) / 2;

  printf("Received reply in %" PRIu64 "μs.\n", end_us - start_us);
  printf("Current time is %" PRIu64 "μs from the epoch, ±%uμs \n", timestamp,
         static_cast<unsigned>(radius));
  int64_t system_offset =
      static_cast<int64_t>(timestamp) - static_cast<int64_t>(end_realtime_us);
  printf("System clock differs from that estimate by %" PRId64 "μs.\n",
         system_offset);

  static const int64_t kTenMinutes = 10 * 60 * 1000000;
  if (imaxabs(system_offset) > kTenMinutes) {
    return kExitBadSystemTime;
  }

  return 0;
}
} // namespace roughtime

#ifdef __cplusplus
}
#endif