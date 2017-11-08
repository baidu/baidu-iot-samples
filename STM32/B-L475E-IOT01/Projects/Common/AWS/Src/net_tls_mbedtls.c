/**
  ******************************************************************************
  * @file    net_tls_mbedtls.c
  * @author  MCD Application Team
  * @version V1.0.0
  * @date    11-July-2017
  * @brief   Network abstraction at transport layer level. mbedTLS implementation.
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
#ifdef USE_MBED_TLS
/* Includes ------------------------------------------------------------------*/
#include "net_internal.h"

/* Private defines -----------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
int net_sock_create_mbedtls(net_hnd_t nethnd, net_sockhnd_t * sockhnd, net_proto_t proto);
int net_sock_open_mbedtls(net_sockhnd_t sockhnd, const char * hostname, int dstport);
int net_sock_recv_mbedtls(net_sockhnd_t sockhnd, uint8_t * const buf, size_t len);
int net_sock_send_mbedtls(net_sockhnd_t sockhnd, const uint8_t * buf, size_t len);
int net_sock_close_mbedtls(net_sockhnd_t sockhnd);
int net_sock_destroy_mbedtls(net_sockhnd_t sockhnd);

static void my_debug( void *ctx, int level, const char *file, int line, const char *str );
static void internal_close(net_sock_ctxt_t * sock);

/* Functions Definition ------------------------------------------------------*/

int net_sock_create_mbedtls(net_hnd_t nethnd, net_sockhnd_t * sockhnd, net_proto_t proto)
{
  int rc = NET_ERR;
  net_ctxt_t *ctxt = (net_ctxt_t *) nethnd;
  net_sock_ctxt_t * sock = NULL;
  net_tls_data_t * tlsData = NULL;
    
  sock = net_malloc(sizeof(net_sock_ctxt_t));
  if (sock == NULL) 
  {
    msg_error("net_sock_create allocation 1 failed.\n");
    rc = NET_ERR;
  } 
  else
  {
    memset(sock, 0, sizeof(net_sock_ctxt_t));
    tlsData = net_malloc(sizeof(net_tls_data_t));
    if (tlsData == NULL)
    {
      msg_error("net_sock_create allocation 2 failed.\n");
      net_free(sock);
      rc = NET_ERR;
    }
    else
    {
      /* Generic socket */
      memset(tlsData, 0, sizeof(net_tls_data_t));
      sock->net = ctxt;
      sock->next = ctxt->sock_list;
      sock->methods.create  = (net_sock_create_mbedtls);  /* This reference is not supposed to be used. */
      sock->methods.open    = (net_sock_open_mbedtls);
      sock->methods.recv    = (net_sock_recv_mbedtls);
      sock->methods.send    = (net_sock_send_mbedtls);
      sock->methods.close   = (net_sock_close_mbedtls);
      sock->methods.destroy = (net_sock_destroy_mbedtls);
      sock->proto = proto;
      sock->blocking        = NET_BLOCKING;       /* Value by default */
      sock->read_timeout    = NET_READ_TIMEOUT;   /* Value by default */
      sock->write_timeout   = NET_WRITE_TIMEOUT;  /* Value by default */
      sock->tlsData = tlsData;
      ctxt->sock_list = sock;                 /* Insert at the head of the list */
      *sockhnd = (net_sockhnd_t) sock;
      rc = NET_OK;
    }
  }
  
  return rc;
}


