/**
  ******************************************************************************
  * @file    http_util.c
  * @author  MCD Application Team
  * @version V0.0.1
  * @date    11-August-2017
  * @brief   Helper functions for building HTTP GET and POST requests, and 
  *          stream reading.
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
#include <string.h>
#include "main.h"
#include "http_util.h"
#include "msg.h"
#include "net.h"

/* Private defines -----------------------------------------------------------*/
#define HTTP_MAX_HOST_SIZE        80      /**< Max length of the http server hostname. */
#define HTTP_MAX_QUERY_SIZE       50      /**< Max length of the http query string. */
#define HTTP_READ_BUFFER_SIZE     1050    /**< Size of the HTTP receive buffer.
                                               Must be larger than the HTTP response header length. That is about 300-800 bytes. */

#define MIN(a,b)        ( ((a)<(b)) ? (a) : (b) )

/* Private typedef -----------------------------------------------------------*/
/** 
 * @brief HTTP progressive download internal session context. 
 */
typedef struct {
  net_sockhnd_t sock;                 /**< Network socket handle. */
  char query[HTTP_MAX_QUERY_SIZE];    /**< HTTP query parsed from the URL. Must be resent in each range request. */
  char hostname[HTTP_MAX_HOST_SIZE];  /**< HTTP full qualified server name. Must be resent in each range request. */
  bool connection_is_open;            /**< HTTP keep-alive connection status. */
} http_context_t;

/**
 * @brief HTTP progressive download socket emulation context.
 */
typedef struct {
  http_handle_t http_hnd;   /**< HTTP progressive download handle. */
  int read_pos;             /**< Offset to the last byte read from the socket. */
  int res_size;             /**< Size of the resource being downloaded. Constant for a static file. */
  char *pUrl;               /**< Reference to the download URL. */
} http_sock_context_t;


/* Private variables ----------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Functions Definition ------------------------------------------------------*/

/**
 * @brief   Create an HTTP request string from the parameter list.
 * @note    The request buffer is allocated by the callee and must be released 
 *          by calling http_req_destroy() after usage.
 * @param   Out:  req_buf         Output buffer, allocated by the callee.
 * @param   In:   query           HTTP query string.
 * @param   In:   hostname        Remote hostname.
 * @param   In:   offset          Offset to the first byte to be read from the requested document 
 *                                in case of a partial content request. Must be set to 0 overwise.
 * @param   In:   size            Maximum length of the chunck of the requested document to be read.
 * @param   In:   extra_headers   HTTP headers to be appended to the request header. Single buffer. Each line must be ended by \r\n.
 * @param   In:   post_buf_size   Size of the document body to be sent, in case of a POST request. Must be 0 for a GET request.
 * @retval  Error code
 *            >0              Success, length of the request string written to req_buf.
 *            HTTP_ERR (<0)   Failure
 */
int http_req_create(char ** const req_buf , const char * const query, const char * const hostname, const size_t offset, const size_t size, const char * const extra_headers, const size_t post_buf_size)
{
  int rc = HTTP_OK;
    
  size_t req_buf_len = strlen(query) + strlen(hostname);
  req_buf_len += (extra_headers == NULL) ? 0 : strlen(extra_headers);
  if (post_buf_size == 0)
  {
    /* HTTP GET request */
    req_buf_len  += (offset + 1) / 10 + (size + 1) / 10     /* Variable length of the Range header */
        + 47;                                               /* Overall fixed-size length of the request string .*/
  }
  else
  { 
    /* HTTP POST request */
    req_buf_len += 48 + post_buf_size/10;                   /* Overall fixed-size length of the request string .*/
  }
  
  *req_buf = malloc(req_buf_len);
  if (req_buf == NULL)
  {
    rc = HTTP_ERR;
  }
  else
  {
    if (post_buf_size == 0)
    {
      /* HTTP GET request */
      rc = snprintf(*req_buf, req_buf_len,
          "GET /%s HTTP/1.1\r\n"
          "Host: %s\r\n"
          "Range: bytes=%d-%d\r\n"
          "%s\r\n\r\n",
          query, hostname, offset, offset + size - 1, (extra_headers == NULL) ? "" : extra_headers);
    }
    else
    {
      /* HTTP POST request */
      rc = snprintf(*req_buf, req_buf_len,
        "POST /%s HTTP/1.1\r\n"
        "Host: %s\r\n"
        "Content-Length: %d\r\n" 
        "%s\r\n\r\n",
        query, hostname, post_buf_size, (extra_headers == NULL) ? "" : extra_headers);
    }
    if (rc >= req_buf_len)
    {
      msg_error("Memory overflow preparing the HTTP request string. %d >= %d\n", rc, req_buf_len);
      rc = HTTP_ERR; /* The allocated destination buffer is too small. */
      free(*req_buf);
    }
  }
  
  return rc; /* Return the string length, or a <0 error code. */
}


