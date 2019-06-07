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

echo
echo "--------------------------"
echo "| mbedTLS LICENSE Notice |"
echo "--------------------------"
echo
echo "Unless specifically indicated otherwise in a file, files are licensed"
echo "under the Apache 2.0 license, as can be found in: apache-2.0.txt"
echo
echo "the apache-2.0.txt file can be accessed here:"
echo "   https://www.apache.org/licenses/LICENSE-2.0"
echo
echo "For more information about the mbedTLS license, please check the LICENSE"
echo "file of the mbedTLS directory after this auto-checkout procedure"
echo "completes, or check their github repository at the following address:"
echo "  https://github.com/ARMmbed/mbedtls"
echo
read -p "Continue to auto-download and build mbedTLS? [Y/N] " -n 1 -r
echo
if [[ ! $REPLY =~ ^[Yy]$ ]]
then
    echo
    echo "exiting build."
    echo
    exit 1
fi

mkdir -p ../../third_party/tls
cd ../../third_party/tls

git clone -b mbedtls-2.12.0 https://github.com/ARMmbed/mbedtls.git
cd mbedtls
# "-O2" comes from mbedtls/library/Makefile "CFLAGS ?= -O2" define
make CFLAGS="-O2 -DMBEDTLS_PLATFORM_MEMORY $1"
echo "mbedTLS Build Complete."

