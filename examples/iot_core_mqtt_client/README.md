# MQTT client example

This example uses the Google Cloud IoT Device SDK for Embedded C to connect a native Linux application to the [Google Cloud IoT Core MQTT bridge](https://cloud.google.com/iot/docs/how-tos/mqtt-bridge#iot-core-mqtt-auth-run-cpp).

## Getting started

Before you begin, generate a [public/private key pair](https://cloud.google.com/iot/docs/how-tos/credentials/keys), store the private key in the `examples/iot_core_mqtt_client/bin` directory, and name the key `ec_private.pem`.

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

## Troubleshooting

### Resolving platform-incompatible build errors

When building the example, if you encounter similar errors like:

```
/usr/bin/ld: skipping incompatible .../third_party/tls/mbedtls/library//libmbedcrypto.a when searching for -lmbedcrypto
```

do the following, from the repository's root directory:

```
rm -rf third_party/tls/mbedtls
make clean
make
cd examples/iot_core_mqtt_client
make clean
make
```

This should download the compatible port of mbedTLS, build the IoTC library and the example.
