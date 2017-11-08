/**
  ******************************************************************************
  * @file    net_tcp_lwip.c
  * @author  MCD Application Team
  * @version V1.0.0
  * @date    11-July-2017
  * @brief   Network abstraction at transport layer level. TCP implementation on
             LwIP.
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

#ifdef USE_LWIP
#include "lwip/netdb.h"

/* Private defines -----------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
int net_sock_create_tcp_lwip(net_hnd_t nethnd, net_sockhnd_t * sockhnd, net_proto_t proto);
int net_sock_open_tcp_lwip(net_sockhnd_t sockhnd, const char * hostname, int dstport);
int net_sock_recv_tcp_lwip(net_sockhnd_t sockhnd, uint8_t * buf, size_t len);
int net_sock_send_tcp_lwip( net_sockhnd_t sockhnd, const uint8_t * buf, size_t len);
int net_sock_close_tcp_lwip(net_sockhnd_t sockhnd);
int net_sock_destroy_tcp_lwip(net_sockhnd_t sockhnd);

/* Functions Definition ------------------------------------------------------*/

int net_sock_create_tcp_lwip(net_hnd_t nethnd, net_sockhnd_t * sockhnd, net_proto_t proto)
{
  int rc = NET_ERR;
  net_ctxt_t *ctxt = (net_ctxt_t *) nethnd;
  net_sock_ctxt_t *sock = NULL;
  
  sock = malloc(sizeof(net_sock_ctxt_t));
  if (sock == NULL)
  {
    msg_error("net_sock_create allocation failed.\n");
    rc = NET_ERR;
  }
  else
  {
    memset(sock, 0, sizeof(net_sock_ctxt_t));
    sock->net = ctxt;
    sock->next = ctxt->sock_list;
    sock->methods.create  = (net_sock_create_tcp_lwip);
    sock->methods.open    = (net_sock_open_tcp_lwip);
    sock->methods.recv    = (net_sock_recv_tcp_lwip);
    sock->methods.send    = (net_sock_send_tcp_lwip);
    sock->methods.close   = (net_sock_close_tcp_lwip);
    sock->methods.destroy = (net_sock_destroy_tcp_lwip);
    sock->proto = proto;
    sock->blocking      = NET_BLOCKING;      /* Value by default */
    sock->read_timeout  = NET_READ_TIMEOUT;  /* Value by default */
    sock->write_timeout = NET_WRITE_TIMEOUT; /* Value by default */
    ctxt->sock_list = sock;                   /* Insert at the head of the list */
    *sockhnd = (net_sockhnd_t) sock;

    rc = NET_OK;
  }
  
  return rc;
}


int net_sock_open_tcp_lwip(net_sockhnd_t sockhnd, const char * hostname, int dstport)
{
  int rc = NET_OK;
  net_sock_ctxt_t *sock = (net_sock_ctxt_t * ) sockhnd;
  
  char portBuffer[6];
  struct addrinfo hints;
  struct addrinfo *list;
  struct addrinfo *current;
  int socket = -1;
  memset(&hints, 0, sizeof(hints));
  hints.ai_family   = AF_INET;        // TODO: Add IP version to the create() function parameter list. 
  switch (sock->proto)
  {
    case NET_PROTO_TCP:
      hints.ai_socktype = SOCK_STREAM;
      hints.ai_protocol = IPPROTO_TCP;
      break;
    default:
      return NET_PARAM;
  }
  
  snprintf(portBuffer, 5, "%d", dstport);
  
  if(getaddrinfo(hostname, portBuffer, &hints, &list) != 0)
  {
    msg_info("The address of %s could not be resolved.\n", hostname);
    rc = NET_NOT_FOUND;
  }
  else
  {
    for(current = list; current != NULL; current = current->ai_next)
    {
      socket = (int) socket(current->ai_family, current->ai_socktype, current->ai_protocol);
      if(socket >= 0)
      {
        if (sock->read_timeout != 0)
        {
#if !LWIP_SO_RCVTIMEO || !LWIP_SO_RCVRCVTIMEO_NONSTANDARD
#error  lwipopt.h must define LWIP_SO_RCVTIMEO so that the TCP read timeout is supported.
#endif /* !LWIP_SO_RCVTIMEO */           
          int opt = sock->read_timeout;
          if (0 != lwip_setsockopt (socket, SOL_SOCKET, SO_RCVTIMEO, &opt, sizeof(opt)))
          {
            msg_error("Could not set the read timeout.\n");
            rc = NET_ERR;
          }
        }

        if ( (rc == NET_OK) && (sock->write_timeout != 0) )
        {
#if !LWIP_SO_SNDTIMEO || !LWIP_SO_SNDRCVTIMEO_NONSTANDARD
#error  lwipopt.h must define LWIP_SO_SNDTIMEO so that the TCP write timeout is supported.
#endif /* !LWIP_SO_RCVTIMEO */           
          int opt = sock->write_timeout;
          if (0 != lwip_setsockopt (socket, SOL_SOCKET, SO_SNDTIMEO, &opt, sizeof(opt)))
          {
            msg_error("Could not set the write timeout.\n");
            rc = NET_ERR;
          }
        }
        
        if ( (rc == NET_OK) && (0 == connect(socket, current->ai_addr, (uint32_t)current->ai_addrlen)) )
        {
          sock->underlying_sock_ctxt = (net_sockhnd_t) socket;
          rc = NET_OK;
          break;
        }
      }
      close(socket);
      rc = NET_NOT_FOUND;
    }
    freeaddrinfo(list);
  }  
 
  return rc;
}


