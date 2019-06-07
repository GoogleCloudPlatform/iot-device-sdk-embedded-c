#!/bin/bash
# Copyright 2018-2019 Google LLC
#
# This is part of the Google Cloud IoT Device SDK for Embedded C.
# It is licensed under the BSD 3-Clause license; you may not use this file
# except in compliance with the License.
#
# You may obtain a copy of the License at:
#  https://opensource.org/licenses/BSD-3-Clause
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

supported_wolfssl_version="v3.15.3-stable"

# Helper function to exit with error in a consistent manner.
exit_with_error() {
  echo
  echo " Exiting Build due to Error."
  echo
  exit -1
}

# Helper function to see if a binary exists
# in the default path of the environment.
check_exists () {
  if hash $1 2>/dev/null;
    then
      true
    else
      false
  fi
}

# Prints a message saying that the environment is missing
# one or more build executable depdendencies that are
# required to build wolfSSL.
print_missing_tools_header() {
  echo
  echo " !! Missing Tools Error !!"
  echo " WolfSSL's build system requires autotools to be installed on your host system."
  echo " Please run the following packages before proceeding:"
}

# Prints installation instructions for missing OSX components
missing_installation_osx() {
  print_missing_tools_header
  echo "    brew update"
  if ! check_exists autoconf ; then
  echo "    brew install autoconf"
  fi
  if ! check_exists automake ; then
  echo "    brew install automake"
  fi
  if ! check_exists libtoolize 2>/dev/null; then
  echo "    brew install libtool"
  fi
}

# Prints installation instructions for missing Ubuntu components.
missing_installation_ubuntu() {
  print_missing_tools_header
  if ! check_exists autoconf ; then
  echo "    sudo apt-get install autoconf"
  fi
  if ! check_exists automake ; then
  echo "    sudo apt-get install autotools-dev"
  fi
  if ! check_exists libtoolize 2>/dev/null; then
  echo "    sudo apt-get install libtool"
  fi
}

missing_installation_generic() {
  echo " The build enviornment is missing the suite of autotools "
  echo " required to build wolfssl, specifically autoconf, automake"
  echo " and libtool."
  echo
  echo " Please install these tools before attempting to"
  echo " build again."
  echo
  echo " NOTE: it has been detected that you are using a non-supported"
  echo " host enviornment (not Ubuntu or OSX). You may encounter"
  echo " issues when building this library or when building WolfSSL."
  echo
}

check_for_build_binaries() {
  error_detected=false;
  if ! check_exists autoconf ; then
    error_detected=true
  elif ! check_exists automake ; then
    error_detected=true
  elif ! check_exists libtoolize ; then
    error_detected=true
  fi

  if ! $error_detected ; then
    return
  fi

  if [[ "$OSTYPE" == "linux-gnu" ]]; then
    missing_installation_ubuntu
  elif
    [[ "$OSTYPE" == "darwin"* ]]; then
    missing_installation_osx
  else
    missing_installation_generic
  fi

  exit_with_error
}

missing_wolfssl_sources() {
    echo
    echo " Error, missing third_party/tls/wolfssl directory or source files."
    echo
    echo " Please visit https://www.wolfssl.com/ or "
    echo " https://github.com/wolfSSL/wolfssl to obtain a copy of wolfSSL's"
    echo " sources and place them in the third_party/tls/wolfssl directory"
    echo " before attempting to build again."
    echo

    exit_with_error
}

check_for_wolfssl() {
    if ! [ -f "wolfssl/configure.ac" ]  ; then
        missing_wolfssl_sources
    fi
}

echo
echo " ******** "
echo " * Attempting to build wolfSSL using res/tls/build_wolfssl.sh."
echo " *"
echo " * NOTE: this library's most recently tested and supported version of"
echo " * wolfSSL is: $supported_wolfssl_version"
echo " *"
echo " * This build process assumes that you have downloaded or git"
echo " * cloned the above version of WolfSSL from its github project and"
echo " * placed the files into the  third_party/tls/wolfssl directory of this"
echo " * repository."
echo " *"
echo " * Please be sure to visit https://www.wolfssl.com/ for more"
echo " * information on wolfSSL, including commercial licensing options."
echo " ******** "
echo
read -p "Are you ready to proceed with the build? [Y/N] " -n 1 -r
if [[ ! $REPLY =~ ^[Yy]$ ]]
then
    echo
    echo "exiting build."
    echo
    exit 1
fi

mkdir -p ../../third_party/tls
cd ../../third_party/tls

echo
echo
echo " Performing sanity checks..."
# Ensure that we have a proper bulid environment
echo " Checking for build dependencies..."
check_for_build_binaries

# Ensure that the directory exists
echo " Checking for existence of wolfSSL sources..."
check_for_wolfssl

echo " Prebuild checks passed!"
echo
echo " Executing build third_party/tls/wolfssl..."
echo

cd wolfssl

(autoreconf --install && ./configure `cat ../../../res/tls/wolfssl.conf` && make )
echo " Build script complete."
