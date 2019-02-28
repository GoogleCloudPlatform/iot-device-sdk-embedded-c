# Google Cloud IoT Core Device SDK for Embedded C version  1.0
#### February 22, 2019

- Added FreeRTOS example. See `examples/freertos_linux/Linux_gcc_gcp_iot/README.md` for more information.

- Added Zephyr example.  See `examples/zephyr_native_posix/README.md` for more information.

- Added ATECC608 Secure Element reference implementation for generating JWT signatures with private keys.  See `src/bsp/crypto/cryptoauthlib/iotc_bsp_crypto_cryptoauthlib.c` for more information.

- Connection API Changes:
  - JWT creation and formation was added to a new header file `include/iotc_jwt.h`.
  - `iotc_connect` and `iotc_connect_to` now take the standard MQTT connect credentials of username, password, and client_id.  A JWT, formatted according to [Cloud IoT Core specifications](https://cloud.google.com/iot/docs/how-tos/credentials/jwts), should be passed as the MQTT password when connecting to Cloud IoT Core.
  - For more information, see the Device SDK [API](https://googlecloudplatform.github.io/iot-device-sdk-embedded-c/api/html/index.html) or [BSP](https://googlecloudplatform.github.io/iot-device-sdk-embedded-c/bsp/html/index.html) reference.

# Google Cloud IoT Core Device SDK for Embedded C version 0.7
#### October 1, 2018

The initial release of the Cloud IoT Core Device SDK for Embedded C.  The SDK connects embedded devices to Google Cloud IoT Core via MQTT.

The SDK uses a Board Support Package (BSP) architecture to compartmentalize device-specific code.  This modular approach aids in the porting process. Engineers only need to customize a few files to port the Device SDK to their device. A reference BSP implementation is provided for POSIX systems and two reference TLS implementations are provided for use with either the mbedTLS or wolfSSL embedded libraries. TLS implementations are required for both TLS connectivity and for private-key JWT signatures. JWTs serve as client authentication tokens when connecting to Google Clout IoT Core.

For more documentation on the process of porting the SDK to your embedded device, or for using another TLS library, please see the README.md and the /doc/porting_guide.md documents. For more information about Google Cloud IoT Core, please visit: https://cloud.google.com/iot-core.

The Device SDK is developed on Ubuntu Linux with GNU Make.
