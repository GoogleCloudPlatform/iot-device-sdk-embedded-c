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

#ifndef __IOTC_LAYER_STACK_H__
#define __IOTC_LAYER_STACK_H__

#include "iotc_layer.h"
#include "iotc_layer_default_functions.h"
#include "iotc_layer_macros.h"
#include "iotc_layers_ids.h"

#ifdef __cplusplus
extern "C"
{
#endif

/* -----------------------------------------------------------------------
 * LAYERS SETTINGS
 * ----------------------------------------------------------------------- */
#ifndef IOTC_NO_TLS_LAYER
#define IOTC_DEFAULT_LAYER_CHAIN                       \
    IOTC_LAYER_TYPE_IO                                 \
    , IOTC_LAYER_TYPE_TLS, IOTC_LAYER_TYPE_MQTT_CODEC, \
        IOTC_LAYER_TYPE_MQTT_LOGIC, IOTC_LAYER_TYPE_CONTROL_TOPIC
#else
#define IOTC_DEFAULT_LAYER_CHAIN                              \
    IOTC_LAYER_TYPE_IO                                        \
    , IOTC_LAYER_TYPE_MQTT_CODEC, IOTC_LAYER_TYPE_MQTT_LOGIC, \
        IOTC_LAYER_TYPE_CONTROL_TOPIC
#endif

    IOTC_DECLARE_LAYER_CHAIN_SCHEME(IOTC_LAYER_CHAIN_DEFAULT,
                                    IOTC_DEFAULT_LAYER_CHAIN);

#include "iotc_io_net_layer.h"
#ifndef IOTC_NO_TLS_LAYER
#include "iotc_tls_layer.h"
#include "iotc_tls_layer_state.h"
#endif

/*-----------------------------------------------------------------------*/
#include "iotc_control_topic_layer.h"
#include "iotc_mqtt_codec_layer.h"
#include "iotc_mqtt_codec_layer_data.h"
#include "iotc_mqtt_logic_layer.h"
#include "iotc_mqtt_logic_layer_data.h"

    IOTC_DECLARE_LAYER_TYPES_BEGIN(iotc_layer_types_g)
    IOTC_LAYER_TYPES_ADD(IOTC_LAYER_TYPE_IO, &iotc_io_net_layer_push,
                         &iotc_io_net_layer_pull, &iotc_io_net_layer_close,
                         &iotc_io_net_layer_close_externally,
                         &iotc_io_net_layer_init, &iotc_io_net_layer_connect,
                         &iotc_layer_default_post_connect)
#ifndef IOTC_NO_TLS_LAYER
    , IOTC_LAYER_TYPES_ADD(IOTC_LAYER_TYPE_TLS, &iotc_tls_layer_push,
                           &iotc_tls_layer_pull, &iotc_tls_layer_close,
                           &iotc_tls_layer_close_externally, &iotc_tls_layer_init,
                           &iotc_tls_layer_connect,
                           &iotc_layer_default_post_connect)
#endif
          ,
        IOTC_LAYER_TYPES_ADD(
            IOTC_LAYER_TYPE_MQTT_CODEC, &iotc_mqtt_codec_layer_push,
            &iotc_mqtt_codec_layer_pull, &iotc_mqtt_codec_layer_close,
            &iotc_mqtt_codec_layer_close_externally, &iotc_mqtt_codec_layer_init,
            &iotc_mqtt_codec_layer_connect, &iotc_layer_default_post_connect),
        IOTC_LAYER_TYPES_ADD(
            IOTC_LAYER_TYPE_MQTT_LOGIC, &iotc_mqtt_logic_layer_push,
            &iotc_mqtt_logic_layer_pull, &iotc_mqtt_logic_layer_close,
            &iotc_mqtt_logic_layer_close_externally, &iotc_mqtt_logic_layer_init,
            &iotc_mqtt_logic_layer_connect, &iotc_mqtt_logic_layer_post_connect),
        IOTC_LAYER_TYPES_ADD(IOTC_LAYER_TYPE_CONTROL_TOPIC,
                             &iotc_control_topic_layer_push,
                             &iotc_control_topic_layer_pull,
                             &iotc_control_topic_layer_close,
                             &iotc_control_topic_layer_close_externally,
                             &iotc_control_topic_layer_init,
                             &iotc_control_topic_layer_connect,
                             &iotc_layer_default_post_connect)
            IOTC_DECLARE_LAYER_TYPES_END()

#ifdef __cplusplus
}
#endif

#else

#error Cannot include more then once!

#endif /* __IOTC_LAYER_STACK_H__ */
