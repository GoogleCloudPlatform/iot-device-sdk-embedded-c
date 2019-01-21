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
bool iotc_roughtime_create_socket(int *out_socket, const char *server_address) {
  std::string address(server_address);

  const size_t colon_offset = address.rfind(':');
  if (colon_offset == std::string ::npos) {
    fprintf(stderr, "No port number in server address: %s\n", address.c_str());
    return false;
  }
  std::string host(address.substr(0, colon_offset));
  const std::string port_str(address.substr(colon_offset + 1));

  struct addrinfo hints;
  memset(&hints, 0, sizeof(hints));
  hints.ai_socktype = SOCK_DGRAM;
  hints.ai_protocol = IPPROTO_UDP;
  hints.ai_flags = AI_NUMERICSERV;

  if (!host.empty() && host[0] == '[' && host[host.size() - 1] == ']') {
    host = host.substr(1, host.size() - 1);
    hints.ai_family = AF_INET6;
    hints.ai_flags |= AI_NUMERICHOST;
  }

  if (!host.empty() && host[0] == '[' && host[host.size() - 1] == ']') {
    host = host.substr(1, host.size() - 1);
    hints.ai_family = AF_INET6;
    hints.ai_flags |= AI_NUMERICHOST;
  }

  struct addrinfo *addrs;
  int r = getaddrinfo(host.c_str(), port_str.c_str(), &hints, &addrs);
  if (r != 0) {
    fprintf(stderr, "Failed to resolve %s: %s", address.c_str(),
            gai_strerror(r));
    return false;
  }

  int sock = socket(addrs->ai_family, addrs->ai_socktype, addrs->ai_protocol);
  if (sock < 0) {
    perror("Failed to create UDP socket");
    freeaddrinfo(addrs);
    return false;
  }

  if (connect(sock, addrs->ai_addr, addrs->ai_addrlen)) {
    perror("Failed to connect UDP socket");
    freeaddrinfo(addrs);
    close(sock);
    return false;
  }

  char dest_str[INET6_ADDRSTRLEN];
  r = getnameinfo(addrs->ai_addr, addrs->ai_addrlen, dest_str, sizeof(dest_str),
                  NULL /* don't want port information */, 0, NI_NUMERICHOST);
  freeaddrinfo(addrs);

  if (r != 0) {
    fprintf(stderr, "getnameinfo: %s", gai_strerror(r));
    close(sock);
    return false;
  }

  printf("Sending request to %s, port %s.\n", dest_str, port_str.c_str());
  *out_socket = sock;
  return true;
}

int iotc_roughtime_getcurrenttime(int socket, const char *name,
                                  const char *public_key) {
  std::string server_name(name);
  std::string server_public_key(public_key);

  uint8_t nonce[roughtime::kNonceLength];
  RAND_bytes(nonce, sizeof(nonce));
  const std::string request = roughtime::CreateRequest(nonce);

  struct timeval timeout;
  timeout.tv_sec = kTimeoutSeconds;
  timeout.tv_usec = 0;
  setsockopt(socket, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));

  ssize_t sent_len;
  do {
    sent_len = send(socket, request.data(), request.size(), 0 /* flags */);
  } while (sent_len == -1 && errno == EINTR);
  const uint64_t start_us = roughtime::MonotonicUs();

  if (sent_len < 0 || static_cast<size_t>(sent_len) != request.size()) {
    perror("send on UDP socket");
    close(socket);
    return kExitNetworkError;
  }

  uint8_t recv_buf[roughtime::kMinRequestSize];
  ssize_t buf_len;
  do {
    buf_len = recv(socket, recv_buf, sizeof(recv_buf), 0 /* flags */);
  } while (buf_len == -1 && errno == EINTR);

  const uint64_t end_us = roughtime::MonotonicUs();
  const uint64_t end_realtime_us = roughtime::RealtimeUs();

  close(socket);

  if (buf_len == -1) {
    if (errno == EINTR) {
      fprintf(stderr, "No response from %s with %d seconds.\n",
              server_name.c_str(), kTimeoutSeconds);
      return kExitTimeout;
    }

    perror("recv from UDP socket");
    return kExitNetworkError;
  }

  roughtime::rough_time_t timestamp;
  uint32_t radius;
  std::string error;
  if (!roughtime::ParseResponse(
          &timestamp, &radius, &error,
          reinterpret_cast<const uint8_t *>(server_public_key.data()), recv_buf,
          buf_len, nonce)) {
    fprintf(stderr, "Response from %s failed verification: %s",
            server_name.c_str(), error.c_str());
    return kExitBadReply;
  }

  // We assume that the path to the Roughtime server is symmetric and thus add
  // half the round-trip time to the server's timestamp to produce our estimate
  // of the current time.
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