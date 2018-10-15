#include "iotc_bsp_io_fs.h"
#include "iotc_error.h"

/* helper function that translates the errno errors to the
 * iotc_bsp_io_fs_state_t values */
iotc_state_t iotc_fs_bsp_io_fs_2_iotc_state(
    iotc_bsp_io_fs_state_t bsp_state_value);
