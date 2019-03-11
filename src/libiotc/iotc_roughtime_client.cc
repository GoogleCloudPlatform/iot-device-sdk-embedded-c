#include <iotc_bsp_io_net.h>
#include <iotc_bsp_rng.h>
#include <iotc_bsp_time.h>
#include <iotc_debug.h>
#include <iotc_roughtime_client.h>

#include "client.h"
#include "protocol.h"

// kTimeoutSeconds is the number of seconds that we will wait for a reply
// from the server.
static const int kTimeoutSeconds = 3;
// kRetrialTimes is the number of time retrying to connect with server when if
// it got failed
static const int kRetrialTimes = 3;

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
  const int port = std::stoi(kPortStr.c_str());

  if (IOTC_BSP_IO_NET_STATE_OK !=
      iotc_bsp_io_net_socket_connect(
          reinterpret_cast<iotc_bsp_socket_t*>(out_socket), host.c_str(), port,
          SOCKET_DGRAM)) {
    iotc_debug_logger("ERROR: Connect to the socket");
    return IOTC_ROUGHTIME_ERROR;
  }

  if (IOTC_BSP_IO_NET_STATE_OK !=
      iotc_bsp_io_net_connection_check(*out_socket, host.c_str(), port)) {
    iotc_debug_logger("ERROR: Check the connection");
    return IOTC_ROUGHTIME_ERROR;
  }

  return IOTC_ROUGHTIME_OK;
}

