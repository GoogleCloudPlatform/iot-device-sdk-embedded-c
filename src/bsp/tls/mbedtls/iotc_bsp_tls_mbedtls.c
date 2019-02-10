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

#include <assert.h>
#include <iotc_allocator.h>
#include <iotc_bsp_debug.h>
#include <iotc_bsp_tls.h>
#include <stddef.h>
#include <string.h>

#define MBEDTLS_DEBUG_LOG 1

#include <mbedtls/ctr_drbg.h>
#include <mbedtls/entropy.h>
#include <mbedtls/error.h>
#include <mbedtls/platform.h>
#include <mbedtls/ssl.h>

/**
 * @brief If the libiotc's certificate buffer's last character is '\n' (common
 * after file reading, and replicated in iotc_RootCA_list for consistency),
 * replace it with a NULL terminator
 */
static void mbedtls_prepare_certificate_buffer(uint8_t* cert_buffer,
                                               size_t cert_buffer_len) {
  uint8_t* const c = &cert_buffer[cert_buffer_len - 1];

  if ('\n' == *c) {
    *c = '\0';
  }
}

/**
 * @typedef mbedtls_tls_context_t
 * @brief holds data important for mbedtls related bsp functions
 **/
typedef struct mbedtls_tls_context_s {
  mbedtls_ssl_context ssl;
  mbedtls_ssl_config conf;

  mbedtls_entropy_context entropy;
  mbedtls_ctr_drbg_context ctr_drbg;

  mbedtls_x509_crt cacert;
} mbedtls_tls_context_t;

int iotc_mbedtls_recv(void* libiotc_io_callback_context, unsigned char* buf,
                      size_t len) {
  assert(NULL != libiotc_io_callback_context);
  assert(NULL != buf);
  assert(0 < len);

  iotc_bsp_debug_format("[ %s ]", __FUNCTION__);

  int bytes_read = 0;
  iotc_bsp_tls_state_t ret = iotc_bsp_tls_recv_callback(
      (char*)buf, len, libiotc_io_callback_context, &bytes_read);

  switch (ret) {
    case IOTC_BSP_TLS_STATE_OK:
      return bytes_read;
    case IOTC_BSP_TLS_STATE_WANT_READ:
      return MBEDTLS_ERR_SSL_WANT_READ;
    default:
      iotc_bsp_debug_format("unexpected state %d", ret);
#if MBEDTLS_DEBUG_LOG
      char errorString[80] = {'\0'};
      mbedtls_strerror(ret, errorString, sizeof(errorString));
      iotc_bsp_debug_format("Recv error - %s", errorString);
#endif
      return MBEDTLS_ERR_SSL_INTERNAL_ERROR;
  }

  return MBEDTLS_ERR_SSL_INTERNAL_ERROR;
}

int iotc_mbedtls_send(void* libiotc_io_callback_context,
                      const unsigned char* buf, size_t len) {
  assert(NULL != libiotc_io_callback_context);
  assert(NULL != buf);
  assert(0 < len);

  iotc_bsp_debug_format("[ %s ]", __FUNCTION__);

  int bytes_sent = 0;
  iotc_bsp_tls_state_t ret = iotc_bsp_tls_send_callback(
      (char*)buf, len, libiotc_io_callback_context, &bytes_sent);

  switch (ret) {
    case IOTC_BSP_TLS_STATE_OK:
      return bytes_sent;
    case IOTC_BSP_TLS_STATE_WANT_WRITE:
      return MBEDTLS_ERR_SSL_WANT_WRITE;
    default:
      iotc_bsp_debug_format("unexpected state %d", ret);
#if MBEDTLS_DEBUG_LOG
      char errorString[80] = {'\0'};
      mbedtls_strerror(ret, errorString, sizeof(errorString));
      iotc_bsp_debug_format("Send error - %s", errorString);
#endif
      return MBEDTLS_ERR_SSL_INTERNAL_ERROR;
  }

  return MBEDTLS_ERR_SSL_INTERNAL_ERROR;
}

