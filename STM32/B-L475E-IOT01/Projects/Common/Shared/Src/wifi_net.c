/**
  ******************************************************************************
  * @file    wifi_net.c
  * @author  MCD Application Team
  * @version V0.0.1
  * @date    11-August-2017
  * @brief   Wifi-specific network initialization.
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
#include "main.h"
#include "wifi.h"
#include "iot_flash_config.h"

#define  WIFI_CONNECT_MAX_ATTEMPT_COUNT  3

#ifdef ES_WIFI_MAX_SSID_NAME_SIZE
#define WIFI_PRODUCT_INFO_SIZE                      ES_WIFI_MAX_SSID_NAME_SIZE
#define WIFI_PAYLOAD_SIZE                           ES_WIFI_PAYLOAD_SIZE
#endif

int net_if_init(void * if_ctxt);
int net_if_deinit(void * if_ctxt);

int net_if_init(void * if_ctxt)
{
  const char *ssid;
  const char  *psk;
  WIFI_Ecn_t security_mode;
  char moduleinfo[WIFI_PRODUCT_INFO_SIZE];
  WIFI_Status_t wifiRes;
  uint8_t macAddress[6];
  int wifiConnectCounter = 0;  

  printf("\n*** WIFI connection ***\n\n");
    
  while (checkWiFiCredentials(&ssid, &psk, (uint8_t *) &security_mode) != HAL_OK)
  {
    printf("Your WIFI parameters need to be entered to proceed\n");
    updateWiFiCredentials(&ssid, &psk, (uint8_t *) &security_mode);
  }
  
  /*  Wifi Module initialization */
  printf("Initializing the WiFi module\n");
  
  wifiRes = WIFI_Init();
  if ( WIFI_STATUS_OK != wifiRes )
  {
    printf("Failed to initialize WIFI module\n");
    return -1;
  }
    
  /* Retrieve the WiFi module mac address to confirm that it is detected and communicating. */
  WIFI_GetModuleName(moduleinfo);
  printf("Module initialized successfully: %s",moduleinfo);
  
  WIFI_GetModuleID(moduleinfo);
  printf(" %s",moduleinfo);
  
  WIFI_GetModuleFwRevision(moduleinfo);
  printf(" %s\n",moduleinfo);
  
  printf("Retrieving the WiFi module MAC address:");
  wifiRes = WIFI_GetMAC_Address( (uint8_t*)macAddress);
  if ( WIFI_STATUS_OK == wifiRes)
  {
    printf(" %02x:%02x:%02x:%02x:%02x:%02x\n",
         macAddress[0], macAddress[1], macAddress[2],
         macAddress[3], macAddress[4], macAddress[5]);
  }
  else
  {
       printf("Failed to get MAC address\n");
  }
  /* Connect to the specified SSID. */

  printf("\n");
  do 
  {
    printf("\rConnecting to AP: %s  Attempt %d/%d ...",ssid, ++wifiConnectCounter,WIFI_CONNECT_MAX_ATTEMPT_COUNT);
    wifiRes = WIFI_Connect(ssid, psk, security_mode);
    if (wifiRes == WIFI_STATUS_OK) break;
  } 
  while (wifiConnectCounter < WIFI_CONNECT_MAX_ATTEMPT_COUNT);
  
  if (wifiRes == WIFI_STATUS_OK)
  {
    printf("\nConnected to AP %s\n",ssid);
  }
  else
  {
    printf("\nFailed to connect to AP %s\n",ssid);
  }
  
  return (wifiRes == WIFI_STATUS_OK)?0:-1;
}



int net_if_deinit(void * if_ctxt)
{
  return 0;
}


int network_check_credential(void)
{
  const char *ssid;
  const char *psk;
  uint8_t   security_mode;
  return checkWiFiCredentials(&ssid, &psk, &security_mode);
}



void network_enter_credential(void)
{
  const char *ssid;
  const char  *psk;
  WIFI_Ecn_t security_mode;

  updateWiFiCredentials(&ssid, &psk, (uint8_t *) &security_mode);
}


/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
