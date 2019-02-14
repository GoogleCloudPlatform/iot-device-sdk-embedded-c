# Google Cloud IoT Core Device SDK for Embedded C version 0.7
#### October 1 2018

The initial release of the Cloud IoT Core Device SDK for Embedded C.  The SDK connects embedded devices to Google Cloud IoT Core via MQTT.

The SDK uses a Board Support Package (BSP) architecture to compartmentalize device-specific code.  This modular approach aids in the porting process. Engineers only need to focus on a few files to customize the Device SDK for their device. A reference BSP implementation is provided for POSIX systems and two reference TLS implementations are provided for use with either the mbedTLS or wolfSSL embedded libraries. TLS implementations are required for both TLS connectivity and for private-key JWT signatures. JWTs serve as client authentication tokens when connecting to Google Clout IoT Core.

For more documentation on the process of porting the SDK to your embedded device, or for using another TLS library, please see the README.md and the /doc/porting_guide.md documents. For more information about Google Cloud IoT Core, please visit: https://cloud.google.com/iot-core.

The Device SDK is developed on Ubuntu Linux with GNU Make.
