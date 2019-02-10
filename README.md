# Google Cloud IoT Device SDK for Embedded C
___

The Google Cloud IoT Device SDK for Embedded C is an easy-to-port, open source C client that connects low-end IoT devices to Google Cloud IoT Core.

The IoT Device SDK supports concurrent Pub/Sub traffic on a non-blocking socket implementation that runs on POSIX, RTOS, and no-OS devices. A Board Support Package (BSP) facilitates portability and provides platform-specific implementations through a set of functions and directories. The BSP helps you implement networking, Transport Layer Security (TLS), memory management, random number generators, crypto and time functions, without working through MQTT internals.

For more details, see the User Guide in the `docs` directory.

## Source
To get the source, clone from the master branch of the Google IoT Device SDK for Embedded C GitHub repository:

[https://github.com/googlecloudplatform/iot-device-sdk-embedded-c](https://github.com/googlecloudplatform/iot-device-sdk-embedded-c)

### Directory structure

- **bin**: Executables and libraries produced by a build.
- **doc**: Documentation: Doxygen API, User Guide, Porting Guide.
- **examples**: Example source with makefiles. After you build with make, this directory also contains the example executables.
- **include**: Header files of the IoT Device SDK API. You must add this directory to the header include path when compiling your application against the source.
- **include/bsp**: Header files of the Board Support Package (BSP). Functions declared here must be defined in a device-specific BSP implementation. When compiling your BSP source, make sure this directory is on the include path.
- **make**: Build system configuration files.
- **obj**: Object files generated during a build.
- **res**: Resource files (for example, trusted root CA certificates.)
- **src**: The source files of the IoT Device SDK. A BSP implementation is provided for POSIX.
- **third_party**: Third-party open source components.
- **tools**: Scripts used by the maintainers of the repository.

## Building

The default build environment for the IoT Device SDK for Embedded C is `make`. It invokes [GCC ](https://www.gnu.org/software/gcc/) to produce a native build on a POSIX host. This serves as the default development environment on Ubuntu.

To build the Google Cloud IoT Edge Embedded C Client simply run:

        make

The source can be cross-compiled to other target platforms via custom toolchains and the makefile, or with the specific device SDK’s IDE. For more information, see the Porting Guide in `doc/porting_guide.md`.


### Building a TLS static library

By default, the IoT Device SDK is built with secure connection support and thus requires a third-party TLS implementation to link against. When executing `make`, the build system defaults to mbedTLS.

As a result, the following commands are synonymous:

        make
        make IOTC_BSP_TLS=mbedtls

You can also configure the make system to build a client that uses wolfSSL:

        make clean
        make IOTC_BSP_TLS=wolfssl

The value of IOTC_BSP_TLS determines which script is run:

        mbedtls: res/tls/build_mbedtls.sh
        wolfssl: res/tls/build_wolfssl.sh


The mbedTLS build script includes a git clone and branch checkout of the mbedTLS source (upon confirmation of the license agreement). However, the wolfSSL build script requires that you clone the repository yourself. When you attempt to run `make IOTC_BSP_TLS=wolfSSL`, instructions are provided for cloning the repo.

For more details on running the scripts, see [Security](#security).

### Building and executing tests

    make tests

By default, the build process for `tests` includes test execution as the final step. You can also execute the tests manually:

    cd bin/{host_os}/tests
    ./iotc_utests
    ./iotc_gtests
    ./iotc_itests


### Building the examples

Before building the examples, build both the IoT Device SDK static library and a a third party TLS library, as described in the preceding sections. Then build the examples:

    cd examples
    make

The examples require device credentials for the Google Cloud IoT Core MQTT bridge, including the device's private key. For more information on credentials, see [Creating Public/Private Key Pairs](https://cloud.google.com/iot/docs/how-tos/credentials/keys) in the Cloud IoT Core documentation.

For a secure connection, the [Google Root CA PEM file](https://pki.google.com/roots.pem) must exist in the example directory. This is included in the build process.


### Cross-compilation

Here's a summary of the cross-compilation process:

- Extend the build system with a new cross-compilation preset in the file `make/mt-config/mt-presets.mk`.
- Create a set of files that represents the Board Support Package (BSP) implementation you've written for your platform. Store these in the directory `src/bsp/platform/TARGET_PLATFORM`.
- Build a TLS library for the target platform, in the directory `src/bsp/tls/TARGET_TLS_SOLUTION`.

For complete details, see the Porting Guide in `doc/porting_guide.md`.

## Security

The Cloud IoT Devce SDK supports secure connection through a third-party TLS library. The SDK has been tested against [mbedTLS](https://tls.mbed.org), and [wolfSSL](https://www.wolfssl.com).

This repository does not directly include TLS libraries; you can clone the TLS git repositories and place them in the `third_party/tls/mbedtls` and `third_party/tls/wolfssl` directories, respectively. Running `make` without any parameters will start a build that includes git checkout, build configuration, and compilation of the mbedTLS library.

The Cloud IoT Device SDK supports other TLS libraries through the BSP TLS API. See the Porting Guide in `doc/porting_guide.md` for information about configuring the build process to work with your preferred library.  Additionally, check the user guide `doc/user_guide.md` to make sure your TLS implementation meets the security requirements for connecting to Cloud IoT Core.

## Stability and QA

19 combinations of compilers and feature sets are continuously built. 58 functional, 23 integration and 217 unit tests are executed after each build. Tests are executed against the TLS libraries [mbedTLS](https://tls.mbed.org) and [wolfSSL](https://www.wolfssl.com).

Branch      | Build status
------------|-------------
master      | ![travis-private-repo-icon-master]

[travis-private-repo-icon-master]: https://travis-ci.com/GoogleCloudPlatform/iot-device-sdk-embedded-c.svg?token=tzWdJymp9duuAGWpamkM&branch=master


## Contributing

For information about contributing to this repository, see `CONTRIBUTING.MD`.

## Learn more

Review the following documentation:

- **doc/user_guide.md**: User Guide, covering client features and usage.
- **doc/porting_guide.md**: Porting Guide, providing information about porting the client to target devices.
- **doc/doxygen/**: Generated API reference.

## License

Copyright 2018-2019 Google LLC

Licensed under the BSD 3-Clause license.

For more information, see `LICENSE.md`.

This is not an officially supported Google product.
