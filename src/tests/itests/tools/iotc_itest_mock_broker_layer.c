/* Copyright 2018-2020 Google LLC
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

#include "iotc_itest_mock_broker_layer.h"
#include "iotc_globals.h"
#include "iotc_itest_helpers.h"
#include "iotc_itest_layerchain_ct_ml_mc.h"
#include "iotc_itest_mock_broker_layerchain.h"
#include "iotc_layer_macros.h"
#include "iotc_mqtt_logic_layer_data_helpers.h"
#include "iotc_tuples.h"
#include "iotc_types_internal.h"

#ifdef __cplusplus
extern "C" {
#endif

extern iotc_context_t* iotc_context;             // IoTC Client context
extern iotc_context_t* iotc_context_mockbroker;  // test mock broker context

/************************************************************************************
 * mock broker primary layer
 *********************************************************
 ************************************************************************************/
iotc_state_t iotc_mock_broker_layer_push__ERROR_CHANNEL() {
  return mock_type(iotc_state_t);
}

iotc_mock_broker_control_t
iotc_mock_broker_layer__check_expected__LAYER_LEVEL() {
  return mock_type(iotc_mock_broker_control_t);
}

iotc_mock_broker_control_t
iotc_mock_broker_layer__check_expected__MQTT_LEVEL() {
  return mock_type(iotc_mock_broker_control_t);
}

#define IOTC_MOCK_BROKER_CONDITIONAL__CHECK_EXPECTED(variable_to_check, level) \
  if (CONTROL_SKIP_CHECK_EXPECTED !=                                           \
      iotc_mock_broker_layer__check_expected__##level()) {                     \
    check_expected(variable_to_check);                                         \
  }

/* next layer is not null only for the SUT layerchain */
#define IS_MOCK_BROKER_LAYER_CHAIN (NULL == IOTC_NEXT_LAYER(context))

iotc_state_t iotc_mock_broker_layer_push(void* context, void* data,
                                         iotc_state_t in_out_state) {
  IOTC_LAYER_FUNCTION_PRINT_FUNCTION_DIGEST();

  IOTC_MOCK_BROKER_CONDITIONAL__CHECK_EXPECTED(in_out_state, LAYER_LEVEL);

  const iotc_mock_broker_control_t control =
      mock_type(iotc_mock_broker_control_t);

  if (control != CONTROL_CONTINUE) {
    iotc_data_desc_t* buffer = (iotc_data_desc_t*)data;
    iotc_free_desc(&buffer);

    in_out_state = iotc_mock_broker_layer_push__ERROR_CHANNEL();

    return IOTC_PROCESS_PUSH_ON_NEXT_LAYER(context, 0, in_out_state);
  }

  if (in_out_state == IOTC_STATE_OK) {
    /* duplicate the received data since it will be forwarded in two directions
     */
    iotc_data_desc_t* orig = (iotc_data_desc_t*)data;
    iotc_data_desc_t* copy =
        iotc_make_desc_from_buffer_copy(orig->data_ptr, orig->length);

    /* forward to mockbroker layerchain, note the PUSH to PULL conversion */
    iotc_evtd_execute_in(
        iotc_globals.evtd_instance,
        iotc_make_handle(
            iotc_itest_find_layer(iotc_context_mockbroker,
                                  IOTC_LAYER_TYPE_MOCKBROKER_MQTT_CODEC)
                ->layer_funcs->pull,
            &iotc_itest_find_layer(iotc_context_mockbroker,
                                   IOTC_LAYER_TYPE_MOCKBROKER_MQTT_CODEC)
                 ->layer_connection,
            copy, IOTC_STATE_OK),
        1, NULL);

    /* forward to prev layer, this is the default libiotc behavior */
    return IOTC_PROCESS_PUSH_ON_PREV_LAYER(context, data, in_out_state);
  } else {
    if (!IS_MOCK_BROKER_LAYER_CHAIN) {
      /* default libiotc behaviour */
      return IOTC_PROCESS_PUSH_ON_NEXT_LAYER(context, data, in_out_state);
    } else if (IOTC_STATE_WRITTEN == in_out_state ||
               IOTC_STATE_FAILED_WRITING == in_out_state) {
      /* this is the mockbroker layerchain behaviour */

      iotc_layer_t* layer = (iotc_layer_t*)IOTC_THIS_LAYER(context);
      iotc_mock_broker_data_t* layer_data =
          (iotc_mock_broker_data_t*)layer->user_data;

      if (NULL != layer_data) {
        iotc_free_desc(&layer_data->outgoing_publish_content);
      }

      iotc_mqtt_written_data_t* written_data = (iotc_mqtt_written_data_t*)data;
      IOTC_SAFE_FREE_TUPLE(written_data);
    }

    return IOTC_STATE_OK;
  }
}

