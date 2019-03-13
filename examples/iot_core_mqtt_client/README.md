# MQTT client example

This example uses the Google Cloud IoT Device SDK for Embedded C to connect a native Linux application to the [Google Cloud IoT Core MQTT bridge](https://cloud.google.com/iot/docs/how-tos/mqtt-bridge#iot-core-mqtt-auth-run-cpp).

## Getting started

1. Go to the repository's root directory:

```
cd iot-device-sdk-embedded-c
```

1. Generate a [public/private key pair](https://cloud.google.com/iot/docs/how-tos/credentials/keys), and store it in the example's directory.

1. Build the IoT Device SDK.

```
make clean_all
make
```

1. Build the example application.

```
make -C examples iot_core_mqtt_client
```

1. Run the following command to connect to Cloud IoT Core and issue a `PUBLISH` message every five seconds.

```
examples/iot_core_mqtt_client/bin/iot_core_mqtt_client \
    -p <i><b>PROJECT_ID</b></i> \
    -f examples/iot_core_mqtt_client/ec_private.pem
    -d projects/<i><b>PROJECT_ID</b></i>/locations/<i><b>REGION</b></i>/registries/<i><b>REGISTRY_ID</b></i>/devices/<i><b>DEVICE_ID</b></i> \
    -t /devices/<i><b>DEVICE_ID</b></i>/state \
```
