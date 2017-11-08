/**
  ******************************************************************************
  * @file    cloud.c
  * @author  MCD Application Team
  * @version V1.0.0
  * @date    11-July-2017
  * @brief   .
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "main.h"
#include "net.h"
#include "timedate.h"
#include "heap.h"
#include "rfu.h"
#include "cloud.h"

extern int net_if_init(void * if_ctxt);
extern int net_if_deinit(void * if_ctxt);

/* Private defines -----------------------------------------------------------*/
#define CLOUD_TIMEDATE_TLS_VERIFICATION_IGNORE  /**< Accept to connect to a server which is not verified by TLS */

/* Private typedef -----------------------------------------------------------*/
typedef enum
{
  CLOUD_DEMO_WIFI_INITIALIZATION_ERROR      = -2,
  CLOUD_DEMO_MAC_ADDRESS_ERROR              = -3,
  CLOUD_DEMO_WIFI_CONNECTION_ERROR          = -4,
  CLOUD_DEMO_IP_ADDRESS_ERROR               = -5,
  CLOUD_DEMO_CONNECTION_ERROR               = -6,
  CLOUD_DEMO_TIMEDATE_ERROR                 = -7
} CLOUD_DEMO_Error_t;


/* Private function prototypes -----------------------------------------------*/
void CLOUD_Error_Handler(int errorCode);
int cloud_check_credential(void);

/* Exported functions --------------------------------------------------------*/
/**
  * @brief  Ask yes/no question.
  * @param  None
  * @retval None
  */
bool check(char *s)
{
  char console_yn;
  do
  {
    msg_info("%s",s);
    console_yn= getchar();
	msg_info("\b");
  }
  while((console_yn != 'y') && (console_yn != 'n') && (console_yn != '\n'));
  if (console_yn == 'y') return true;
  return false;
}


/**
  * @brief  This function is executed in case of error occurrence.
  * @param  None
  * @retval None
  */
void CLOUD_Error_Handler(int errorCode)
{
  switch (errorCode)
  {
    case (CLOUD_DEMO_WIFI_INITIALIZATION_ERROR):
    {
      msg_info("Error initializing the module!\n");
      
      break;
    }
    case (CLOUD_DEMO_MAC_ADDRESS_ERROR):
    {
      msg_info("Error detecting module!\n");
      
      break;
    }
    case (CLOUD_DEMO_WIFI_CONNECTION_ERROR):
    {
      msg_info("Error connecting to AP!\n");
      
      break;
    }
    case (CLOUD_DEMO_IP_ADDRESS_ERROR):
    {
      msg_info("Error retrieving IP address!\n");
      
      break;
    }
    case (CLOUD_DEMO_CONNECTION_ERROR):
    {
      msg_info("Error connecting to Cloud!\n");
      
      break;
    }
    case (CLOUD_DEMO_TIMEDATE_ERROR):
    {
      msg_info("Error initializing the RTC from the network time!\n");
      
      break;
    }
    default:
    {
      break;
    }
  }
  
  while (1)
  {
    BSP_LED_Toggle(LED_GREEN);
    HAL_Delay(200);
  }
}


int cloud_check_credential()
{
    if (checkTLSCredentials()) return 1;
    if (checkServerDevice()) return 1;
    return 0;
}

static void cloud_enter_credential(void)
{
  
  updateTLSCredentials();
  

#ifdef AWS
  if ((checkServerDevice() != 0) || check("Do you want to update your device credentials ? (y/n)\n")) 
  {
    AWSServerDeviceConfig();
   }
#endif

#ifdef AZURE
  if ((checkServerDevice() != 0) || check("Do you want to update your device credentials ? (y/n)\n"))
  {
    AzureDeviceConfig();
   }
#endif

#ifdef Baidu
  if ((checkServerDevice() != 0) || check("Do you want to update your device credentials ? (y/n)\n")) 
  {
    BaiduDeviceConfig();
   }
#endif
}


