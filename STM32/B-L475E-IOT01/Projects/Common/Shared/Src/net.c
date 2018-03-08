/**
  ******************************************************************************
  * @file    net.c
  * @author  MCD Application Team
  * @version V0.0.1
  * @date    11-August-2017
  * @brief   Network abstraction at transport layer level.
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

/* Includes ------------------------------------------------------------------*/
#include "net_internal.h"

#ifdef USE_WIFI
extern int net_sock_create_tcp_wifi(net_hnd_t nethnd, net_sockhnd_t * sockhnd, net_proto_t proto);
#endif  /* USE_WIFI */
#ifdef USE_C2C
extern int net_sock_create_tcp_c2c(net_hnd_t nethnd, net_sockhnd_t * sockhnd, net_proto_t proto);
#endif  /* USE_C2C */
#ifdef USE_LWIP
extern int net_sock_create_tcp_lwip(net_hnd_t nethnd, net_sockhnd_t * sockhnd, net_proto_t proto);
#endif /* USE_LWIP */
#ifdef USE_MBED_TLS
extern int net_sock_create_mbedtls(net_hnd_t nethnd, net_sockhnd_t * sockhnd, net_proto_t proto);
#endif /* USE_MBED_TLS */

/* Private defines -----------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Functions Definition ------------------------------------------------------*/

int net_init(net_hnd_t * nethnd, net_if_t interface, net_if_init_t *f_netinit)
{
  int rc = NET_ERR;
  net_ctxt_t *ctxt = NULL;

  if (f_netinit == NULL)
  {
    rc = NET_PARAM;
  }
  else
  {
    ctxt = net_malloc(sizeof(net_ctxt_t));
    if (ctxt == NULL)
    {
      msg_error("net_init: allocation failed.\n");
      rc = NET_ERR;
    }
    else
    {
      memset(ctxt, 0, sizeof(net_ctxt_t));
      switch(interface)
      {
    #ifdef USE_WIFI
        case NET_IF_WLAN:
          ctxt->itf = interface;  // TODO: register a list of function pointers in function of the interface type. (to be provided by the caller?)
          if(f_netinit(NULL) == 0)
          {
            rc = NET_OK;
          }
          break;
    #endif /* USE_WIFI */
    #ifdef USE_C2C
        case NET_IF_C2C:
          ctxt->itf = interface;  // TODO: register a list of function pointers in function of the interface type. (to be provided by the caller?)
          if(f_netinit(NULL) == 0)
          {
            rc = NET_OK;
          }
          break;
    #endif /* USE_C2C */
    #ifdef USE_LWIP
        case NET_IF_ETH:
          ctxt->itf = interface;  // TODO: register a list of function pointers in function of the interface type. (to be provided by the caller?)
          if(f_netinit(&ctxt->lwip_netif) == 0)
          {
            rc = NET_OK;
          }
          break;
    #endif /* USE_LWIP */
        default:
          msg_error("net_init: interface type of %d not implemented.\n", interface);
          rc = NET_PARAM;
      }
    }
  }

  if (rc == NET_OK)
  {
    *nethnd = (net_hnd_t) ctxt;
  }
  else
  {
    if (ctxt != NULL)
    {
      net_free(ctxt);
    }
  }
  
  return rc;
}


int net_deinit(net_hnd_t nethnd, net_if_deinit_t *f_netdeinit)
{
  int rc = NET_ERR;
  net_ctxt_t *ctxt = (net_ctxt_t *) nethnd;
  
  if (f_netdeinit == NULL)
  {
    rc = NET_PARAM;
  }
  else
  {
    if (ctxt->sock_list != NULL)
    {
      rc = NET_PARAM;
    }
    else
    {
      switch(ctxt->itf)
      {
    #ifdef USE_WIFI
        case NET_IF_WLAN:
          f_netdeinit(NULL);
          rc = NET_OK;
          break;
    #endif /* USE_WIFI */
    #ifdef USE_C2C
        case NET_IF_C2C:
          f_netdeinit(NULL);
          rc = NET_OK;
          break;
    #endif /* USE_C2C */
    #ifdef USE_LWIP
        case NET_IF_ETH:
          f_netdeinit(&ctxt->lwip_netif);
          rc = NET_OK;
          break;
    #endif /* USE_LWIP */
        default:
          msg_error("net_deinit: interface type of %d not implemented.\n", ctxt->itf);
          rc = NET_PARAM;
      }
      
      if (rc == NET_OK)
      {
        net_free((void *)nethnd);
      }
    }
  }
  return rc;
}


