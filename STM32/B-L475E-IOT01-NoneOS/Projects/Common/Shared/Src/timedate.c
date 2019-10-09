/**
  ******************************************************************************
  * @file    timedate.c
  * @author  MCD Application Team
  * @version V0.0.1
  * @date    11-August-2017
  * @brief   Configuration of the RTC value.
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
#include <stdio.h>
#include "main.h"
#include "timedate.h"
#include "net.h"
#include "msg.h"

/** Host from which the time/date will be retrieved.*/
#define TIME_SOURCE_HTTP_HOST   "www.gandi.net"
#define TIME_SOURCE_HTTP_PORT   443
#define TIME_SOURCE_HTTP_PROTO  NET_PROTO_TLS

/** Size of the HTTP read buffer. 
 *  Should be large enough to contain a complete HTTP response header. */
#define NET_BUF_SIZE  1000

/* Private typedef -----------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
static const char http_request[] = "HEAD / HTTP/1.1\r\nHost: "TIME_SOURCE_HTTP_HOST"\r\n\r\n";

/* Functions Definition ------------------------------------------------------*/

/** 
 * @brief Set the RTC time and date from an HTTP response.
 * @param In:  force_apply     Force applying the time/date retrieved from the server, even if the server certificate verification failed.
                               Useful for initializing the RTC when it is not backed up by a battery. 
 * @note  Pre-conditions:
 *   . Wifi network connected
 *   . One free socket
 * @retval  Error code
 *            TD_OK             
 *            TD_ERR_CONNECT      Could not connect to the network and join the web server.
 *            TD_ERR_HTTP         Could not parse the time and date from the web server response.
 *            TD_ERR_RTC          Could not set the RTC.
 *            TD_ERR_TLS_CERT     The server certificate verification failed. Applicable only when force_apply is false.
 *                                .
 */