int platform_init(void)
{
  int updated_network_credential=0;
  int updated_cloud_credential=0;
  
  net_ipaddr_t ipAddr;
  net_macaddr_t macAddr;
  const firmware_version_t  * fw_version;

  rfu_getVersion(&fw_version, false);
  
#ifdef HEAP_DEBUG
  stack_measure_prologue();
#endif

  msg_info("\n");
  msg_info("**************************************************************************\n");
  msg_info("***   STM32 IoT Discovery kit for STM32L475 MCU    ***\n");
  msg_info("***   %s Cloud Connectivity Demonstration\n",
           fw_version->name);
  msg_info("***   FW version %d.%d.%d - %s, %s                         ***\n", 
           fw_version->major, fw_version->minor, fw_version->patch, fw_version->build_date, fw_version->build_time);
  msg_info("**************************************************************************\n");
    
  msg_info("\n*** Board personalisation ***\n\n");
  
#ifdef USE_WIFI
  //  /* Possibility to update the parameters if the user button is pushed*/
  if (network_check_credential()==0)
  {
    msg_info("Press the User button (Blue) within the next 5 seconds if you want to update the configuration\n"
         "(NetWork or Cloud security credentials)\n\n");

   if ( Button_WaitForPush(5000) )
   {
      if (check("Do you want to configure the Wifi credentials ? (y/n)\n"))  network_enter_credential();
   }
    updated_network_credential=1;

  }
#endif

//  if (cloud_check_credential()!=0)
//  {
//    updated_cloud_credential=1;
//    cloud_enter_credential();
//  }
//
//  /* Possibility to update the parameters if the user button is pushed and parameters have not been just entered */
//  if (updated_cloud_credential==0 || updated_network_credential==0)
//  {
//    msg_info("Press the User button (Blue) within the next 5 seconds if you want to update the configuration\n"
//         "(NetWork or Cloud security credentials)\n\n");
//
//    if ( Button_WaitForPush(5000) )
//    {
//#ifdef USE_WIFI
//      if (check("Do you want to configure the Wifi credentials ? (y/n)\n"))  network_enter_credential();
//#endif
//      if (check("Do you want to configure the Cloud credentials ? (y/n)\n")) cloud_enter_credential();
//    }
//  }

  if (net_init(&hnet, NET_IF, (net_if_init)) != NET_OK)
  {
    CLOUD_Error_Handler(CLOUD_DEMO_IP_ADDRESS_ERROR);
    return -1;
  }

  if (net_get_mac_address(hnet, &macAddr) == NET_OK)
  {
    msg_info("Mac address: %02x:%02x:%02x:%02x:%02x:%02x\n",
             macAddr.mac[0], macAddr.mac[1], macAddr.mac[2], macAddr.mac[3], macAddr.mac[4], macAddr.mac[5]);
  }
  else
  {
    CLOUD_Error_Handler(CLOUD_DEMO_MAC_ADDRESS_ERROR);
    return -1;

  }
    
  /* Slight delay since the module seems to take some time prior to being able
   to retrieve its IP address after a connection. */
  HAL_Delay(500);

  msg_info("Retrieving the IP address.\n");

  if (net_get_ip_address(hnet, &ipAddr) != NET_OK)
  {
    CLOUD_Error_Handler(CLOUD_DEMO_IP_ADDRESS_ERROR);
    return -1;
  }
  else
  {
    switch(ipAddr.ipv)
    {
      case NET_IP_V4:
        msg_info("IP address: %d.%d.%d.%d\n", ipAddr.ip[12], ipAddr.ip[13], ipAddr.ip[14], ipAddr.ip[15]);
        break;
      case NET_IP_V6:
      default:
        CLOUD_Error_Handler(CLOUD_DEMO_IP_ADDRESS_ERROR);
        return -1;
    }

  }
  /* End of network initialisation */

  msg_info("Setting the RTC from the network time.\n");
//#ifdef CLOUD_TIMEDATE_TLS_VERIFICATION_IGNORE
//  if (setRTCTimeDateFromNetwork(true) == TD_OK)
//#else   /* CLOUD_TIMEDATE_TLS_VERIFICATION_IGNORE */
//    if ( (setRTCTimeDateFromNetwork(false) == TD_OK) || (setRTCTimeDateFromNetwork(true) == TD_OK) )
//#endif  /* CLOUD_TIMEDATE_TLS_VERIFICATION_IGNORE */
//
//  {
//#if defined(RFU) && !defined(FIREWALL_MBEDLIB)
//    updateFirmwareVersion();
//#endif  /* RFU */
//  }
//  else
//  {
//    CLOUD_Error_Handler(CLOUD_DEMO_TIMEDATE_ERROR);
//    return -1;
//  }
  
#ifdef SENSOR
  int res = init_sensors();
  if(0 != res) 
  {
    msg_error("init_sensors returned error : %d\n", res);
  }
#endif /* SENSOR */
   
 return 0; 
}


void    platform_deinit()
{
   /* Start of Cloud connectivity demonstration */
  msg_info("\n*** Cloud connectivity demonstration ***\n\n");
  msg_info("Cloud connectivity demonstration completed\n");


  (void)net_deinit(hnet, (net_if_deinit));

#ifdef   HEAP_DEBUG  
  stack_measure_epilogue();
  uint32_t heap_max,heap_current,stack_size;
  
#ifdef   FIREWALL_MBEDLIB
  heap_firewall_stat(&heap_max,&heap_current,&stack_size);
  msg_info("FireWall Heap Max allocation 0x%x (%d), current allocation 0x%x (%d), Stack max size 0x%x (%d)\n",heap_max,heap_max,heap_current,heap_current,stack_size,stack_size);
#endif    

  heap_stat(&heap_max,&heap_current,&stack_size);  
  msg_info("Heap Max allocation 0x%x (%d), current allocation 0x%x (%d), Stack max size 0x%x (%d)\n",heap_max,heap_max,heap_current,heap_current,stack_size,stack_size);
#endif 
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
