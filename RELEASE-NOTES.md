# Google Cloud IoT Core Device SDK for Embedded C version  1.0.3
#### April 27, 2021

- Updated the POSIX BSP to use `open` instead of `fopen` for more consistent error detection, [#100](https://github.com/GoogleCloudPlatform/iot-device-sdk-embedded-c/pull/100) by [@pnfisher](https://github.com/pnfisher).
- Fixed a non null-terminated issue in message.topic with ‘iotc_user_subscription_callback_’, [#111](https://github.com/GoogleCloudPlatform/iot-device-sdk-embedded-c/issues/111) by [@csobrinho](https://github.com/csobrinho).
- Added overflow protection to ‘__iotc_calloc()’ calls, [#119](https://github.com/GoogleCloudPlatform/iot-device-sdk-embedded-c/pull/119)
- Updates to the Zephyr Troubleshooting guide, [#104](https://github.com/GoogleCloudPlatform/iot-device-sdk-embedded-c/pull/104) by [@galz10](https://github.com/galz10).
- Fixes to links within the User Guide, [#102](https://github.com/GoogleCloudPlatform/iot-device-sdk-embedded-c/pull/102) by [@mikevoyt](https://github.com/mikevoyt).
- Thank you to all of our contributors!

# Google Cloud IoT Core Device SDK for Embedded C version  1.0.2
#### February 4, 2020

- Wildcard topic name support in MQTT subscriptions, [#89](https://github.com/GoogleCloudPlatform/iot-device-sdk-embedded-c/pull/89) by [@sheindel](https://github.com/sheindel), thank you!
- Documentation overhaul, see Device SDK [API](https://googlecloudplatform.github.io/iot-device-sdk-embedded-c/api/html/) and [BSP](https://googlecloudplatform.github.io/iot-device-sdk-embedded-c/bsp/html/) references.
- Zephyr integration fixes, using Zephyr v1.14.0.
- Improvements on the 'make' build system (wolfssl fixes, addressing rebuild
  issues).

# Google Cloud IoT Core Device SDK for Embedded C version  1.0.1
#### April 8, 2019

- The TLS Board Support Package (BSP) reference implementation for mbedTLS has been updated to use deterministic ECDSA signatures.

- `iotc_get_state_string` may now be used to query error strings for application debugging. See `include/iotc_error.h` to enable this feature.

- Updated the gtest framework compilation standard to c++14.  This does not affect the IoT Device SDK library sources.

- Documentation:
  - Updates to the Device SDK [API](https://googlecloudplatform.github.io/iot-device-sdk-embedded-c/api/html/) and [BSP](https://googlecloudplatform.github.io/iot-device-sdk-embedded-c/bsp/html/) doxygen documentation.
  - [`README.md`](README.md) formatting updates.
  - [`CONTRIBUTING.md`](CONTRIBUTING.md) includes a new pull request processes and a link to the Google style guidelines.

- Networking BSP:
  - Added UDP and IPv6 support.
  - Removed the function `iotc_bsp_io_net_create_socket`. Sockets should now be created in BSP implementations of `iotc_bsp_iot_net_socket_connect`.
  - See `include/bsp/iotc_bsp_io_net.h` for more information about these changes.
  - Removed `src/bsp/platform/posix/iotc_bsp_hton.h` as it was no longer required.

- Time API:
  - The `iotc_time_t` typedef is now defined as an `int64_t`.
  - Added the function `iotc_bsp_time_getmonotonictime_milliseconds`.
  - See `include/bsp/iotc_time.h` for more information about these changes.

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