iotc_bsp_tls_state_t iotc_bsp_tls_init(
    iotc_bsp_tls_context_t** tls_context,
    iotc_bsp_tls_init_params_t* init_params) {
  assert(NULL != tls_context);
  assert(NULL == *tls_context);
  assert(NULL != init_params);

  iotc_bsp_debug_format("[ %s ]", __FUNCTION__);

  if (NULL == tls_context || NULL != *tls_context) {
    return IOTC_BSP_TLS_STATE_INIT_ERROR;
  }

  /* return state used for checking each mbedtls function */
  int ret_state = 0;

  /* RNG related string */
  const char personalization[] = "iotc_bsp_mbedtls_more_entropy_pls";

#ifdef MBEDTLS_PLATFORM_MEMORY
  mbedtls_platform_set_calloc_free(init_params->fp_libiotc_calloc,
                                   init_params->fp_libiotc_free);
#endif

  mbedtls_tls_context_t* mbedtls_tls_context =
      (mbedtls_tls_context_t*)mbedtls_calloc(sizeof(mbedtls_tls_context_t), 1);

  /* save tls context, this value will be passed back in other BSP TLS functions
   */
  *tls_context = mbedtls_tls_context;

  /* initialise the mbedtls context */
  mbedtls_ssl_init(&mbedtls_tls_context->ssl);
  mbedtls_ssl_config_init(&mbedtls_tls_context->conf);

  /* initialise RNG */
  mbedtls_entropy_init(&mbedtls_tls_context->entropy);
  mbedtls_ctr_drbg_init(&mbedtls_tls_context->ctr_drbg);

  if ((ret_state = mbedtls_ctr_drbg_seed(
           &mbedtls_tls_context->ctr_drbg, mbedtls_entropy_func,
           &mbedtls_tls_context->entropy, (const unsigned char*)personalization,
           sizeof(personalization))) != 0) {
    iotc_bsp_debug_format(" failed ! mbedtls_ctr_drbg_seed returned %d",
                          ret_state);
    goto err_handling;
  }

  /* register I/O functions */
  mbedtls_ssl_set_bio(&mbedtls_tls_context->ssl,
                      init_params->libiotc_io_callback_context,
                      iotc_mbedtls_send, iotc_mbedtls_recv, NULL);

  ret_state = mbedtls_ssl_config_defaults(
      &mbedtls_tls_context->conf, MBEDTLS_SSL_IS_CLIENT,
      MBEDTLS_SSL_TRANSPORT_STREAM, MBEDTLS_SSL_PRESET_DEFAULT);

#ifdef IOTC_DISABLE_CERTVERIFY
  mbedtls_ssl_conf_authmode(&mbedtls_tls_context->conf,
                            MBEDTLS_SSL_VERIFY_NONE);
#else
  mbedtls_ssl_conf_authmode(&mbedtls_tls_context->conf,
                            MBEDTLS_SSL_VERIFY_REQUIRED);
#endif

  /* init & parse the CA certificates */
  mbedtls_x509_crt_init(&mbedtls_tls_context->cacert);

  /* this is required via the mbedtls in order to parse the PEM certificate
   * correctly - mbedtls requires '\0' at the end of the buffer that contains
   * PEM certificate */
  mbedtls_prepare_certificate_buffer(init_params->ca_cert_pem_buf,
                                     init_params->ca_cert_pem_buf_length);

  ret_state = mbedtls_x509_crt_parse(&mbedtls_tls_context->cacert,
                                     init_params->ca_cert_pem_buf,
                                     init_params->ca_cert_pem_buf_length);

  if (ret_state < 0) {
    iotc_bsp_debug_format("failed ! mbedtls_x509_crt_parse returned %d",
                          ret_state);
    goto err_handling;
  }

  /* set the ca certificate chain */
  mbedtls_ssl_conf_ca_chain(&mbedtls_tls_context->conf,
                            &mbedtls_tls_context->cacert, NULL);
  mbedtls_ssl_conf_rng(&mbedtls_tls_context->conf, mbedtls_ctr_drbg_random,
                       &mbedtls_tls_context->ctr_drbg);

  if ((ret_state = mbedtls_ssl_setup(&mbedtls_tls_context->ssl,
                                     &mbedtls_tls_context->conf)) != 0) {
    iotc_bsp_debug_format(" failed  ! mbedtls_ssl_setup returned %d",
                          ret_state);
    goto err_handling;
  }

  /* setting the hostname will enable SNI if the SNI is enabled in config.h */
  if ((ret_state = mbedtls_ssl_set_hostname(&mbedtls_tls_context->ssl,
                                            init_params->domain_name)) != 0) {
    iotc_bsp_debug_format(" failed  ! mbedtls_ssl_set_hostname returned %d",
                          ret_state);
    goto err_handling;
  }

  return IOTC_BSP_TLS_STATE_OK;

err_handling:
  return IOTC_BSP_TLS_STATE_INIT_ERROR;
}