/**
 * @brief   Destroy an HTTP request string created by http_req_create();
 * @param   In:   req_buf         Buffer to be freed.
 */
void http_req_destroy(const char *req_buf)
{
  free((void*)req_buf);
}


/**
 * @brief   Open an HTTP progressive download session.
 * @note    The internal session context is allocated by the callee.
 * @param   In: pHnd    Pointer on the session handle.
 * @param   In: url     Source of the progressive download. e.g. http://john.doe:80/foo.bar
 * @retval  Error code
 *            HTTP_OK   (0)  Success
 *            HTTP_ERR (<0)  Failure
 */
int http_open(http_handle_t * const pHnd, const char *url)
{
  http_context_t * pCtx = 0;
  int port;
  int ret = 0;

  pCtx = (http_context_t *) malloc(sizeof(http_context_t));

  if (pCtx != NULL)
  {
    pCtx->connection_is_open = false;
    bool tls = false;
    if (HTTP_OK == http_url_parse(pCtx->hostname, HTTP_MAX_HOST_SIZE, &port, &tls, pCtx->query, HTTP_MAX_QUERY_SIZE, url))
    {
      ret = net_sock_create(hnet, &pCtx->sock, (tls == true) ? NET_PROTO_TLS : NET_PROTO_TCP);
      if (NET_OK != ret)
      {
        msg_error("Could not create a socket.\n");
      }
      else
      {
        if (tls == true)
        {
          ret  = net_sock_setopt(pCtx->sock, "tls_ca_certs", (void *)lUserConfig.tls_root_ca_cert, strlen(lUserConfig.tls_root_ca_cert) + 1);
          ret |= net_sock_setopt(pCtx->sock, "tls_server_name", (void *)pCtx->hostname, strlen(pCtx->hostname) + 1);
          ret |= net_sock_setopt(pCtx->sock, "tls_server_verification", NULL, 0);
        }
      }
      
      if (NET_OK != ret)
      {
        msg_error("Could not set the socket options.\n");
      }
      else
      {
        ret = net_sock_open(pCtx->sock, (const char *) pCtx->hostname, port);
      }
      
      if (NET_OK != ret)
      {
        msg_error("Could not open a socket.\n");
      }
      else
      {
        *pHnd = (http_handle_t) pCtx;
        pCtx->connection_is_open = true;
        return HTTP_OK;
      }
    }

    free(pCtx);
  }
  return HTTP_ERR;
}


/**
 * @brief   Close an HTTP progressive download session.
 * @note    The internal session context is freed by the callee.
 * @param   In: hnd   Session handle.
 * @retval  Error code
 *            HTTP_OK   (0)  Success
 *            HTTP_ERR (<0)  Failure
 */
