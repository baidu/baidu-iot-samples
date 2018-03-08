/**
  ******************************************************************************
  * @file    net.h
  * @author  MCD Application Team
  * @version V0.0.1
  * @date    11-August-2017
  * @brief   Network API.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __NET_H__
#define __NET_H__

#include <stdint.h>

/** Function return codes. */
#define  NET_OK          0   /**< Success */
#define  NET_ERR        -1   /**< Internal error */
#define  NET_PARAM      -2   /**< Wrong parameters, or wrong state. */
#define  NET_TIMEOUT    -3   /**< Timeout rearched during a blocking function call. */
#define  NET_EOF        -4   /**< Connection dropped during the operation. */
#define  NET_NOT_FOUND  -5   /**< The remote host could not be reached. */
#define  NET_AUTH       -6   /**< The remote host cound not be authentified. */

/* Socket options definitions:
 * Important: All contents are passed by reference.
 *            The callee does not make a copy.
 *            The caller is responsible for maintaining the passed memory area valid until the socket is destroyed.
 *
 *    Name                      Content                                                         Effect
 *    tls_ca_certs              List of the root CA cerficates. PEM format.                     TLS lib configuration.
 *    tls_ca_crl                Certificate revocation list. PEM format.                        TLS lib configuration.
 *    tls_dev_cert              Client certificate. PEM format.                                 TLS lib configuration.
 *    tls_dev_key               Client private key. PEM format.                                 TLS lib configuration.
 *    tls_dev_pwd               Client private key password. Binary format.                     TLS lib configuration.
 *    tls_server_verification   NULL                                                            TLS lib configuration.
 *    tls_server_noverification NULL                                                            TLS lib configuration.
 *            Default option:   tls_server_verification
 *  
 *    tls_server_name           Check pattern for the server certificate verification. String.  TLS lib configuration.
 *    sock_blocking             NULL.                                                           The recv calls are blocking until
 *                                                                                                  - at least one byte may be returned,
 *                                                                                                  - or the sock_read_timeout is reached.
 *    sock_noblocking         NULL.                                                           The recv calls are not blocking.
 *                                                                                                The available payload is immediately returned, even if empty.
 *            Default option:   sock_blocking
 *  
 *    sock_read_timeout       Timeout in ms. Ascii format.
 *    sock_write_timeout      Timeout in ms. Ascii format.                                    Applied to TCP sockets only.
 */

typedef int32_t * net_hnd_t;         /**< Network interface handle. */
typedef int32_t * net_sockhnd_t;     /**< Socket handle. */

/** Network interface identifier.
 *  Restriction: One interface of each type at max. */
typedef enum {
  NET_IF_NONE = 0,
  NET_IF_WLAN,
  NET_IF_ETH,
  NET_IF_C2C,
  NET_IF_BNEP
} net_if_t;

/** Socket protocol. */
typedef enum {
  NET_PROTO_NONE = 0,
  NET_PROTO_TCP,
  NET_PROTO_TLS
} net_proto_t;

/** IP protocol version. */
typedef enum {
  NET_IP_V4,
  NET_IP_V6
} net_ip_version_t;

/** MAC address. */
typedef struct {
  uint8_t mac[6];
} net_macaddr_t;

/** IP address. */
typedef struct {
  net_ip_version_t ipv;
  uint8_t ip[16];         /**< Binary format. Network byte order. IPv4 mapped IPv6 format. E.g. 10.2.3.4 is  ::ffff:a02:304 or 0xFFFF0A020304*/
} net_ipaddr_t;


/**
 * @brief   Callback type: initialize the network interface and connect to the LAN.
 * @param   In:   if_ctxt       Pointer to the interface-specific native context.
 * @retval  The callback must return a non-zero error code in case of failure.
 */
typedef int net_if_init_t(void * if_ctxt);

/**
 * @brief   Callback type: de-initialize the network interface.
 * @param   In:   if_ctxt       Pointer to the interface-specific native context.
 * @retval  The callback must return a non-zero error code in case of failure.
 */
typedef int net_if_deinit_t(void * if_ctxt);

/* External interface ---------------------------------------------------------------*/

/**
 * @brief   Initialize a network interface..
 * @param   Out:  nethnd      Handle to the new interface.
 * @param   In:   interface   Interface identifier.
 * @param   In:   f_netinit   Callback to a platform/application specific initialization of the network interface.
 * @retval  Status
 *            NET_OK      Success.
 *            NET_PARAM   The specified interface is not supported.
 *            NET_ERR     Error.
 */
int net_init(net_hnd_t * nethnd, net_if_t interface, net_if_init_t *f_netinit);

/**
 * @brief   De-initialize a network interface.
 * @param   In:  nethnd       Network interface.
 * @param   In:  f_netdeinit  Callback to a platform/application specific de-initialization of the network interface.
 * @retval  Status
 *            NET_OK      Success.
 *            NET_PARAM   State error: Some sockets are still open on this interface.
 *            NET_ERR     Internal error.
 */
