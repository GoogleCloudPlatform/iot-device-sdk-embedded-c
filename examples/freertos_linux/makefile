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

IOTC_TARGET_PLATFORM := freertos-linux

include ../common/rules.mk

FREERTOS_DIR=$(IOTC_CLIENT_PATH)/third_party/FreeRTOSv10.1.1/FreeRTOS

FREERTOS_SRC_DIR=${FREERTOS_DIR}/Source
FREERTOS_INC_DIR=${FREERTOS_DIR}/Source/include

PORTABLE_SRC_DIR=$(IOTC_CLIENT_PATH)/third_party/freertos-addons/Linux/portable/GCC/Linux
PORTABLE_SRC_MEM_MANG_DIR=${FREERTOS_DIR}/Source/portable/MemMang

IOTC_INC_DIR=../../include

CC = gcc

CFLAGS += -Wall -Werror -Wextra -Wpedantic -pthread -O0 -g
LDFLAGS += -Wall -Werror -Wextra -Wpedantic -pthread -O0 -g



INCLUDE_DIRS +=	-I. \
				-I${FREERTOS_INC_DIR} \
				-I${PORTABLE_SRC_DIR} \
				-I${IOTC_INC_DIR} \


VPATH += ${FREERTOS_SRC_DIR} \
		 ${PORTABLE_SRC_DIR} \
		 ${PORTABLE_SRC_MEM_MANG_DIR} \


TARGET = freertos_linux

default: all

all: ${TARGET}

SRC_COMMON = \
      	  ../common/src/commandline.c \
	  ../common/src/example_init.c

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

OBJ_COMMON = ${SRC_COMMON:.c=.o}
OBJ_COMMON := $(subst ../common/src/,,$(OBJ_COMMON))
${OBJ_COMMON}: %.o: ../common/src/%.c
	${CC} ${INCLUDE_DIRS} ${CFLAGS} -c $< -o $@

OBJ = ${SRC:.c=.o}
${OBJ}: %.o: %.c
	${CC} ${INCLUDE_DIRS} ${CFLAGS} -c $< -o $@

FREERTOS_OJB = ${FREERTOS_SRC:.c=.o}
${FREERTOS_OJB}: %.o: %.c
	${CC} -I. -I${FREERTOS_INC_DIR} -I${PORTABLE_SRC_DIR} ${CFLAGS} -c $< -o $@


$(TARGET):	${FREERTOS_OJB} ${OBJ} ${OBJ_COMMON}
	${CC} ${LDFLAGS} ${LD_EXE_FLAGS} -o $@ ${FREERTOS_OJB} ${OBJ} ${OBJ_COMMON} $(IOTC_FLAGS_LINKER)
	@cp $(IOTC_CLIENT_ROOTCA_LIST) $(dir $@)


.PHONY : clean
clean:
	-rm ${OBJ}
	-rm *~
	-rm ${TARGET}
	-rm *.o
	-rm $(notdir ${IOTC_CLIENT_ROOTCA_LIST})


