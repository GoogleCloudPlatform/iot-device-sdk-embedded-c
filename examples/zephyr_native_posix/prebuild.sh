# Copyright 2019 Google LLC
#
# This is part of the Google Cloud IoT Edge Embedded C Client,
# it is licensed under the BSD 3-Clause license; you may not use this file
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

pip3 install --user pyelftools

mkdir -p build; cd build;

cmake -DBOARD=native_posix ..

make zephyr/include/generated/autoconf.h
make syscall_list_h_target
make syscall_macros_h_target
make kobj_types_h_target
