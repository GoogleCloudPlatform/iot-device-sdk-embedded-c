/****************************************************************************
 *
 *  Copyright (c) 2017, Michael Becker (michael.f.becker@gmail.com)
 *
 *  This file is part of the FreeRTOS Add-ons project.
 *
 *  Source Code:
 *  https://github.com/michaelbecker/freertos-addons
 *
 *  Project Page:
 *  http://michaelbecker.github.io/freertos-addons/
 *
 *  On-line Documentation:
 *  http://michaelbecker.github.io/freertos-addons/docs/html/index.html
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a
 *  copy of this software and associated documentation files
 *  (the "Software"), to deal in the Software without restriction, including
 *  without limitation the rights to use, copy, modify, merge, publish,
 *  distribute, sublicense, and/or sell copies of the Software, and to
 *  permit persons to whom the Software is furnished to do so,subject to the
 *  following conditions:
 *
 *  + The above copyright notice and this permission notice shall be included
 *    in all copies or substantial portions of the Software.
 *  + Credit is appreciated, but not required, if you find this project
 *    useful enough to include in your application, product, device, etc.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 *  OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 *  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 *  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 *  CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 *  TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 *  SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 ***************************************************************************/

/* Copyright 2018-2019 Google LLC
 *
 * This is part of the Google Cloud IoT Device SDK for Embedded C.
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
#include "FreeRTOS.h"
#include "task.h"

#include <iotc.h>
#include <iotc_jwt.h>
#include "commandline.h"
#include "example_utils.h"

iotc_crypto_key_data_t iotc_connect_private_key_data;
char ec_private_key_pem[PRIVATE_KEY_BUFFER_SIZE] = {0};

/**
 * The FreeRTOS task for Embedded C Client. Initializes the client
 * then starts a while loop calling the client's tick function inside.
 */
void task_function_gcpiot_embedded_c(void *parameters) {
  const size_t task_delay = (size_t)parameters;

  printf("[ INFO ] Starting GCP IoT Embedded C Client...\n");

  /* 1. Initialize the IoTC library. */
  iotc_initialize();

  /* 2. Create a context handle. */
  iotc_context_handle_t context_handle = iotc_create_context();

  /* 3. Create a JWT to connect to GCP IoT Core MQTT Bridge. */
  char jwt[IOTC_JWT_SIZE] = {0};
  size_t bytes_written = 0;
  iotc_state_t state = iotc_create_iotcore_jwt(
      iotc_core_parameters.project_id,
      /*jwt_expiration_period_sec=*/3600, &iotc_connect_private_key_data, jwt,
      IOTC_JWT_SIZE, &bytes_written);

  if (IOTC_STATE_OK != state) {
    printf("[ FAIL ] iotc_create_iotcore_jwt returned with error: %ul", state);
    iotc_shutdown();
    return;
  }

  const uint16_t connection_timeout = 10;
  const uint16_t keepalive_timeout = 3;

  /* 4. Connect to GCP IoT Core MQTT Bridge. */
  iotc_connect(context_handle, /*username=*/"unused", /*password=*/jwt,
               /*client_id=*/
               iotc_core_parameters.device_path, connection_timeout,
               keepalive_timeout, &on_connection_state_changed);

  /* 5. Run task loop. */
  while (1) {
    printf(".");
    fflush(stdout);

#if 0
    /* Calling the non-blocking tick call processes a single client cycle making
     * the client a bit slower than calling the blocking call. */
    iotc_events_process_tick();
#else
    /* Calling the blocking function processes more client cycles thus makes the
     * client faster. At some point this blocking call will wait in a socket
     * select call. And at some point the FreeRTOS tick interrupt will kick in
     * which makes the select call to exit with errno 4:
     * #define EINTR        4  // Interrupted system call
     * Making this blocking call return.
     */
    iotc_events_process_blocking();
#endif

    vTaskDelay(task_delay);
  }

  configASSERT(!"CANNOT EXIT FROM A TASK");
}

/**
 * The FreeRTOS task for an examplary custom application action.
 */
void task_function_custom_application_logic(void *parameters) {
  const size_t task_delay = (size_t)parameters;

  printf("[ INFO ] Starting Custom Application Logic Task...\n");

  while (1) {
    printf("[ custom application logic ] ");
    printf("[ heap left: %lu bytes ]\n", xPortGetFreeHeapSize());

    vTaskDelay(task_delay);
  }

  configASSERT(!"CANNOT EXIT FROM A TASK");
}

int main(int argc, char *argv[]) {
  BaseType_t rc;
  const uint16_t stack_depth = 1000;

  printf("[ INFO ] GCP IoT client example for FreeRTOS Linux.\n");

  /* Parse the GCP IoT Core parameters (project_id, registry_id, device_id,
   * private_key, publish topic, publish message) */
  if (0 != iotc_parse_commandline_flags(argc, argv)) {
    return -1;
  }

  /* Read the private key that will be used for authentication. */
  if (0 != load_ec_private_key_pem_from_posix_fs(ec_private_key_pem,
                                                 PRIVATE_KEY_BUFFER_SIZE)) {
    printf("[ FAIL ] Application exiting due to private key load error.\n\n");
    return -1;
  }

  /* Format the key type descriptors so the client understands
     what type of key is being represeted. In this case, a PEM encoded
     byte array of a ES256 key. */
  iotc_connect_private_key_data.crypto_key_signature_algorithm =
      IOTC_CRYPTO_KEY_SIGNATURE_ALGORITHM_ES256;
  iotc_connect_private_key_data.crypto_key_union_type =
      IOTC_CRYPTO_KEY_UNION_TYPE_PEM;
  iotc_connect_private_key_data.crypto_key_union.key_pem.key =
      ec_private_key_pem;

  /**
   * Create the Embedded C Client task.
   */
  rc = xTaskCreate(task_function_gcpiot_embedded_c, "gcpiot embedded c client",
                   stack_depth, /*pvParameters=*/(void *)(500),
                   /*uxPriority=*/1, /*pxCreatedTask=*/NULL);
  /**
   *  Make sure our task was created.
   */
  configASSERT(rc == pdPASS);

  /**
   * Create an application task for demonstration purposes.
   */
  rc = xTaskCreate(task_function_custom_application_logic,
                   "custom application logic", stack_depth,
                   /*pvParameters=*/(void *)500, /*uxPriority=*/1,
                   /*pxCreatedTask=*/NULL);
  /**
   *  Make sure our task was created.
   */
  configASSERT(rc == pdPASS);

  /**
   *  Start FreeRTOS here.
   */
  vTaskStartScheduler();

  /*
   *  We shouldn't ever get here unless someone calls
   *  vTaskEndScheduler(). Note that there appears to be a
   *  bug in the Linux FreeRTOS simulator that crashes when
   *  this is called.
   */
  printf("[ INFO ] Scheduler ended.\n");

  return 0;
}

void vAssertCalled(unsigned long ulLine, const char *const pcFileName) {
  printf("[ FAIL ] Assertion: %s : %d\n", pcFileName, (int)ulLine);
  while (1)
    ;
}

unsigned long ulGetRunTimeCounterValue(void) { return 0; }

void vConfigureTimerForRunTimeStats(void) { return; }

void vApplicationMallocFailedHook(void) {
  while (1)
    ;
}
