#include <iotc_time.h>
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif
enum ExitCode {
  kExitBadSystemTime = 1,
  kExitBadArguments = 2,
  kExitNoServer = 3,
  kExitNetworkError = 4,
  kExitTimeout = 5,
  kExitBadReply = 6,
};

bool iotc_roughtime_create_socket(int *out_socket, const char *address);

int iotc_roughtime_getcurrenttime(int socket, const char *server_name,
                                  const char *server_public_key);
#ifdef __cplusplus
}
#endif