iotc_bsp_tls_state_t iotc_bsp_tls_connect(iotc_bsp_tls_context_t* tls_context) {
  assert(NULL != tls_context);

  iotc_bsp_debug_format("[ %s ]", __FUNCTION__);

  mbedtls_tls_context_t* mbedtls_tls_context = tls_context;

  const int ret_state = mbedtls_ssl_handshake(&mbedtls_tls_context->ssl);

  switch (ret_state) {
    case MBEDTLS_ERR_SSL_WANT_READ:
      return IOTC_BSP_TLS_STATE_WANT_READ;
    case MBEDTLS_ERR_SSL_WANT_WRITE:
      return IOTC_BSP_TLS_STATE_WANT_WRITE;
    case MBEDTLS_ERR_X509_CERT_VERIFY_FAILED:
      return IOTC_BSP_TLS_STATE_CERT_ERROR;
    case 0:
      break;
    default:
#if MBEDTLS_DEBUG_LOG
    {
      char errorString[80] = {'\0'};
      mbedtls_strerror(ret_state, errorString, sizeof(errorString));
      iotc_bsp_debug_format("Connection error - %s (%d)", errorString,
                            ret_state);
    }
#endif
      return IOTC_BSP_TLS_STATE_CONNECT_ERROR;
  }

  /* after succesfull connection unload the certificate */
  mbedtls_x509_crt_free(&mbedtls_tls_context->cacert);

  return IOTC_BSP_TLS_STATE_OK;
}

iotc_bsp_tls_state_t iotc_bsp_tls_read(iotc_bsp_tls_context_t* tls_context,
                                       uint8_t* data_ptr, size_t data_size,
                                       int* bytes_read) {
  assert(NULL != tls_context);
  assert(NULL != data_ptr);
  assert(0 < data_size);
  assert(NULL != bytes_read);

  iotc_bsp_debug_format("[ %s ]", __FUNCTION__);

  mbedtls_tls_context_t* mbedtls_tls_context = tls_context;

  int ret_state =
      mbedtls_ssl_read(&mbedtls_tls_context->ssl, data_ptr, data_size);

  if (ret_state > 0) {
    *bytes_read = ret_state;
    return IOTC_BSP_TLS_STATE_OK;
  }

  if (MBEDTLS_ERR_SSL_WANT_READ == ret_state) {
    return IOTC_BSP_TLS_STATE_WANT_READ;
  }

  iotc_bsp_debug_format("mbetls error during reading ret=%d", ret_state);
  return IOTC_BSP_TLS_STATE_READ_ERROR;
}

iotc_bsp_tls_state_t iotc_bsp_tls_write(iotc_bsp_tls_context_t* tls_context,
                                        uint8_t* data_ptr, size_t data_size,
                                        int* bytes_written) {
  assert(NULL != tls_context);
  assert(NULL != data_ptr);
  assert(0 < data_size);
  assert(NULL != bytes_written);

  iotc_bsp_debug_format("[ %s ]", __FUNCTION__);

  mbedtls_tls_context_t* mbedtls_tls_context = tls_context;

  int ret_state =
      mbedtls_ssl_write(&mbedtls_tls_context->ssl, data_ptr, data_size);

  if (ret_state > 0) {
    *bytes_written = ret_state;
    return IOTC_BSP_TLS_STATE_OK;
  }

  if (MBEDTLS_ERR_SSL_WANT_WRITE == ret_state) {
    return IOTC_BSP_TLS_STATE_WANT_WRITE;
  }

  iotc_bsp_debug_format("mbetls error during writing ret=%d", ret_state);
  return IOTC_BSP_TLS_STATE_WRITE_ERROR;
}

int iotc_bsp_tls_pending(iotc_bsp_tls_context_t* tls_context) {
  assert(NULL != tls_context);

  mbedtls_tls_context_t* mbedtls_tls_context = tls_context;

  return (int)mbedtls_ssl_get_bytes_avail(&mbedtls_tls_context->ssl);
}

iotc_bsp_tls_state_t iotc_bsp_tls_cleanup(
    iotc_bsp_tls_context_t** tls_context) {
  assert(NULL != tls_context);

  iotc_bsp_debug_format("[ %s ]", __FUNCTION__);

  mbedtls_tls_context_t* mbedtls_tls_context = *tls_context;

  if (NULL != mbedtls_tls_context) {
    mbedtls_x509_crt_free(&mbedtls_tls_context->cacert);
    mbedtls_ssl_config_free(&mbedtls_tls_context->conf);
    mbedtls_ssl_free(&mbedtls_tls_context->ssl);
    mbedtls_ctr_drbg_free(&mbedtls_tls_context->ctr_drbg);
    mbedtls_entropy_free(&mbedtls_tls_context->entropy);

    mbedtls_free(*tls_context);

    *tls_context = NULL;
  }

  return IOTC_BSP_TLS_STATE_OK;
}