int net_sock_open_mbedtls(net_sockhnd_t sockhnd, const char * hostname, int dstport)
{
  int rc = NET_ERR;
  net_sock_ctxt_t *sock = (net_sock_ctxt_t * ) sockhnd;
  net_tls_data_t * tlsData = sock->tlsData;

  /* mbedTLS instance */
  int ret = 0;
  const unsigned char *pers = (unsigned char *)"net_tls";
#if 0 // 2k should be large enough! Not needed anyway.
#ifdef msg_debug
  unsigned char buf[MBEDTLS_SSL_MAX_CONTENT_LEN + 1];
#endif
#endif // 0

  mbedtls_platform_set_calloc_free(heap_alloc, heap_free);  /* Common to all sockets. */
  mbedtls_ssl_config_init(&tlsData->conf);
  mbedtls_ssl_conf_dbg(&tlsData->conf, my_debug, stdout);
  mbedtls_ctr_drbg_init(&tlsData->ctr_drbg);
  mbedtls_x509_crt_init(&tlsData->cacert);
  if (tlsData->tls_dev_cert != NULL)
  {
    mbedtls_x509_crt_init(&tlsData->clicert);
  }
  if (tlsData->tls_dev_key != NULL)
  {
    mbedtls_pk_init(&tlsData->pkey);
  }
  mbedtls_debug_set_threshold(1);

  /* Entropy generator init */
  mbedtls_entropy_init(&tlsData->entropy);
  if( (ret = mbedtls_entropy_add_source(&tlsData->entropy, mbedtls_hardware_poll, (void*)&hrng, 1, MBEDTLS_ENTROPY_SOURCE_STRONG)) != 0 )
  {
    msg_error( " failed\n  ! mbedtls_entropy_add_source returned -0x%x\n", -ret );
    internal_close(sock);
    return NET_ERR;
  }
  if( (ret = mbedtls_ctr_drbg_seed(&tlsData->ctr_drbg, mbedtls_entropy_func, &tlsData->entropy, pers, strlen((char const *)pers))) != 0 )
  {
    msg_error(" failed\n  ! mbedtls_ctr_drbg_seed returned -0x%x\n", -ret);
    internal_close(sock);
    return NET_ERR;
  }

  /* Root CA */
  if( (ret = mbedtls_x509_crt_parse(&tlsData->cacert, (unsigned char const *)tlsData->tls_ca_certs, strlen((char const *) tlsData->tls_ca_certs) + 1)) != 0 )
  { 
    msg_error(" failed\n  !  mbedtls_x509_crt_parse returned -0x%x while parsing root cert\n", -ret);
    internal_close(sock);
    return NET_ERR;
  }
  if (tlsData->tls_ca_crl != NULL)
  {
    if( (ret = mbedtls_x509_crl_parse(&tlsData->cacrl, (unsigned char const *)tlsData->tls_ca_crl, strlen((char const *) tlsData->tls_ca_crl) + 1)) != 0 )
    { 
      msg_error(" failed\n  !  mbedtls_x509_crt_parse returned -0x%x while parsing the cert revocation list\n", -ret);
      internal_close(sock);
      return NET_ERR;
    }
  }

  /* Client cert. and key */
  if( (tlsData->tls_dev_cert != NULL) && (tlsData->tls_dev_key != NULL) )
  {
    if( (ret = mbedtls_x509_crt_parse(&tlsData->clicert, (unsigned char const *)tlsData->tls_dev_cert, strlen((char const *)tlsData->tls_dev_cert) + 1)) != 0 )
    {
      msg_error(" failed\n  !  mbedtls_x509_crt_parse returned -0x%x while parsing device cert\n", -ret);
      internal_close(sock);
      return NET_ERR;
    }
#ifdef FIREWALL_MBEDLIB
    /* Note: The firewall mbedTLS protection does not allow to protect the device private key with a password. */
    if( (ret = mbedtls_firewall_pk_parse_key(&tlsData->pkey, (unsigned char const *)tlsData->tls_dev_key, (size_t)0 ,
           (unsigned char const *)"", 0)) != 0 )
    {
      msg_error(" failed\n  !  mbedtls_pk_parse_key returned -0x%x while parsing private key\n\n", -ret);
      internal_close(sock);
      return NET_ERR;
    }
    /* the key is converted to an RSA structure here :  pk_parse_key_pkcs1_der
       the info pointer are changed in pk_wrap.c*/
    extern mbedtls_pk_info_t mbedtls_firewall_info;
    tlsData->pkey.pk_info = &mbedtls_firewall_info;
#else /* FIREWALL_MBEDLIB */
    if( (ret = mbedtls_pk_parse_key(&tlsData->pkey, (unsigned char const *)tlsData->tls_dev_key, strlen((char const *)tlsData->tls_dev_key) + 1,
           (unsigned char const *)tlsData->tls_dev_pwd, tlsData->tls_dev_pwd_len)) != 0 )
    {
      msg_error(" failed\n  !  mbedtls_pk_parse_key returned -0x%x while parsing private key\n\n", -ret);
      internal_close(sock);
      return NET_ERR;
    }
#endif  /* FIREWALL_MBEDLIB */
  }
  
  /* TCP Connection */
  msg_debug("  . Connecting to %s:%d...", hostname, dstport);
  if( (ret = net_sock_create(hnet, &sock->underlying_sock_ctxt, NET_PROTO_TCP)) != NET_OK )
  {
    msg_error(" failed to create a TCP socket  ! net_sock_create returned %d\n", ret);
    internal_close(sock);
    return NET_ERR;
  }
  
  if( (ret = net_sock_setopt(sock->underlying_sock_ctxt, "sock_noblocking", NULL, 0)) != NET_OK )
  {
    msg_error(" failed setting the noblocking socket option.\n");
    if (net_sock_destroy(sock->underlying_sock_ctxt) != NET_OK )
    {
      msg_error("Failed destroying the socket.\n");
    }
    internal_close(sock);
    return NET_ERR;
  }
 
  /* TLS Connection */
  if( (ret = mbedtls_ssl_config_defaults(&tlsData->conf, MBEDTLS_SSL_IS_CLIENT, MBEDTLS_SSL_TRANSPORT_STREAM, MBEDTLS_SSL_PRESET_DEFAULT)) != 0)
  {
    msg_error(" failed\n  ! mbedtls_ssl_config_defaults returned -0x%x\n\n", -ret);
    internal_close(sock);
    return NET_ERR;
  }

#if 0
  mbedtls_ssl_conf_cert_profile(&sock->conf, &mbedtls_x509_crt_amazon_suite);
  // TODO: Allow the user to select a TLS profile?
#endif
  /* Only for debug
   * mbedtls_ssl_conf_verify(&(tlsDataParams->conf), _iot_tls_verify_cert, NULL); */
  if(tlsData->tls_srv_verification == true)
  {
    mbedtls_ssl_conf_authmode(&tlsData->conf, MBEDTLS_SSL_VERIFY_REQUIRED);
  }
  else
  {
    mbedtls_ssl_conf_authmode(&tlsData->conf, MBEDTLS_SSL_VERIFY_OPTIONAL);
  }

  mbedtls_ssl_conf_rng(&tlsData->conf, mbedtls_ctr_drbg_random, &tlsData->ctr_drbg);
  mbedtls_ssl_conf_ca_chain(&tlsData->conf, &tlsData->cacert, (tlsData->tls_ca_crl != NULL) ? &tlsData->cacrl : NULL);

  if( (tlsData->tls_dev_cert != NULL) && (tlsData->tls_dev_key != NULL) )
  {
    if( (ret = mbedtls_ssl_conf_own_cert(&tlsData->conf, &tlsData->clicert, &tlsData->pkey)) != 0)
    {
      msg_error(" failed\n  ! mbedtls_ssl_conf_own_cert returned -0x%x\n\n", -ret);
      internal_close(sock);
      return NET_ERR;
    }
  }

  mbedtls_ssl_conf_read_timeout(&tlsData->conf, sock->read_timeout);

  if( (ret = mbedtls_ssl_setup(&tlsData->ssl, &tlsData->conf)) != 0 )
  {
    msg_error(" failed\n  ! mbedtls_ssl_setup returned -0x%x\n\n", -ret);
    
    if (net_sock_destroy(sock->underlying_sock_ctxt) != NET_OK )
    {
      msg_error("Failed destroying the socket.\n");
    }
    internal_close(sock);
    return NET_ERR;
  }
  if(tlsData->tls_srv_name != NULL)
  {
    if( (ret = mbedtls_ssl_set_hostname(&tlsData->ssl, tlsData->tls_srv_name)) != 0 )
    {
      msg_error(" failed\n  ! mbedtls_ssl_set_hostname returned %d\n\n", ret);
      internal_close(sock);
      return NET_ERR;
    }
  }

  msg_debug("\n\nSSL state connect : %d ", sock->tlsData->ssl.state);

  if( (ret = net_sock_open(sock->underlying_sock_ctxt, hostname, dstport)) != NET_OK )
  {
    msg_error(" failed to connect to %s:%d  ! net_sock_open returned %d\n", hostname, dstport, ret);
    if (net_sock_destroy(sock->underlying_sock_ctxt) != NET_OK )
    {
      msg_error("Failed destroying the socket.\n");
    }
    internal_close(sock);
    return NET_ERR;
  }
  
  mbedtls_ssl_set_bio(&tlsData->ssl, (void *) sock->underlying_sock_ctxt, mbedtls_net_send, mbedtls_net_recv, NULL);
  /* TODO: Move to a more generic, blocking read interface. */

  msg_debug("\n\nSSL state connect : %d ", sock->tlsData->ssl.state);
  msg_debug("  . Performing the SSL/TLS handshake...");

  while( (ret = mbedtls_ssl_handshake(&tlsData->ssl)) != 0 )
  {
    if( (ret != MBEDTLS_ERR_SSL_WANT_READ) && (ret != MBEDTLS_ERR_SSL_WANT_WRITE) )
    {
      if( (tlsData->flags = mbedtls_ssl_get_verify_result(&tlsData->ssl)) != 0 )
      {
        char vrfy_buf[512];
        mbedtls_x509_crt_verify_info(vrfy_buf, sizeof(vrfy_buf), "  ! ", tlsData->flags);
        if (tlsData->tls_srv_verification == true)
        {
          msg_error("Server verification:\n%s\n", vrfy_buf);
        }
        else
        {
          msg_info("Server verification:\n%s\n", vrfy_buf);
        }
      }
      msg_error(" failed\n  ! mbedtls_ssl_handshake returned -0x%x\n", -ret);
     
      if (net_sock_close(sock->underlying_sock_ctxt) != NET_OK )
      {
        msg_error("Failed closing the socket.\n");
      }
      if (net_sock_destroy(sock->underlying_sock_ctxt) != NET_OK )
      {
        msg_error("Failed destroying the socket.\n");
      }
      internal_close(sock);
      
      return (ret == MBEDTLS_ERR_X509_CERT_VERIFY_FAILED) ? NET_AUTH : NET_ERR;
    }
  }

  msg_debug(" ok\n    [ Protocol is %s ]\n    [ Ciphersuite is %s ]\n",
     mbedtls_ssl_get_version(&sock->tlsData->ssl),
     mbedtls_ssl_get_ciphersuite(&sock->tlsData->ssl));

  if( (ret = mbedtls_ssl_get_record_expansion(&tlsData->ssl)) >= 0)
  {
    msg_debug("    [ Record expansion is %d ]\n", ret);
  }
  else
  {
    msg_debug("    [ Record expansion is unknown (compression) ]\n");
  }

  msg_debug("  . Verifying peer X.509 certificate...");


#if 0
#ifdef msg_debug
  if (mbedtls_ssl_get_peer_cert(&sock->ssl) != NULL)
  {
    msg_debug("  . Peer certificate information    ...\n");
    mbedtls_x509_crt_info((char *) buf, sizeof(buf) - 1, "      ", mbedtls_ssl_get_peer_cert(&sock->ssl));
    msg_debug("%s\n", buf);
  }
#endif
#endif // 0
   
  rc = NET_OK;
    
  return rc;
}


