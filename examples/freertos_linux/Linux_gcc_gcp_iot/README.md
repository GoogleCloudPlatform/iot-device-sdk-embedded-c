# FreeRTOS example

This example uses the Google Cloud IoT Device SDK for Embedded C to connect a FreeRTOS application to the [Google Cloud IoT Core MQTT bridge](https://cloud.google.com/iot/docs/how-tos/mqtt-bridge#iot-core-mqtt-auth-run-cpp).

## Getting started

Follow the steps below to connect the FreeRTOS application to the MQTT bridge.

1. Go to the repository's root directory:

```
cd iot-device-sdk-embedded-c
```

1. Generate a [public/private key pair](https://cloud.google.com/iot/docs/how-tos/credentials/keys), and store it in the example's directory.

1. Download the [FreeRTOS kernel](https://www.freertos.org/index.html), the [FreeRTOS Linux simulator](https://www.freertos.org/FreeRTOS-simulator-for-Linux.html), and port the Device SDK to the FreeRTOS application.

```
make clean_all
make PRESET=FREERTOS_POSIX_REL
```

1. Build the example application.

```
make -C examples/freertos_linux/Linux_gcc_gcp_iot
```

1. Run the following command to connect to Cloud IoT Core and issue a `PUBLISH` message every five seconds.

```
examples/freertos_linux/Linux_gcc_gcp_iot/Linux_gcc_gcp_iot \
    -p <i><b>PROJECT_ID</b></i> \
    -f examples/freertos_linux/Linux_gcc_gcp_iot/Linux_gcc_gcp_iot/ec_private.pem
    -d projects/<i><b>PROJECT_ID</b></i>/locations/<i><b>REGION</b></i>/registries/<i><b>REGISTRY_ID</b></i>/devices/<i><b>DEVICE_ID</b></i> \
    -t /devices/<i><b>DEVICE_ID</b></i>/state \
```

## Troubleshooting

### Validating Cloud IoT Core credentials

Build the MQTT client example to validate your Cloud IoT Core credentials.
