# Copyright 2018 Google LLC
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

#
# DDB, NOTE: this was copied from the previous Makefile configuration's avr make target.  I'm not sure how this works
# nor how I should really be testing it so I'm putting it here for now.
#

AVRTEST_SRCDIR ?= ../import/avrtest/
AVRTEST_CFLAGS += -I$(AVRTEST_SRCDIR)
AVRTEST_CFLAGS += -W -Wall -Wno-unused-parameter -Wno-attributes

IOTC_INCLUDE_FLAGS += -I$(AVRTEST_SRCDIR)

IOTC_TEST_ELF := $(IOTC_BINDIR)/libiotc_avr_unit_test.elf
IOTC_TEST_SIM := $(IOTC_BINDIR)/avrtest

IOTC_UTEST_SOURCES += $(IOTC_OBJDIR)/avrtest/exit.o
IOTC_TEST_DEPENDS += $(IOTC_TEST_SIM)

$(IOTC_BINDIR)/libiotc_avr_unit_test: $(IOTC_TEST_ELF) $(IOTC_TEST_DEPENDS)
	@-mkdir -p $(dir $@)
	$(IOTC_TEST_SIM) -mmcu=avr6 $(IOTC_TEST_ELF)

$(IOTC_TEST_ELF): $(IOTC_UTEST_SOURCES) $(IOTC_TEST_DEPENDS)
	@-mkdir -p $(dir $@)
	$(CC) $(IOTC_CONFIG_FLAGS) $(IOTC_INCLUDE_FLAGS) $(IOTC_UTEST_SOURCES) -o $@

$(AVRTEST_SRCDIR)/gen-flag-tables: $(AVRTEST_SRCDIR)/gen-flag-tables.c
	gcc $(AVRTEST_CFLAGS) $^ -o $@

$(AVRTEST_SRCDIR)/flag-tables.c: $(AVRTEST_SRCDIR)/gen-flag-tables
	$^ > $@

$(IOTC_TEST_SIM): $(AVRTEST_SRCDIR)/flag-tables.c $(AVRTEST_SRCDIR)/avrtest.c
	@-mkdir -p $(dir $@)
	gcc $(AVRTEST_CFLAGS) $(AVRTEST_SRCDIR)/avrtest.c -o $@

$(IOTC_OBJDIR)/avrtest/exit.o: $(AVRTEST_SRCDIR)/dejagnuboards/exit.c
	@-mkdir -p $(dir $@)
	avr-gcc $(IOTC_CONFIG_FLAGS) -mmcu=atmega2560 -c $^ -o $@
