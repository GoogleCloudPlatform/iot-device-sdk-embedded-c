/* Copyright 2018 Google LLC
 *
 * This is part of the Google Cloud IoT Edge Embedded C Client,
 * it is licensed under the BSD 3-Clause license; you may not use this file
 * except in compliance with the License.
 *
 * You may obtain a copy of the License at:
 *  https://opensource.org/licenses/BSD-3-Clause
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdio.h>

void vAssertCalled( unsigned long ulLine, const char * const pcFileName )
{
      printf("ASSERT: %s : %d\n", pcFileName, (int)ulLine);
          while(1);
}

void vApplicationMallocFailedHook(void) {
  printf("%s\n", __FUNCTION__);
  while (1)
    ;
}