iotc_state_t iotc_mock_broker_layer_pull(void* context, void* data,
                                         iotc_state_t in_out_state) {
  IOTC_LAYER_FUNCTION_PRINT_FUNCTION_DIGEST();

  enum iotc_connack_return_code {
    IOTC_CONNACK_ACCEPTED = 0,
    IOTC_CONNACK_REFUSED_PROTOCOL_VERSION = 1,
    IOTC_CONNACK_REFUSED_IDENTIFIER_REJECTED = 2,
    IOTC_CONNACK_REFUSED_SERVER_UNAVAILABLE = 3,
    IOTC_CONNACK_REFUSED_BAD_USERNAME_PASSWORD = 4,
    IOTC_CONNACK_REFUSED_NOT_AUTHORIZED = 5
  };

  IOTC_MOCK_BROKER_CONDITIONAL__CHECK_EXPECTED(in_out_state, LAYER_LEVEL);

  iotc_mqtt_message_t* recvd_msg = (iotc_mqtt_message_t*)data;

  /* mock broker behavior: decoded MQTT messages arrive here,
   * note the PULL to PUSH conversion */
  if (in_out_state == IOTC_STATE_OK) {
    assert(NULL != recvd_msg);  // sanity check

    // const uint16_t msg_id = iotc_mqtt_get_message_id( recvd_msg );
    const iotc_mqtt_type_t recvd_msg_type =
        recvd_msg->common.common_u.common_bits.type;

    iotc_debug_format("mock broker received message with type %d",
                      recvd_msg_type);

    IOTC_MOCK_BROKER_CONDITIONAL__CHECK_EXPECTED(recvd_msg_type, MQTT_LEVEL);

    switch (recvd_msg_type) {
      case IOTC_MQTT_TYPE_CONNECT: {
        IOTC_ALLOC(iotc_mqtt_message_t, msg_connack, in_out_state);
        IOTC_CHECK_STATE(in_out_state = fill_with_connack_data(
                             msg_connack, IOTC_CONNACK_ACCEPTED));

        iotc_mqtt_message_free(&recvd_msg);
        return IOTC_PROCESS_PUSH_ON_PREV_LAYER(context, msg_connack,
                                               in_out_state);
      } break;

      case IOTC_MQTT_TYPE_SUBSCRIBE: {
        const char* subscribe_topic_name =
            (const char*)recvd_msg->subscribe.topics->name->data_ptr;

        iotc_debug_format("subscribe arrived on topic `%s`",
                          subscribe_topic_name);

        IOTC_MOCK_BROKER_CONDITIONAL__CHECK_EXPECTED(subscribe_topic_name,
                                                     MQTT_LEVEL);

        IOTC_ALLOC(iotc_mqtt_message_t, msg_suback, in_out_state);
        // note: fill subscribe can be used for fill suback only because the
        // memory map of the two structs are identical since unions are used
        IOTC_CHECK_STATE(
            in_out_state = fill_with_subscribe_data(
                msg_suback,
                "unused topic name",  // topic name is not used in SUBACK, so
                                      // this string is irrelevant
                recvd_msg->subscribe.message_id,
                recvd_msg->subscribe.topics->iotc_mqtt_topic_pair_payload_u.qos,
                IOTC_MQTT_DUP_FALSE));

        msg_suback->common.common_u.common_bits.type = IOTC_MQTT_TYPE_SUBACK;

        iotc_mqtt_message_free(&recvd_msg);
        return IOTC_PROCESS_PUSH_ON_PREV_LAYER(context, msg_suback,
                                               in_out_state);
      } break;

      case IOTC_MQTT_TYPE_PUBLISH: {
        const char* publish_topic_name =
            (const char*)recvd_msg->publish.topic_name->data_ptr;

        iotc_debug_format("publish arrived on topic `%s`, msgid: %d",
                          publish_topic_name, recvd_msg->publish.message_id);

        IOTC_MOCK_BROKER_CONDITIONAL__CHECK_EXPECTED(publish_topic_name,
                                                     MQTT_LEVEL);

        /* PUBACK if necessary */
        if (0 < recvd_msg->common.common_u.common_bits.qos) {
          IOTC_ALLOC(iotc_mqtt_message_t, msg_puback, in_out_state);
          IOTC_CHECK_STATE(in_out_state = fill_with_puback_data(
                               msg_puback, recvd_msg->publish.message_id));

          iotc_mqtt_message_free(&recvd_msg);
          return IOTC_PROCESS_PUSH_ON_PREV_LAYER(context, msg_puback,
                                                 in_out_state);
        }
      } break;

      case IOTC_MQTT_TYPE_PINGREQ: {
        const iotc_mock_broker_control_t control =
            mock_type(iotc_mock_broker_control_t);

        if (CONTROL_PULL_PINGREQ_SUPPRESS_RESPONSE != control) {
          IOTC_ALLOC(iotc_mqtt_message_t, mqtt_pingresp, in_out_state);
          memcpy(mqtt_pingresp, recvd_msg, sizeof(iotc_mqtt_message_t));

          mqtt_pingresp->common.common_u.common_bits.type =
              IOTC_MQTT_TYPE_PINGRESP;

          in_out_state = IOTC_PROCESS_PUSH_ON_PREV_LAYER(context, mqtt_pingresp,
                                                         in_out_state);
        }
      } break;

      case IOTC_MQTT_TYPE_DISCONNECT:
        // do nothing
        iotc_debug_printf(
            "================= DISCONNECT arrived at mock broker\n");
        break;

      default:
        iotc_debug_printf("*** *** unhandled message arrived\n");
        in_out_state = IOTC_MQTT_UNKNOWN_MESSAGE_ID;
        break;
    }
  }

err_handling:

  iotc_mqtt_message_free(&recvd_msg);

  return in_out_state;
}

