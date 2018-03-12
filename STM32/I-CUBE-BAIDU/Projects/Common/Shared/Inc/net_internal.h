/**
  ******************************************************************************
  * @file    net_internal.h
  * @author  MCD Application Team
  * @version V0.0.1
  * @date    11-August-2017
  * @brief   Network abstraction at transport layer level. Internal definitions.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2017 STMicroelectronics International N.V. 
  * All rights reserved.</center></h2>
  *
  * Redistribution and use in source and binary forms, with or without 
  * modification, are permitted, provided that the following conditions are met:
  *
  * 1. Redistribution of source code must retain the above copyright notice, 
  *    this list of conditions and the following disclaimer.
  * 2. Redistributions in binary form must reproduce the above copyright notice,
  *    this list of conditions and the following disclaimer in the documentation
  *    and/or other materials provided with the distribution.
  * 3. Neither the name of STMicroelectronics nor the names of other 
  *    contributors to this software may be used to endorse or promote products 
  *    derived from this software without specific written permission.
  * 4. This software, including modifications and/or derivative works of this 
  *    software, must execute solely and exclusively on microcontroller or
  *    microprocessor devices manufactured by or for STMicroelectronics.
  * 5. Redistribution and use of this software other than as permitted under 
  *    this license is void and will automatically terminate your rights under 
  *    this license. 
  *
  * THIS SOFTWARE IS PROVIDED BY STMICROELECTRONICS AND CONTRIBUTORS "AS IS" 
  * AND ANY EXPRESS, IMPLIED OR STATUTORY WARRANTIES, INCLUDING, BUT NOT 
  * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A 
  * PARTICULAR PURPOSE AND NON-INFRINGEMENT OF THIRD PARTY INTELLECTUAL PROPERTY
  * RIGHTS ARE DISCLAIMED TO THE FULLEST EXTENT PERMITTED BY LAW. IN NO EVENT 
  * SHALL STMICROELECTRONICS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
  * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
  * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, 
  * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
  * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING 
  * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
  * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */
#ifndef __NET_INTERNAL_H__
#define __NET_INTERNAL_H__

/* Includes ------------------------------------------------------------------*/
#include <stdlib.h>   /* atoi() */
#include "main.h"
#include "msg.h"
#include "net.h"

#ifdef USE_MBED_TLS

#if !defined(MBEDTLS_CONFIG_FILE)
#include "mbedtls/config.h"
#else
#include MBEDTLS_CONFIG_FILE
#endif

#include "mbedtls/platform.h"
#include "mbedtls/ssl.h"
#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/certs.h"
#include "mbedtls/x509.h"
#include "mbedtls/error.h"
#include "mbedtls/debug.h"
#include "mbedtls/timing.h"
#include "mbedtls_net.h"  /* mbedTLS data callbacks, implemented on WiFi_LL */
#include "heap.h"         /* memory allocator overloading */

#endif /* USE_MBED_TLS */


#ifdef USE_WIFI
#include "wifi.h"

//exported functions
int net_sock_create_tcp_wifi(net_hnd_t nethnd, net_sockhnd_t * sockhnd, net_proto_t proto);
int net_sock_open_tcp_wifi(net_sockhnd_t sockhnd, const char * hostname, int dstport);
int net_sock_recv_tcp_wifi(net_sockhnd_t sockhnd, const uint8_t * buf, size_t len);
int net_sock_send_tcp_wifi(net_sockhnd_t sockhnd, const uint8_t * buf, size_t len);
int net_sock_close_tcp_wifi(net_sockhnd_t sockhnd);
int net_sock_destroy_tcp_wifi(net_sockhnd_t sockhnd);
#endif

#ifdef USE_C2C
#include "c2c.h"
#endif

#ifdef USE_LWIP
#include "lwip/netif.h"
#endif /* USE_LWIP_ETH */

/* Private defines -----------------------------------------------------------*/
#define NET_DEFAULT_BLOCKING_WRITE_TIMEOUT  2000
#define NET_DEFAULT_BLOCKING_READ_TIMEOUT   2000
#define NET_DEFAULT_BLOCKING                true


