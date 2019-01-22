# Google Cloud IoT Edge Embedded C Client Porting Guide
##### Copyright 2018 Google LLC

## Table of Contents
1. [Introduction](#introduction)
2. [Building](#building)
4. [Porting](#porting)
6. [Additional Resources](#additional-resources)

# Introduction

To port the Google Cloud IoT Edge Embedded C Client to new platforms, you'll need to:

* Adapt the make build environment to your toolchain
* Write the code for a custom Board Support Package (BSP) implementation to tie client functionality to the device SDK

This document contains two main sections: Building and Porting.

**Building** describes how to configure the provided makefile system to build different library configurations on a host/target POSIX system. This section will help you understand the build system before you attempt to cross-compile to an embedded target.

**Porting** describes the steps you can take to implement a version of the Board Support Package (BSP) to run the client on your target embedded device. This section also walks you through defining the make variables required to build a client static library for your target.

## Audience and scope
This document is intended for developers of embedded devices who have access to the source code of the [Google Cloud IoT Edge Embedded C Client](https://github.com/googlecloudplatform/iot-edge-sdk-embedded-c) and want to run it on their custom device SDK.

This document provides details of the C Client's build steps, build configuration flags, and the location of the functions needed to port the client to new platforms.

For general information about the Embedded C Client's features and application usage, see the Google Cloud IoT Edge Embedded C Client User Guide in `docs/user_guide.md` within the client repository.


## Notes about the codebase

The Embedded C Client has been designed to accommodate a variety of IoT devices. Default host/target support is provided for POSIX systems (tested on Ubuntu Linux), and the client has also been tested on RTOS and no-OS devices.

Most of the codebase is cross-platform and resides in the `src/libiotc/` directory. Platform-specific code is in the `src/bsp/` directory.

The codebase is built against C99 standards.

# Building

We recommend building the Embedded C Client for Linux before attempting to cross-compile to another target platform. The process can serve as an introduction to the basic architecture, tools, and building steps of the Embedded C Client source. This basic knowledge will help you later do a full cross-compiled port of the software to an embedded device.

## Build system: make environment

Note: this document assumes you're familiar with the default build process as described in the `README.md` in the root directory of this repository.

The Embedded C Client uses `make` to build its source files. The main makefile is in the root directory of the repository.

More make-related files (.mk files) are included by the main makefile depending on the build configuration.  These supporting files alter the list of compiled source files, toolchain command line arguments, and so on. Three main makefile flags define the configuration:  TARGET, CONFIG and IOTC_BSP_TLS.

* **TARGET** determines which target platform you're compiling for. This affects toolchain flags, toolchain and file pathing, and the BSP sources you're attempting to compile.  If not specified, this flag defaults to **TARGET=linux-static-release** or **TARGET=osx-static-release**, depending your host system.  Note that currently OS X builds are unsupported.

* **CONFIG** determines which Cloud IoT Edge Embedded C software modules the system will compile into the library.  Potential modules include components like the memory limiter, the use of TLS BSP, debug logging, stack tracing on reported memory leaks, etc. This value defaults to **CONFIG=posix_fs-posix_platform-tls_bsp-memory_limiter** for your local POSIX machine development purposes.
  * Specific CONFIG options are described in the section **CONFIG and TARGET Parameters** below.

* **IOTC_BSP_TLS** determines which Transport Layer Security (TLS) BSP implementation the makefile will attempt to compile.  The selected BSP interfaces with your desired embedded TLS library to encrypt data sent from the client over the network socket.  If you do not define this explicitly on the make command line, the default **IOTC_BSP_TLS=mbedtls** will be used.  The other out-of-the-box alternative is **IOTC_BSP_TLS=wolfssl**. Both of these options will attempt to configure and build the third-party TLS library sources in the `third_party/tls/mbedtls` or `third_party/tls/wolfssl`, respectively, and the build system will prompt the user with instructions on how to populate these directories with the required TLS library sources.
  * Note: The source for the third-party libraries are not included in the repo by default. Our make command will attempt to download mbedtls automatically and configure it for use with this client.  For wolfSSL, no automatic download script is provided, but instructions on how and where to download the wolfSSL sources will be provided to you when you run make.
  * If you need to define your own BSP implementation, see the **TLS BSP** section later in this document.
  * If you're using a hardware TLS option instead of a software one, the best practice is to compile without a TLS BSP. Instead, invoke the device SDK's secure socket API directly from the IoTC Client's Networking BSP. To build with a TLS BSP:
    * Do not define IOTC_BSP_TLS on the make command line
    * Change the `tls_bsp` config parameter to`tls_socket`.  For example:  `make CONFIG=posix_fs-posix_platform-tls_socket-memory_limiter`

For specific parameters for CONFIG and TARGET, see the **CONFIG and TARGET parameters** section below.

## Build system: IDE builds

Although the make build system is suitable for Linux/Unix builds, some embedded SDKs and toolchains may not support building via `make`. Often these SDKs supply their own Integrated Development Environments (IDEs).

It may be helpful to create a native IoT Edge Embedded C Client build on POSIX before attempting to import the client's source into an IDE.  That way you can see the build process working locally and providing the following useful information:

- A list of the source files that were compiled (.c)
- A list of the preprocessor definitions that were required for the build CONFIG you selected (-D)
- The flags passed to the compiler/toolchain
- The header file include paths that were used (-I)
- the link paths that were used to link a binary (-L)
- the linker library names that were used (-l)

You should be able to use this information in your IDE's preprocessor environment/build settings.  To get a better understanding of the build environment, you can run each of the following commands individually:

### make MAKEFILE_DEBUG=1
Logs all makefile variables and their corresponding values.

### make MD=
On a clean build, logs all of the commands, executables, and parameters that are produced when attempting to build.

### make -n
Like make MD= but does not actually execute the logged commands.


## CONFIG and TARGET parameters

The CONFIG and TARGET variables are defined on the command line. For additional general information about CONFIG and TARGET, see the section **Build system: make environment**.

Parameter values consist of a multi-component, single-string listing of module names.  Spaces are not permitted; delineate options with the minus sign or hyphen character '-'.

If you do not set the TARGET or CONFIG makefile variable, default values are used. These are logged to the command line for your reference.

The following values can appear in any order. Flags are separated by the - character.

### TARGET flags

A typical TARGET flag looks like this:

    make TARGET=linux-static-debug

It consists of a platform flag, an output library flag, and a Build Type Flag, as described below:

#### Platform flag

   - `[ linux | osx | arm-linux ]`   - Sets the target platform.  `arm-linux`
                                     is used by the continuous integration
                                     service to smoke-test cross-compiled
                                     builds.  osx is currently unsupported.

#### Output library type

   - `[ static | dynamic ]`          - Sets the output library type.

#### Build Type

   - `[ debug | release ]`          - Sets the build type.  The `debug`
                                     option passes the standard toolchain
                                     debug symbols to the compiler. It also
                                     provide runtime debug output from the
                                     library by adding the IOTC_DEBUG_OUTPUT=1
                                     preprocessor definition. To suppress this, add
                                     IOTC_DEBUG_OUTPUT=0 to your make
                                     command line.

### CONFIG flags

A typical CONFIG flag looks like this:

    make CONFIG=posix_fs-posix_platform-tls_bsp-memory_limiter

#### Optional feature flags

   - `threading`            - POSIX only. Causes pub, sub, and connection
                            callbacks to be called on separate threads. If not set, application callbacks are
                            called on the sole main thread of the Cloud IoT Edge
                            Embedded C Client.

#### File system flags

Used for reading public root CAs for service authentication during the TLS handshaking process.
   - `posix_fs`          - POSIX implementation of file system calls
   - `memory_fs`         - A file system compiled into memory
   - `dummy_fs`          - Empty implementation for testing purposes
   - `expose_fs`         - Adds a new API function that allows the external
                         definition of file system calls.

#### Development flags

   - `memory_limiter`    - Enables memory limiting and monitoring. The purpose of this feature
                         is to aid the development process by simulating a cap
                         on the available amount of memory. Additionally, a
                         memory monitor is employed for tracking and hunting
                         down memory leaks while testing.  When a leak occurs,
                         a stack trace of the initial allocation is logged
                         if posix_platform was also defined.
   - `mqtt_localhost`    - Instructs the client to connect to a localhost MQTT server
                         instead of the Cloud IoT Core MQTT bridge. This may
                         be helpful if testing with a local MQTT broker.
   - `no_certverify`     - Reduces security by disabling TLS certificate
                         verification of the service's identifying cert.
                         For development purposes only.
   - `tls_bsp`           - Instructs the client to use third-party TLS 1.2
                         implementations to encrypt data before it's sent over
                         network sockets.
   - `tls_socket`        - Counterpart of `tls_bsp`: the client will not
                         include a TLS layer that invokes a TLS BSP for
                         network security. This be may helpful in connecting
                         to a local and unsecure mosquitto broker to test, or
                        when working with SDKs in which TLS is handled directly in
                         the socket API. Note that the   Cloud IoT Core MQTT
                         bridge will not accept connections without TLS.

#### Platform selector flags

   - posix_platform    - Selects the implementation for non-BSP time and non-BSP
                         memory solutions

Platform configurations will eventually be deprecated. Currently they configure the build system to include critical section implementations for invoking callbacks on new threads.

For best results, define `posix_platform` and omit `threading` from your CONFIG options when building for custom platforms.  `threading` is currently omitted by default.

For more information about threadsafe callback support, see the Cloud IoT Edge Embedded C Client User Guide: `doc/user_guide.md`.


## Example applications

Application binaries and source can be found in the directory `examples/`.

These examples use the Embedded C Client to connect to Google Cloud IoT Core service, subscribe to topics, publish data, and receive data.  They can be built on POSIX by running `make` in the `examples/` directory.

The source of the examples can help you understand how to initialize and use the C API of the client.  Consult the comments in the example code, as well as the `readme.md`, `doc/user_guide.md`, and API reference in `doc/doxygen/api`.

You must pass Cloud IoT Core device credentials over the command line to the examples.  For more information, execute the example with no arguments and a list of argument options will be returned.

For more information on generating Cloud IoT Core device credentials, see [Create Public/Private Key Pairs](https://cloud.google.com/iot/docs/how-tos/credentials/keys) in the Cloud IoT Core documentation.


# Porting

## Board Support Package (BSP) source locations

The Board Support Package (BSP) is the well-defined set of functions that the Cloud IoT Edge Embedded C Client invokes in order to interact with a platform's specific networking, file IO, [TLS](https://en.wikipedia.org/wiki/Transport_Layer_Security), memory management, random number generator, crypto, and time SDKs.

The BSP implementation resides in the directory `src/bsp`. Use this directory when porting the C Client to your device SDK. You can ignore the MQTT codec and the non-blocking/asynchronous engine that appear elsewhere in the source.

BSP **function declarations** are in the `include/bsp`
directory. For generated function documentation, see `doc/doxygen/bsp/html/index.html`.

BSP functions are organized into logical subsystems as follows:

### BSP modules

- BSP IO NET: Networking Stack Integration (`include/bsp/iotc_bsp_io_net.h`)
- BSP TLS: Transport Layer Security Integration (`include/bsp/iotc_bsp_tls.h`)
- BSP MEM: Heap Memory Management (`include/bsp/iotc_bsp_mem.h`)
- BSP RNG: Random Number Generator (`include/bsp/iotc_bsp_rng.h`)
- BSP CRYPTO: ECC, SHA256, Base64 (`include/bsp/iotc_bsp_crypto.h`)
- BSP TIME: Time Function (`include/bsp/iotc_bsp_time.h`)


### BSP reference implementations

Reference function implementations of POSIX BSPs and supported TLS libraries are provided in two directories: `src/bsp/platform` and `src/bsp/tls`.

The `platform` directory contains reference BSP implementations for networking, file IO, memory management, random number generator, and time functionality. The `tls` directory contains reference BSP implementations for cryptographic functionality and TLS support via the mbedTLS or wolfSSL libraries, which supply secure TLS v1.2 connections over TCP/IP with embedded device footprints in mind.

### POSIX BSP

A POSIX platform implementation is provided for your reference in the directory `src/bsp/platforms/posix`.

### Custom BSP

If your target platform is not POSIX compliant (most IoT embedded devices are not POSIX compliant), use these steps:

- Create a new implementation in a new directory `src/bsp/platform/[NEW_PLATFORM_NAME]` using the BSP headers and generated documentation as a reference.
- Call `make` with the parameter `IOTC_BSP_PLATFORM=[NEW_PLATFORM_NAME]`

### TLS BSP

A reference implementation of a TLS BSP is provided for both `mbedTLS` and `wolfSSL`. See the `src/bsp/tls/mbedtls` and `src/bsp/tls/wolfssl/` directories, respectively.

### Custom TLS BSP

If neither mbedTLS nor wolfSSL fits your target platform or licensing requirements, the build system can be configured to use other TLS BSP implementations.

Before selecting a TLS solution other than those mentioned above, make sure it meets the TLS Implementation Requirements in `doc/user_guide.md`.

To create a new BSP implementation for TLS:

- Implement all of the BSP TLS API functions found in `include/bsp/iotc_bsp_tls.h`. Refer to at least one of the mbedTLS or wolfSSL implementations throughout this process to guide your development.
- Put this implementation in the directory `src/bsp/tls/[NEW_TLS_LIBRARY_NAME]`.
- Copy the file `make/mt-config/mt-tls-mbedtls.mk` to `make/mt-config/mt-tls-[NEW_TLS_LIBRARY_NAME].mk` and set the path variables inside according to the new TLS library's internal directory structure, relative to the base directory of the main makefile:
   - IOTC_TLS_LIB_INC_DIR will be added to the toolchain include path when compiling the Cloud IoT Edge Embedded C Client source.
   - IOTC_TLS_LIB_DIR will be added to the link path during the link step of example applications.
   - IOTC_TLS_LIB_NAME will be added as a `-l` library name parameter during the linking step.
   - IOTC_TLS_LIB_DEP is a path to a library as a makefile dependency. The build will fail and warn you if this file is missing. See `make/mt-config/mt-tls.mk` for more information.
   - IOTC_CONFIG_FLAGS (optional) augment the compile-time preprocessor flags for the rest of your Cloud IoT Edge Embedded C Client source compilation steps. Append to this value only with the makefile += operator.

- Call `make` with the parameter `IOTC_BSP_TLS=[NEW_TLS_LIBRARY_NAME]`.
   - Note that this name must match the directory name you created under `src/bsp/tls` above.  Additionally, this will not build your TLS library directly. Instead, it will build your TLS BSP to work with that library, assuming that the headers are in IOTC_TLS_LIB_INC_DIR and the TLS library is in IOTC_TLS_LIB_DIR, as defined in the previous step.
   - To have the build system build your library as well as the mbedTLS and wolfSSL libraries, use `make/mt-config/mt-tls.mk` and the shell scripts in `res/tls` as examples.



## BSP code porting process

Building a Cloud IoT Edge Embedded C Client consists of building the C Client static library and the TLS static library, and then linking them to the actual client application.

For reference, see existing platform config files like `make/mt-os/mt-linux.mk` and `make/mt-os/mt-osx.mk`.


### Cross-compilation with the `make` Build System

This tutorial uses the platform *NP4000* for illustration purposes.

Your goal is construct the following command to build the Cloud IoT Edge Embedded C Client for the NP4000:

  make PRESET=np4000

To clean up generated files and dependency files, you'll also need to create the following clean command:

  make PRESET=np4000 clean

The next section explains how to create these commands.


### Porting checklist

- [x] Create a new file `make/mt-os/mt-np4000.mk`
   - Include the common `mt` file:

          include make/mt-os/mt-os-common.mk

   - Define CC and AR and provide the full path to your toolchain's compiler and archiver executables. For example:

           CC = ~/downloads/np4000_sdk/np4000_tools/bin/armcl
           AR = ~/downloads/np4000_sdk/tools/bin/armar

   - Add compiler flags by appending them to variable IOTC_COMMON_COMPILER_FLAGS. For example:

           IOTC_COMMON_COMPILER_FLAGS += -I~/downloads/np4000_sdk/include
           IOTC_COMMON_COMPILER_FLAGS += -Dnp4000

   - Flags specific to the C and C++ compiler should be set in IOTC_C_FLAGS and IOTC_CXX_FLAGS respectively.

   - Add archiver flags by appending them to the variable IOTC_ARFLAG:

           IOTC_ARFLAGS := r $(XI)

- [x] Modify `make/mt-config/mt-presets.mk` by adding the following details for your new platform:

   - Define the Cloud IoT Edge Embedded C client feature and target configurations:

           CONFIG_NP4000_MIN = memory_fs-tls_bsp
           TARGET_NP4000_REL = -static-release

   - Define make system variables for PRESET *np4000*

           else ifeq ($(PRESET), np4000)
               CONFIG = $(CONFIG_NP4000_MIN)
               TARGET = $(TARGET_NP4000_REL)
               IOTC_BSP_PLATFORM = np4000
               IOTC_TARGET_PLATFORM = np4000

- [x] Extend the pattern in `make/mt-os/mt-os.mk` to check the TARGET make parameter for `np4000` and include the `make/mt-os/mt-np4000.mk` config file when the np4000 TARGET is found:

       IOTC_CONST_PLATFORM_NP4000 := np4000

       ifneq (,$(findstring $(IOTC_CONST_PLATFORM_NP4000),$(TARGET)))
           IOTC_CONST_PLATFORM_CURRENT := $(IOTC_CONST_PLATFORM_NP4000)
       endif

- [x] Provide BSP implementations for all modules.

    - Create the following source files:
        - `src/bsp/np4000/iotc_bsp_io_fs_np4000.c`
        - `src/bsp/np4000/iotc_bsp_io_net_np4000.c`
        - `src/bsp/np4000/iotc_bsp_mem_np4000.c`
        - `src/bsp/np4000/iotc_bsp_rng_np4000.c`
        - `src/bsp/np4000/iotc_bsp_time_np4000.c`
   - In these files, define the functions declared in the following corresponding Google Cloud IoT Edge Embedded C Client BSP headers:
       - **file storage** ( `include/bsp/iotc_bsp_io_fs.h`)
       - **networking** (`include/bsp/iotc_bsp_io_net.h`)
       - **memory** (`include/bsp/iotc_bsp_mem.h`)
       - **time** (`include/bsp/iotc_bsp_time.h`)
       - **random** (`include/bsp/iotc_bsp_rng.h`)

   **Hint**: To attain a simple successful build, create the files and implement all the BSP API functions with an **empty body**.  You can use files under `src/bsp/platform/dummy` as an example empty starting point. Although these files won't execute properly, they should at least build, and link and then run with errors.

- [x] Select a TLS implementation.
   - The default selection is `mbedTLS`.
   - To select a different TLS library, add

           IOTC_BSP_TLS=wolfSSL

       or

           IOTC_BSP_TLS=myTLSlibrary

       to the make commandline like this:

           make PRESET=np4000 IOTC_BSP_TLS=myTLSlibrary

   - For `mbedTLS` and `wolfSSL`, the BSP TLS implementations are available in  `src/bsp/tls/mbedtls/iotc_bsp_tls_wolfssl.c` and `src/bsp/tls/wolfssl/iotc_bsp_tls_mbedtls.c`, respectively.  The cryptographic functionality (for JWT signing) is in `src/bsp/tls/mbedtls/iotc_bsp_crypto.c` and `src/bsp/tls/wolfssl/iotc_bsp_crypto.c`, respectively.
   - For all other implementations, you'll have to write the code:
       - Create a file `src/bsp/tls/myTLSlibrary/iotc_bsp_tls_myTLSlibrary.c` and implement all functions declared in `include/bsp/iotc_bsp_tls.h`
       - Create a file `src/bsp/tls/myTLSlibrary/iotc_bsp_crypto.c` and implement all the function declared in `includes/bsp/iotc_bsp_crypto.h`
       - Use the  existing mbedTLS and wolfSSL implementations as examples.
   - Create a file `make/mt-config/mt-tls-myTLSlibrary.mk` and add content similar to `make/mt-config/mt-tls-mbedtls.mk` or `make/mt-config/mt-tls-wolfssl.mk`. This file is where you would defined the include directory, the static library directory, the static library file to link against, and the config flags of the custom TLS library, as explained above in the section **Custom TLS BSP**.

- [x] Attempt a build:

       make PRESET=np4000


### Troubleshooting

This section helps you debug any issues.

#### build with MAKEFILE_DEBUG=1

The MAKEFILE_DEBUG variable logs helpful makefile variables to the console.  You may want to compare this information to a similar POSIX build.

To run this command with your own build configuration:

   make PRESET=np4000 MAKEFILE_DEBUG=1

To run it for a POSIX default build:

   make MAKEFILE_DEUBG=1

#### Clean your build
The following command asks make to remove any local .o, .d or .lib files that were generated during the previous make step. This is recommended to create a build from scratch instead of an incremental build:

   make PRESET=np4000 clean


#### Log all toolchain commands
Tells make to output the commands it would execute,  without executing them.  You can then run each one manually to determine what part of the build process is causing an issue (toolchain pathing, missing include path, etc.).

   make PRESET=np4000 -n


# Additional Resources
For more information about the Google Cloud IoT Edge Embedded C Client, see these other documents in the [GitHub repository](https://github.com/googlecloudplatform/iot-edge-sdk-embedded-c):


### README.md

Provides general information about the file structure of the source, how to build on Linux, and a general overview of security.

### doc/user_guide.md

Provides an in-depth description of the client design and features, including MQTT logic, the event system, backoff logic, and platform security requirements.

### doc/doxygen/api

Contains the function specifications for the Google Cloud IoT Edge Embedded C Client application-level API. The functionality and behavior of Connect, Subscribe, and Publish are outlined here.


### doc/doxygen/bsp

Contains the declarations and documentation for the abstracted Board Support Package (BSP) functions you'll need to implement to complete your port.
