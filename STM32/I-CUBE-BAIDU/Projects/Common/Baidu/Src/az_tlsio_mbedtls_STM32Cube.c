// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

/* Adaptation to the net.h interface:
 * <h2><center>&copy; Copyright (c) 2017 STMicroelectronics International N.V. 
 * All rights reserved.</center></h2>
 */

#ifdef USE_MBED_TLS

/* Includes ------------------------------------------------------------------*/
#include <stdlib.h>

#include "mbedtls/config.h"
#include "mbedtls/debug.h"
#include "mbedtls/ssl.h"
#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/error.h"
#include "mbedtls/certs.h"
#include "mbedtls/entropy_poll.h"

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "net.h"
#include "mbedtls_net.h"
#include "msg.h"
#include "azure_c_shared_utility/optimize_size.h"
#include "azure_c_shared_utility/tlsio.h"
#include "az_tlsio_mbedtls_STM32Cube.h"

extern int mbedtls_hardware_poll (void *data, unsigned char *output, size_t len, size_t *olen);
extern void *hrng;
extern void *hnet;

/* Private typedef -----------------------------------------------------------*/
typedef enum TLSIO_STATE_ENUM_TAG
{
    TLSIO_STATE_NOT_OPEN,
    TLSIO_STATE_OPENING_UNDERLYING_IO,
    TLSIO_STATE_IN_HANDSHAKE,
    TLSIO_STATE_OPEN,
    TLSIO_STATE_CLOSING,
    TLSIO_STATE_ERROR
} TLSIO_STATE_ENUM;

typedef struct TLS_IO_INSTANCE_TAG
{
    net_sockhnd_t sockhnd;
    ON_BYTES_RECEIVED on_bytes_received;
    ON_IO_OPEN_COMPLETE on_io_open_complete;
    ON_IO_CLOSE_COMPLETE on_io_close_complete;
    ON_IO_ERROR on_io_error;
    void* on_bytes_received_context;
    void* on_io_open_complete_context;
    void* on_io_close_complete_context;
    void* on_io_error_context;
    TLSIO_STATE_ENUM tlsio_state;
    ON_SEND_COMPLETE on_send_complete;
    void* on_send_complete_callback_context;
    mbedtls_entropy_context    entropy;
    mbedtls_ctr_drbg_context   ctr_drbg;
    mbedtls_ssl_context        ssl;
    mbedtls_ssl_config         config;
    mbedtls_x509_crt           cacert;
    mbedtls_ssl_session        ssn;
} TLS_IO_INSTANCE;

static const IO_INTERFACE_DESCRIPTION tlsio_mbedtls_interface_description =
{
    tlsio_mbedtls_STM32Cube_retrieveoptions,
    tlsio_mbedtls_STM32Cube_create,
    tlsio_mbedtls_STM32Cube_destroy,
    tlsio_mbedtls_STM32Cube_open,
    tlsio_mbedtls_STM32Cube_close,
    tlsio_mbedtls_STM32Cube_send,
    tlsio_mbedtls_STM32Cube_dowork,
    tlsio_mbedtls_STM32Cube_setoption
};

/* Private defines -----------------------------------------------------------*/
#define MBED_TLS_DEBUG_ENABLE

/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
#if defined (MBED_TLS_DEBUG_ENABLE)
void mbedtls_debug(void *ctx, int level,const char *file, int line, const char *str );
#endif

/* Functions Definition ------------------------------------------------------*/

#if defined (MBED_TLS_DEBUG_ENABLE)
void mbedtls_debug(void *ctx, int level,const char *file, int line, const char *str )
{
   ((void) level);
   printf("%s (%d): %s\r\n", file,line,str);
}
#endif

#if 0 // currently unused
static void indicate_error(TLS_IO_INSTANCE* tls_io_instance)
{
    if (tls_io_instance->on_io_error != NULL)
    {
        tls_io_instance->on_io_error(tls_io_instance->on_io_error_context);
    }
}
#endif // 0

static void indicate_open_complete(TLS_IO_INSTANCE* tls_io_instance, IO_OPEN_RESULT open_result)
{
    if (tls_io_instance->on_io_open_complete != NULL)
    {
        tls_io_instance->on_io_open_complete(tls_io_instance->on_io_open_complete_context, open_result);
    }
}