int http_close(const http_handle_t hnd)
{
  int rc = HTTP_ERR;
  
  http_context_t * pCtx = (http_context_t *) hnd;
  if (pCtx != NULL)
  {
    int ret = 0;
    ret = net_sock_close(pCtx->sock);
    ret |= net_sock_destroy(pCtx->sock);
    if (ret == NET_OK)
    {
      rc = HTTP_OK;
      free(pCtx);
    }
    else
    {
      msg_error("Could not close and destroy a socket.\n");
    }
  }
  
  return rc;
}

/**
 * @brief   Read from an HTTP progressive download session.
 * @param   Out: readbuffer     Output buffer.
 * @param   Out: status         HTTP range download status.
 * @param   In: offset          Offset (in bytes) from the start of the remote resource to read from. (Supported only with GET requests).
 * @param   In: size            Size of the chunk to read.
 * @param   In: extra_headers   String containing additional HTTP headers to send. Each line must end with \r\n. "" for no header at all.
 * @param   In: post_buf        Payload of the POST request. If NULL, the call is translated into a GET request.
 * @param   In: post_buf_size   Size of the POST payload.
 * @param   In: hnd             Session handle.
 * @retval  >=0 Success: Number of bytes copied to readbuffer.
 *           <0 Error:
 *                HTTP_ERR            Bad input parameter. Or allocation error.
 *                HTTP_ERR_HTTP       Connection error, or unexpected response from the HTTP server.
 *                HTTP_ERR_CLOSED     The connection was closed by the server during the previous http_read(). It is allowed by the protocol.
 */
