#include <iotc_time.h>
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @typedef iotc_bsp_io_net_state_e
 * @brief Returns result of process receiving time from roughtime server.
 */
typedef enum iotc_roughtime_state_e {
  IOTC_ROUGHTIME_OK = 0,
  IOTC_ROUGHTIME_RECEIVE_TIME_ERROR = 1,
  IOTC_ROUGHTIME_NETWORK_ERROR = 2,
  IOTC_ROUGHTIME_TIMEOUT_ERROR = 3,
  IOTC_ROUGHTIME_ERROR = 4,
} iotc_roughtime_state_t;

/**
 * @typedef iotc_roughtime_timedata_s
 * @brief Ties time data received from roughtime server.
 */
typedef struct iotc_roughtime_timedata_s {
  /**
   * reply_time - time spent on receiving current time from the server
   * timestamp - current time received from the server
   * radius - certainty of timestamp
   */
  uint64_t reply_time;
  uint64_t timestamp;
  uint32_t radius;
} iotc_roughtime_timedata_t;

/**
 * @function
 * @brief Gets current time from roughtime server
 *
 * @param [out] time_data the struct which contains time data got from roughtime
 * server.
 * @param [in] name the name of server from which get current time.
 * @param [in] public_key the public key for server
 * @param [in] server_address the address of server from which get current time.
 * @return
 * - iotc_roughtime_state t - the state of error caused while creating socket
 * and receiving time from roughtime server.
 */
iotc_roughtime_state_t
iotc_roughtime_getcurrenttime(const char* public_key,
                              const char* server_address,
                              iotc_roughtime_timedata_t* time_data);

#ifdef __cplusplus
}
#endif