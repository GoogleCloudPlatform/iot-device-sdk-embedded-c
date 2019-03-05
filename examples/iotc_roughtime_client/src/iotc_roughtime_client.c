#include <iotc_roughtime_client.h>
#include "../../common/src/example_utils.h"

#include "string.h"
#include <stdint.h>
#include <stdio.h>

/* Application variables. */
iotc_crypto_key_data_t iotc_connect_private_key_data;

int main() {
  int socket = 0;
  const char* name = "Cloudflare-Roughtime";
  const char* public_key = "gD63hSj3ScS+wuOeGrubXlq35N1c5Lby/S+T7MNTjxo=";
  const char* server_address = "roughtime.cloudflare.com:2002";

  if (!iotc_roughtime_create_socket(&socket, server_address)) {
    return kExitNetworkError;
  }

  iotc_roughtime_getcurrenttime(socket, name, public_key);

  return 0;
}