int net_get_ip_address(net_hnd_t nethnd, net_ipaddr_t * ipAddress)
{
  net_ctxt_t *ctxt = (net_ctxt_t *) nethnd;
  int rc = NET_ERR;
  
  if (ipAddress == NULL)
  {
    rc = NET_PARAM;
  }
  else
  {
    switch(ctxt->itf)
    {
#ifdef USE_WIFI
      case NET_IF_WLAN:
      {
        uint8_t addr[4];
        /* WIFI_GetIP_Address() returns IPv4 adresses in binary format, network byte order. */
        WIFI_Status_t ret = WIFI_GetIP_Address(addr);
        if (ret == WIFI_STATUS_OK)
        {
          ipAddress->ipv = NET_IP_V4;
          memset(ipAddress->ip, 0, 10);
          ipAddress->ip[10] = 0xFF;
          ipAddress->ip[11] = 0xFF;
          memcpy(&ipAddress->ip[12], addr, 4);
          rc = NET_OK;
        }
        break;
      }
#endif /* USE_WIFI */

#ifdef USE_C2C
      case NET_IF_C2C:
      {
        uint8_t addr[4];
        char IpAddrString[C2C_IPADDR_LIST + 1]; 
        /* C2C_GetActiveIpAddresses() returns IPv4 adresses in binary format, network byte order. */
        // TODO: Check the format of the returned address.
        
        C2C_Ret_t ret = C2C_GetActiveIpAddresses(IpAddrString, addr);
        if (ret == C2C_RET_OK)
        {
          ipAddress->ipv = NET_IP_V4;
          memset(ipAddress->ip, 0, 10);
          ipAddress->ip[10] = 0xFF;
          ipAddress->ip[11] = 0xFF;
          memcpy(&ipAddress->ip[12], addr, 4);
          rc = NET_OK;
        }
        break;
      }
#endif /* USE_C2C */ 
      case NET_IF_ETH:
#ifdef USE_LWIP
      {
        if (ctxt->lwip_netif.ip_addr.addr == 0)
        {
          /* The network interface is not configured. */
          rc = NET_PARAM;
        }
        else
        {
          /* lwIP stores IPv4 addresses in network byte order. */
          u32_t ip = ip4_addr_get_u32(&ctxt->lwip_netif.ip_addr);
          ipAddress->ipv = NET_IP_V4;
          memset(ipAddress->ip, 0, 10);
          ipAddress->ip[10] = 0xFF;
          ipAddress->ip[11] = 0xFF;
          memcpy(&ipAddress->ip[12], &ip, 4);
          rc = NET_OK;
          break;
        }
        break;
      }
#endif /* USE_LWIP */
      default:
        msg_error("net_get_ip_address: interface type of %d not implemented.\n", ctxt->itf);
        rc = NET_PARAM;
    }
  }
  
  return rc;
}


