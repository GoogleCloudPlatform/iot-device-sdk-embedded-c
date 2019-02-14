# FreeRTOS example
___

This example connects a FreeRTOS application to Google Cloud Platform IoT MQTT Bridge with the help of the Cloud IoT Device SDK for Embedded C.

### Steps to build the example
#### Build the Cloud IoT Device SDK with FreeRTOS BSP
- run `make PRESET=FREERTOS_POSIX_REL` in the root directory of the repo.

This step includes the download of the [FreeRTOS Kernel](https://www.freertos.org/index.html) and the [FreeRTOS Linux simulator](https://www.freertos.org/FreeRTOS-simulator-for-Linux.html). Then the FreeRTOS port of the Cloud IoT Device SDK gets built.

#### Build the example
- `cd examples/freertos_linux/Linux_gcc_gcp_iot`
- run `make`

This step generates the `Linux_gcc_gcp_iot` application for Linux.

### Running the example
The steps to provision a device in GCP IoT Device Manager required you to create a Public/Private key pair. This example requires that your private key reside in the current working directory in a PEM formatted file named `ec_private.pem`.

Once there, please run the following command substituting in your IoT Core Device and Project Credentials:

- `./Linux_gcc_gcp_iot -p <GCP IoT Core Project ID> -d projects/<GCP IoT Core Project ID>/locations/<Region>/registries/<GCP IoT Core Registry ID>/devices/<GCP IoT Core Device ID> -t /devices/<GCP IoT Core DeviceID>/state`

If everything works well this should result in a successful MQTT connection to the GCP IoT MQTT Bridge and a PUBLISH message sent to the cloud in five second intervals.

### Troubleshooting
#### Validating GCP IoT credentials
For debugging purposes it might be useful to detach from FreeRTOS solution and use the native example to validate the GCP IoT credentials first. To build a linux version of the library:
- run `make` in the root directory of the repository.
- `cd examples/iot_core_mqtt_client`
- run `make`
- run `./bin/iot_core_mqtt_client -p <GCP IoT Core Project ID> -d projects/<GCP IoT Core Project ID>/locations/<Region>/registries/<GCP IoT Core Registry ID>/devices/<GCP IoT Core Device ID> -t /devices/<GCP IoT Core DeviceID>/state`
