#define WOLFSSL_LEANPSK // 20190124
// #ifndef __linux
// #define __linux // 20190124
// #endif

#include <iotc_bsp_io_net.h>

#include <arpa/inet.h>
#include <assert.h>
#include <errno.h>
#include <inttypes.h>
#include <netdb.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "client.h"
#include "clock.h"
#include "protocol.h"
#include <iotc_bsp_rng.h>
#include <iotc_bsp_time.h>
#include <iotc_debug.h>
#include <iotc_roughtime_client.h>
#include <wolfssl/options.h>
#include <wolfssl/ssl.h>
#include <wolfssl/wolfcrypt/settings.h>

// kTimeoutSeconds is the number of seconds that we will wait for a reply
// from the server.
static const int kTimeoutSeconds = 3;

#ifdef __cplusplus
extern "C" {
#endif

namespace roughtime {

iotc_roughtime_state_t
iotc_roughtime_create_socket(int* out_socket, const char* server_address) {
  std::string address(server_address);

  const size_t kColonOffset = address.rfind(':');
  if (kColonOffset == std::string ::npos) {
    iotc_debug_format(stderr, "No port number in server address: %s\n",
                      address.c_str());
    return IOTC_ROUGHTIME_ERROR;
  }
  std::string host(address.substr(0, kColonOffset));
  const std::string kPortStr(address.substr(kColonOffset + 1));
  int port = std::stoi(kPortStr.c_str());

  if (IOTC_BSP_IO_NET_STATE_OK !=
      iotc_bsp_io_net_socket_connect(
          reinterpret_cast<iotc_bsp_socket_t*>(out_socket), host.c_str(), port,
          SOCKET_DGRAM)) {
    iotc_debug_logger("Connect to the socket");
  }

  if (IOTC_BSP_IO_NET_STATE_OK !=
      iotc_bsp_io_net_connection_check(*out_socket, host.c_str(), port)) {
    iotc_debug_logger("Check the connection");
  }

  return IOTC_ROUGHTIME_OK;
}

iotc_roughtime_state_t
iotc_roughtime_receive_time(int socket, const char* name,
                            const char* public_key,
                            iotc_roughtime_timedata_t* time_data) {

  std::string server_name(name);
  std::string server_public_key(public_key);
  uint8_t nonce[roughtime::kNonceLength];
  size_t socket_evts_size = 1;
  iotc_bsp_io_net_state_t state;
  iotc_bsp_socket_events_t socket_evts[socket_evts_size];

  for (size_t i = 0; i < roughtime::kNonceLength; i++) {
    nonce[i] = (uint8_t)iotc_bsp_rng_get() % 256;
  }
  const std::string kRequest = roughtime::CreateRequest(nonce);
  if (kRequest.empty()) {
    iotc_debug_logger("Roughtime create reaquest "
                      "failed");
    return IOTC_ROUGHTIME_ERROR;
  }

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
    case IOTC_BSP_IO_NET_STATE_TIMEOUT:
      return IOTC_ROUGHTIME_TIMEOUT_ERROR;
    case IOTC_BSP_IO_NET_STATE_ERROR:
      return IOTC_ROUGHTIME_NETWORK_ERROR;
    default:
      break;
    }
  }

  int bytes_written = 0;
  state = iotc_bsp_io_net_write(
      socket, &bytes_written, reinterpret_cast<const uint8_t*>(kRequest.data()),
      kRequest.size());
  if (IOTC_BSP_IO_NET_STATE_ERROR == state) {
    iotc_debug_logger("Write to the socket");
    iotc_bsp_io_net_close_socket(reinterpret_cast<iotc_bsp_socket_t*>(&socket));
    return IOTC_ROUGHTIME_NETWORK_ERROR;
  }
  const uint64_t kStartUs = iotc_bsp_time_getmonotonictime_milliseconds();

  if (bytes_written < 0 ||
      static_cast<size_t>(bytes_written) != kRequest.size()) {
    iotc_debug_logger("Write to the socket");
    close(socket);
    return IOTC_ROUGHTIME_NETWORK_ERROR;
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
    case IOTC_BSP_IO_NET_STATE_TIMEOUT:
      return IOTC_ROUGHTIME_TIMEOUT_ERROR;
    case IOTC_BSP_IO_NET_STATE_ERROR:
      return IOTC_ROUGHTIME_NETWORK_ERROR;
    default:
      break;
    }
  }

  uint8_t recv_buf[roughtime::kMinRequestSize];
  int buf_len;
  state = iotc_bsp_io_net_read(socket, &buf_len, recv_buf, sizeof(recv_buf));
  if (IOTC_BSP_IO_NET_STATE_ERROR == state) {
    iotc_debug_logger("Read from the socket");
    iotc_bsp_io_net_close_socket(reinterpret_cast<iotc_bsp_socket_t*>(&socket));
    return IOTC_ROUGHTIME_NETWORK_ERROR;
  }
  const uint64_t kEndUs = iotc_bsp_time_getmonotonictime_milliseconds();

  iotc_bsp_io_net_close_socket(reinterpret_cast<iotc_bsp_socket_t*>(&socket));

  if (buf_len == -1) {
    if (errno == EINTR) {
      iotc_debug_format(stderr, "No response from %s with %d seconds.\n",
                        server_name.c_str(), kTimeoutSeconds);
      return IOTC_ROUGHTIME_TIMEOUT_ERROR;
    }

    iotc_debug_logger("Read from the socket");
    return IOTC_ROUGHTIME_NETWORK_ERROR;
  }

  std::string error;
  if (!roughtime::ParseResponse(
          &(time_data->timestamp), &(time_data->radius), &error,
          reinterpret_cast<const uint8_t*>(server_public_key.data()), recv_buf,
          buf_len, nonce)) {
    iotc_debug_format(stderr, "Response from %s failed verification: %s",
                      server_name.c_str(), error.c_str());
    return IOTC_ROUGHTIME_RECEIVE_TIME_ERROR;
  }

  // We assume that the path to the Roughtime server is symmetric and thus
  // add half the round-trip time to the server's timestamp to produce our
  // estimate of the current time.
  time_data->reply_time = (kEndUs - kStartUs);
  time_data->timestamp /= 1000;
  time_data->timestamp += (time_data->reply_time) / 2;

  return IOTC_ROUGHTIME_OK;
}

iotc_roughtime_state_t
iotc_roughtime_getcurrenttime(const char* name, const char* public_key,
                              const char* server_address,
                              iotc_roughtime_timedata_t* time_data) {
  int socket;
  iotc_roughtime_state_t state;
  if ((state = iotc_roughtime_create_socket(&socket, server_address)) !=
      IOTC_ROUGHTIME_OK)
    return state;
  if ((state = iotc_roughtime_receive_time(socket, name, public_key,
                                           time_data)) != IOTC_ROUGHTIME_OK)
    return state;
  return IOTC_ROUGHTIME_OK;
}

} // namespace roughtime

#ifdef __cplusplus
}
#endif