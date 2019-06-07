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

#include <memory.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include <iotc_mqtt_parser.h>
#include <iotc_mqtt_message.h>
#include <iotc_macros.h>

const static size_t IOTC_BUFFER_SIZE = 32;

/* This is the fuzzer signature, and we cannot change it */
extern "C" int LLVMFuzzerTestOneInput( const uint8_t* data, size_t size )
{
    iotc_mqtt_parser_t parser;
    iotc_state_t local_state = IOTC_STATE_OK;
    iotc_mqtt_message_t* msg = NULL;

    size_t size_eaten = 0;

    IOTC_ALLOC_AT( iotc_mqtt_message_t, msg, local_state );
    iotc_mqtt_parser_init( &parser );

    while ( size_eaten != size )
    {
        const size_t copied_buffer_size = IOTC_MIN( IOTC_BUFFER_SIZE, ( size - size_eaten ) );

        iotc_data_desc_t* data_desc =
            iotc_make_desc_from_buffer_copy( data + size_eaten, copied_buffer_size );

        local_state = iotc_mqtt_parser_execute( &parser, msg, data_desc );

        if ( local_state == IOTC_STATE_OK || local_state == IOTC_MQTT_PARSER_ERROR )
        {
            iotc_mqtt_message_free( &msg );
            IOTC_ALLOC_AT( iotc_mqtt_message_t, msg, local_state );
        }

        size_eaten += copied_buffer_size - ( data_desc->length - data_desc->curr_pos );

        iotc_free_desc( &data_desc );
    }

    iotc_mqtt_message_free( &msg );

err_handling:
    return 0;
}