int http_read(uint8_t * const readbuffer, http_range_status_t * const status, const size_t offset, const size_t size,
              const char * const extra_headers, const uint8_t * const post_buf, const size_t post_buf_size,
              const http_handle_t hnd)
 {
  int rc = HTTP_OK;
  int file_bytes = 0;
  int send_bytes = 0;
  char *req_buf = NULL;
  http_context_t * pCtx = (http_context_t *) hnd;

  if ((pCtx == NULL) || (readbuffer == NULL))
  {
    rc = HTTP_ERR;
  }
  else
  {
    if (pCtx->connection_is_open == false)
    {
      msg_error("The connection is not open.\n");
      rc = HTTP_ERR_CLOSED;
    }
  }
  
  if (rc == HTTP_OK)
  {
    send_bytes = http_req_create(&req_buf, pCtx->query, pCtx->hostname, offset, size, extra_headers, post_buf_size);
    if (send_bytes < 0)
    {
      rc = HTTP_ERR;
    }
  }
      
  if (rc == HTTP_OK) 
  {
    /* Send the HTTP headers. */
    rc = net_sock_send(pCtx->sock, (uint8_t *) req_buf, send_bytes);
    if (rc != send_bytes)
    {
      msg_error("Header send failed (%d/%d).\n", rc, send_bytes)
      rc = HTTP_ERR_HTTP;
    }
    else
    {
      rc = HTTP_OK;
      /* Send the POST body if applicable. */
      if ( (post_buf != NULL) && (post_buf_size > 0) )
      {
        rc = net_sock_send(pCtx->sock, (uint8_t *) post_buf, post_buf_size);
        if (rc != post_buf_size - 1)
        {
          msg_error("POST body send failed (%d/%d).\n", rc, post_buf_size - 1)
          rc = HTTP_ERR_HTTP;
        } 
      }
      
      /* Get and parse the server response. */ 
      if (rc == HTTP_OK)
      {
        int read_offset = 0;
        int rcv_bytes = 0;
        int ret = 0;
        uint8_t l_readbuffer[HTTP_READ_BUFFER_SIZE + 1]; /* + 1 to close the buffer with a string termination \0 and allow strstr() usage. */
        uint8_t *pBody = NULL;
        int content_length = 0;
        
        memset(l_readbuffer, 0, sizeof(l_readbuffer));
        
        do {
          ret = net_sock_recv(pCtx->sock, l_readbuffer + read_offset, HTTP_READ_BUFFER_SIZE - read_offset);
          if (ret >= 0)
          { 
            // msg_debug("Read %d bytes into the local readbuffer; %d/%d\n", ret, file_bytes, size);
            if (pBody == NULL)
            { 
              /* Looking for the HTTP header.*/
              read_offset += ret; /* Accumulate in the readbuffer until the body tag is found. */
              rcv_bytes += ret;
              
              /* Find the start of the body */
#define BODY_TAG  "\r\n\r\n"
              pBody = (uint8_t *) strstr((char const *)l_readbuffer, BODY_TAG);
              /* Warning: net_sock_recv() may overwrite the unused part of the passed buffer with implementation-dependant contents.
               *          Verify that the found body tag points to the part of the readbuffer which was actually returned by net_sock_recv(). */
              if ( (pBody != NULL) && ((pBody - l_readbuffer) < (read_offset - strlen(BODY_TAG))) )
              {
                pBody += strlen(BODY_TAG);
                read_offset = 0;  
              }
              else
              {
                pBody = NULL;
                msg_warning("Incomplete HTTP header of length %d. Must read further.\n", ret);
              }
 
              /* Parse the relevant headers */
              if ((pBody != NULL))
              {
                /* Get the body payload size. */
#define CL_TAG  "Content-Length: "
                uint8_t * pLen = (uint8_t *) strstr((char const *)l_readbuffer, CL_TAG);
                if ((pLen != NULL) && (pLen < pBody))
                {
                  pLen += strlen(CL_TAG);
                  content_length = atoi((char const *) pLen);
                }
                
                /* Return the Content-Range header into status, only if the full header was received. */
                if (status != NULL)
                {
                  uint8_t *pRange = NULL;
                  pRange = (uint8_t *) strstr((char const *)l_readbuffer, "Content-Range: bytes ");
                  if ((pRange == NULL) || (pRange >= pBody))
                  {
                    ret = -1;
                    msg_error("HTTP parsing: could not find the Content-range header.\n");
                  }
                  else
                  {
                    pRange += strlen("Content-Range: bytes ");
                    if (3 != sscanf((char const *)pRange, "%u-%u/%u", &status->first_byte, &status->last_byte, &status->resource_size))
                    {
                      uint32_t resource_size = 0;
                      if (1 != sscanf((char const *)pRange, "*/%lu", &resource_size))
                      {
                        ret = -1;
                        msg_error("Could not parse the HTTP Content-range header\n");
                      }
                      else
                      { /* Out of range request */
                        msg_error("Out of range request: %d-%d / %lu\n", offset, offset+size-1, resource_size);
                        msg_error("Previous range status: %d-%d / %d\n", status->first_byte, status->last_byte, status->resource_size);
                        ret = -1;
                      }
                    }
                    else
                    {
                      msg_info("returning with range status: %u-%u / %u\n", status->first_byte, status->last_byte, status->resource_size);
                    }
                  }
                }
                
                /* Check whether the server is going to close the connection. */
                uint8_t * pConn = (uint8_t *) strstr((char const *)l_readbuffer, "Connection: close");
                if ((pConn != NULL) && (pConn < pBody))
                {
                  pCtx->connection_is_open = false;
                  msg_warning("The server is dropping the HTTP connection. We will have to reconnect.\n");
                }
                
                /* Copy the start of the body to the destination buffer. */
                if (ret >= 0)
                {
                  int write_size = MIN(rcv_bytes - (pBody - l_readbuffer), size);
                  memcpy(&readbuffer[file_bytes], pBody, write_size);
                  file_bytes += write_size;
                }
              }
            }
            else
            { // HTTP header already retrieved.
              /* Received the next buffer.*/
              int write_size = MIN(ret, size - file_bytes);
              memcpy(&readbuffer[file_bytes], l_readbuffer, write_size);
              file_bytes += write_size;
            }
          }
          else
          {
            msg_error("net_sock_recv() returned %d.\n", ret);  
          }
        } while ((ret >= 0) && (file_bytes < MIN(content_length, size))); // TODO: Need a timeout or a loop limit to increase the resilience to network errors.
        
        if ( (ret < 0) && (rc == HTTP_OK) )
        {
          rc = HTTP_ERR_HTTP;
        }
      } 
    }
    http_req_destroy(req_buf);
  }
   
  return (rc < 0) ? rc : file_bytes;
}