iotc_roughtime_state_t
iotc_roughtime_receive_time(int socket, const char* public_key,
                            iotc_roughtime_timedata_t* time_data) {
  std::string server_public_key(public_key);
  // TODO(b/127770330)
  // uint8_t nonce[roughtime::kNonceLength] = {0};
  uint8_t nonce[roughtime::kNonceLength];
  bzero(nonce, roughtime::kNonceLength);
  iotc_bsp_io_net_state_t state = IOTC_BSP_IO_NET_STATE_OK;
  iotc_bsp_socket_events_t socket_event;

  for (size_t i = 0; i < roughtime::kNonceLength; i++) {
    nonce[i] = (uint8_t)iotc_bsp_rng_get() % 256;
  }
  const std::string kRequest = roughtime::CreateRequest(nonce);
  if (kRequest.empty()) {
    iotc_debug_logger("ERROR: Roughtime create reaquest");
    iotc_bsp_io_net_close_socket(reinterpret_cast<iotc_bsp_socket_t*>(&socket));
    return IOTC_ROUGHTIME_ERROR;
  }

  /* Write to the socket */
  memset(&socket_event, 0, sizeof(iotc_bsp_socket_events_t));
  socket_event.iotc_socket = socket;
  socket_event.in_socket_want_write = 1;
  bool ready_to_write = false;
  while (!ready_to_write) {
    state = iotc_bsp_io_net_select(&socket_event, 1, kTimeoutSeconds);
    switch (state) {
    case IOTC_BSP_IO_NET_STATE_OK:
      if (socket_event.out_socket_can_write == 1) {
        ready_to_write = true;
      }
      break;
    case IOTC_BSP_IO_NET_STATE_TIMEOUT:
      iotc_bsp_io_net_close_socket(
          reinterpret_cast<iotc_bsp_socket_t*>(&socket));
      return IOTC_ROUGHTIME_TIMEOUT_ERROR;
    case IOTC_BSP_IO_NET_STATE_ERROR:
      iotc_bsp_io_net_close_socket(
          reinterpret_cast<iotc_bsp_socket_t*>(&socket));
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
    iotc_debug_logger("ERROR: Write to the socket");
    iotc_bsp_io_net_close_socket(reinterpret_cast<iotc_bsp_socket_t*>(&socket));
    return IOTC_ROUGHTIME_NETWORK_ERROR;
  }
  const uint64_t kStartUs = iotc_bsp_time_getmonotonictime_milliseconds();

  if (bytes_written < 0 ||
      static_cast<size_t>(bytes_written) != kRequest.size()) {
    iotc_debug_logger("ERROR: Write to the socket");
    iotc_bsp_io_net_close_socket(reinterpret_cast<iotc_bsp_socket_t*>(&socket));
    return IOTC_ROUGHTIME_NETWORK_ERROR;
  }

  /* Read from the socket */
  memset(&socket_event, 0, sizeof(iotc_bsp_socket_events_t));
  socket_event.iotc_socket = socket;
  socket_event.in_socket_want_read = 1;
  bool ready_to_read = false;
  while (!ready_to_read) {
    state = iotc_bsp_io_net_select(&socket_event, 1, kTimeoutSeconds);
    switch (state) {
    case IOTC_BSP_IO_NET_STATE_OK:
      if (socket_event.out_socket_can_read == 1) {
        ready_to_read = true;
      }
      break;
    case IOTC_BSP_IO_NET_STATE_TIMEOUT:
      iotc_bsp_io_net_close_socket(
          reinterpret_cast<iotc_bsp_socket_t*>(&socket));
      return IOTC_ROUGHTIME_TIMEOUT_ERROR;
    case IOTC_BSP_IO_NET_STATE_ERROR:
      iotc_bsp_io_net_close_socket(
          reinterpret_cast<iotc_bsp_socket_t*>(&socket));
      return IOTC_ROUGHTIME_NETWORK_ERROR;
    default:
      break;
    }
  }

  uint8_t recv_buf[roughtime::kMinRequestSize];
  int buf_len;
  state = iotc_bsp_io_net_read(socket, &buf_len, recv_buf, sizeof(recv_buf));
  if (IOTC_BSP_IO_NET_STATE_ERROR == state) {
    iotc_debug_logger("ERROR: Read from the socket");
    iotc_bsp_io_net_close_socket(reinterpret_cast<iotc_bsp_socket_t*>(&socket));
    return IOTC_ROUGHTIME_NETWORK_ERROR;
  }
  const uint64_t kEndUs = iotc_bsp_time_getmonotonictime_milliseconds();

  iotc_bsp_io_net_close_socket(reinterpret_cast<iotc_bsp_socket_t*>(&socket));

  std::string error;
  if (!roughtime::ParseResponse(
          &(time_data->timestamp), &(time_data->radius), &error,
          reinterpret_cast<const uint8_t*>(server_public_key.data()), recv_buf,
          buf_len, nonce)) {
    iotc_debug_format("ERROR: Response failed verification: %s",
                      error.c_str());
    return IOTC_ROUGHTIME_RECEIVE_TIME_ERROR;
  }

  // We assume that the path to the Roughtime server is symmetric and thus
  // add half the round-trip time to the server's timestamp to produce our
  // estimate of the current time.
  time_data->reply_time = (kEndUs - kStartUs);
  // Convert precision of timestamp from microsecond to millisecond.
  time_data->timestamp /= 1000;
  time_data->timestamp += (time_data->reply_time) / 2;

  return IOTC_ROUGHTIME_OK;
}

iotc_roughtime_state_t
iotc_roughtime_getcurrenttime(const char* public_key,
                              const char* server_address,
                              iotc_roughtime_timedata_t* time_data) {
  int socket;
  iotc_roughtime_state_t state;
  for (int i = 0; i < kRetrialTimes; i++) {
    if ((state = iotc_roughtime_create_socket(&socket, server_address)) !=
        IOTC_ROUGHTIME_OK) {
      iotc_debug_logger("ERROR: Create socket");
      continue;
    }
    if ((state = iotc_roughtime_receive_time(socket, public_key, time_data)) !=
        IOTC_ROUGHTIME_OK) {
      iotc_debug_logger("ERROR: Receive time");
      continue;
    }
    return IOTC_ROUGHTIME_OK;
  }
  return state;
}
} // namespace roughtime

#ifdef __cplusplus
}
#endif