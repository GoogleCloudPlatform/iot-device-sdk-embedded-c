#include "../../common/src/example_utils.h"
#include <iotc_roughtime_client.h>

#include "string.h"
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>

/* Application variables. */
iotc_crypto_key_data_t iotc_connect_private_key_data;

int main() {
  int socket = 0;
  const char* name = "Cloudflare-Roughtime";
  const char* public_key = "gD63hSj3ScS+wuOeGrubXlq35N1c5Lby/S+T7MNTjxo=";
  const char* server_address = "roughtime.cloudflare.com:2002";
  uint64_t reply_time, timestamp;
  uint32_t radius;
  int64_t system_offset;

  if (!iotc_roughtime_create_socket(&socket, server_address)) {
    return kExitNetworkError;
  }

  int state =
      iotc_roughtime_getcurrenttime(socket, name, public_key, &reply_time,
                                    &timestamp, &radius, &system_offset);
  if (state != 0)
    return state;

  printf("Received reply in %" PRIu64 "μs.\n", reply_time);
  printf("Current time is %" PRIu64 "μs from the epoch, ±%uμs \n", timestamp,
         radius);
  printf("System clock differs from that estimate by %" PRId64 "μs.\n",
         system_offset);

  return 0;
}