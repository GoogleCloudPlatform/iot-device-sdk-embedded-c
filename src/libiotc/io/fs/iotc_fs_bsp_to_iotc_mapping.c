#include <iotc_fs_bsp_to_iotc_mapping.h>

/* helper function that translates the errno errors to the
 * iotc_bsp_io_fs_state_t values */
iotc_state_t iotc_fs_bsp_io_fs_2_iotc_state(
    iotc_bsp_io_fs_state_t bsp_state_value) {
  iotc_state_t ret = IOTC_STATE_OK;

  switch (bsp_state_value) {
    case IOTC_BSP_IO_FS_STATE_OK:
      ret = IOTC_STATE_OK;
      break;
    case IOTC_BSP_IO_FS_ERROR:
      ret = IOTC_FS_ERROR;
      break;
    case IOTC_BSP_IO_FS_INVALID_PARAMETER:
      ret = IOTC_INVALID_PARAMETER;
      break;
    case IOTC_BSP_IO_FS_RESOURCE_NOT_AVAILABLE:
      ret = IOTC_FS_RESOURCE_NOT_AVAILABLE;
      break;
    case IOTC_BSP_IO_FS_OUT_OF_MEMORY:
      ret = IOTC_OUT_OF_MEMORY;
      break;
    default:
      /** IF we're good engineers, then this should never happen */
      ret = IOTC_INTERNAL_ERROR;
      break;
  }

  return ret;
}