int setRTCTimeDateFromNetwork(bool force_apply)
{
  int rc = TD_OK;
  int ret = NET_OK;
  net_sockhnd_t socket = NULL;
  int len = strlen(http_request);
  char buffer[NET_BUF_SIZE + 1]; /* +1 to be sure that the buffer is closed by a \0, so that it may be parsed by string commands. */
  memset(buffer, 0, sizeof(buffer));
 
  ret = net_sock_create(hnet, &socket, TIME_SOURCE_HTTP_PROTO);
  if (ret != NET_OK)
  {
    msg_error("Could not create the socket.\n");
  }
  else
  {
#define NET_READ_TIMEOUT  "3000"
    ret |= net_sock_setopt(socket, "sock_read_timeout", (uint8_t*)NET_READ_TIMEOUT, sizeof(NET_READ_TIMEOUT));
    if (TIME_SOURCE_HTTP_PROTO == NET_PROTO_TLS)
    {
      ret |= net_sock_setopt(socket, "tls_ca_certs", (void *)lUserConfig.tls_root_ca_cert, strlen(lUserConfig.tls_root_ca_cert));
      ret |= net_sock_setopt(socket, "tls_server_name", (uint8_t*)TIME_SOURCE_HTTP_HOST, sizeof(TIME_SOURCE_HTTP_HOST));
      ret |= net_sock_setopt(socket, (force_apply == true) ? "tls_server_noverification" : "tls_server_verification", NULL, 0);
    }
  }
  
  if (ret != NET_OK)
  {
    msg_error("Could not set the socket options.\n");
  }
  else
  {
    ret = net_sock_open(socket, TIME_SOURCE_HTTP_HOST, TIME_SOURCE_HTTP_PORT);
  }
  
  if (ret == NET_AUTH)
  {
    msg_error("An incorrect system time may have resulted in a TLS authentication error.\n");
    rc = TD_ERR_TLS_CERT;
  }
  
  if ( (ret != NET_OK) || (rc != TD_OK) )
  {
    msg_error("Could not open the socket.\n");
  }
  else
  {
    ret = net_sock_send(socket, (uint8_t *) http_request, len);
    
    if (ret != len)
    {
      msg_error("Could not send %d bytes.\n", len);
    }
    else
    {
      char *dateStr = NULL;
      int read = 0;
      do
      {
        len = net_sock_recv(socket, (uint8_t *) buffer + read, NET_BUF_SIZE - read);
        if (len > 0)
        {
          read += len;
          dateStr = strstr(buffer, "Date: ");
        }
      } while ( (dateStr == NULL) && ((len >= 0) || (len == NET_TIMEOUT)) && (read < NET_BUF_SIZE));
      
      if (dateStr == NULL)
      {
        msg_error("No 'Date:' line found in the HTTP response header.\n");
        rc = TD_ERR_HTTP;
      }
      else
      {
        rc = TD_OK;
        char prefix[8], dow[8], month[4]; 
        int day, year, hour, min, sec;
      
        memset(dow, 0, sizeof(dow));
        memset(month, 0, sizeof(month));
        day = year = hour = min = sec = 0;
        
        int count = sscanf(dateStr, "%s %s %d %s %d %02d:%02d:%02d ", prefix, dow, &day, month, &year, &hour, &min, &sec);
        if (count < 8)
        {
          msg_error("At time initialization, only %d out of the 8 time/date data could be parsed from the HTTP response %s\n", count, buffer);
          rc = TD_ERR_HTTP;
        }
        else
        {
          char * str = strstr(dateStr, "\r\n");
          str[0] = '\0';
          msg_info("Configuring the RTC from %s\n", dateStr);
          
          RTC_TimeTypeDef sTime;
          sTime.Hours = hour;
          sTime.Minutes = min;
          sTime.Seconds = sec;
          sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
          sTime.StoreOperation = RTC_STOREOPERATION_RESET;
          if (HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN) != HAL_OK)
          {
            rc = TD_ERR_RTC;
          }

          RTC_DateTypeDef sDate;
          if (strcmp(dow, "Mon,") == 0) { sDate.WeekDay = RTC_WEEKDAY_MONDAY; } else
            if (strcmp(dow, "Tue,") == 0) { sDate.WeekDay = RTC_WEEKDAY_TUESDAY; } else
              if (strcmp(dow, "Wed,") == 0) { sDate.WeekDay = RTC_WEEKDAY_WEDNESDAY; } else  
                if (strcmp(dow, "Thu,") == 0) { sDate.WeekDay = RTC_WEEKDAY_THURSDAY; } else  
                  if (strcmp(dow, "Fri,") == 0) { sDate.WeekDay = RTC_WEEKDAY_FRIDAY; } else
                    if (strcmp(dow, "Sat,") == 0) { sDate.WeekDay = RTC_WEEKDAY_SATURDAY; } else
                      if (strcmp(dow, "Sun,") == 0) { sDate.WeekDay = RTC_WEEKDAY_SUNDAY; } else ret = -1;

          if (strcmp(month, "Jan") == 0) { sDate.Month = RTC_MONTH_JANUARY; } else
            if (strcmp(month, "Feb") == 0) { sDate.Month = RTC_MONTH_FEBRUARY; } else
              if (strcmp(month, "Mar") == 0) { sDate.Month = RTC_MONTH_MARCH; } else
                if (strcmp(month, "Apr") == 0) { sDate.Month = RTC_MONTH_APRIL; } else
                  if (strcmp(month, "May") == 0) { sDate.Month = RTC_MONTH_MAY; } else
                    if (strcmp(month, "Jun") == 0) { sDate.Month = RTC_MONTH_JUNE; } else
                      if (strcmp(month, "Jul") == 0) { sDate.Month = RTC_MONTH_JULY; } else
                        if (strcmp(month, "Aug") == 0) { sDate.Month = RTC_MONTH_AUGUST; } else
                          if (strcmp(month, "Sep") == 0) { sDate.Month = RTC_MONTH_SEPTEMBER; } else
                            if (strcmp(month, "Oct") == 0) { sDate.Month = RTC_MONTH_OCTOBER; } else
                              if (strcmp(month, "Nov") == 0) { sDate.Month = RTC_MONTH_NOVEMBER; } else
                                if (strcmp(month, "Dec") == 0) { sDate.Month = RTC_MONTH_DECEMBER; } else ret = -1;

          sDate.Date = day;
          sDate.Year = year - 2000;
          if (HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BIN) != HAL_OK)
          {
            rc = TD_ERR_RTC;
          }
        }
      }
    }
    
    ret = net_sock_close(socket);  
  }
  
  if (socket != NULL)
  {
    ret |= net_sock_destroy(socket);
  }
  /* Translate a socket closure error in network error. */
  if ((rc == TD_OK) && (ret != NET_OK))
  {
    rc = TD_ERR_CONNECT;
  }
  
  return rc;
}


/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
