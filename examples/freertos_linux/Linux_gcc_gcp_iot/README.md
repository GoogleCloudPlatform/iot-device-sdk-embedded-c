# FreeRTOS example

This example uses the Google Cloud IoT Device SDK for Embedded C to connect a FreeRTOS application to the [Google Cloud IoT Core MQTT bridge](https://cloud.google.com/iot/docs/how-tos/mqtt-bridge#iot-core-mqtt-auth-run-cpp).

## Getting started

Follow the steps below to connect the FreeRTOS application to the MQTT bridge.

Before you begin, generate a [public/private key pair](https://cloud.google.com/iot/docs/how-tos/credentials/keys), store the private key in the `examples/freertos_linux/Linux_gcc_gcp_iot` directory, and name the key `ec_private.pem`.

1. Run `make PRESET=FREERTOS_POSIX_REL` in the root directory of the repository. This command downloads the [FreeRTOS kernel](https://www.freertos.org/index.html), the [FreeRTOS Linux simulator](https://www.freertos.org/FreeRTOS-simulator-for-Linux.html), and ports the Device SDK to the FreeRTOS application.

2. From the root directory, generate the `Linux_gcc_gcp_iot` application for Linux.

```
cd examples/freertos_linux/Linux_gcc_gcp_iot \
make
```

3. Run the following command to connect to Cloud IoT Core and issue a `PUBLISH` message every five seconds.

<pre>
./Linux_gcc_gcp_iot -p <i><b>PROJECT_ID</b></i> -d projects/<i><b>PROJECT_ID</b></i>/locations/<i><b>REGION</b></i>/registries/<i><b>REGISTRY_ID</b></i>/devices/<i><b>DEVICE_ID</b></i> -t /devices/<i><b>DEVICE_ID</b></i>/state
</pre>

## Troubleshooting

### Validating Cloud IoT Core credentials

Build the MQTT client example to validate your Cloud IoT Core credentials.

### Resolving platform-incompatible build errors

When building the example, if you encounter similar errors like:

```

```

do the following, from the repository's root directory:

```
rm -rf third_party/tls/mbedtls
make clean
make make PRESET=ZEPHYR
cd examples/zephyr_native_posix/build
make clean
make
```

This should download the compatible port of mbedTLS, build the IoTC library and the example.