int net_sock_recv_mbedtls(net_sockhnd_t sockhnd, uint8_t * buf, size_t len)
{
  int rc = 0;
  net_sock_ctxt_t *sock = (net_sock_ctxt_t * ) sockhnd;
  net_tls_data_t * tlsData = sock->tlsData;
  int read = 0;
  int ret = 0;
  uint32_t start_time = HAL_GetTick();
  
  do
  {
    if ( (sock->blocking == true) && (net_timeout_left_ms(start_time, HAL_GetTick(), sock->read_timeout) <= 0) )
    {
      rc = NET_TIMEOUT;
      break;
    }
    
    ret = mbedtls_ssl_read(&tlsData->ssl, buf + read, len - read);
    if (ret > 0)
    {
      read += ret;
    }
    else
    {
      switch (ret)
      {
        case 0:
          rc = NET_EOF;
          break;
        case MBEDTLS_ERR_SSL_WANT_READ:
        case MBEDTLS_ERR_SSL_WANT_WRITE:
          /* Nothing to do. The while() below handles the case. */
          break;
        case MBEDTLS_ERR_SSL_TIMEOUT: /* In case a blocking read function was passed through mbedtls_ssl_set_bio() */
          rc = NET_TIMEOUT;
          break;
        default:
          msg_error(" failed\n  ! mbedtls_ssl_read returned -0x%x\n\n", -ret);
          rc = NET_ERR;
      }
    }
  } while ( ((ret == MBEDTLS_ERR_SSL_WANT_READ) || (ret == MBEDTLS_ERR_SSL_WANT_WRITE)) && (sock->blocking == true) && (rc == 0) );
    
  return (rc < 0) ? rc : read;
}


