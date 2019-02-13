# Google IoT Device SDK Embedded C Client version 0.7
#### October 1 2018

The initial release of the IoT Core Device SDK for Embedded C.  The SDK is designed to execute on embedded devices to provide MQTT connectivity to Google Cloud IoT Core.

The SDK uses a Board Support Package (BSP) architecture to compartmentalize device-specific code.  This modular approach should aid in the porting process as engineers need only focus on a few set of files to customize the client to their device SDK. A reference BSP implementation has been provided for POSIX systems, and two reference TLS implementations have been provided for use with either the mbedTLS or wolfSSL embedded libraries. TLS implementations are required for both TLS connectivity and for private-key signatures of JWTs, which serve as client authentication tokens when connecting to Google Clout IoT Core.

For more documentation on the process of porting the SDK to your embedded device, or for using another TLS library, please see the the README.md and the /doc/porting_guide.md of the github repository. For more information about Google Cloud IoT Core, please visit: https://cloud.google.com/iot-core.

The SDK was developed on Ubuntu Linux using gnu make.
