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

# Targets.
$(IOTC_EXAMPLE_OBJDIR)/%.o : $(IOTC_EXAMPLE_COMMON_SRCDIR)/%.c
	$(info [$(CC)] $@)
	@-mkdir -p $(dir $@)
	$(MD) $(CC) $(IOTC_EXAMPLE_COMPILER_FLAGS) $(IOTC_EXAMPLE_INCLUDE_FLAGS) -c $< -o $@
	$(MD) $(CC) $(IOTC_EXAMPLE_COMPILER_FLAGS) $(IOTC_EXAMPLE_INCLUDE_FLAGS) -MM $< -MT $@ -MF $(@:.o=.d)

$(IOTC_EXAMPLE_OBJDIR)/%.o : $(IOTC_EXAMPLE_SRCDIR)/%.c
	$(info [$(CC)] $@)
	@-mkdir -p $(dir $@)
	$(MD) $(CC) $(IOTC_EXAMPLE_COMPILER_FLAGS) $(IOTC_EXAMPLE_INCLUDE_FLAGS) -c $< -o $@
	$(MD) $(CC) $(IOTC_EXAMPLE_COMPILER_FLAGS) $(IOTC_EXAMPLE_INCLUDE_FLAGS) -MM $< -MT $@ -MF $(@:.o=.d)

$(IOTC_EXAMPLE_BINARY) : $(IOTC_EXAMPLE_OBJS) $(IOTC_EXAMPLE_COMMON_OBJS)
	$(info [$(CC)] $@)
	@-mkdir -p $(dir $@)
	$(MD) $(CC) $(IOTC_EXAMPLE_OBJS) $(IOTC_EXAMPLE_COMMON_OBJS) $(IOTC_EXAMPLE_LINKER_FLAGS) -o $@