static int decode_ssl_received_bytes(TLS_IO_INSTANCE* tls_io_instance)
{
    int result = 0;
    unsigned char buffer[64];
    int rcv_bytes = 1;

    while (rcv_bytes > 0)
    {
        rcv_bytes = mbedtls_ssl_read(&tls_io_instance->ssl, buffer, sizeof(buffer));
        if (rcv_bytes > 0)
        {
            if (tls_io_instance->on_bytes_received != NULL)
            {
                tls_io_instance->on_bytes_received(tls_io_instance->on_bytes_received_context, buffer, rcv_bytes);
            }
        }
    }

    return result;
}

static int on_handshake_done(mbedtls_ssl_context* ssl, void* context)
{
    TLS_IO_INSTANCE* tls_io_instance = (TLS_IO_INSTANCE*)context;

    if (tls_io_instance->tlsio_state == TLSIO_STATE_IN_HANDSHAKE)
    {
        tls_io_instance->tlsio_state = TLSIO_STATE_OPEN;
        indicate_open_complete(tls_io_instance, IO_OPEN_OK);
    }

    return 0;
}
 
static int mbedtls_connect(void* context) {
    TLS_IO_INSTANCE* tls_io_instance = (TLS_IO_INSTANCE*)context;
    int result = 0;

    do {
        result = mbedtls_ssl_handshake(&tls_io_instance->ssl);
    }
    while( result == MBEDTLS_ERR_SSL_WANT_READ || result == MBEDTLS_ERR_SSL_WANT_WRITE );

    if(result == 0)
    {
        on_handshake_done(&tls_io_instance->ssl,(void *)tls_io_instance);
    }

    return result;
}


int g_created = 0;

#if 0 // currently unused
static void on_underlying_io_close_complete(void* context)
{
    TLS_IO_INSTANCE* tls_io_instance = (TLS_IO_INSTANCE*)context;

    if (tls_io_instance->tlsio_state == TLSIO_STATE_CLOSING)
    {
        if (tls_io_instance->on_io_close_complete != NULL)
        {
            tls_io_instance->on_io_close_complete(tls_io_instance->on_io_close_complete_context);
        }
//	if (g_created)
//		tls_io_instance->tlsio_state = TLSIO_STATE_NOT_OPEN;
    }
}
#endif // 0

static void mbedtls_init(void *instance,const char *host) {
    TLS_IO_INSTANCE *result = (TLS_IO_INSTANCE *)instance;
    char *pers = "azure_iot_client";

    // mbedTLS initialize...
    mbedtls_entropy_init(&result->entropy);
    mbedtls_ctr_drbg_init(&result->ctr_drbg);
    mbedtls_ssl_init(&result->ssl);
    mbedtls_ssl_session_init(&result->ssn);
    mbedtls_ssl_config_init(&result->config);
    mbedtls_x509_crt_init(&result->cacert);
    mbedtls_entropy_add_source(&result->entropy,mbedtls_hardware_poll,(void*)&hrng,1,MBEDTLS_ENTROPY_SOURCE_STRONG);
    mbedtls_ctr_drbg_seed(&result->ctr_drbg,mbedtls_entropy_func,&result->entropy,(const unsigned char *)pers,strlen(pers));
    mbedtls_ssl_config_defaults(&result->config,MBEDTLS_SSL_IS_CLIENT,MBEDTLS_SSL_TRANSPORT_STREAM,MBEDTLS_SSL_PRESET_DEFAULT);
    mbedtls_ssl_conf_rng(&result->config,mbedtls_ctr_drbg_random,&result->ctr_drbg);
    mbedtls_ssl_conf_authmode(&result->config,MBEDTLS_SSL_VERIFY_REQUIRED);
    mbedtls_ssl_conf_min_version(&result->config,MBEDTLS_SSL_MAJOR_VERSION_3,MBEDTLS_SSL_MINOR_VERSION_3);          // v1.2
    mbedtls_ssl_set_bio(&result->ssl,result->sockhnd,mbedtls_net_send,mbedtls_net_recv,NULL);
    mbedtls_ssl_set_hostname(&result->ssl,host);
    mbedtls_ssl_set_session(&result->ssl,&result->ssn);

#if defined (MBED_TLS_DEBUG_ENABLE)
    mbedtls_ssl_conf_dbg(&result->config, mbedtls_debug,stdout);
    mbedtls_debug_set_threshold(1);
#endif

    mbedtls_ssl_setup(&result->ssl,&result->config);
}

OPTIONHANDLER_HANDLE tlsio_mbedtls_STM32Cube_retrieveoptions(CONCRETE_IO_HANDLE handle)
{
    (void)handle;
    return NULL;
}

