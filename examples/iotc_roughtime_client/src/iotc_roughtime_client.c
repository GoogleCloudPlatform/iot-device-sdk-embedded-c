#include <iotc_roughtime_client.h>

#include "string.h"
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>

#include "../../common/src/example_utils.h"

/* Application variables. */
iotc_crypto_key_data_t iotc_connect_private_key_data;

int main() {
  const char* name = "Cloudflare-Roughtime";
  const char* public_key = "gD63hSj3ScS+wuOeGrubXlq35N1c5Lby/S+T7MNTjxo=";
  const char* server_address = "roughtime.cloudflare.com:2002";
  iotc_roughtime_state_t state;
  iotc_roughtime_timedata_t* time_data =
      (iotc_roughtime_timedata_t*)malloc(sizeof(iotc_roughtime_timedata_t));
  if ((state = iotc_roughtime_getcurrenttime(name, public_key, server_address,
                                             time_data)) != IOTC_ROUGHTIME_OK) {
    return state;
  }

  printf("Received reply in %" PRIu64 "ms.\n", time_data->reply_time);
  printf("Current time is %" PRIu64 "ms from the epoch, ±%uμs \n",
         time_data->timestamp, time_data->radius);

  return state;
}