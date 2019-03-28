/* Copyright 2018-2019 Google LLC
 *
 * This is part of the Google Cloud IoT Device SDK for Embedded C.
 * It is licensed under the BSD 3-Clause license; you may not use this file
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

#ifndef __IOTC_TEST_ECHOSERVER_H__
#define __IOTC_TEST_ECHOSERVER_H__

#include <netdb.h>
#include <stdint.h>
#include <stdio.h>
#include <thread>
#include <unistd.h>

namespace iotctest {

const uint16_t kTimeoutSeconds = 1;
const uint16_t kBufferSize = 1024;

/**
 * @class
 * @brief is for testing end to end connection.
 *
 * This class is an API of echo server which is used to test end to end
 * connection between client and server.
 */
class EchoTestServer {
  public:
    enum class ServerError {
      kSuccess = 0,
      kInternalError = 1,
      kFailedAccept = 2,
      kFailedRecvFrom = 3,
      kFailedSendTo = 4,
      kFailedGetAddrInfo = 5,
      kFailedSetSockOpt = 6,
      kFailedListen = 7,
    };
    static std::unique_ptr<EchoTestServer> Create(std::string host, uint16_t port,
                                                  uint16_t socket_type,
                                                  uint16_t protocol_type);
    virtual ~EchoTestServer();

    /**
     * @function
     * @brief runs proper echo server regarding the type of server
     * socket(TCP/UDP).
     */
    void Run();

    void Stop();

  private:
    const std::string host_;
    const uint16_t test_port_, socket_type_, protocol_type_;

    EchoTestServer(std::string host, uint16_t port, uint16_t socket_type,
                  uint16_t protocol_type);

    /**
     * @function
     * @brief Creates and binds server socket.
     *
     * This function creates srerver socket with address information from
     * predefined port and socket type(TCP/ UDP) and regardless of its IP version.
     * It Sets socket options and in the case of TCP, listens for the socket.
     *
     * @return
     * - kSuccess - if server socket successfully created.
     * - kFailedGetAddrInfo - if getaddrinfo call finsished wiht error.
     * - kFailedSetSockOpt - if setsockopt call finished with error.
     * - kFailedListen - if listen call finished with error.
     * - kInternalError - otherwise.
     */
    ServerError CreateServer();

    /**
     * @function
     * @brief tests if the connection between a client and a server is successful.
     *
     * This function consists of sending back whatever text the client sent using
     * TCP protocol.
     *
     * @return
     * - kSuccess - if successfully run echo server with TCP.
     * - kFailedAccept - if accept call finished with error.
     */
    ServerError RunTcpServer();

    /**
     * @function
     * @brief tests if the connection between a client and a server is successful.
     *
     * This function consists of sending back whatever text the client sent using
     * UDP protocol.
     *
     * @return
     * - kSuccess - if successfully run echo server with UDP.
     * - kFailedRecvFrom - if recvfrom call finished with error.
     * - kFailedSendTo - if sendto call finished with error.
     */
    ServerError RunUdpServer();

    std::unique_ptr<std::thread> server_thread_;
    bool runnable_ = true;
    int server_socket_, client_socket_, recv_len_;
    char recv_buf_[kBufferSize];
};
}; // namespace iotctest
#endif