CONCRETE_IO_HANDLE tlsio_mbedtls_STM32Cube_create(void* io_create_parameters)
{
    TLSIO_CONFIG* tls_io_config = io_create_parameters;
    TLS_IO_INSTANCE* result;
    bool connected = false;
    int ret = 0;
    
    if (tls_io_config == NULL)
    {
        LogError("NULL tls_io_config");
        result = NULL;
    }
    else
    {
        result = malloc(sizeof(TLS_IO_INSTANCE));
        if (result != NULL)
        {
            result->sockhnd = NULL;
            
            result->on_bytes_received = NULL;
            result->on_bytes_received_context = NULL;

            result->on_io_open_complete = NULL;
            result->on_io_open_complete_context = NULL;

            result->on_io_close_complete = NULL;
            result->on_io_close_complete_context = NULL;

            result->on_io_error = NULL;
            result->on_io_error_context = NULL;

            
            if((ret = net_sock_create(hnet, &result->sockhnd, NET_PROTO_TCP)) != NET_OK )
            {
              msg_error(" failed to create a TCP socket  ! net_sock_create %d\n", ret);
            }
            else
            {
               if( (ret = net_sock_setopt(result->sockhnd, "sock_noblocking", NULL, 0)) != NET_OK )
               {
                 msg_error(" failed to set the TCP socket noblocking ! net_sock_setopt %d\n", ret);
               }
               else
               {
                  if((ret = net_sock_open(result->sockhnd, tls_io_config->hostname, tls_io_config->port)) != NET_OK)
                  {
                    msg_error(" failed to connect to %s:%d  ! net_sock_open %d\n", tls_io_config->hostname, tls_io_config->port, ret);
                  }
                  else
                  {
                    connected = true;
                  }
               }
            }
            
            if (connected == false)
            {
                LogError("socket xio create failed");
                free(result);
                result = NULL;
            }
            else
            {    
                result->on_send_complete = NULL;
                result->on_send_complete_callback_context = NULL;
                
                // mbedTLS initialize
                mbedtls_init((void *)result,tls_io_config->hostname);
                result->tlsio_state = TLSIO_STATE_NOT_OPEN;
                g_created = 1;
            }
        }
    }

    return result;
}

void tlsio_mbedtls_STM32Cube_destroy(CONCRETE_IO_HANDLE tls_io)
{
    int ret = 0;
    if (tls_io != NULL)
    {
        TLS_IO_INSTANCE* tls_io_instance = (TLS_IO_INSTANCE*)tls_io;
        net_sockhnd_t socket = tls_io_instance->sockhnd;

        // mbedTLS cleanup...
        mbedtls_ssl_close_notify(&tls_io_instance->ssl);
        mbedtls_ssl_free(&tls_io_instance->ssl);
        mbedtls_ssl_config_free(&tls_io_instance->config);
        mbedtls_x509_crt_free(&tls_io_instance->cacert);
        mbedtls_ctr_drbg_free(&tls_io_instance->ctr_drbg);
        mbedtls_entropy_free(&tls_io_instance->entropy);

        ret = net_sock_close(socket);
        ret |= net_sock_destroy(socket);
        if (ret != NET_OK)
        {
          msg_error("net_sock_close() or net_sock_destroy() failed.\n");
        }
        
        free(tls_io);
    }
    g_created = 0;
}

int tlsio_mbedtls_STM32Cube_open(CONCRETE_IO_HANDLE tls_io, ON_IO_OPEN_COMPLETE on_io_open_complete, void* on_io_open_complete_context, ON_BYTES_RECEIVED on_bytes_received, void* on_bytes_received_context, ON_IO_ERROR on_io_error, void* on_io_error_context)
{
    int result = 0;

    if (tls_io == NULL)
    {
        LogError("NULL tls_io");
        result = __FAILURE__;
    }
    else
    {
        TLS_IO_INSTANCE* tls_io_instance = (TLS_IO_INSTANCE*)tls_io;

        if (tls_io_instance->tlsio_state != TLSIO_STATE_NOT_OPEN)
        {
            LogError("IO should not be open: %d\n", tls_io_instance->tlsio_state);
            result =  __FAILURE__;
        }
        else
        {
            tls_io_instance->on_bytes_received = on_bytes_received;
            tls_io_instance->on_bytes_received_context = on_bytes_received_context;

            tls_io_instance->on_io_open_complete = on_io_open_complete;
            tls_io_instance->on_io_open_complete_context = on_io_open_complete_context;

            tls_io_instance->on_io_error = on_io_error;
            tls_io_instance->on_io_error_context = on_io_error_context;

            tls_io_instance->tlsio_state = TLSIO_STATE_OPENING_UNDERLYING_IO;
            tls_io_instance->tlsio_state = TLSIO_STATE_IN_HANDSHAKE;

            result = mbedtls_connect(tls_io_instance);
            if (result != 0)
            {
                tls_io_instance->tlsio_state = TLSIO_STATE_ERROR;
                indicate_open_complete(tls_io_instance, IO_OPEN_ERROR);
            }

            result = 0;
        }
    }

    return result;
}

