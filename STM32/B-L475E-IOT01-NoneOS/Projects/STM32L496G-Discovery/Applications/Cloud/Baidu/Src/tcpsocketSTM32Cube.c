/**
 ******************************************************************************
 * @file    tcpsocketSTM32Cube.c
 * @author  Central LAB
 * @version V3.0.0
 * @date    21-April-2017
 * @brief   Adapter to tcp Socket
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; COPYRIGHT(c) 2017 STMicroelectronics</center></h2>
 *
 * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 *        http://www.st.com/software_license_agreement_liberty_v2
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *   1. Redistributions of source code must retain the above copyright notice,
 *      this list of conditions and the following disclaimer.
 *   2. Redistributions in binary form must reproduce the above copyright notice,
 *      this list of conditions and the following disclaimer in the documentation
 *      and/or other materials provided with the distribution.
 *   3. Neither the name of STMicroelectronics nor the names of its contributors
 *      may be used to endorse or promote products derived from this software
 *      without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 ******************************************************************************
 */

//#define DEBUG_TCP_CLIENT
 
#ifdef DEBUG_TCP_CLIENT
#include "STM32CubeInterface.h"
#endif /* DEBUG_TCP_CLIENT */


#include <stddef.h>
#include <stdlib.h>
#include "tcpsocketSTM32Cube.h"
#include "azure_c_shared_utility/xlogging.h"
#include "net_internal.h"

extern net_hnd_t hnet;
#ifdef USE_C2C
extern int net_sock_create_tcp_c2c(net_hnd_t nethnd, net_sockhnd_t * sockhnd, net_proto_t proto);
#endif  /* USE_C2C */
/**
 * @brief Function for allocating a TCP Socket Structure
 * @param None
 * @retval TCPSOCKETCONNECTION_HANDLE Pointer to the TCP Socket structure
 */
TCPSOCKETCONNECTION_HANDLE tcpsocketconnection_create(void)
{
#ifdef DEBUG_TCP_CLIENT
  AZURE_PRINTF(">>tcpsocketconnection_create...TBD\r\n");
#endif /* DEBUG_TCP_CLIENT */
  TCPSOCKETCONNECTION_HANDLE handler;
  handler=(TCPSOCKETCONNECTION_HANDLE )malloc(sizeof(TCPSOCKETCONNECTION));
  
  net_sock_create_tcp_c2c(hnet, &handler->SocketHandle, NET_PROTO_TCP);
  return handler;
}

/**
 * @brief Function not implemented
 * @param TCPSOCKETCONNECTION_HANDLE tcpSocketConnectionHandle Pointer to the TCP Socket structure
 * @param int blocking
 * @param  unsigned int timeout
 * @retval None
 */
void tcpsocketconnection_set_blocking(TCPSOCKETCONNECTION_HANDLE tcpSocketConnectionHandle, int blocking, unsigned int timeout)
{
#ifdef DEBUG_TCP_CLIENT
  AZURE_PRINTF(">>tcpsocketconnection_set_blocking...TBD\r\n");
#endif /* DEBUG_TCP_CLIENT */
}

/**
 * @brief Function for de-allocating a TCP Socket Structure
 * @param TCPSOCKETCONNECTION_HANDLE tcpSocketConnectionHandle Pointer to the TCP Socket structure
 * @retval None
 */
void tcpsocketconnection_destroy(TCPSOCKETCONNECTION_HANDLE tcpSocketConnectionHandle)
{
  net_sock_destroy_tcp_c2c(tcpSocketConnectionHandle->SocketHandle);
  free(tcpSocketConnectionHandle);
}

/**
 * @brief Function for opening a Socket whit one host using a specific port
 * @param TCPSOCKETCONNECTION_HANDLE tcpSocketConnectionHandle Pointer to the TCP Socket structure
 * @param char* host Host address
 * @param int port Port for TCP connection
 * @retval Int OK/Error (0/1)
 */
int tcpsocketconnection_connect(TCPSOCKETCONNECTION_HANDLE tcpSocketConnectionHandle, const char* host, const int port)
{
	return net_sock_open_tcp_c2c(tcpSocketConnectionHandle->SocketHandle, host, port);
}

/**
 * @brief Function for closing a Socket
 * @param TCPSOCKETCONNECTION_HANDLE tcpSocketConnectionHandle Pointer to the TCP Socket structure
 * @retval None
 */
void tcpsocketconnection_close(TCPSOCKETCONNECTION_HANDLE tcpSocketConnectionHandle)
{
  net_sock_close_tcp_c2c(tcpSocketConnectionHandle->SocketHandle);
  return;
}

/**
 * @brief Function for sending data to a socket
 * @param TCPSOCKETCONNECTION_HANDLE tcpSocketConnectionHandle Pointer to the TCP Socket structure
 * @param char* data source buffer
 * @param int length data leght
 * @retval int Number of Bytes written
 */
int tcpsocketconnection_send(TCPSOCKETCONNECTION_HANDLE tcpSocketConnectionHandle, const char* data, int length)
{
	return net_sock_send_tcp_c2c(tcpSocketConnectionHandle->SocketHandle, data, length);
}

/**
 * @brief Function for sending data to a socket
 * @param TCPSOCKETCONNECTION_HANDLE tcpSocketConnectionHandle Pointer to the TCP Socket structure
 * @param char* data source buffer
 * @param int length data leght
 * @retval int Number of Bytes written
 */
int tcpsocketconnection_send_all(TCPSOCKETCONNECTION_HANDLE tcpSocketConnectionHandle, const char* data, int length)
{
	return net_sock_send_tcp_c2c(tcpSocketConnectionHandle->SocketHandle, data, length);
}

/**
 * @brief Function for reading data from a socket
 * @param TCPSOCKETCONNECTION_HANDLE tcpSocketConnectionHandle Pointer to the TCP Socket structure
 * @param char* data destination buffer
 * @param int length data leght to read
 * @retval int Number of Bytes read
 */
int tcpsocketconnection_receive(TCPSOCKETCONNECTION_HANDLE tcpSocketConnectionHandle, char* data, int length)
{
  return net_sock_recv_tcp_c2c(tcpSocketConnectionHandle->SocketHandle, data, length);
}

/**
 * @brief Function for reading data from a socket
 * @param TCPSOCKETCONNECTION_HANDLE tcpSocketConnectionHandle Pointer to the TCP Socket structure
 * @param char* data destination buffer
 * @param int length data leght to read
 * @retval int Number of Bytes read
 */
int tcpsocketconnection_receive_all(TCPSOCKETCONNECTION_HANDLE tcpSocketConnectionHandle, char* data, int length)
{
	  return net_sock_recv_tcp_c2c(tcpSocketConnectionHandle->SocketHandle, data, length);
}
/******************* (C) COPYRIGHT 2017 STMicroelectronics *****END OF FILE****/
