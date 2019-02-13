# FreeRTOS example
___

This example connects a FreeRTOS application to Google Cloud Platform IoT MQTT Bridge with the help of the Cloud IoT Device SDK
library.

### Steps to build the example
#### Build the Cloud IoT Device SDK with FreeRTOS BSP
- run `make PRESET=FREERTOS_POSIX_REL` in the root directory of the repo.

This step includes the download of the [FreeRTOS Kernel](https://www.freertos.org/index.html) and the [FreeRTOS Linux simulator](https://www.freertos.org/FreeRTOS-simulator-for-Linux.html). Then the FreeRTOS port of the Cloud IoT Device SDK gets built.

#### Build the example
- `cd examples/freertos_linux/Linux_gcc_gcp_iot`
- run `make`

This step generates the `Linux_gcc_gcp_iot` application for Linux.

### Running the example
The device creation in the GCP IoT Device Manager required a public key to be uploaded to the device. You will need the private key pair now. An `ec_private.pem` file should contain this private key in the current working directory. Run the example with the following command: 

- `./Linux_gcc_gcp_iot -p <GCP IoT Core Project ID> -d projects/<GCP IoT Core Project ID>/locations/<Region>/registries/<GCP IoT Core Registry ID>/devices/<GCP IoT Core Device ID> -t /devices/<GCP IoT Core DeviceID>/state`

If everything works well this should result in a successful MQTT connection to GCP IoT MQTT Bridge and regular 5s MQTT PUBLISH messages sent to the cloud.

### Troubleshooting
#### Validating GCP IoT credentials
For debugging purposes it might be useful to detach from FreeRTOS solution and use the native example to validate the GCP IoT credentials first. To build a linux version of the library:
- run `make` in the root directory of the repository.
- `cd examples/iot_core_mqtt_client`
- run `make`
- run `./bin/iot_core_mqtt_client -p <GCP IoT Core Project ID> -d projects/<GCP IoT Core Project ID>/locations/<Region>/registries/<GCP IoT Core Registry ID>/devices/<GCP IoT Core Device ID> -t /devices/<GCP IoT Core DeviceID>/state`