/* Private typedef -----------------------------------------------------------*/
typedef struct net_ctxt_s net_ctxt_t;
typedef struct net_sock_ctxt_s net_sock_ctxt_t;

typedef int net_sock_create_t(net_hnd_t nethnd, net_sockhnd_t * sockhnd, net_proto_t proto);
typedef int net_sock_open_t(net_sockhnd_t sockhnd, const char * hostname, int dstport);
typedef int net_sock_recv_t(net_sockhnd_t sockhnd, const uint8_t * buf, size_t len);
typedef int net_sock_send_t(net_sockhnd_t sockhnd, const uint8_t * buf, size_t len);
typedef int net_sock_close_t(net_sockhnd_t sockhnd);
typedef int net_sock_destroy_t(net_sockhnd_t sockhnd);

typedef struct {
  net_sock_create_t  * create;
  net_sock_open_t    * open;
  net_sock_recv_t    * recv;
  net_sock_send_t    * send;
  net_sock_close_t   * close;
  net_sock_destroy_t * destroy;
} net_sock_methods_t;

#ifdef USE_MBED_TLS
typedef struct {
  unsigned char * tls_ca_certs; /**< Socket option. */
  unsigned char * tls_ca_crl;   /**< Socket option. */
  unsigned char * tls_dev_cert; /**< Socket option. */
  unsigned char * tls_dev_key;  /**< Socket option. */
  uint8_t * tls_dev_pwd;        /**< Socket option. */
  size_t tls_dev_pwd_len;       /**< Socket option / meta. */
  bool tls_srv_verification;    /**< Socket option. */
  char * tls_srv_name;          /**< Socket option. */
  /* mbedTLS objects */
  mbedtls_entropy_context entropy;
	mbedtls_ctr_drbg_context ctr_drbg;
	mbedtls_ssl_context ssl;
	mbedtls_ssl_config conf;
	uint32_t flags;
	mbedtls_x509_crt cacert;
	mbedtls_x509_crl cacrl;               /** Optional certificate revocation list */
	mbedtls_x509_crt clicert;
	mbedtls_pk_context pkey;
} net_tls_data_t;
#endif /* USE_MBED_TLS */

/** Network socket context. */
struct net_sock_ctxt_s {
  net_ctxt_t * net;                     /**< Network interface to which the socket is attached. */
  net_sock_ctxt_t * next;               /**< Linear linked list (not circular) of the sockets opened on the same network interface. */
  net_sock_methods_t methods;           /**< Proto-specific function pointers. */
  net_proto_t proto;                    /**< Socket type. */
  bool blocking;                        /**< Socket option. */
  uint16_t read_timeout;                /**< Socket option. */
  uint16_t write_timeout;               /**< Socket option. */
#ifdef USE_MBED_TLS
  net_tls_data_t * tlsData;             /**< TLS specific context. */
#endif  /* USE_MBED_TLS */
  net_sockhnd_t underlying_sock_ctxt;   /**< Socket context of the underlying software layer. */
};

/** Network interface context. */
struct net_ctxt_s {
  net_if_t itf;
  net_sock_ctxt_t * sock_list;  /**< Linked list of the sockets opened on the network interface. */
#ifdef USE_LWIP
  struct netif lwip_netif;       /**< LwIP interface context. */
#endif /* USE_LWIP */
};

/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
#define MIN(a,b)  ((a) < (b) ? (a) : (b))
#define MAX(a,b)  ((a) < (b) ? (b) : (a))

#define net_malloc(a) malloc((a))
#define net_free(a)   free((a))

int32_t net_timeout_left_ms(uint32_t init, uint32_t now, uint32_t timeout);
#ifdef USE_MBED_TLS
extern int mbedtls_hardware_poll( void *data, unsigned char *output, size_t len, size_t *olen );
#endif /* USE_MBED_TLS */

#endif /* __NET_INTERNAL_H__ */
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