int net_get_mac_address(net_hnd_t nethnd, net_macaddr_t * macAddress)
{
  net_ctxt_t *ctxt = (net_ctxt_t *) nethnd;
  int rc = NET_ERR;
  switch (ctxt->itf)
  {
#ifdef USE_WIFI
    case NET_IF_WLAN:
    {
      uint8_t addr[6];  /* WIFI_GetMAC_Address() returns in binary format. */
      WIFI_Status_t ret = WIFI_GetMAC_Address(addr);
      if (ret == WIFI_STATUS_OK)
      {
        memcpy(macAddress->mac, addr, MIN(sizeof(macAddress->mac), 6));
        rc = NET_OK;
      }
      break;
    }
#endif /* USE_WIFI */
#ifdef USE_C2C
    case NET_IF_C2C:
    {
      uint8_t addr[6] = { 1, 2, 3, 4, 5, 6 };  /* WIFI_GetMAC_Address() returns in binary format. */
      // TODO: Is there a C2C HW address which could be fit here?
      /*WIFI_Status_t ret = WIFI_GetMAC_Address(addr);
      if (ret == WIFI_STATUS_OK)
      */
      {
        memcpy(macAddress->mac, addr, MIN(sizeof(macAddress->mac), 6));
        rc = NET_OK;
      }
      break;
    }
#endif /* USE_C2C */
#ifdef USE_LWIP
    case NET_IF_ETH:
    {
      if (ctxt->lwip_netif.hwaddr_len != 6)
      {
        msg_error("Unexpected HW address length: %u.\n", ctxt->lwip_netif.hwaddr_len);
        rc = NET_ERR;
      }
      else
      {
        memcpy(macAddress->mac, ctxt->lwip_netif.hwaddr, MIN(sizeof(macAddress->mac), ctxt->lwip_netif.hwaddr_len));
        rc = NET_OK;
      }
    }
    break;
#endif /* USE_LWIP */
    default:
      msg_error("net_get_mac_address: interface type of %d not implemented.\n", ctxt->itf);
      rc = NET_PARAM;
  }
  
  return rc;
}


int net_sock_create(net_hnd_t nethnd, net_sockhnd_t * sockhnd, net_proto_t proto)
{
  net_ctxt_t *ctxt = (net_ctxt_t *) nethnd;
  switch(proto)
  {
    case NET_PROTO_TCP:
      switch(ctxt->itf)
      {
#ifdef USE_WIFI
        case NET_IF_WLAN:
          return net_sock_create_tcp_wifi(nethnd, sockhnd, proto);
#endif /* USE_WIFI */
#ifdef USE_C2C
        case NET_IF_C2C:
          return net_sock_create_tcp_c2c(nethnd, sockhnd, proto);
#endif /* USE_C2C */          
#ifdef USE_LWIP
        case NET_IF_ETH:
          return net_sock_create_tcp_lwip(nethnd, sockhnd, proto);
#endif /* USE_LWIP */
        default:
          ;
      }
      break;
#ifdef USE_MBED_TLS
    case NET_PROTO_TLS:
      return net_sock_create_mbedtls(nethnd, sockhnd, proto);
#endif /* USE_MBED_TLS */
    default:
      msg_error("net_sock_create: interface type of %d not implemented.\n", ctxt->itf);
      return NET_PARAM;
  }
  return NET_PARAM;
}


int net_sock_open(net_sockhnd_t sockhnd, const char * hostname, int dstport)
{
  net_sock_ctxt_t *sock = (net_sock_ctxt_t *) sockhnd;
  return sock->methods.open(sockhnd, hostname, dstport);
}