/**
 * @brief   Tells whether an HTTP progressive download session is still open, or has been closed by the server.
 * @param   In: hnd   Session handle.
 * @retval  true:   The session is open.
 *          false:  The session is closed: the handle should be freed by calling http_close().
 */
bool http_is_open(const http_handle_t hnd)
{
  http_context_t * pCtx = (http_context_t *) hnd;
  return pCtx->connection_is_open;
}


/**
 * @brief   Parse and split an http url into hostname/port number/query string.
 * @param   Out: host           Hostname. Allocated by the caller.
 * @param   In:  host_max_len   Maximum length of the hostname.
 * @param   Out: proto          Application protocol (HTTP or HTTPS).
 * @param   Out: port           Port number.
 * @param   Out: query          HTTP query. Allocated by the caller.
 * @param   Out: tls            This is an HTTPS URL.
 * @param   In:  query_max_len  Maximum length of the query.
 * @param   In:  url            URL to be parsed.
 * @retval  Error code
 *            HTTP_OK   (0)  Success
 *            HTTP_ERR (<0)  Failure
 */
int http_url_parse(char * const host, const int host_max_len, int * const port, bool * tls, char * const query, const int query_max_len, const char * url)
{
  bool has_query = true;
  const char * pc = NULL;

#ifdef URL_TRACE
  msg_debug("url: %s\n", url);
#endif

  memset(host, 0, host_max_len);
  memset(query, 0, query_max_len);

#define HT_PX  "http://"
#define HTS_PX  "https://"
  
  if (0 == strncmp(url, HTS_PX, strlen(HTS_PX)))
  {
    *tls = true;
    pc = url + strlen(HTS_PX);
  }
  
  if (0 == strncmp(url, HT_PX, strlen(HT_PX)))
  {
    *tls = false;
    pc = url + strlen(HT_PX);
  }
  
  if ( pc != NULL )
  {
    char *pHostTailCol = strchr(pc, ':');
    char *pHostTailSla = strchr(pc, '/');
    if ((pHostTailCol <= pHostTailSla) && (pHostTailCol != NULL))
    {
      /*host = */strncpy(host, pc, MIN(pHostTailCol - pc, host_max_len));
      pc = pHostTailCol + 1;
      sscanf(pc, "%d", port);
      pc = strchr(pc, '/');
      if (pc != NULL)
      {
        pc++;
      } else
      {
        has_query = false;
      }
    } else
    {
      if (pHostTailSla != NULL)
      {
        strncpy(host, pc, MIN(pHostTailSla - pc, host_max_len));
        pc = pHostTailSla + 1;
      } else
      {
        /*host = */strncpy(host, pc, host_max_len);
        has_query = false;
      }
      *port = 80;
    }
    if (has_query)
    {
      /*query = */strncpy(query, pc, query_max_len);
    }

#ifdef URL_TRACE
    msg_debug("http host: %s\n", host);
    msg_debug("http port: %d\n", *port);
    msg_debug("http query: %s\n", query);
#endif

    return HTTP_OK;
  } else
  {
    /* Not an HTTP url. */
    return HTTP_ERR;
  }
}


/**
 * @brief   Open a socket-like connection over the passed HTTP progressive download session.
 * @param   Out:  http_sock_hnd   HTTP socket-like handle.
 * @param   In:   url             Source of the progressive download. e.g. http://john.doe:80/foo.bar
 * @retval  Error code
 *            HTTP_OK   (0)  Success
 *            HTTP_ERR (<0)  Failure
 */
