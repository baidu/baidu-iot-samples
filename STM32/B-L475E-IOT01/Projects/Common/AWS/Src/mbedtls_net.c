/**
  ******************************************************************************
  * @file    mbedtls_net.c
  * @author  MCD Application Team
  * @version V1.0.0
  * @date    11-July-2017
  * @brief   Mbedtls network porting layer. Callbacks implementation.
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
#include "mbedtls_net.h"
#include "mbedtls/ssl.h"
#include "net.h"
#include "msg.h"

/* Non-blocking interface implementation. Timeout is not applicable. */
int mbedtls_net_recv(void *ctx, unsigned char *buf, size_t len)
{
  int ret = net_sock_recv((net_sockhnd_t) ctx, buf, len);
  
  if (ret > 0)
  {
    return ret;
  }
  
  if(ret == 0)
  {
    return  MBEDTLS_ERR_SSL_WANT_READ; 
  }
  
  msg_error("mbedtls_net_recv(): error %d in net_sock_recv() - requestedLen=%d\n", ret, len);
 
  return MBEDTLS_ERR_SSL_INTERNAL_ERROR;
}


int mbedtls_net_send(void *ctx, const unsigned char *buf, size_t len)
{
  int ret = net_sock_send((net_sockhnd_t) ctx, buf, len);
  
  if (ret > 0)
  {
    return ret;
  }
  
  if(ret == 0)
  {
    return  MBEDTLS_ERR_SSL_WANT_WRITE; 
  }
  msg_error("mbedtls_net_send(): error %d in net_sock_send() - requestedLen=%d\n", ret, len);
  
  /* TODO: The underlying layers do not allow to distinguish between
   *          MBEDTLS_ERR_SSL_INTERNAL_ERROR,
   *          MBEDTLS_ERR_SSL_PEER_CLOSE_NOTIFY,
   *          MBEDTLS_ERR_SSL_CONN_EOF.
   *  Most often, the error is due to the closure of the connection by the remote host. */ 
  
  return MBEDTLS_ERR_SSL_PEER_CLOSE_NOTIFY;
  
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