int net_sock_recv_tcp_lwip(net_sockhnd_t sockhnd, uint8_t * buf, size_t len)
{
  int rc = 0;
  net_sock_ctxt_t *sock = (net_sock_ctxt_t * ) sockhnd;
    
  if (sock->underlying_sock_ctxt < 0)
  {
    rc = NET_PARAM;
  }
  else
  {
    int ret = -1;
    do
    {
      ret = recv((int)sock->underlying_sock_ctxt, buf, len, MSG_DONTWAIT);
      if( ret < 0 )
      {
        switch(errno)
        {
          case EWOULDBLOCK:
          case EINTR:
            /* Incomplete read. The caller should try again. */
            break;
          case EPIPE:
          case ECONNRESET:
            rc = NET_EOF;
            break;
          case ERR_TIMEOUT:
            rc = NET_TIMEOUT;
            break;
          default:
            rc = NET_ERR;
        }
      }
      else
      {
        rc = ret;
      }
    } while ( (sock->blocking == true) && (rc == 0) );
  }

  return rc;
}


int net_sock_send_tcp_lwip( net_sockhnd_t sockhnd, const uint8_t * buf, size_t len)
{
  int rc = 0;
  net_sock_ctxt_t *sock = (net_sock_ctxt_t * ) sockhnd;
  
  if (sock->underlying_sock_ctxt < 0)
  {
    rc = NET_PARAM;
  }
  else
  {
    int ret = -1;
    do
    {
      ret = write((int)sock->underlying_sock_ctxt, buf, len);
      if(ret < 0)
      {
        switch(ret)
        {
          case EPIPE:
          case ECONNRESET:
            rc = NET_EOF;
            break;
          case EINTR:
           /* Incomplete read. The caller should try again. */
            break;
          case ERR_TIMEOUT:
            rc = NET_TIMEOUT;
            break;
          default:
            rc = NET_ERR;
        }
      }
      else
      {
        rc = ret;
      }
    } while ( (sock->blocking == true) && (rc == 0) );
  }
  
  return rc;
}


int net_sock_close_tcp_lwip(net_sockhnd_t sockhnd)
{
  int rc = NET_ERR;
  net_sock_ctxt_t *sock = (net_sock_ctxt_t * ) sockhnd;
  
  if(0 == shutdown((int)sock->underlying_sock_ctxt, SHUT_RDWR))
  {
    if (0 == close((int)sock->underlying_sock_ctxt))
    {
      sock->underlying_sock_ctxt = (net_sockhnd_t) -1;
      rc = NET_OK;
    }
    else
    {
      msg_error("Could not close the socket %d.\n", (int)sock->underlying_sock_ctxt);
    }
  }
  else
  {
    msg_error("Could not shutdown the socket %d.\n", (int)sock->underlying_sock_ctxt);
  }
  
  return rc;
}


int net_sock_destroy_tcp_lwip(net_sockhnd_t sockhnd)
{
  // TODO: If destroy() implementation is not specific to the underlying network interface,
  //       it should be moved to net.c.
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
    free(sock);
  }
  
  return rc;
}

#endif /* USE_LWIP */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
