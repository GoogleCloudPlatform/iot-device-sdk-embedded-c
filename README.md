# Google Cloud IoT Device SDK for Embedded C
___

The Google Cloud IoT Device SDK for Embedded C consists of one main element: the Google Cloud IoT Device Embedded C Client.  This codebase is an easy-to-port, open-source C client that connects low-end IoT devices to Google Cloud IoT Core.

The Embedded C Client supports concurrent Pub/Sub traffic on a non-blocking socket implementation that runs on POSIX, RTOS, and no-OS devices. A Board Support Package (BSP) facilitates portability and provides platform-specific implementations through a set of functions and directories. The BSP helps you implement crypto and time functions, networking, Transport Layer Security (TLS), memory management, and random number generators—without working through MQTT internals.

For more details, see the user guide in the `docs` directory.

## Source
To get the source, clone from the master branch of the Google Cloud IoT Device Embedded C Client GitHub repository:

[https://github.com/googlecloudplatform/iot-device-sdk-embedded-c](https://github.com/googlecloudplatform/iot-device-sdk-embedded-c)

### Directory structure

- **bin**: Executables and libraries produced by a build.
- **doc**: Documentation: Doxygen API, user guide, and porting guide.
- **examples**: Example source with makefiles. After you build with Make, this directory will also contain the example executables.
- **include**: Header files of the Cloud IoT Device Embedded C Client API. You must add this directory to the header include path when compiling your application against the source.
- **include/bsp**: Header files of the Board Support Package (BSP). Functions declared here must be defined in a device-specific BSP implementation. When compiling your BSP source, make sure this directory is on the include path.
- **make**: Build system configuration files.
- **obj**: Object files generated during a build.
- **res**: Resource files (for example, trusted root CA certificates.)
- **src**: The source files of the Cloud IoT Device Embedded C Client. A BSP implementation is provided for POSIX.
- **third_party**: Third-party open-source components.
- **tools**: Scripts used by the maintainers of the repository.

## Building

The default build environment for the Google Cloud IoT Device Embedded C Client is `Make`. It invokes [GCC](https://www.gnu.org/software/gcc/) to produce a native build on a POSIX host. This serves as the default development environment on Ubuntu.

Run the following command to build the Google Cloud IoT Device Embedded C Client:

        make

The source can be cross-compiled to other target platforms via custom toolchains and the makefile, or with a specific device SDK’s IDE. For more information, see the porting guide in `doc/porting_guide.md`.


### Building a TLS static library

By default, the Embedded C Client is built with secure connection support and thus requires a third-party TLS implementation to link against. When executing `make`, the build system defaults to mbedTLS.

As a result, the following commands are synonymous.

        make
        make IOTC_BSP_TLS=mbedtls

You can also configure the Make system to build a client that uses wolfSSL.

        make clean
        make IOTC_BSP_TLS=wolfssl

The value of IOTC_BSP_TLS determines which script is run.

        mbedtls: res/tls/build_mbedtls.sh
        wolfssl: res/tls/build_wolfssl.sh


The mbedTLS build script includes a git clone and branch checkout of the mbedTLS source (upon confirmation of the license agreement). However, the wolfSSL build script requires you to clone the repository yourself. When you run `make IOTC_BSP_TLS=wolfSSL`, instructions are provided for cloning the repo.

For more details on running the scripts, see [Security](#security).

### Building and executing tests

    make tests

By default, test execution is the final step of the `tests` build process. You can also execute the tests manually.

    cd bin/{host_os}/tests
    ./iotc_utests
    ./iotc_gtests
    ./iotc_itests


### Building the examples

Before building the examples, build both the Google Cloud IoT Device Embedded C Client static library and a TLS library, as described in the preceding sections. Then build the examples.

    cd examples
    make

The examples require device credentials for the Google Cloud IoT Core MQTT bridge, including the device's private key. For more information on credentials, see [Creating public/private key pairs](https://cloud.google.com/iot/docs/how-tos/credentials/keys) in the Cloud IoT Core documentation.

To securely connect to Cloud IoT Core, the [Google Root CA PEM file](https://pki.google.com/roots.pem) must be in the example directory. The Google Root CA PEM file is included in the build process.


### Cross-compilation

Follow the steps below to perform the cross-compilation process.

- Extend the build system with a new cross-compilation preset in the file `make/mt-config/mt-presets.mk`.
- Create a set of files that represent the Board Support Package (BSP) implementation you've written for your platform. Store these in the directory `src/bsp/platform/TARGET_PLATFORM`.
- Build a TLS library for the target platform in the directory `src/bsp/tls/TARGET_TLS_SOLUTION`.

For more details on the cross-compilation process, see the porting guide in `doc/porting_guide.md`.

## Security

The Cloud IoT Device Embedded C Client supports secure connection through a third-party TLS library. The client has been tested against [mbedTLS](https://tls.mbed.org) and [wolfSSL](https://www.wolfssl.com).

This repository does not directly include TLS libraries; you can clone the TLS git repositories and place them in the `third_party/tls/mbedtls` and `third_party/tls/wolfssl` directories, respectively. Running `make` without any parameters will start a build that includes git checkout, build configuration, and compilation of the mbedTLS library.

The Cloud IoT Device Embedded C Client supports other TLS libraries through the BSP TLS API. For information about configuring the build process to work with your preferred library, see the porting guide in `doc/porting_guide.md`.  Additionally, check the user guide `doc/user_guide.md` to make sure your TLS implementation meets the security requirements to connect to Cloud IoT Core.

## Stability and QA

19 combinations of compilers and feature sets are continuously built. 58 functional, 23 integration and 199 unit tests are executed after each build. Tests are executed against the TLS libraries [mbedTLS](https://tls.mbed.org) and [wolfSSL](https://www.wolfssl.com).

Branch      | Build status
------------|-------------
master      | ![travis-private-repo-icon-master]

[travis-private-repo-icon-master]: https://travis-ci.com/GoogleCloudPlatform/iot-edge-sdk-embedded-c.svg?token=tzWdJymp9duuAGWpamkM&branch=master


## Contributing

For information about contributing to this repository, see `CONTRIBUTING.MD`.

## Learn more

Review the following documentation.

- **doc/user_guide.md**: User guide that covers client features and usage.
- **doc/porting_guide.md**: Porting guide that provides information about porting the client to target devices.
- **doc/doxygen/**: Generated API reference.

## License

Copyright 2018-2019 Google LLC

Licensed under the BSD 3-Clause license.

For more information, see `LICENSE.md`.

This is not an officially supported Google product.
