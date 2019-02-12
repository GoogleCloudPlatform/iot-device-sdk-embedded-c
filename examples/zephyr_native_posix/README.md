# Zephyr RTOS example
___

This example connects a Zephyr native_posix board application to
Google Cloud Platform IoT MQTT Bridge with the help of the Embedded C Client
library.

### Steps to build the example
#### Build the Embedded C Client with Zephyr BSP
- run `make PRESET=ZEPHYR` in the root directory of the repo.

This step includes `git clone` of the Zephyr repository, setting Zephyr required environment variables, and prebuild steps of the example to auto-generate .h files required by the Zephyr BSP of the Embedded C Client.

#### Build the example
- `cd examples/zephyr_native_posix/build`
- run `make`

This step generates the zephyr/zephyr.exe native_posix board application for POSIX.

### Running the example
The device creation in the GCP IoT Device Manager required a public key to be uploaded to the device. You will need the private key pair now. An `ec_private.pem` file should contain this private key in the current working directory.
- `zephyr/zephyr.exe -testargs -p <GCP IoT Core Project ID> -d projects/<GCP IoT Core Project ID>/locations/<Region>/registries/<GCP IoT Core Registry ID>/devices/<GCP IoT Core Device ID> -t /devices/<GCP IoT Core DeviceID>/state`

If everything works well this should result in successful MQTT connection to GCP IoT MQTT Bridge and regular 5s MQTT PUBLISH messages set to the cloud.


### Troubleshooting
#### Setting up the native_posix board internet access
By default the Zephyr application claims IP 192.0.2.1 and is in the same subnet with the `zeth` virtual network adapter with IP 192.0.2.2. This subnet has to be connected to the internet. To start the `zeth` virtual network adapter and connect the subnet to internet see the Zephyr instructions:
- [Networking with native_posix board](https://docs.zephyrproject.org/latest/guides/networking/native_posix_setup.html)
- [Setting up Zephyr and NAT/masquerading on host to access Internet](https://docs.zephyrproject.org/latest/guides/networking/qemu_setup.html#setting-up-zephyr-and-nat-masquerading-on-host-to-access-internet) 
- To break down complexity and help debugging a practical step is to ensure internet connectivity by running the [Socket HTTP GET Example](https://docs.zephyrproject.org/latest/samples/net/sockets/http_get/README.html) first.

#### Set up the Zephyr Development System
Although the `make PRESET=ZEPHYR` should set the environment variables for the lifetime of the build. You might end up requiring the Zephyr envronment to be set up permanently in you terminal. To do this follow the instructions on page [Set Up a Development System](https://docs.zephyrproject.org/latest/getting_started/index.html#set-up-a-development-system).

#### Validating GCP IoT credentials
For debugging purposes it might be useful to detach from the Zephyr solution and use the native example to validate the GCP IoT credentials. To build a linux version of the library:
- run `make` in the root directory of the repository.
- `cd examples/mqtt_logic_producer`
- run `make`
- run `./bin/mqtt_logic_producer -p <GCP IoT Core Project ID> -d projects/<GCP IoT Core Project ID>/locations/<Region>/registries/<GCP IoT Core Registry ID>/devices/<GCP IoT Core Device ID> -t /devices/<GCP IoT Core DeviceID>/state`

Switching the Zephyr and linux builds back and forth has a flaw though. The mbedTLS library needs to be 32bit or 64bit respectively. Therefore, switching targets will requires the deletetion of the mbedTLS directory forcing an mbedTLS rebuild: `rm -rf third_party/tls/mbedtls`