int net_sock_send_mbedtls( net_sockhnd_t sockhnd, const uint8_t * buf, size_t len)
{
  int rc = 0;
  net_sock_ctxt_t *sock = (net_sock_ctxt_t * ) sockhnd;
  net_tls_data_t * tlsData = sock->tlsData;
  int sent = 0;
  int ret = 0;
  uint32_t start_time = HAL_GetTick();
  
  do
  {
    if ( (sock->blocking == true) && (net_timeout_left_ms(start_time, HAL_GetTick(), sock->write_timeout) <= 0) )
    {
      rc = NET_TIMEOUT;
      break;
    }
    
    ret = mbedtls_ssl_write(&tlsData->ssl, buf + sent, len - sent);
    if (ret > 0)
    {
      sent += ret;
    }
    else
    {
      switch(ret)
      {
        case 0:
          rc = NET_EOF;
          break;
        case MBEDTLS_ERR_SSL_WANT_READ:
        case MBEDTLS_ERR_SSL_WANT_WRITE:
          /* Nothing to do. The while() below handles the case. */
          break;
        default:
          msg_error(" failed\n  ! mbedtls_ssl_write returned -0x%x\n\n", -ret);
          rc = NET_ERR;
      }
    }
  } while ( ((ret == MBEDTLS_ERR_SSL_WANT_READ) || (ret == MBEDTLS_ERR_SSL_WANT_WRITE)) && (sock->blocking == true) && (rc == 0));
  
  return (rc < 0) ? rc : sent;
}


