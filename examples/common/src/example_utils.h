/* Copyright 2018-2019 Google LLC
 *
 * This is part of the Google Cloud IoT Device SDK for Embedded C,
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

#ifndef __IOTC_EXAMPLES_COMMON_EXAMPLE_UTILS_H__
#define __IOTC_EXAMPLES_COMMON_EXAMPLE_UTILS_H__

#define PRIVATE_KEY_BUFFER_SIZE 256

#include <iotc.h>

/* Attempts to load the client's identifying private key from disk so that the
   byte data may be passed to the 'iotc_connect function'. Please note that the
   IoTC API and Board Support Package have various means to use private keys.
   This example assumes the use of one that must be provided to a TLS
   implementation in a buffer, but secure chips with slot-based key stores can
   also be used. Please see the Crypto BSP for more information. */
int load_ec_private_key_pem_from_posix_fs(char* buf_ec_private_key_pem,
                                          size_t buf_len);

/* A callback function that will be invoked whenever the connection state
   has changed.

   Information about the status of the connection will be provided by the
   IoTC library via the function parameters of this callback.

   DATA:
    a iotc_connection_data_t structure which holds a connection_state parameter
    that can be monitored to determine the health of the connection:

    IOTC_CONNECTION_STATE_OPENED - the connection has been successfully
   established, and TLS handshaking has verified the server certificate.  The
   client is read to publish and subscribe to topics.
    IOTC_CONNECTION_STATE_OPEN_FAILED - there was an error in the connection
        process and the connection could not be made. You can determine the
        reason in the iotc_state_t, which contains a standard IoTC error code.
    IOTC_CONNECTION_STATE_CLOSED - disconnected from Google Cloud IoT Core
   service, with the reason provided in iotc_state_t. Note: there are two
   possible reasons of being disconnected: a) the application initiated the
   disconnection procedure through a call to iotc_shutdown_connection b) there
   has been a problem with maintaining the connection, or the service shutdown
   the connection expectantly.

        Determining which of these scenarios has occured can be done by
        checking the iotc_state_t parameter of the callback.

        if state == IOTC_STATE_OK , the application intentionally disconnected
        if state != IOTC_STATE_OK , the disconnection was unexpected, and the
   state contains an error code as to what happened.

        In the case of this example,
        the application's response to a successful connection is to create a
            repeating timed task to publish a message on the configured topic.
            If an intentional disconnection occurs, then the client application
            exists.  If an unforeseen disconnection occurs, or if the connection
            fails, then the application reattempts the connection.  */
void on_connection_state_changed(iotc_context_handle_t in_context_handle,
                                 void* data, iotc_state_t state);

/* A function that publishes to the topic that was specified in the command
   line parameters of the application. This is invoked directly upon connect
   in the 'on_connection_state_changed' function, but also by the IoTC Client's
   event system on a 5 second interval. */
void publish_function(iotc_context_handle_t context_handle,
                      iotc_timed_task_handle_t timed_task, void* user_data);

#endif /* __IOTC_EXAMPLES_COMMON_EXAMPLE_UTILS_H__ */
