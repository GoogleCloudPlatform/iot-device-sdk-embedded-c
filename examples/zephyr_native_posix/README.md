# Zephyr RTOS example

This example uses the Google Cloud IoT Device SDK for Embedded C to connect a Zephyr native_posix board application to the [Google Cloud IoT Core MQTT bridge](https://cloud.google.com/iot/docs/how-tos/mqtt-bridge#iot-core-mqtt-auth-run-cpp).

## Getting started
Follow the steps below to connect the Zephyr application to the MQTT bridge.

Before you begin, generate a [public/private key pair](https://cloud.google.com/iot/docs/how-tos/credentials/keys), store the private key in the `examples/zephyr_native_posix/zephyr` directory, and name the key `ec_private.pem`.

1. Run `make PRESET=ZEPHYR` in the root directory of the repository. This command includes `git clone` of the Zephyr repository, sets Zephyr required environment variables, and auto-generates `.h` files that the Zephyr BSP requries.

2. From the root directory, generate the Zephyr native_posix board application.

```
cd examples/zephyr_native_posix/build \
make
```

3. Run the following command to connect to Cloud IoT Core and issue a `PUBLISH` message every five seconds.

<pre>
zephyr/zephyr.exe -testargs -p <i><b>PROJECT_ID</b></i> -d projects/<i><b>PROJECT_ID</b></i>/locations/<i><b>REGION</b></i>/registries/<i><b>REGISTRY_ID</b></i>/devices/<i><b>DEVICE_ID</b></i> -t /devices/<i><b>DEVICE_ID</b></i>/state
</pre>

## Troubleshooting

### Installing Dependecies 
If you're recieving a configuring incomplete error then you might not have the correct compiler installed, to download 
device-tree-compiler run the following command:

```bash
sudo apt-get install device-tree-compiler gperf
```

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