int http_sock_open(http_sock_handle_t * const http_sock_hnd, const char *url)
{
  int rc = HTTP_ERR;
  http_sock_context_t * sock_ctx = malloc(sizeof(http_sock_context_t));
  
  if (sock_ctx != NULL)
  {
    /* Keep a copy of the URL in case we must re-open the connection later. */
    size_t url_buffer_length = strlen(url) + 1;
    sock_ctx->pUrl = malloc(url_buffer_length);
    if (sock_ctx->pUrl == NULL)
    {
      free(sock_ctx);
    }
    else
    {
      memcpy(sock_ctx->pUrl, url, url_buffer_length);
    
      if (HTTP_OK == http_open(&sock_ctx->http_hnd, url))
      {
        sock_ctx->read_pos = 0;
        sock_ctx->res_size = -1;
        *http_sock_hnd = (http_sock_handle_t) sock_ctx;
        rc = HTTP_OK;
      }
    }
  }
  return rc;
}

/**
 * @brief   Close a socket-like connection.
 * @param   In:  http_sock_hnd   HTTP socket-like handle.
 * @retval  Error code
 *            HTTP_OK   (0)  Success
 *            HTTP_ERR (<0)  Failure
 */
int http_sock_close(const http_sock_handle_t http_sock_hnd)
{
  int rc = HTTP_ERR;
  http_sock_context_t * sock_ctx = (http_sock_context_t *) http_sock_hnd;
  if (http_sock_hnd != NULL)
  {
    if (HTTP_OK == http_close(sock_ctx->http_hnd))
    {
      free(sock_ctx->pUrl);
      free(sock_ctx);
      rc = HTTP_OK;
    }
  }
  return rc;
}

/**
 * @brief   Receive data from an socket-like connection.
 * @param   Out: readbuffer     Destination buffer. Allocated by the caller.
 * @param   In:  size           Maximum length of the read data.
 * @param   In:  http_sock_hnd  HTTP socket-like handle.
 * @retval  >=0 Success: Number of bytes copied to the destination buffer.
 *           <0 Error:
 *                HTTP_ERR       Bad input parameter.
 *                HTTP_ERR_HTTP  HTTP progressive download error.
 */
int http_sock_rcv(uint8_t *const readbuffer, const int size, const http_sock_handle_t http_sock_hnd)
{
  int rc = HTTP_ERR;
  http_sock_context_t * sock_ctx = (http_sock_context_t *) http_sock_hnd;
  http_range_status_t httpRange = { 0, 0, 0, false };
  
  msg_info("Requesting %d bytes from pos %d, out of %d\n", size, sock_ctx->read_pos, sock_ctx->res_size);
  if ((http_sock_hnd != NULL) && (readbuffer != NULL))
  {
    if ((sock_ctx->res_size >= 0) && (sock_ctx->read_pos + size > sock_ctx->res_size))
    { /* Not supported. The size of the downloaded resource is constant. */
      msg_error("Tried to read out of file.\n");
      rc = HTTP_ERR_HTTP;
    }
    else
    {
      int hret = HTTP_OK;
      while (http_is_open(sock_ctx->http_hnd) == false) /* Loop until we get an open socket. */
      {
        http_close(sock_ctx->http_hnd);
        if (HTTP_OK == http_open(&sock_ctx->http_hnd, sock_ctx->pUrl))
        {
          do
          {
            hret = http_read(readbuffer, &httpRange, sock_ctx->read_pos, size, NULL, NULL, 0, sock_ctx->http_hnd);
          } while (hret == HTTP_ERR_HTTP); /* Retry - for debug purpose. */
        }
        else
        {
          msg_error("Could not reopen the HTTP socket. Trying once more.\n");
        }
      }
      
      if (hret >= 0)
      {
        if (sock_ctx->res_size < 0)
        {
          /* Initialize it.*/
          sock_ctx->res_size = httpRange.resource_size;
        }
        sock_ctx->read_pos += hret;
      }
      
      rc = hret;
    }
  }
  
  return rc;
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
