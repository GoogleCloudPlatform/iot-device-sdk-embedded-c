# Zephyr RTOS example

This example uses the Google Cloud IoT Device SDK for Embedded C to connect a Zephyr native_posix board application to the [Google Cloud IoT Core MQTT bridge](https://cloud.google.com/iot/docs/how-tos/mqtt-bridge#iot-core-mqtt-auth-run-cpp).

## Getting started
Follow the steps below to connect the Zephyr application to the MQTT bridge.

1. Go to the repository's root directory.

```
cd iot-device-sdk-embedded-c
```

1. Generate a [public/private key pair](https://cloud.google.com/iot/docs/how-tos/credentials/keys), and store it in the example's directory.

```
openssl ecparam -genkey -name prime256v1 -noout -out examples/zephyr_native_posix/zephyr/ec_private.pem
openssl ec -in examples/zephyr_native_posix/zephyr/ec_private.pem -pubout -out examples/zephyr_native_posix/zephyr/ec_public.pem
```

1. Clone the Zephyr repository, and set it up for Zephyr BSP, then build the IoT Device SDK.

```
make clean_all
make PRESET=ZEPHYR
```

1. Build the example application.

```
make -C examples/zephyr_native_posix/build
```

1. Run the following command to connect to Cloud IoT Core and issue a `PUBLISH` message every five seconds.

```
examples/zephyr_native_posix/zephyr/zephyr.exe -testargs \
    -p <i><b>PROJECT_ID</b></i> \
    -f examples/zephyr_native_posix/zephyr/ec_private.pem
    -d projects/<i><b>PROJECT_ID</b></i>/locations/<i><b>REGION</b></i>/registries/<i><b>REGISTRY_ID</b></i>/devices/<i><b>DEVICE_ID</b></i> \
    -t /devices/<i><b>DEVICE_ID</b></i>/state \
```

## Troubleshooting

### Setting up internet access on the native_posix board
By default, the Zephyr application claims IP 192.0.2.1 and is in the same subnet with the `zeth` virtual network adapter at IP 192.0.2.2. This subnet must be connected to the internet.

To ensure internet connectivity, run the [socket HTTP GET example](https://docs.zephyrproject.org/latest/samples/net/sockets/http_get/README.html).

Read the following references to start the `zeth` virtual network adapter and connect the subnet to internet see the Zephyr instructions.
- [Networking with native_posix board](https://docs.zephyrproject.org/latest/guides/networking/native_posix_setup.html)
- [Setting up Zephyr and NAT and masquerading on host to access internet](https://docs.zephyrproject.org/latest/guides/networking/qemu_setup.html#setting-up-zephyr-and-nat-masquerading-on-host-to-access-internet)

### Setting up the Zephyr development system

Another way to set environment variables is by permanently set up the Zephyr environment in your terminal. To permanently install the Zephyr environment, follow the instructions on [Set up a development system](https://docs.zephyrproject.org/latest/getting_started/index.html#set-up-a-development-system).

### Validating Cloud IoT Core credentials

Build the [MQTT client example](https://github.com/GoogleCloudPlatform/iot-device-sdk-embedded-c/tree/docs_updates/examples/iot_core_mqtt_client) to validate your Cloud IoT Core credentials.