iotc_state_t iotc_mock_broker_layer_close(void* context, void* data,
                                          iotc_state_t in_out_state) {
  IOTC_LAYER_FUNCTION_PRINT_FUNCTION_DIGEST();

  IOTC_MOCK_BROKER_CONDITIONAL__CHECK_EXPECTED(in_out_state, LAYER_LEVEL);

  iotc_layer_t* layer = (iotc_layer_t*)IOTC_THIS_LAYER(context);
  iotc_mock_broker_data_t* layer_data =
      (iotc_mock_broker_data_t*)layer->user_data;

  IOTC_SAFE_FREE(layer_data);

  return IOTC_PROCESS_CLOSE_ON_PREV_LAYER(context, data, in_out_state);
}

iotc_state_t iotc_mock_broker_layer_close_externally(
    void* context, void* data, iotc_state_t in_out_state) {
  IOTC_LAYER_FUNCTION_PRINT_FUNCTION_DIGEST();

  IOTC_UNUSED(context);
  IOTC_UNUSED(data);

  return IOTC_PROCESS_CLOSE_EXTERNALLY_ON_NEXT_LAYER(context, data,
                                                     in_out_state);
}

iotc_state_t iotc_mock_broker_layer_init(void* context, void* data,
                                         iotc_state_t in_out_state) {
  IOTC_LAYER_FUNCTION_PRINT_FUNCTION_DIGEST();

  IOTC_MOCK_BROKER_CONDITIONAL__CHECK_EXPECTED(in_out_state, LAYER_LEVEL);

  const iotc_mock_broker_control_t control =
      mock_type(iotc_mock_broker_control_t);

  if (control == CONTROL_ERROR) {
    const iotc_state_t return_state = mock_type(iotc_state_t);
    return IOTC_PROCESS_CONNECT_ON_THIS_LAYER(context, data, return_state);
  }

  if (IS_MOCK_BROKER_LAYER_CHAIN) {
    iotc_layer_t* layer = (iotc_layer_t*)IOTC_THIS_LAYER(context);
    layer->user_data = data;
  }

  return IOTC_PROCESS_INIT_ON_PREV_LAYER(context, data, in_out_state);
}

iotc_state_t iotc_mock_broker_layer_connect(void* context, void* data,
                                            iotc_state_t in_out_state) {
  IOTC_LAYER_FUNCTION_PRINT_FUNCTION_DIGEST();

  IOTC_MOCK_BROKER_CONDITIONAL__CHECK_EXPECTED(in_out_state, LAYER_LEVEL);

  if (!IS_MOCK_BROKER_LAYER_CHAIN) {
    return IOTC_PROCESS_CONNECT_ON_NEXT_LAYER(context, data, in_out_state);
  }

  return IOTC_STATE_OK;
}

/************************************************************************************
 * mock broker secondary layer
 *******************************************************
 ************************************************************************************/