int net_sock_setopt(net_sockhnd_t sockhnd, const char * optname, const uint8_t * optbuf, size_t optlen)
{
  int rc = NET_PARAM;
  net_sock_ctxt_t *sock = (net_sock_ctxt_t *) sockhnd;
  bool has_opt_data = (optbuf != NULL) && (optlen > 0);

#ifdef USE_MBED_TLS
  net_tls_data_t * tlsData = sock->tlsData;
  if ( (sock->proto == NET_PROTO_TLS) && (tlsData != NULL) )
  {
    if (strcmp(optname, "tls_ca_certs") == 0)
    {
      if (has_opt_data)
      {
        tlsData->tls_ca_certs = (unsigned char *) optbuf;
        rc = NET_OK;
      }
    }
    if (strcmp(optname, "tls_dev_cert") == 0)
    {
      if (has_opt_data)
      {
        tlsData->tls_dev_cert = (unsigned char *) optbuf;
        rc = NET_OK;
      }
    }
    if (strcmp(optname, "tls_dev_key") == 0)
    {
      if (has_opt_data)
      {
        tlsData->tls_dev_key = (unsigned char *) optbuf;
        rc = NET_OK;
      }
    }
    if (strcmp(optname, "tls_dev_pwd") == 0)
    {
      if (has_opt_data)
      {
        tlsData->tls_dev_pwd = (uint8_t *) optbuf;
        tlsData->tls_dev_pwd_len = optlen;
        rc = NET_OK;
      }
    }
    if (strcmp(optname, "tls_server_verification") == 0)
    {
      if (!has_opt_data)
      {
        tlsData->tls_srv_verification = true;
        rc = NET_OK;
      }
    }
    if (strcmp(optname, "tls_server_noverification") == 0)
    {
      if (!has_opt_data)
      {
        tlsData->tls_srv_verification = false;
        rc = NET_OK;
      }
    }
    if (strcmp(optname, "tls_server_name") == 0)
    {
      if (has_opt_data)
      {
        tlsData->tls_srv_name = (char *) optbuf;
        rc = NET_OK;
      }
    }
  }
#endif /* USE_MBED_TLS */
  
  if (strcmp(optname, "sock_blocking") == 0)
  {
    if (!has_opt_data)
    {
      sock->blocking = true;
      rc = NET_OK;
    }
  }
  if (strcmp(optname, "sock_noblocking") == 0)
  {
    if (!has_opt_data)
    {
      sock->blocking = false;
      rc = NET_OK;
    }
  }
  if (strcmp(optname, "sock_read_timeout") == 0)
  {
    if (has_opt_data)
    {
      sock->read_timeout = atoi((char const *)optbuf);
      rc = NET_OK;
    }
  }
  if (strcmp(optname, "sock_write_timeout") == 0)
  {
    if (has_opt_data)
    {
      sock->write_timeout = atoi((char const *)optbuf);
      rc = NET_OK;
    }
  }
  return rc;
}


int net_sock_recv(net_sockhnd_t sockhnd, uint8_t * const buf, size_t len)
{
  net_sock_ctxt_t *sock = (net_sock_ctxt_t *) sockhnd;
  return sock->methods.recv(sockhnd, buf, len);
}


int net_sock_send(net_sockhnd_t sockhnd, const uint8_t * buf, size_t len)
{
  net_sock_ctxt_t *sock = (net_sock_ctxt_t *) sockhnd;
  return sock->methods.send(sockhnd, buf, len);
} 
  

int net_sock_close(net_sockhnd_t sockhnd)
{
  net_sock_ctxt_t *sock = (net_sock_ctxt_t *) sockhnd;
  return sock->methods.close(sockhnd);
}


int net_sock_destroy(net_sockhnd_t sockhnd)
{
  net_sock_ctxt_t *sock = (net_sock_ctxt_t *) sockhnd;
  return sock->methods.destroy(sockhnd);
}


/* Library Private Functions Definition ------------------------------------------------------*/

/**
 * @brief   Return the integer difference between 'init + timeout' and 'now'.
 *          The implementation is robust to uint32_t overflows.
 * @param   In:   init      Reference index.
 * @param   In:   now       Current index.
 * @param   In:   timeout   Target index.
 * @retval  Number of units from now to target.
 */
int32_t net_timeout_left_ms(uint32_t init, uint32_t now, uint32_t timeout)
{
  int32_t ret = 0;
  uint32_t wrap_end = 0;
  
  if (now < init)
  { // Timer wrap-around detected
    // printf("Timer: wrap-around detected from %d to %d\n", init, now);
    wrap_end = UINT32_MAX - init;
  }
  ret = wrap_end - (now - init) + timeout;

  return ret;
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
