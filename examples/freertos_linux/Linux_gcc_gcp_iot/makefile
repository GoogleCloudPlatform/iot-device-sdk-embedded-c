#############################################################################
#
#  Copyright (c) 2017, Michael Becker (michael.f.becker@gmail.com)
#
#  This file is part of the FreeRTOS Add-ons project.
#
#  Source Code:
#  https://github.com/michaelbecker/freertos-addons
#
#  Project Page:
#  http://michaelbecker.github.io/freertos-addons/
#
#  On-line Documentation:
#  http://michaelbecker.github.io/freertos-addons/docs/html/index.html
#
#  Permission is hereby granted, free of charge, to any person obtaining a
#  copy of this software and associated documentation files (the "Software"),
#  to deal in the Software without restriction, including without limitation
#  the rights to use, copy, modify, merge, publish, distribute, sublicense,
#  and/or sell copies of the Software, and to permit persons to whom the
#  Software is furnished to do so,subject to the following conditions:
#
#  + The above copyright notice and this permission notice shall be included
#    in all copies or substantial portions of the Software.
#  + Credit is appreciated, but not required, if you find this project useful
#    enough to include in your application, product, device, etc.
#
#  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
#  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
#  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
#  THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
#  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
#  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
#  DEALINGS IN THE SOFTWARE.
#
#############################################################################

# Copyright 2018-2019 Google LLC
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

CC = gcc

# GCP IoT specific settings.
IOTC_EXAMPLE_NAME := Linux_gcc_gcp_iot
IOTC_EXAMPLE_TARGET_PLATFORM := freertos-linux
IOTC_EXAMPLE_SRCDIR := $(CURDIR)
IOTC_ROOT_DIR := $(CURDIR)/../../../

include ../../rules.mk

# FreeRTOS specific settings.
FREERTOS_DIR=$(IOTC_ROOT_DIR)/third_party/FreeRTOSv10.1.1/FreeRTOS
FREERTOS_SRC_DIR=$(FREERTOS_DIR)/Source
FREERTOS_INC_DIR=$(FREERTOS_DIR)/Source/include
PORTABLE_SRC_DIR=${IOTC_ROOT_DIR}third_party/freertos-addons/Linux/portable/GCC/Linux
PORTABLE_SRC_MEM_MANG_DIR=${FREERTOS_DIR}/Source/portable/MemMang

CFLAGS += -Wall -Werror -Wextra -pthread -O0 -g
LDFLAGS += -Wall -Werror -Wextra -pthread -O0 -g

INCLUDE_DIRS +=	-I. \
				-I${FREERTOS_INC_DIR} \
				-I${PORTABLE_SRC_DIR} \
				${IOTC_EXAMPLE_INCLUDE_FLAGS} \

VPATH += ${FREERTOS_SRC_DIR} \
		 ${PORTABLE_SRC_DIR} \
		 ${PORTABLE_SRC_MEM_MANG_DIR} \

TARGET = Linux_gcc_gcp_iot

default: all

all: ${TARGET}

include ../../targets.mk

SRC = \
	  main.c

FREERTOS_SRC= \
			  event_groups.c \
			  list.c \
			  queue.c \
			  tasks.c \
			  timers.c \
			  port.c \
			  heap_4.c \
			  croutine.c \

OBJ = ${SRC:.c=.o}
${OBJ}: %.o: %.c
	${CC} ${INCLUDE_DIRS} ${CFLAGS} -c $< -o $@

FREERTOS_OBJ = ${FREERTOS_SRC:.c=.o}
${FREERTOS_OBJ}: %.o: %.c
	${CC} -I. -I${FREERTOS_INC_DIR} -I${PORTABLE_SRC_DIR} ${CFLAGS} -c $< -o $@

$(TARGET):	${FREERTOS_OBJ} ${OBJ} ${IOTC_EXAMPLE_COMMON_OBJS}
	${CC} ${LDFLAGS} ${LD_EXE_FLAGS} -o $@ ${FREERTOS_OBJ} ${OBJ} ${IOTC_EXAMPLE_COMMON_OBJS} ${IOTC_EXAMPLE_LINKER_FLAGS}

.PHONY : clean
clean:
	@-rm -f ${OBJ}
	@-rm -f ${TARGET}
	@-rm -f *.o
	@-rm -rf $(IOTC_EXAMPLE_OBJDIR)
	@-rm -rf $(IOTC_EXAMPLE_BINDIR)