int tlsio_mbedtls_STM32Cube_close(CONCRETE_IO_HANDLE tls_io, ON_IO_CLOSE_COMPLETE on_io_close_complete, void* callback_context)
{
    int result = 0;

    if (tls_io == NULL)
    {
        result = __FAILURE__;
    }
    else
    {
        TLS_IO_INSTANCE* tls_io_instance = (TLS_IO_INSTANCE*)tls_io;

        if ((tls_io_instance->tlsio_state == TLSIO_STATE_NOT_OPEN) ||
            (tls_io_instance->tlsio_state == TLSIO_STATE_CLOSING))
        {
            result = __FAILURE__;
        }
        else
        {
            tls_io_instance->tlsio_state = TLSIO_STATE_CLOSING;
            tls_io_instance->on_io_close_complete = on_io_close_complete;
            tls_io_instance->on_io_close_complete_context = callback_context;
            /* Note: The underlying socket cannot be closed at this stage because 
             * mbedtls_ssl_close_notify() which is called from tlsio_mbedtls_STM32Cube_destroy()
             * needs the connection.
             */
            result = 0;
        }
    }

    return result;
}

int tlsio_mbedtls_STM32Cube_send(CONCRETE_IO_HANDLE tls_io, const void* buffer, size_t size, ON_SEND_COMPLETE on_send_complete, void* callback_context)
{
    int result;

    if (tls_io == NULL)
    {
        result = __FAILURE__;
    }
    else
    {
        TLS_IO_INSTANCE* tls_io_instance = (TLS_IO_INSTANCE*)tls_io;

        if (tls_io_instance->tlsio_state != TLSIO_STATE_OPEN)
        {
            result = __FAILURE__;
        }
        else
        {
            tls_io_instance->on_send_complete = on_send_complete;
            tls_io_instance->on_send_complete_callback_context = callback_context;

            int res = mbedtls_ssl_write(&tls_io_instance->ssl, buffer, size);
            if (res != size)
            {
                result = __FAILURE__;
            }
            else
            {
                result = 0;
            }
        }
    }

    return result;
}

void tlsio_mbedtls_STM32Cube_dowork(CONCRETE_IO_HANDLE tls_io)
{
    if (tls_io != NULL)
    {
        TLS_IO_INSTANCE* tls_io_instance = (TLS_IO_INSTANCE*)tls_io;

        if ((tls_io_instance->tlsio_state != TLSIO_STATE_NOT_OPEN) &&
            (tls_io_instance->tlsio_state != TLSIO_STATE_ERROR))
        {
            decode_ssl_received_bytes(tls_io_instance);
            /* Note: The Wifi LL is not threaded, and there is no "posted work".
             * The receive low level processing is done in the IT handler and in
             * the mbedTLS receive wrapper.
             */
            //xio_dowork(tls_io_instance->socket_io);
        }
    }
}

const IO_INTERFACE_DESCRIPTION* tlsio_mbedtls_STM32Cube_get_interface_description(void)
{
    return &tlsio_mbedtls_interface_description;
}

int tlsio_mbedtls_STM32Cube_setoption(CONCRETE_IO_HANDLE tls_io, const char* optionName, const void* value)
{
    int result = 0;

    if (tls_io == NULL || optionName == NULL)
    {
        result = __FAILURE__;
    }
    else
    {
        TLS_IO_INSTANCE* tls_io_instance = (TLS_IO_INSTANCE*)tls_io;

        if (strcmp("TrustedCerts", optionName) == 0)
        {
            result = mbedtls_x509_crt_parse(&tls_io_instance->cacert,(const unsigned char *)value,(int)(strlen(value)+1));
            if( result != 0 )
            {
                result = __FAILURE__;
            }
            else
            {
                mbedtls_ssl_conf_ca_chain(&tls_io_instance->config,&tls_io_instance->cacert,NULL);
            }
        }
        else
        {
          /* Note: Setting the socket timeout could be considered, but its implementation would 
           * conflict with the mbedTLS wrapper read timeout when the blocking interface is use.
           */
            //result = xio_setoption(tls_io_instance->socket_io, optionName, value);
          msg_error("Unsupported option setting.\n");
        }
    }

    return result;
}

#endif // USE_MBED_TLS
