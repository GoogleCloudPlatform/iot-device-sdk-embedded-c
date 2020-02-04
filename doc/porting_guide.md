# Google Cloud IoT Device SDK for Embedded C Porting Guide

##### Copyright 2018-2020 Google LLC

## Table of Contents
1. [Introduction](#introduction)
2. [Building](#building)
3. [Porting](#porting)
4. [Additional Resources](#additional-resources)

# Introduction

To run the Google Cloud IoT Device SDK for Embedded C on new platforms, first read the [Building](#building) section to adapt the `make` environment to your toolchain. Then, review the [Porting](#porting) section to customize the Board Support Package (BSP) for the target device.

## Before you begin
Before you port the Device SDK to a new platform, review the `README.md` in the root directory of this repository and be familar with the [default build process](https://github.com/GoogleCloudPlatform/iot-device-sdk-embedded-c#building).

## Audience and scope
This document is for embedded device developers who want to run the [Device SDK](https://github.com/googlecloudplatform/iot-device-sdk-embedded-c) on their custom device.

This document provides details of the Device SDK's build steps, build configuration flags, and the location of the functions needed to port the SDK to new platforms.

For general information about the Device SDK's features and application usage, see the user guide in `docs/user_guide.md`.

## Notes about the codebase

The Device SDK accommodates a variety of IoT devices. Default host/target support is provided for POSIX systems (tested on Ubuntu Linux). The Device SDK was also tested on RTOS and no-OS devices.

Most of the codebase is cross-platform and resides in the `src/libiotc/` directory. Platform-specific code is in the `src/bsp/` directory.

The codebase is built against [C99 standards](https://en.wikipedia.org/wiki/C99).

# Building

This section describes how to configure make to cross-compile the Device SDK for your platform.

We recommend building the Device SDK for a Linux distribution before cross-compiling to another target platform. Building for Linux introduces you to the basic architecture and tools of the Device SDK source. This basic knowledge then helps you cross-compile the software to an embedded device.

## Build environment

The Device SDK builds its source files with `make`. The main makefile is in the root directory of the repository.

Depending on the build configuration, the main makefile includes more make-related (.mk) files that alter the list of compiled source files and toolchain command line arguments. Three flags in the main makefile specify the build configuration:  `TARGET`, `CONFIG` and `IOTC_BSP_TLS`.

* `TARGET` determines the platform for which you're compiling. This affects toolchain flags, file pathing, and BSP sources.  If this flag isn't specified, it defaults to `TARGET=linux-static-release` or `TARGET=osx-static-release`, depending your host system. OS X host builds are unsupported.

* `CONFIG` determines the Device SDK software modules, like the memory limiter or debug logging, that the system compiles into the library. This value defaults to `CONFIG=posix_fs-posix_platform-tls_bsp-memory_limiter` for local POSIX machine development.

* `IOTC_BSP_TLS` determines the Transport Layer Security (TLS) BSP implementation that the makefile compiles. The TLS BSP selection configures the Device SDK to encrypt data sent from the Device SDK over the network socket with the desired embedded TLS library. The default value of this flag is `IOTC_BSP_TLS=mbedtls`; the default implemenation is in `src/bsp/tls/mbedtls`. This also configures the build system to do cryptographic key signatures in mbedTLS via the implementation in `src/bsp/crypto/mbedtls`. The flag for the out-of-the-box wolfSSL implemenation, which resides in `src/bsp/tls/wolfssl` and `src/bsp/crypto/wolfssl`, is `IOTC_BSP_TLS=wolfssl`. Both of these implementations configure and build the third-party TLS library sources in the `third_party/tls/mbedtls` or `third_party/tls/wolfssl`, respectively. The build system prompts the user with instructions on how to populate these directories with the required TLS library sources.
  * The sources for the third-party TLS libraries aren't included in the repo by default. The `make` command automatically downloads mbedtls and configures it for the Device SDK. The `make` command doesn't automatically download wolfSSL; however, running `make` automatically provides instructions on how and where to download wolfSSL.
  * To define your own BSP implementation, see the [TLS BSP](#tls-bsp) section later in this document.
  * If you use a hardware TLS instead of a software TLS, compile without a TLS BSP and invoke the Device SDK's secure socket API directly from the Device SDK's networking BSP. To compile without a TLS BSP, follow the additional steps below.
    * Do not define `IOTC_BSP_TLS` on the `make` command line.
    * Change the `tls_bsp` config parameter to `tls_socket`.  For example, run `make CONFIG=posix_fs-posix_platform-tls_socket-memory_limiter`.

Specific `CONFIG` options are described in the [`CONFIG` and `TARGET` parameters](#config-and-target-parameters) section.

## IDE builds

Although the `make` build system is suitable for Linux/Unix builds, some embedded SDKs and toolchains may not support building via `make`. Often, these SDKs supply their own Integrated Development Environments (IDEs).

Before importing the Device SDK's source into an IDE, create a native Device SDK build on POSIX to see the following useful information for your IDE's preprocessor environment settings.

- A list of the source files that were compiled (.c)
- A list of the preprocessor definitions that were required for the build `CONFIG` you selected (-D)
- The flags passed to the compiler/toolchain
- The header file include paths that were used (-I)
- the link paths that were used to link a binary (-L)
- the linker library names that were used (-l)

Execute the following commands for more information about the build environment.

- Run `make MAKEFILE_DEBUG=1` to log all makefile variables and their corresponding values.

- Run `make MD=` on a clean build to log all of the commands, executables, and parameters that are created during to build process. `make MD=` also executes all of the logged commands.

- `make -n` is like `make MD=` but `make -n` doesn't execute the logged commands.

## CONFIG and TARGET parameters

The `CONFIG` and `TARGET` parameters are defined on the command line. For additional information about `CONFIG` and `TARGET`, see [Build environment](#build-environment).

Parameter values consist of a multi-component, single-string listing of module names. Spaces are not permitted; seperate modules with the hyphen ('-') character.

### TARGET

A typical `TARGET` argument consists of a platform flag, an output library flag and a build type flag.

>make TARGET=linux-static-debug

#### Platform flag

   - `[ linux | osx | arm-linux ]`   - Sets the target platform. The
                                     continuous integration services uses `arm-linux` to smoke-test cross-compliled builds. OS X is currently unsupported.

#### Output library flag

   - `[ static | dynamic ]`          - Sets the output library type.

#### Build type flag

   - `[ debug | release ]`          - Sets the build type. The `debug`
                                     option passes the standard toolchain
                                     debug symbols to the compiler. The `debug`option also provides runtime debug output from the library by adding the `IOTC_DEBUG_OUTPUT=1` preprocessor definition. To suppress this, append
                                     `IOTC_DEBUG_OUTPUT=0` to the `make` command.

### CONFIG

A typical `CONFIG` argument consists of an option feature flag, a file system flag, a development flag, and a platform sector flag.

>make CONFIG=posix_fs-posix_platform-tls_bsp-memory_limiter

#### Optional feature flag

   - `threading`            - POSIX only. Causes publication, subscription, and
                            connection callbacks to be called on separate threads. If not set, application callbacks are called on the main thread of the Device SDK's event system.

#### File system flag

The file system flag is for reading public root CAs for service authentication during the TLS handshake.

   - `posix_fs`          - POSIX implementation of file system calls.
   - `memory_fs`         - A file system compiled into memory.
   - `dummy_fs`          - Empty implementation for testing purposes.
   - `expose_fs`         - Adds a new API function that allows the external
                         definition of file system calls.

#### Development flags

   - `memory_limiter`    - Enables memory limiting and monitoring to simulate
                         caps on the available amount of memory. Additionally,
                         a memory monitor tracks memory leaks while testing.  If [`posix_platform`](#platform-selector-flags) is defined, then the Device SDK also logs a stack trace of the initial allocation.
   - `mqtt_localhost`    - Instructs the Device SDK's MQTT client to connect
                         to a localhost MQTT server instead of the [Cloud IoT Core MQTT bridge](https://cloud.google.com/iot/docs/how-tos/mqtt-bridge).
   - `no_certverify`     - Disables TLS certificate verification of the
                         service's identifying cert to reduce security. For development purposes only.
   - `tls_bsp`           - Instructs the Device SDK's MQTT client to use
                         third-party TLS 1.2 implementations to encrypt data before sending it over network sockets.
   - `tls_socket`        - Counterpart of `tls_bsp`. Prevents the MQTT client
                         from including a TLS layer that invokes a TLS BSP. This increases network security. Note that the [Cloud IoT Core MQTT bridge](https://cloud.google.com/iot/docs/how-tos/mqtt-bridge) will not accept connections without TLS.

#### Platform selector flag

   - posix_platform    - Selects the implementation for non-BSP time and
                       memory solutions

Platform configurations configure the build system to include critical section implementations for invoking callbacks on new threads.

For best results, define `posix_platform` and omit `threading` from your `CONFIG` options when building for custom platforms. `threading` is currently omitted by default.

For more information about threadsafe callback support, see the user guide in `doc/user_guide.md`.

## Example applications

Application binaries and sources are in the `examples/` directory.

These examples use the Device SDK to connect to Cloud IoT Core, subscribe to Cloud Pub/Sub topics, publish information to Cloud IoT Core, and receive data from Cloud IoT Core.

The examples' source code shows you how to initialize and use the Device SDK's C API.  For more information about the C API, consult the comments in the [example code](https://github.com/GoogleCloudPlatform/iot-device-sdk-embedded-c/examples), [`README.md`](../README.md), [`doc/user_guide.md`](user_guide.md), and the [API reference](https://googlecloudplatform.github.io/iot-device-sdk-embedded-c/api/html/index.html).

To build the examples follow the [instructions in the main `README.md`](https://github.com/GoogleCloudPlatform/iot-device-sdk-embedded-c/#building-the-examples).

# Porting

This section describes how to customize the BSP. After adapting the `make` environment to your toolchain, customize the BSP for your device to port the Device SDK to a new platform.

## Board Support Package (BSP) source locations

The Board Support Package (BSP) is the well-defined set of functions that the Device SDK invokes in order to interact with a platform's specific networking, file IO, [TLS](https://en.wikipedia.org/wiki/Transport_Layer_Security), memory management, random number generator, crypto, and time SDKs.

The BSP implementation is in the `src/bsp` directory. When porting the Device SDK to your platform SDK, ignore the MQTT codec and the non-blocking/asynchronous engine that appear elsewhere in the source.

BSP function declarations are in the `include/bsp` directory. For generated function documentation, see the Device SDK [BSP reference](https://googlecloudplatform.github.io/iot-device-sdk-embedded-c/bsp/html/index.html).

BSP functions are organized into logical subsystems as follows.

### BSP modules

- BSP IO NET: networking stack integration (`include/bsp/iotc_bsp_io_net.h`)
- BSP TLS: Transport Layer Security integration (`include/bsp/iotc_bsp_tls.h`)
- BSP MEM: heap memory management (`include/bsp/iotc_bsp_mem.h`)
- BSP RNG: random number generator (`include/bsp/iotc_bsp_rng.h`)
- BSP CRYPTO: ECC, SHA256, Base64 (`include/bsp/iotc_bsp_crypto.h`)
- BSP TIME: time function (`include/bsp/iotc_bsp_time.h`)

### BSP reference implementations

Reference function implementations of POSIX BSPs and supported TLS libraries are provided in the `src/bsp/platform`, `src/bsp/tls`, and `src/bsp/crypto` directories.

The `platform` directory contains reference BSP implementations for networking, file IO, memory management, random number generation, and time functionality. The `tls` directory contains reference BSP implementations for cryptographic functionality and TLS support via the mbedTLS or wolfSSL libraries, which supply secure TLS v1.2 connections over TCP/IP for embedded device footprints.

### POSIX BSP

A POSIX platform implementation is provided for your reference in the `src/bsp/platforms/posix` directory.

### Custom BSP

If your target platform is not POSIX compliant (most IoT embedded devices are not POSIX compliant), complete the following steps.

1. Create a new implementation in a new directory <code>src/bsp/platform/<i><b>NEW_PLATFORM_NAME</b></i></code>. For reference, see the BSP headers and [generated documentation](`doc/doxygen/bsp/html/index.html`).
2. Call `make` with the parameter <code>IOTC_BSP_PLATFORM=<i><b>NEW_PLATFORM_NAME</b></i></code>.

### TLS BSP

 `mbedTLS` and `wolfSSL` TLS BSP implementations are in the `src/bsp/tls/mbedtls` and `src/bsp/tls/wolfssl/` directories, respectively, with Key signature functionaly leveraged in `src/bsp/crypto/mbedtls` and `src/bsp/crypto/wolfssl`, respectively. The BSP TLS implementations are in `src/bsp/tls/mbedtls/iotc_bsp_tls_mbedtls.c` and `src/bsp/tls/wolfssl/iotc_bsp_tls_wolfssl.c`. The corresponding cryptographic implementations (for JWT signing) are in `src/bsp/crypto/mbedtls/iotc_bsp_crypto.c` and `src/bsp/crypto/wolfssl/iotc_bsp_crypto.c`.

### Custom TLS BSP

If neither mbedTLS nor wolfSSL fits your target platform or licensing requirements, you can configure the build system to use other TLS BSP implementations.

Make sure the custom TLS BSP meets the TLS Implementation Requirements defined in `doc/user_guide.md`.

Complete the following steps to create a new BSP implementation for TLS.

1. Implement the BSP TLS API functions in `include/bsp/iotc_bsp_tls.h`. Refer to at least one of the mbedTLS or wolfSSL implementations throughout this process to guide your development.
2. Create a directory <code>src/bsp/tls/<i><b>NEW_TLS_LIBRARY_NAME</b></i></code> and a directory <code>src/bsp/crypto/<i><b>NEW_TLS_LIBRARY_NAME</b></i></code> to store the new TLS and crypto implementation.
3. Copy the file `make/mt-config/mt-tls-mbedtls.mk` to <code>make/mt-config/mt-tls-<i><b>NEW_TLS_LIBRARY_NAME</b></i>.mk</code>. 
4. Redefine the path variables in <code>make/mt-config/mt-tls-<i><b>NEW_TLS_LIBRARY_NAME</b></i>.mk</code> according to the new TLS library's internal directory structure, relative to the base directory of the main makefile:
   - IOTC_TLS_LIB_INC_DIR is added to the toolchain include path when the Device SDK compiles.
   - IOTC_TLS_LIB_DIR is added to the TLS library when linking it to a client application on a new platform.
   - IOTC_TLS_LIB_NAME is added as a `-l` library name parameter when linking the TLS library to a client application on a new platform.
   - IOTC_TLS_LIB_DEP is a path to a library as a makefile dependency. The build will fail and warn you if this file is missing. See `make/mt-config/mt-tls.mk` for more information.
   - IOTC_CONFIG_FLAGS (optional) augments the subsequent compile-time preprocessor flags. Append to this value only with the makefile += operator.
5. Call `make` with the parameter <code>IOTC_BSP_TLS=<b>NEW_TLS_LIBRARY_NAME</b></code>.
   - The library name must match the directory name you created under `src/bsp/tls` above.  Additionally, this command doesn't build your TLS library directly. Instead, it builds your TLS BSP to work with that library.
   - To build your custom library and the mbedTLS and wolfSSL libraries, refer to `make/mt-config/mt-tls.mk` and the shell scripts in `res/tls`.

## BSP code porting process

After building the SDK and TLS static libraries, follow the instructions below to link the libraries to a client application on a new platform.

The existing platform config files `make/mt-os/mt-linux.mk` and `make/mt-os/mt-osx.mk` demonstrate how to link the libraries to Linux and OS X, respectively. 

1. Create a new file <code>make/mt-os/mt-<i><b>NEW_PLATFORM_NAME</b></i>.mk</code>.
   - Include the common `mt` file.

          include make/mt-os/mt-os-common.mk

   - Define CC and AR. Provide the full path to your toolchain's compiler and archiver executables. For example, the following definitions are for a [FreeRTOS](https://github.com/GoogleCloudPlatform/iot-device-sdk-embedded-c/tree/development/examples/freertos_linux/Linux_gcc_gcp_iot) build.

           CC = ~/downloads/FreeRTOS_sdk/FreeRTOS_tools/bin/armcl
           AR = ~/downloads/FreeRTOS_sdk/tools/bin/armar

   - Add compiler flags by appending them to variable IOTC_COMMON_COMPILER_FLAGS. For example:

           IOTC_COMMON_COMPILER_FLAGS += -I~/downloads/FreeRTOS_sdk/include
           IOTC_COMMON_COMPILER_FLAGS += -DFreeRTOS

   - Set flags specific to the C and C++ compiler in IOTC_C_FLAGS and IOTC_CXX_FLAGS, respectively.

   - Add archiver flags by appending them to the IOTC_ARFLAG variable.

           IOTC_ARFLAGS := r $(XI)

2. Add details about the new platform to `make/mt-config/mt-presets.mk`.

   - Define the Device SDK feature and target configurations.

           CONFIG_<i><b>NEW_PLATFORM_NAME</b></i>_MIN = memory_fs-tls_bsp
           TARGET_<i><b>NEW_PLATFORM_NAME</b></i>_REL = -static-release

   - Define make system variables for <code>PRESET <i><b>NEW_PLATFORM_NAME</b></i></code>

           else ifeq ($(PRESET), <i><b>NEW_PLATFORM_NAME</b></i>)
               CONFIG = $(CONFIG_<i><b>NEW_PLATFORM_NAME</b></i>_MIN)
               TARGET = $(TARGET_<i><b>NEW_PLATFORM_NAME</b></i>_REL)
               IOTC_BSP_PLATFORM = <i><b>NEW_PLATFORM_NAME</b></i>
               IOTC_TARGET_PLATFORM = <i><b>NEW_PLATFORM_NAME</b></i>

3. Extend `make/mt-os/mt-os.mk` to check <code>TARGET = $(TARGET_<i><b>NEW_PLATFORM_NAME</b></i>_REL</code> and then include the <code>make/mt-os/mt-<b>NEW_PLATFORM_NAME</b>.mk</code> config file.

       IOTC_CONST_PLATFORM_NP4000 := <b>NEW_PLATFORM_NAME</b>

       ifneq (,$(findstring $(IOTC_CONST_PLATFORM_<b>NEW_PLATFORM_NAME</b>),$(TARGET)))
           IOTC_CONST_PLATFORM_CURRENT := $(IOTC_CONST_PLATFORM_<b>NEW_PLATFORM_NAME</b>)
       endif

4. Provide BSP implementations for all [modules](#BSP_modules).

    - Create the following source files.
        - <code>src/bsp/<i><b>NEW_PLATFORM_NAME</b></i>/iotc_bsp_io_fs_<i><b>NEW_PLATFORM_NAME</b></i>.c</code>
        - <code>src/bsp/<i><b>NEW_PLATFORM_NAME</b></i>/iotc_bsp_io_net_<i><b>NEW_PLATFORM_NAME</b></i>.c</code>
        - <code>src/bsp/<i><b>NEW_PLATFORM_NAME</b></i>/iotc_bsp_mem_<i><b>NEW_PLATFORM_NAME</b></i>.c</code>
        - <code>src/bsp/<i><b>NEW_PLATFORM_NAME</b></i>/iotc_bsp_rng_<i><b>NEW_PLATFORM_NAME</b></i>.c</code>
        - <code>src/bsp/<i><b>NEW_PLATFORM_NAME</b></i>/iotc_bsp_time_<i><b>NEW_PLATFORM_NAME</b></i>.c</code>
  
   - In each these source files, define the functions declared in the corresponding Device SDK BSP headers.
       - File storage: `include/bsp/iotc_bsp_io_fs.h`
       - Networking: `include/bsp/iotc_bsp_io_net.h`
       - Memory allocation: `include/bsp/iotc_bsp_mem.h`
       - Time: `include/bsp/iotc_bsp_time.h`
       - Random number generation: `include/bsp/iotc_bsp_rng.h`

5. Select a TLS implementation.

   - The default library is `mbedTLS`.
   - To select the `wolfSSL` library, execute the following command.

     <pre>
     make PRESET=<i><b>NEW_PLATFORM_NAME</b></i> IOTC_BSP_TLS=wolfSSL
     </pre>
           
  - To use a custom TLS implementation, execute the following command.

    <pre>
    make PRESET=<i><b>NEW_PLATFORM_NAME</b></i> IOTC_BSP_TLS=<i><b>NEW_TLS_LIBRARY_NAME</b></i>
    </pre>

  - For more information on custom TLS implementations, see [Custom TLS BSP](#custom-tls-bsp).

6. Build the device SDK for the new platform.

<pre>
make PRESET=<i><b>NEW_PLATFORM_NAME</b></i>
</pre>

After linking the libraries to a new platform, the BSP is customized to your device and the porting process is complete.

### Troubleshooting

Try the suggestions in this section to debug issues.

#### Log makefile variables to the console

The `MAKEFILE_DEBUG` variable logs makefile variables to the console.  You can compare the logs for your own build configuration with the logs for a POSIX default build.

To run this command with your own build configuration:

<pre>
make PRESET=<i><b>NEW_PLATFORM_NAME</b></i> MAKEFILE_DEBUG=1
</pre>

To run it for a POSIX default build:

>make MAKEFILE_DEUBG=1

#### Clean your build
The following command removes local `.o`, `.d` or `.lib` files generated during the previous `make` step.

<pre>
make PRESET=<i><b>NEW_PLATFORM_NAME</b></i> clean
</pre>

We recommend cleaning your build when creating one from scratch.

#### Log all toolchain commands
The following command logs the commands that `make` would execute. It only logs the commands; it doesn't execute them.

<pre>
make PRESET=<i><b>NEW_PLATFORM_NAME</b></i> -n
</pre>

Then, you can manually run each `make` command in the output to determine the part of the build process that's causing an issue.

# Additional resources
For more information about the Device SDK, see these other documents in the [GitHub repository](https://github.com/googlecloudplatform/iot-device-sdk-embedded-c):

- [`README.md`](https://github.com/GoogleCloudPlatform/iot-device-sdk-embedded-c) provides general information about the file structure of the source, how to build on Linux, and a general overview of security.

- [`doc/user_guide.md`](https://github.com/GoogleCloudPlatform/iot-device-sdk-embedded-c/blob/development/doc/user_guide.md) provides an in-depth description of the Device SDK design and features, including MQTT logic, the event system, backoff logic, and platform security requirements.

- [`doc/doxygen/api`](https://googlecloudplatform.github.io/iot-device-sdk-embedded-c/api/html/index.html) contains the function specifications for the Device SDK application-level API.

- [`doc/doxygen/bsp`](https://googlecloudplatform.github.io/iot-device-sdk-embedded-c/bsp/html/index.html) contains the declarations and documentation for the abstracted Board Support Package (BSP) functions to port the Device SDK to new platforms.
