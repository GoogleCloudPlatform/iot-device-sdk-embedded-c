# Copyright 2018 Google LLC
#
# This is part of the Google Cloud IoT Device SDK for Embedded C,
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


# Batch Build Tool for IoTC Client Examples
#
# These Examples are built against a pre-built IoTC Client library.
#
# The pre-built IoTC Client library paths are in IOTC_CLIENT_INC_PATH
# and IOTC_CLIENT_LIB_PATH in rules.mk. Their default values reflect the
# original relative location of examples to IoTC Client. If this
# relative location is altered the varables shall be adjusted either
# directly in rules.mk or in commandline like
# 'make IOTC_CLIENT_INC_PATH=new/path/include IOTC_CLIENT_LIB_PATH=new/path/obj/osx'.
#
# Examples can be batch-built with 'make' in this directory or one-by-one
# 'make' in each example directory.
#
# 	Targets:
#
# 		all: 	builds all examples
#		clean: 	cleans all examples
#
#		[EXAMPLE_NAME]: builds only the specified example
#						'make mqtt_logic_example'
#
# Result binaries are under [EXAMPLE_NAME]/bin
#
# If you wish to create your own application based on a IoTC example source and
# makefile structure, then the following steps are suggested:
#		- create a new subdirectory under examples: my_new_example
#		- copy mqtt_logic_example/Makefile to my_new_example/Makefile and initialize
#		  variable IOTC_EXAMPLE_NAME with 'my_new_example'
#		- put your code into my_new_exapmle/src/my_new_example.c
#		- add the new example to the examples list: IOTC_EXAMPLES_ALL += my_new_example
#		- execute 'make my_new_example'
#
# It should be 'easy' to alter these makefiles to cross-compile. This requires the
# following modification in rules.mk:
# 		- available cross-compiled IoTC Client library
#		- changing the compiler and linker variables: CC and AR
# 		- adjusting linked libraries

MD=@

IOTC_EXAMPLES_ALL := iot_core_mqtt_client

all: $(IOTC_EXAMPLES_ALL)

.PHONY : $(IOTC_EXAMPLES_ALL)

$(IOTC_EXAMPLES_ALL) :
	$(MD) $(MAKE) -C $(CURDIR)/$@ all

clean:
	@for M in $(IOTC_EXAMPLES_ALL); do \
		$(MAKE) -C $(CURDIR)/$$M clean; \
	done;