int net_sock_close_mbedtls(net_sockhnd_t sockhnd)
{
  int rc = NET_ERR;
  net_sock_ctxt_t *sock = (net_sock_ctxt_t * ) sockhnd;
  int ret = 0;
#if 0 /* Closure notification is probably not applicable on client side. */ 
  net_tls_data_t * tlsData = sock->tlsData;
  do
  {
    ret = mbedtls_ssl_close_notify(&tlsData->ssl);
  }
  while ( (ret == MBEDTLS_ERR_SSL_WANT_WRITE) || (ret == MBEDTLS_ERR_SSL_WANT_READ) );

  /* All other negative return values indicate connection needs to be reset.
   * No further action required since this is disconnect call */
#endif /* 0 */
  ret = net_sock_close(sock->underlying_sock_ctxt);
  ret |= net_sock_destroy(sock->underlying_sock_ctxt);
  if (ret != NET_OK)
  {
    msg_error("net_sock_close() or net_sock_destroy() failed.\n");
  }
  else
  {
    internal_close(sock);
    rc = NET_OK;
  }

  return rc;
}


int net_sock_destroy_mbedtls(net_sockhnd_t sockhnd)
{
  int rc = NET_ERR;
  net_sock_ctxt_t *sock = (net_sock_ctxt_t * ) sockhnd;
  net_ctxt_t *ctxt = sock->net;

  /* Find the parent in the linked list.
   * Unlink and free. 
   */
  if (sock == ctxt->sock_list)
  {
    ctxt->sock_list = sock->next;
    rc = NET_OK;
  }
  else
  {
    net_sock_ctxt_t *cur = ctxt->sock_list;
    do
    {
      if (cur->next == sock)
      {
        cur->next = sock->next;
        rc = NET_OK;
        break;
      }
      cur = cur->next;
    } while(cur->next != NULL);
  }
  if (rc == NET_OK)
  {
    net_free(sock->tlsData);
    net_free(sock);
  }
  
  return rc;
}

static void my_debug( void *ctx, int level,
                      const char *file, int line,
                      const char *str )
{
  ((void) level);
#if defined ( __GNUC__ ) && !defined(__CC_ARM) && defined (HAS_RTOS)
  ((void) ctx);
  mbedtls_printf("%s:%04d: %s\n", file, line, str );
#else
  mbedtls_fprintf((FILE *) ctx, "%s:%04d: %s\n", file, line, str );
#endif
}


static void internal_close(net_sock_ctxt_t * sock)
{
  net_tls_data_t * tlsData = sock->tlsData;
  
  sock->underlying_sock_ctxt = (net_sockhnd_t) -1;
 
  mbedtls_x509_crt_free(&tlsData->clicert);
  mbedtls_pk_free(&tlsData->pkey);
  mbedtls_x509_crt_free(&tlsData->cacert);
  mbedtls_x509_crl_free(&tlsData->cacrl);
  mbedtls_ssl_free(&tlsData->ssl);
  mbedtls_ssl_config_free(&tlsData->conf);
  mbedtls_ctr_drbg_free(&tlsData->ctr_drbg);
  mbedtls_entropy_free(&tlsData->entropy);
  
  return;
}

#endif /* USE_MBED_TLS */
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