int net_deinit(net_hnd_t nethnd, net_if_deinit_t *f_netdeinit);

/**
 * @brief   Retrieve the IP address from an initialized network interface.
 * @param   In:   nethnd      Network interface.
 * @param   Out:  ipAddress   IP address. Allocated by the caller.
 * @retval  Status
 *            NET_OK      Success.
 *            NET_PARAM   State error: The interface is not initialized, or a NULL pointer is passed.
 *            NET_ERR     Internal error.
 */
int net_get_ip_address(net_hnd_t nethnd, net_ipaddr_t * ipAddress);

/**
 * @brief   Retrieve the MAC address from a network interface.
 * @param   In:   nethnd      Network interface.
 * @param   Out:  macAddress  MAC address. Allocated by the caller.
 * @retval  Status
 *            NET_OK      Success.
 *            NET_PARAM   Invalid parameter.
 *            NET_ERR     Internal error.
 */
int net_get_mac_address(net_hnd_t nethnd, net_macaddr_t * macAddress);

/**
 * @brief   Create a socket and attach it to a network interface.
 * @param   In:   nethnd    Network interface.
 * @param   Out:  sockhnd   New socket. Allocated by the caller.
 * @param   In:   proto     Socket protocol.
 * @retval  Status
 *            NET_OK      Success.
 *            NET_ERR     Internal error.
 *            NET_PARAM   Protocol not supported.
 */
int net_sock_create(net_hnd_t nethnd, net_sockhnd_t * sockhnd, net_proto_t proto);

/**
 * @brief   Open a socket and connect to the destination..
 * @param   In:   sockhnd   Socket.
 * @param   In:   hostname  Destination host. Hostname or IP address string.
 * @param   In:   dstport   Destination port.
 * @retval  Status
 *            NET_OK        Success.
 *            NET_ERR       Internal error.
 *            NET_NOT_FOUND The remote host could not be reached, or its name could not be resolved.
 */
int net_sock_open(net_sockhnd_t sockhnd, const char * hostname, int dstport);

/**
 * @brief   Set a socket option.
 * @note    May be called before the socket is opened.
 * @note    The function does not copy the option contents to the context storage:
 *          The caller must keep the data available at their passed location until the socket is closed.
 * @param   In:   sockhnd   Socket.
 * @param   In:   optname   Option name.
 * @param   In:   optbuf    Option payload.
 * @param   In:   optlen    Option payload length.
 *                          If the data is an ASCII string, the length must include the \0 termination char.
 * @retval  Status
 *            NET_OK      Success.
 *            NET_PARAM   The option is not supported, or the contents are inconsistent with the option.
 *            NET_ERR     Internal error.
 *            NET_AUTH    The remote host cound not be authentified. (Applicable to NET_PROTO_TLS sockets).
 */
int net_sock_setopt(net_sockhnd_t sockhnd, const char * optname, const uint8_t * optbuf, size_t optlen);

/**
 * @brief   Read from a socket.
 * @note    If the "sock_blocking" option was set, the function will not return until the requested length
 *          is received, or the "sock_read_timeout" is reached.
 *          If the "sock_noblocking" option was set, the function will return immediately up to the requested length.
 * @param   In:   sockhnd   Socket.
 * @param   Out:  buf       Destination buffer. Allocated by the caller.
 * @param   In:   len       Length to be read.
 * @retval  Status
 *            >=0           Success. Number of bytes read.
 *            NET_TIMEOUT   In "sock_blocking" mode, the read timeout was reached.
 *            NET_EOF       The connection was closed.
 *            NET_ERR       Internal error.
 *            NET_PARAM     Invalid parameter passed.
 */
int net_sock_recv(net_sockhnd_t sockhnd, uint8_t * const buf, size_t len);

/**
 * @brief   Send through a socket.
 * @param   In:   sockhnd   Socket.
 * @param   In:   buf       Source buffer.
 * @param   In:   len       Length to be sent.
 * @retval  Status
 *            >=0           Success, number of bytes written.
 *            NET_TIMEOUT   In "sock_blocking" mode, the send timeout was reached.
 *            NET_ERR       Internal error.
 *            NET_PARAM     Invalid parameter passed.
 */
int net_sock_send(net_sockhnd_t sockhnd, const uint8_t * buf, size_t len);

/**
 * @brief   Close a socket.
 * @note    A closed socket may be re-configured thanks to net_sock_setopt() and re-opened.
 * @param   In:   sockhnd   Socket.
 * @retval  Status
 *            NET_OK        Success.
 *            NET_ERR       Internal error.
 *            NET_PARAM     Invalid paramter passed.
 */
int net_sock_close(net_sockhnd_t sockhnd);

/**
 * @brief   Destroy a socket.
 * @param   In:   sockhnd   Socket.
 * @retval  Status
 *            NET_OK        Success.
 *            NET_ERR       Internal error.
 */
int net_sock_destroy(net_sockhnd_t sockhnd);


#endif /* __NET_H__ */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
