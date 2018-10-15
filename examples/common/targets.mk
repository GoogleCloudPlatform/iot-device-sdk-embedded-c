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

all: $(IOTC_EXAMPLE_BIN)

-include $(IOTC_EXAMPLE_DEPS)

$(IOTC_EXAMPLE_OBJDIR)/common/%.o : $(CURDIR)/../common/src/%.c
	$(info [$(CC)] $@)
	@-mkdir -p $(dir $@)
	$(MD) $(CC) $(IOTC_FLAGS_COMPILER) $(IOTC_FLAGS_INCLUDE) -c $< -o $@
	$(MD) $(CC) $(IOTC_FLAGS_COMPILER) $(IOTC_FLAGS_INCLUDE) -MM $< -MT $@ -MF $(@:.o=.d)

$(IOTC_EXAMPLE_OBJDIR)/%.o : $(IOTC_EXAMPLE_SRCDIR)/%.c
	$(info [$(CC)] $@)
	@-mkdir -p $(dir $@)
	$(MD) $(CC) $(IOTC_FLAGS_COMPILER) $(IOTC_FLAGS_INCLUDE) -c $< -o $@
	$(MD) $(CC) $(IOTC_FLAGS_COMPILER) $(IOTC_FLAGS_INCLUDE) -MM $< -MT $@ -MF $(@:.o=.d)

$(IOTC_EXAMPLE_BIN) : $(IOTC_EXAMPLE_OBJS)
	$(info [$(CC)] $@)
	@-mkdir -p $(dir $@)
	@cp $(IOTC_CLIENT_ROOTCA_LIST) $(dir $@)
	$(MD) $(CC) $(IOTC_EXAMPLE_OBJS) $(IOTC_FLAGS_LINKER) -o $@

clean:
	$(info [clean] $(IOTC_EXAMPLE_NAME) )
	@rm -rf $(IOTC_EXAMPLE_OBJDIR)
	@rm -rf $(IOTC_EXAMPLE_BINDIR)
