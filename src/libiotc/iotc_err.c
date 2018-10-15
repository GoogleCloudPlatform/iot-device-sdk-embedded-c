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

#include "iotc_err.h"
#include "iotc_macros.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef IOTC_OPT_NO_ERROR_STRINGS
const char* iotc_error_string[IOTC_ERROR_COUNT] = {
    "IOTC_STATE_OK",                    /* 0 IOTC_STATE_OK */
    "IOTC_STATE_TIMEOUT",               /* 1 IOTC_STATE_TIMEOUT */
    "IOTC_STATE_WANT_READ",             /* 2 IOTC_STATE_WANT_READ */
    "IOTC_STATE_WANT_WRITE",            /* 3 IOTC_STATE_WANT_WRITE */
    "IOTC_STATE_WRITTEN",               /* 4 IOTC_STATE_WRITTEN */
    "IOTC_STATE_FAILED_WRITING",        /* 5 IOTC_STATE_FAILED_WRITING */
    "IOTC_BACKOFF_TERMINAL",            /* 6 IOTC_BACKOFF_TERMINAL */
    "IOTC_OUT_OF_MEMORY",               /* 7 IOTC_OUT_OF_MEMORY */
    "IOTC_SOCKET_INITIALIZATION_ERROR", /* 8 IOTC_SOCKET_INITIALIZATION_ERROR */
    "IOTC_SOCKET_GETHOSTBYNAME_ERROR",  /* 9 IOTC_SOCKET_GETHOSTBYNAME_ERROR */
    "IOTC_SOCKET_GETSOCKOPT_ERROR",     /* 10 IOTC_SOCKET_GETSOCKOPT_ERROR */
    "IOTC_SOCKET_ERROR",                /* 11 IOTC_SOCKET_ERROR */
    "IOTC_SOCKET_CONNECTION_ERROR",     /* 12 IOTC_SOCKET_CONNECTION_ERROR */
    "IOTC_SOCKET_SHUTDOWN_ERROR",       /* 13 IOTC_SOCKET_SHUTDOWN_ERROR */
    "IOTC_SOCKET_WRITE_ERROR",          /* 14 IOTC_SOCKET_WRITE_ERROR */
    "IOTC_SOCKET_READ_ERROR",           /* 15 IOTC_SOCKET_READ_ERROR */
    "IOTC_SOCKET_NO_ACTIVE_CONNECTION_ERROR", /* 16
                                                 IOTC_SOCKET_NO_ACTIVE_CONNECTION_ERROR
                                               */
    "IOTC_CONNECTION_RESET_BY_PEER_ERROR",    /* 17
                                                 IOTC_CONNECTION_RESET_BY_PEER_ERROR
                                               */
    "IOTC_FD_HANDLER_NOT_FOUND",              /* 18 IOTC_FD_HANDLER_NOT_FOUND */
    "IOTC_TLS_INITALIZATION_ERROR",        /* 19 IOTC_TLS_INITALIZATION_ERROR */
    "IOTC_TLS_FAILED_LOADING_CERTIFICATE", /* 20
                                              IOTC_TLS_FAILED_LOADING_CERTIFICATE
                                            */
    "IOTC_TLS_CONNECT_ERROR",              /* 21 IOTC_TLS_CONNECT_ERROR */
    "IOTC_TLS_WRITE_ERROR",                /* 22 IOTC_TLS_WRITE_ERROR */
    "IOTC_TLS_READ_ERROR",                 /* 23 IOTC_TLS_READ_ERROR */
    "IOTC_MQTT_SERIALIZER_ERROR",          /* 24 IOTC_MQTT_SERIALIZER_ERROR */
    "IOTC_MQTT_PARSER_ERROR",              /* 25 IOTC_MQTT_PARSER_ERROR */
    "IOTC_MQTT_UNKNOWN_MESSAGE_ID",        /* 26 IOTC_MQTT_UNKNOWN_MESSAGE_ID */
    "IOTC_MQTT_LOGIC_UNKNOWN_TASK_ID", /* 27 IOTC_MQTT_LOGIC_UNKNOWN_TASK_ID */
    "IOTC_MQTT_LOGIC_WRONG_SCENARIO_TYPE",     /* 28
                                                  IOTC_MQTT_LOGIC_WRONG_SCENARIO_TYPE
                                                */
    "IOTC_MQTT_LOGIC_WRONG_MESSAGE_RECEIVED",  /* 29
                                                  IOTC_MQTT_LOGIC_WRONG_MESSAGE_RECEIVED
                                                */
    "IOTC_MQTT_UNACCEPTABLE_PROTOCOL_VERSION", /* 30
                                                * IOTC_MQTT_UNACCEPTABLE_PROTOCOL_VERSION
                                                */
    "IOTC_MQTT_IDENTIFIER_REJECTED",      /* 31 IOTC_MQTT_IDENTIFIER_REJECTED */
    "IOTC_MQTT_SERVER_UNAVAILIBLE",       /* 32 IOTC_MQTT_SERVER_UNAVAILIBLE */
    "IOTC_MQTT_BAD_USERNAME_OR_PASSWORD", /* 33
                                             IOTC_MQTT_BAD_USERNAME_OR_PASSWORD
                                           */
    "IOTC_MQTT_NOT_AUTHORIZED",           /* 34 IOTC_MQTT_NOT_AUTHORIZED */
    "IOTC_MQTT_CONNECT_UNKNOWN_RETURN_CODE", /* 35
                                                IOTC_MQTT_CONNECT_UNKNOWN_RETURN_CODE
                                              */
    "IOTC_MQTT_MESSAGE_CLASS_UNKNOWN_ERROR", /* 36
                                                IOTC_MQTT_MESSAGE_CLASS_UNKNOWN
                                              */
    "IOTC_MQTT_PAYLOAD_SIZE_TOO_LARGE", /* 37 IOTC_MQTT_PAYLOAD_SIZE_TOO_LARGE
                                         */
    "IOTC_MQTT_SUBSCRIBE_FAILED",       /* 38 IOTC_MQTT_SUBSCRIBE_FAILED */
    "IOTC_MQTT_SUBSCRIBE_SUCCESSFULL",  /* 39 IOTC_MQTT_SUBSCRIBE_SUCCESSFULL */
    "IOTC_INTERNAL_ERROR",              /* 40 IOTC_INTERNAL_ERROR */
    "IOTC_NOT_INITIALIZED",             /* 41 IOTC_NOT_INITIALIZED */
    "IOTC_FAILED_INITIALIZATION",       /* 42 IOTC_FAILED_INITIALIZATION */
    "IOTC_ALREADY_INITIALIZED",         /* 43 IOTC_ALREADY_INITIALIZED */
    "IOTC_INVALID_PARAMETER",           /* 44 IOTC_INVALID_PARAMETER */
    "IOTC_UNSET_HANDLER_ERROR",         /* 45 IOTC_UNSET_HANDLER_ERROR */
    "IOTC_NOT_IMPLEMENTED",             /* 46 IOTC_NOT_IMPLEMENTED */
    "IOTC_ELEMENT_NOT_FOUND",           /* 47 IOTC_ELEMENT_NOT_FOUND */
    "IOTC_SERIALIZATION_ERROR",         /* 48 IOTC_SERIALIZATION_ERROR */
    "IOTC_TRUNCATION_WARNING",          /* 49 IOTC_TRUNCATION_WARNING */
    "IOTC_BUFFER_OVERFLOW",             /* 50 IOTC_BUFFER_OVERFLOW */
    "IOTC_THREAD_ERROR",                /* 51 IOTC_THREAD_ERROR */
    "The passed or default context is NULL", /* 52 IOTC_NULL_CONTEXT */
    "The last will topic cannot be NULL",    /* 53 IOTC_NULL_WILL_TOPIC */
    "The last will message cannot be NULL",  /* 54 IOTC_NULL_WILL_MESSAGE */
    "IOTC_NO_MORE_RESOURCE_AVAILABLE",  /* 55 IOTC_NO_MORE_RESOURCE_AVAILABLE */
    "IOTC_FS_RESOURCE_NOT_AVAILABLE",   /* 56 IOTC_FS_RESOURCE_NOT_AVAILABLE*/
    "IOTC_FS_ERROR",                    /* 57 IOTC_FS_ERROR */
    "IOTC_NOT_SUPPORTED",               /* 58 IOTC_NOT_SUPPORTED */
    "IOTC_EVENT_PROCESS_STOPPED",       /* 59 IOTC_EVENT_PROCESS_STOPPED */
    "IOTC_STATE_RESEND",                /* 60 IOTC_STATE_RESEND */
    "IOTC_NULL_HOST",                   /* 61 IOTC_STATE_RESEND */
    "IOTC_TLS_FAILED_CERT_ERROR",       /* 62 IOTC_TLS_FAILED_CERT_ERROR */
    "IOTC_FS_OPEN_ERROR",               /* 63 IOTC_FS_OPEN_ERROR */
    "IOTC_FS_OPEN_READ_ONLY",           /* 64 IOTC_FS_OPEN_READ_ONLY */
    "IOTC_FS_READ_ERROR",               /* 65 IOTC_FS_READ_ERROR */
    "IOTC_FS_WRITE_ERROR",              /* 66 IOTC_FS_WRITE_ERROR */
    "IOTC_FS_CLOSE_ERROR",              /* 67 IOTC_FS_CLOSE_ERROR */
    "IOTC_FS_REMOVE_ERROR",             /* 68 IOTC_FS_REMOVE_ERROR */
    "IOTC_NULL_PROJECT_ID_ERROR",       /* 69 IOTC_NULL_PROJECT_ID_ERROR */
    "IOTC_ALG_NOT_SUPPORTED_ERROR",     /* 70 IOTC_ALG_NOT_SUPPORTED_ERROR */
    "IOTC_JWT_FORMATTION_ERROR",        /* 71 IOTC_JWT_FORMATTION_ERROR */
    "IOTC_JWT_PROJECTID_TOO_LONG_ERROR" /* 72 IOTC_JWT_PROJECTID_TOO_LONG_ERROR
                                         */
    "IOTC_NULL_DEVICE_PATH_ERROR",      /* 73 IOTC_NULL_DEVICE_PATH_ERROR */
    "IOTC_BUFFER_TOO_SMALL_ERROR",      /* 74 IOTC_BUFFER_TOO_SMALL_ERROR */
    "IOTC_NULL_PRIVATE_KEY_DATA_ERROR" /* 75 IOTC_NULL_PRIVATE_KEY_DATA_ERROR */
};
#else
const char empty_sting[] = "";
#endif /* IOTC_OPT_NO_ERROR_STRINGS */

const char* iotc_get_state_string(iotc_state_t e) {
#ifdef IOTC_OPT_NO_ERROR_STRINGS
  IOTC_UNUSED(e);
  return empty_sting;
#else
  return iotc_error_string[IOTC_MIN((short)e, IOTC_ERROR_COUNT - 1)];
#endif
}

#ifdef __cplusplus
}
#endif
