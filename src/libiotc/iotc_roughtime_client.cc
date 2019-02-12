#define WOLFSSL_LEANPSK // 20190124
// #ifndef __linux
// #define __linux // 20190124
// #endif

#include <iotc_bsp_io_net.h>
#include <iotc_roughtime_client.h>
#include <wolfssl/options.h>
#include <wolfssl/ssl.h>
#include <wolfssl/wolfcrypt/settings.h>

#include <arpa/inet.h>
#include <assert.h>
#include <errno.h>
#include <inttypes.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "client.h"
#include "clock.h"
#include "protocol.h"
#include <iostream>
#include <stdio.h>
#include <string.h>

// kTimeoutSeconds is the number of seconds that we will wait for a reply
// from the server.
static const int kTimeoutSeconds = 3;

#ifdef __cplusplus
extern "C" {
#endif

namespace roughtime {

bool iotc_roughtime_create_socket(int* out_socket, const char* server_address) {
  std::string address(server_address);

  const size_t kColonOffset = address.rfind(':');
  if (kColonOffset == std::string ::npos) {
    fprintf(stderr, "No port number in server address: %s\n", address.c_str());
    return false;
  }
  std::string host(address.substr(0, kColonOffset));
  const std::string kPortStr(address.substr(kColonOffset + 1));
  int port = std::stoi(kPortStr.c_str());

  if (IOTC_BSP_IO_NET_STATE_OK !=
      iotc_bsp_io_net_socket_connect(
          reinterpret_cast<iotc_bsp_socket_t*>(out_socket), host.c_str(),
          port)) {
    perror("Connect to the socket");
  }

  if (IOTC_BSP_IO_NET_STATE_OK !=
      iotc_bsp_io_net_connection_check(*out_socket, host.c_str(), port)) {
    perror("Check the connection");
  }

  return true;
}

int iotc_roughtime_getcurrenttime(int socket, const char* name,
                                  const char* public_key) {

  std::string server_name(name);
  std::string server_public_key(public_key);
  uint8_t nonce[roughtime::kNonceLength];
  size_t socket_evts_size = 1;
  iotc_bsp_io_net_state_t state;
  iotc_bsp_socket_events_t socket_evts[socket_evts_size];

  wolfSSL_RAND_bytes(nonce, sizeof(nonce));
  const std::string kRequest = roughtime::CreateRequest(nonce);

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
        ready_to_write = true;
      }
      break;
      // TODO(sungju): error handling
    case IOTC_BSP_IO_NET_STATE_TIMEOUT:
      break;
    case IOTC_BSP_IO_NET_STATE_ERROR:
      break;
    default:
      break;
    }
  }

  int bytes_written = 0;
  state = iotc_bsp_io_net_write(
      socket, &bytes_written, reinterpret_cast<const uint8_t*>(kRequest.data()),
      kRequest.size());
  if (IOTC_BSP_IO_NET_STATE_ERROR == state) {
    perror("Write to the socket");
    iotc_bsp_io_net_close_socket(reinterpret_cast<iotc_bsp_socket_t*>(&socket));
    return kExitNetworkError;
  }
  const uint64_t start_us = roughtime::MonotonicUs();

  if (bytes_written < 0 ||
      static_cast<size_t>(bytes_written) != kRequest.size()) {
    perror("Write to the socket");
    close(socket);
    return kExitNetworkError;
  }

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
        ready_to_read = true;
      }
      break;
      // TODO(sungju): error handling
    case IOTC_BSP_IO_NET_STATE_TIMEOUT:
      break;
    case IOTC_BSP_IO_NET_STATE_ERROR:
      break;
    default:
      break;
    }
  }

  uint8_t recv_buf[roughtime::kMinRequestSize];
  int buf_len;
  state = iotc_bsp_io_net_read(socket, &buf_len, recv_buf, sizeof(recv_buf));
  if (IOTC_BSP_IO_NET_STATE_ERROR == state) {
    perror("Read from the socket");
    iotc_bsp_io_net_close_socket(reinterpret_cast<iotc_bsp_socket_t*>(&socket));
    return kExitNetworkError;
  }
  const uint64_t kEndUs = roughtime::MonotonicUs();
  const uint64_t kEndRealtimeUs = roughtime::RealtimeUs();

  iotc_bsp_io_net_close_socket(reinterpret_cast<iotc_bsp_socket_t*>(&socket));

  if (buf_len == -1) {
    if (errno == EINTR) {
      fprintf(stderr, "No response from %s with %d seconds.\n",
              server_name.c_str(), kTimeoutSeconds);
      return kExitTimeout;
    }

    perror("Read from the socket");
    return kExitNetworkError;
  }

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
  timestamp += (kEndUs - start_us) / 2;

  printf("Received reply in %" PRIu64 "μs.\n", kEndUs - start_us);
  printf("Current time is %" PRIu64 "μs from the epoch, ±%uμs \n", timestamp,
         static_cast<unsigned>(radius));
  int64_t system_offset =
      static_cast<int64_t>(timestamp) - static_cast<int64_t>(kEndRealtimeUs);
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