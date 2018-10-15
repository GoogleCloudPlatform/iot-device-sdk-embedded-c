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

#ifndef __IOTC_LAYER_H__
#define __IOTC_LAYER_H__

#include <stdint.h>

#include "iotc_config.h"
#include "iotc_error.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef iotc_state_t(iotc_layer_func_t)(void* context, void* data,
                                        iotc_state_t state);

/* The interface of a layer which combines both: simplicity and
 * functionality of the 'on demand processing' idea.
 *
 * This design is based on the assumption that the communication with the
 * server is bidirectional and the data is processed in chunks/packets. This
 * interface design is generic and allows us to implement different protcols,
 * serialization, and tls operations when can translate the data before it
 * progressing to the subsequent layer. All data handling options are contained
 * within the layer and are an extension of the basic operations of a push/pull
 * interface.
 *
 * In a very basic sense the client sends some bytes to the server and then it
 * waits for the response. After receiving the response, it is passed by to the
 * processing layer through its pull function. This signals the processing layer
 * that there is new data to process.
 *
 * push = data to written
 * pull = data to be read
 *
 * Please note that for the IoTC Client, layers have an notion that other layers
 * exist, with there being both a NEXT layer and a PREVIOUS layer. If you were
 * to map the stack of layers, these two terms are always from the vantage point
 * of the network socket.  That is, data coming from the network will be pushed
 * to the NEXT layer, continuing through each layer to decrypt, parse out the
 * MQTT message which is then acted upon, including incoming messages which may
 * be sent all the way to the Client Application.  PREVIOUS layers are ones
 * which lie closer to the Networking socket than the current layer does.
 *
 * The term 'on demand processing' refers to the fact that the system is driven
 * by the netowkr connection with the notion that incoming data from the network
 * is a catalyst for operations, as are events that are enqueud by the Client
 * Application. If neither of these two events occur, then the client lies
 * nearly dormant (save for a keep alive task timer which periodically sends
 * PINGREQ MQTT messages.)
 */
typedef struct iotc_layer_interface_s {
  /* Called whenever the prev layer wants to send more data to over the
   * connection. */
  iotc_layer_func_t* push;

  /* Called by the previous layer whenever there is data that is ready to be
     ingested, and the data's source is the 'next' layer. */
  iotc_layer_func_t* pull;

  /* Invoked whenever the processing chain supposed to be closed. */
  iotc_layer_func_t* close;

  /* Called whenver the processing chain is going to be closed from the
   * actions taken in the prev layer. */
  iotc_layer_func_t* close_externally;

  /* Called whenever we want to init the layer. */
  iotc_layer_func_t* init;

  /* Called whenever we want this layer to handle its part in the connection
   * process. */
  iotc_layer_func_t* connect;

  /* Called whenever we want to cause layers to do something in reaction
   * to a connect event occuring and before push or pull are inovked */
  iotc_layer_func_t* post_connect;
} iotc_layer_interface_t;

/**
 * @brief Describes the current state that the layer is in.
 */
typedef enum {
  IOTC_LAYER_STATE_NONE = 0,
  IOTC_LAYER_STATE_CONNECTING,
  IOTC_LAYER_STATE_CONNECTED,
  IOTC_LAYER_STATE_CLOSING,
  IOTC_LAYER_STATE_CLOSED
} iotc_layer_state_t;

typedef unsigned char iotc_layer_type_id_t;

typedef struct iotc_layer_type_s {
  iotc_layer_type_id_t layer_type_id;
  iotc_layer_interface_t layer_interface;
} iotc_layer_type_t;

#if IOTC_DEBUG_EXTRA_INFO
typedef struct iotc_layer_debug_info {
  int debug_line_init;
  const char* debug_file_init;

  int debug_line_connect;
  const char* debug_file_connect;

  int debug_line_last_call;
  const char* debug_file_last_call;
} iotc_layer_debug_info_t;
#endif

typedef struct iotc_layer_connectivity_s {
  struct iotc_layer_s* self;
  struct iotc_layer_s* next;
  struct iotc_layer_s* prev;
} iotc_layer_connectivity_t;

typedef struct iotc_layer_s {
  iotc_layer_interface_t* layer_funcs;
  iotc_layer_connectivity_t layer_connection;
  iotc_layer_type_id_t layer_type_id;
  void* user_data;
  struct iotc_context_data_s* context_data;
  iotc_layer_state_t layer_state;
#if IOTC_DEBUG_EXTRA_INFO
  iotc_layer_debug_info_t debug_info;
#endif
} iotc_layer_t;

typedef struct iotc_layer_factory_interface_s {
  /* describes the type to create, mainly for sainity checks and nice error
   * reporting */
  iotc_layer_type_id_t type_id_to_create;

  /* enables the placement initialization which is separeted from the
   * allocation/deallocation */
  iotc_layer_t* (*placement_create)(iotc_layer_t* layer, void* user_data,
                                    struct iotc_context_data_s* context_data);

  /* placement delete same as create but with oposite effect, may be used to
   * clean and/or deallocate memory */
  iotc_layer_t* (*placement_delete)(iotc_layer_t* layer);

  /* strict layer allocation, may implement different strategies for allocation
   *  of the memory required for layer */
  iotc_layer_t* (*alloc)(iotc_layer_type_t* type);

  /* strict deallocation of layer's memory */
  void (*free)(iotc_layer_type_t* type, iotc_layer_t* layer);
} iotc_layer_factory_interface_t;

#ifdef __cplusplus
}
#endif

#endif /* __IOTC_LAYER_H__ */
