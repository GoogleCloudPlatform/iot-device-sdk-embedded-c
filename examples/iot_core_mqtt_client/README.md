# MQTT client example

This example uses the Google Cloud IoT Device SDK for Embedded C to connect a native Linux application to the [Google Cloud IoT Core MQTT bridge](https://cloud.google.com/iot/docs/how-tos/mqtt-bridge#iot-core-mqtt-auth-run-cpp).

## Getting started

1. Run `make` in the root directory of the repository.

2. From the root directory, generate the native example application.

```
cd examples/iot_core_mqtt_client \
make
```

3. Run the following commands, substituting in your device and project information.

<pre>
make \
cd bin \
./iot_core_mqtt_client -p <i><b>PROJECT_ID</b></i> -d projects/<i><b>PROJECT_ID</b></i>/locations/<i><b>REGION</b></i>/registries/<i><b>REGISTRY_ID</b></i>/devices/<i><b>DEVICE_ID</b></i> -t /devices/<i><b>DEVICE_ID</b></i>/state
</pre>

To securely connect to Cloud IoT Core, the [Google Root CA PEM file](https://pki.google.com/roots.pem) must be in the example directory. The `make`process automatically downloads the Google Root CA PEM file to the example directory.
