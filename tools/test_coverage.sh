# Copyright 2018-2020 Google LLC
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

#!/bin/bash
echo
echo
echo =======================================
echo = Building Test Coverage HTML reports =
echo =======================================
echo
echo Checking for installed dependencies...
echo
if hash gcov 2>/dev/null; then
  echo gcov detected in environment.
else
  echo  "  **ERROR**"
  echo  "    gcov is required to create test coverage report."
  echo  "    gcov is provided as part of the gnu gcc toolchain on ubuntu."
  echo
  exit -1
fi

echo
if hash lcov 2>/dev/null; then
  echo locv detected in environment.
else
  echo "  **ERROR**"
  echo "    lcov is required to create test coverage report."
  echo "    try: sudo apt-get install lcov"
  echo
  exit -1
fi

echo
echo cleaning...
make clean
rm -rf test_coverage
mkdir test_coverage

echo
echo building and running unit tests...
make IOTC_COMMON_COMPILER_FLAGS="-fprofile-arcs -ftest-coverage" utests -j

echo
echo generating unit tests report...
lcov --capture --directory . --output-file test_coverage/utest_coverage.info
lcov --remove test_coverage/utest_coverage.info '*/src/import/*' '*/src/tests/*' '*/src/bsp/*' -o test_coverage/utest_coverage.info
genhtml test_coverage/utest_coverage.info --output-directory test_coverage/utests
rm -f test_coverage/utest_coverage.info

echo
echo cleaning intermediate files...
rm -f iotc_utests.gcda
rm -f iotc_utests.gcno
rm -f iotc_libiotc_driver.gcno

echo
echo cleaning build...
make clean

echo
echo building and running integration testsxi
make IOTC_COMMON_COMPILER_FLAGS="-fprofile-arcs -ftest-coverage" itests -j

echo
echo generating integration test report...
lcov --capture --directory . --output-file test_coverage/itest_coverage.info
lcov --remove test_coverage/itest_coverage.info '*/src/import/*' '*/tests/*' '*/src/bsp/*' -o test_coverage/itest_coverage.info
genhtml test_coverage/itest_coverage.info --output-directory test_coverage/itests
rm -f test_coverage/itest_coverage.info

echo
echo cleaning intermediate files...
rm -f iotc_utests.gcda
rm -f iotc_utests.gcno
rm -f iotc_libiotc_driver.gcno

# linking .o files built with profile-arcs and test-coverage in a subsequent
# standard build will create link errors, so clean them up for convenience.
echo
echo cleaning build...
make clean

echo Done!