iotc_state_t iotc_mock_broker_secondary_layer_push(void* context, void* data,
                                                   iotc_state_t in_out_state) {
  IOTC_UNUSED(itest_mock_broker_codec_layer_chain);
  IOTC_UNUSED(IOTC_LAYER_CHAIN_MOCK_BROKER_CODECSIZE_SUFFIX);
  IOTC_UNUSED(itest_ct_ml_mc_layer_chain);
  IOTC_UNUSED(IOTC_LAYER_CHAIN_CT_ML_MCSIZE_SUFFIX);
  IOTC_LAYER_FUNCTION_PRINT_FUNCTION_DIGEST();

  IOTC_MOCK_BROKER_CONDITIONAL__CHECK_EXPECTED(in_out_state, LAYER_LEVEL);

  const iotc_mock_broker_control_t control =
      mock_type(iotc_mock_broker_control_t);

  if (control == CONTROL_ERROR) {
    iotc_data_desc_t* buffer = (iotc_data_desc_t*)data;
    iotc_free_desc(&buffer);

    in_out_state = mock_type(iotc_state_t);

    iotc_evtd_execute_in(
        iotc_globals.evtd_instance,
        iotc_make_handle(
            iotc_itest_find_layer(iotc_context, IOTC_LAYER_TYPE_MQTT_CODEC_SUT)
                ->layer_funcs->pull,
            &iotc_itest_find_layer(iotc_context, IOTC_LAYER_TYPE_MQTT_CODEC_SUT)
                 ->layer_connection,
            NULL, in_out_state),
        1, NULL);

    return IOTC_PROCESS_PUSH_ON_NEXT_LAYER(context, NULL, IOTC_STATE_WRITTEN);
  }

  if (in_out_state == IOTC_STATE_OK) {
    /* duplicate the data before forwarding it to SUT layer since this is done
     * by the network as well. This is required for PUBLISH payloads which are
     * not copied between layers */
    iotc_data_desc_t* orig = (iotc_data_desc_t*)data;
    iotc_data_desc_t* copy =
        iotc_make_desc_from_buffer_copy(orig->data_ptr, orig->length);

    /* data_desc deallocation is done by the real IO layer too */
    iotc_free_desc(&orig);

    /* jump to SUT libiotc's codec layer pull function, mimicing incoming
     * encoded message */
    iotc_evtd_execute_in(
        iotc_globals.evtd_instance,
        iotc_make_handle(
            iotc_itest_find_layer(iotc_context, IOTC_LAYER_TYPE_MQTT_CODEC_SUT)
                ->layer_funcs->pull,
            &iotc_itest_find_layer(iotc_context, IOTC_LAYER_TYPE_MQTT_CODEC_SUT)
                 ->layer_connection,
            copy, in_out_state),
        1, NULL);

    return IOTC_PROCESS_PUSH_ON_NEXT_LAYER(context, NULL, IOTC_STATE_WRITTEN);
  }

  return IOTC_PROCESS_PUSH_ON_NEXT_LAYER(context, NULL, in_out_state);
}

iotc_state_t iotc_mock_broker_secondary_layer_pull(void* context, void* data,
                                                   iotc_state_t in_out_state) {
  IOTC_LAYER_FUNCTION_PRINT_FUNCTION_DIGEST();

  IOTC_UNUSED(context);
  IOTC_UNUSED(data);
  IOTC_UNUSED(in_out_state);

  /* release the payload */
  iotc_data_desc_t* desc = (iotc_data_desc_t*)data;
  iotc_free_desc(&desc);

  return IOTC_STATE_OK;
}

iotc_state_t iotc_mock_broker_secondary_layer_close(void* context, void* data,
                                                    iotc_state_t in_out_state) {
  IOTC_LAYER_FUNCTION_PRINT_FUNCTION_DIGEST();

  IOTC_UNUSED(context);
  IOTC_UNUSED(data);
  IOTC_UNUSED(in_out_state);

  return IOTC_PROCESS_CLOSE_EXTERNALLY_ON_THIS_LAYER(context, data,
                                                     in_out_state);
}

iotc_state_t iotc_mock_broker_secondary_layer_close_externally(
    void* context, void* data, iotc_state_t in_out_state) {
  IOTC_LAYER_FUNCTION_PRINT_FUNCTION_DIGEST();

  IOTC_UNUSED(context);
  IOTC_UNUSED(data);
  IOTC_UNUSED(in_out_state);

  return IOTC_PROCESS_CLOSE_EXTERNALLY_ON_NEXT_LAYER(context, data,
                                                     in_out_state);
}

iotc_state_t iotc_mock_broker_secondary_layer_init(void* context, void* data,
                                                   iotc_state_t in_out_state) {
  IOTC_LAYER_FUNCTION_PRINT_FUNCTION_DIGEST();

  return IOTC_PROCESS_CONNECT_ON_THIS_LAYER(context, data, in_out_state);
}

iotc_state_t iotc_mock_broker_secondary_layer_connect(
    void* context, void* data, iotc_state_t in_out_state) {
  IOTC_LAYER_FUNCTION_PRINT_FUNCTION_DIGEST();

  return IOTC_PROCESS_CONNECT_ON_NEXT_LAYER(context, data, in_out_state);
}

#ifdef __cplusplus
}
#endif
