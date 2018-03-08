/**
  ******************************************************************************
  * @file    c2c.c
  * @author  MCD Application Team
  * @version V0.0.1
  * @date    11-August-2017
  * @brief   C2C interface file.
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
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/

#include "c2c.h"
/* Private define ------------------------------------------------------------*/
#define C2C_CONTEXT_1   1

/* Private variables ---------------------------------------------------------*/

Ug96Object_t       Ug96C2cObj;

/* Private functions ---------------------------------------------------------*/
void C2C_ConvertIpAddrToString(const uint8_t* IpArray, char* ReturnString);


void C2C_ConvertIpAddrToString(const uint8_t* IpArray, char* ReturnString)
{
  snprintf((char*)ReturnString, 16, "%d.%d.%d.%d", IpArray[0], IpArray[1], IpArray[2], IpArray[3]);
}  

/* Public functions ---------------------------------------------------------*/
/**
  * @brief  Initialiaze the LowLayer part of the C2C core
  * @param  registration_timeout_sec: timeout (seconds)
  * @retval Operation status
  */
C2C_RegStatus_t C2C_Init(uint8_t registration_timeout_sec)
{
  C2C_RegStatus_t ret = C2C_REGSTATUS_UNKNOWN;
  uint8_t i=0;
  uint8_t ug96_init_status;
  
  ug96_init_status = UG96_RegisterBusIO(&Ug96C2cObj, 
                           UART_C2C_Init, 
                           UART_C2C_DeInit,
                           UART_C2C_SendData,
                           UART_C2C_SendOneChar,                           
                           UART_C2C_ReceiveData,
                           UART_C2C_FlushBuffer);
  
  HAL_Delay(3000);
  
  if(ug96_init_status == UG96_STATUS_OK)
  {
    
    if(UG96_Init(&Ug96C2cObj) == UG96_STATUS_READY)
    {
       while(i < registration_timeout_sec)
       {  
         i++; 
         ret = (C2C_RegStatus_t) UG96_GetNetworkRegistrationStatus(&Ug96C2cObj);
         if ((ret == C2C_REGSTATUS_HOME_NETWORK) || (ret == C2C_REGSTATUS_ROAMING))
         {
           i=registration_timeout_sec;
         }
         else
         {
           HAL_Delay(1000); 
         }
       }
    } 
    else
    {
      ret = C2C_REGSTATUS_ERROR;
    }
    
  }
  return ret;
}

/**
  * @brief  List a defined number of vailable operators (modem needs a minute to reply, don't use it)
  * @param  Qvalue: Quality Level
  * @retval Operation status
  */
C2C_Ret_t C2C_GetSignalQualityStatus(int32_t *Qvalue)
{
  C2C_Ret_t ret = C2C_RET_ERROR;  
  
  if(UG96_GetSignalQualityStatus(&Ug96C2cObj, Qvalue) == UG96_STATUS_OK)
  {
    ret = C2C_RET_OK;  
  }
  return ret;
}

/**
  * @brief  List a defined number of vailable operators (modem needs a minute to reply, don't use it)
  * @param  Operators: pointer to string
  * @retval Operation status
  */
C2C_Ret_t C2C_ListOperators(char *Operators)
{
  C2C_Ret_t ret = C2C_RET_ERROR;  
  
  if(UG96_ListOperators(&Ug96C2cObj, Operators) == UG96_STATUS_OK)
  {
    ret = C2C_RET_OK;  
  }
  return ret;
}

/**
  * @brief  Get current Operator
  * @param  Operators: pointer to string
  * @retval Operation status
  */
C2C_Ret_t C2C_GetCurrentOperator(char *Operator)
{
  C2C_Ret_t ret = C2C_RET_ERROR;  
  
  if(UG96_GetCurrentOperator(&Ug96C2cObj, Operator) == UG96_STATUS_OK)
  {
    ret = C2C_RET_OK;  
  }
  return ret;
}


/**
  * @brief  Force registration to specific Network Operator (by operator code).
  * @param  OperatorCode: http://www.imei.info/operator-codes/
  * @retval Operation Status.
  */
C2C_Ret_t  C2C_ForceOperator(int32_t OperatorCode)
{
  C2C_Ret_t ret = C2C_RET_ERROR;  
  
  if(UG96_ForceOperator(&Ug96C2cObj, OperatorCode) == UG96_STATUS_OK)
  {
    ret = C2C_RET_OK;  
  }
  return ret;  
}


/**
  * @brief  Configure a TCP/UDP Access Point
  * @param  ContextType : 1: IPV4
  * @param  Apn : access point name
  * @param  Username : Got IP Address
  * @param  Password : Network IP mask
  * @param  AuthenticationMode : 0: none, 1: PAP
  * @retval Operation status
  */
C2C_Ret_t C2C_ConfigureAP(uint8_t ContextType,
                          const char* Apn, 
                          const char* Username,
                          const char* Password,
                          uint8_t AuthenticationMode)
{
  C2C_Ret_t ret = C2C_RET_ERROR;  
  UG96_APConfig_t ApConfig;

  ApConfig.ContextID = C2C_CONTEXT_1;
  //strncpy((char*)ApConfig.ApnString, (char*)Apn, UG96_MAX_APN_NAME_SIZE);
  //strncpy((char*)ApConfig.Username, (char*)Username, UG96_MAX_USER_NAME_SIZE);
  //strncpy((char*)ApConfig.Password, (char*)Password, UG96_MAX_PSW_NAME_SIZE);
  strncpy((char*)ApConfig.ApnString, (char*)Apn, sizeof(ApConfig.ApnString));
  strncpy((char*)ApConfig.Username, (char*)Username, sizeof(ApConfig.Username));
  strncpy((char*)ApConfig.Password, (char*)Password, sizeof(ApConfig.Password));
  ApConfig.Authentication = (UG96_Authent_t) AuthenticationMode;

  if(UG96_ConfigureAP(&Ug96C2cObj, &ApConfig) == UG96_STATUS_OK)
  {
    ret = C2C_RET_OK;
  }

  return ret;
}


/**
  * @brief  Join a PDP Access Point
  * @retval Operation status
  */
C2C_Ret_t C2C_Connect(void)
{
  C2C_Ret_t ret = C2C_RET_ERROR;  
    
  if(UG96_Activate(&Ug96C2cObj, C2C_CONTEXT_1) == UG96_STATUS_OK)
  {
    ret = C2C_RET_OK;
  }
  return ret;  
}

/**
  * @brief  Leave a PDP Access Point
  * @retval Operation status
  */
C2C_Ret_t C2C_Disconnect(void)
{
  C2C_Ret_t ret = C2C_RET_ERROR;  
    
  if(UG96_Deactivate(&Ug96C2cObj, C2C_CONTEXT_1) == UG96_STATUS_OK)
  {
    ret = C2C_RET_OK;
  }
  return ret;  
}

/**
  * @brief  List the IP addresses for each context configured/activated
  * @param  IPaddr_string: pointer where to retrive the string with all active IP info 
  * @param  IPaddr_int: pointer where to retrive the first active IP adr in int_array[] format  
  * @retval Operation status
  */
C2C_Ret_t C2C_GetActiveIpAddresses(char *IPaddr_string, uint8_t* IPaddr_int)
{
  C2C_Ret_t ret = C2C_RET_ERROR;  
  
  if(UG96_GetActiveIpAddresses(&Ug96C2cObj, IPaddr_string, IPaddr_int) == UG96_STATUS_OK)
  {
    ret = C2C_RET_OK;  
  }
  return ret;
}

/**
  * @brief  Ping an IP address in the network
  * @param  host_addr_string: domain name (e.g. www.amazon.com) or dotted decimal IP addr
  * @param  count: PING repetitions (default 4) (max 10)
  * @param  timeout_sec: timeout for each repetition in seconds 
  * @retval Operation status
  */
C2C_Ret_t   C2C_Ping(char *host_addr_string, uint16_t count, uint16_t timeout_sec)
{
  C2C_Ret_t ret = C2C_RET_ERROR;  
  
  if(UG96_Ping(&Ug96C2cObj, C2C_CONTEXT_1, host_addr_string, count, timeout_sec) == UG96_STATUS_OK)
  {
    ret = C2C_RET_OK;  
  }
  return ret;
}




/**
  * @brief  for a given host name it enquires the DNS google service to resolve the Ip addr
  * @param  host_addr_string: domain name (e.g. www.amazon.com) 
  * @param  ipaddr : pointer where to store the retrieved Ip addr in uint8_t[4] format
  * @retval Operation status
  */
C2C_Ret_t       C2C_GetHostAddress(char* host_addr_string, uint8_t* ipaddr)
{
  C2C_Ret_t ret = C2C_RET_ERROR;  
  
  if(UG96_ResolveHostIPviaDNS(&Ug96C2cObj, C2C_CONTEXT_1, host_addr_string, ipaddr) == UG96_STATUS_OK)
  {
    ret = C2C_RET_OK;  
  }  
  return ret;
}


/**
  * @brief  Configure and start a client connection
  * @param  socket : Connection ID
  * @param  type : Connection type TCP/UDP/TCP_LISTENER/UDP_SERVICE
  * @param  host_url : name of the connection (e.g. www.amazon.com)
  * @param  ipaddr : Ip addr in array numbers uint8_t[4]
  * @param  port : Remote port
  * @param  local_port : Local port
  * @retval Operation status
  */
C2C_Ret_t C2C_OpenClientConnection(uint32_t socket, C2C_Protocol_t type, const char* host_url, uint8_t* ipaddr, uint16_t port, uint16_t local_port)
{
  C2C_Ret_t ret = C2C_RET_ERROR; 
  char converted_ipaddr[16];
  UG96_Conn_t conn;
  
  conn.ConnectID = socket;
  conn.RemotePort = port;
  conn.LocalPort = local_port;
  conn.Type = (type == C2C_TCP_PROTOCOL)? UG96_TCP_CONNECTION : UG96_UDP_CONNECTION;
#ifdef ACCESS_MODE_DIRECT_PUSH
  conn.AccessMode = UG96_DIRECT_PUSH; 
#else
  conn.AccessMode = UG96_BUFFER_MODE; 
#endif /* ACCESS_MODE_DIRECT_PUSH */
  
  if(host_url == NULL)        
  {
    C2C_ConvertIpAddrToString(ipaddr, converted_ipaddr);
    conn.Url = converted_ipaddr;
  }
  else
  {
    conn.Url = (char *)host_url; 
  }  
  if(UG96_OpenClientConnection(&Ug96C2cObj, C2C_CONTEXT_1, &conn) == UG96_STATUS_OPEN)
  {
    ret = C2C_RET_OK;
  }
  return ret;
}

   
/**
  * @brief  Configure and start a client connection
  * @param  socket : Connection ID
  * @retval Operation status
  */
C2C_Ret_t C2C_CloseClientConnection(uint32_t socket)
{
  C2C_Ret_t ret = C2C_RET_ERROR;  
  UG96_Conn_t conn;
  
  conn.ConnectID = socket;  
  if(UG96_CloseClientConnection(&Ug96C2cObj, &conn) == UG96_STATUS_OK)
  {
    ret = C2C_RET_OK;
  }
  return ret;
}


///**
//  * @brief  Configure and start a Server
//  * @param  type : Connection type TCP/UDP
//  * @param  name : name of the connection
//  * @param  port : Remote port
//  * @retval Operation status
//  */
//WIFI_Status_t WIFI_StartServer(uint32_t socket, WIFI_Protocol_t protocol, const char* name, uint16_t port)
//{
//  WIFI_Status_t ret = WIFI_STATUS_ERROR;
//  if(ESP8266_StartServer(&Esp8266WifiObj, port)== ESP8266_STATUS_OK)
//  {
//    ret = WIFI_STATUS_OK;
//  }
//  return ret;
//}
//
///**
//  * @brief  Stop a server
//  * @retval Operation status
//  */
//WIFI_Status_t WIFI_StopServer(uint32_t socket)
//{
//  WIFI_Status_t ret = WIFI_STATUS_ERROR;
//  
//  if(ESP8266_StopServer(&Esp8266WifiObj)== ESP8266_STATUS_OK)
//  {
//    ret = WIFI_STATUS_OK;
//  }
//  return ret;
//}


/**
  * @brief  Send Data on a socket
  * @param  pdata : pointer to data to be sent
  * @param  IN: Reqlen : length of data to be sent
  * @param  OUT: SentDatalen : Data actually sent
  * @param  Timeout: ms
  * @retval Operation status
  */
C2C_SendStatus_t C2C_SendData(uint32_t socket, uint8_t *pdata, uint16_t Reqlen, uint16_t *SentDatalen, uint32_t Timeout)
{
  C2C_SendStatus_t ret = C2C_SEND_ERROR;
  UG96_Status_t status;
  
  status = UG96_SendData(&Ug96C2cObj, socket, pdata, Reqlen, SentDatalen, Timeout);
  if( status == UG96_STATUS_SENT)
  {
    ret = C2C_SEND_OK;
  }
  if( status == UG96_STATUS_SENTFAIL)
  {
    ret = C2C_SEND_FAIL;
  }
  
  return ret;
}

/**
  * @brief  Receive Data from a socket
  * @param  pdata : pointer to Rx buffer
  * @param  IN: Reqlen : in UG96_BUFFER_MODE the req len, in UG96_DIRECT_PUSH is the max leng available in pdata[] buffer
  * @param  OUT: *RcvDatalen :  pointer to length of data
  * @param  Timeout: ms
  * @retval Operation status
  */
C2C_Ret_t C2C_ReceiveData(uint8_t socket, uint8_t *pdata, uint16_t Reqlen, uint16_t *RcvDatalen, uint32_t Timeout)
{
  C2C_Ret_t ret = C2C_RET_ERROR;  

  if(UG96_ReceiveData(&Ug96C2cObj, socket, pdata, Reqlen, RcvDatalen, Timeout) == UG96_STATUS_OK)
  {
    ret = C2C_RET_OK; 
  }
  return ret;
}

/**
  * @brief  Retrieve last error details from C2C modem
  * @param  ErrorString: pointer to string
  * @retval Operation status
  */
C2C_Ret_t C2C_RetrieveLastErrorDetails(char *ErrorString)
{
  C2C_Ret_t ret = C2C_RET_ERROR;  
  
  if(UG96_RetrieveLastErrorDetails(&Ug96C2cObj, ErrorString) == UG96_STATUS_OK)
  {
    ret = C2C_RET_OK;  
  }
  return ret;
}

///**
//  * @brief  Customize module data
//  * @param  name : MFC name
//  * @param  Mac :  Mac Address
//  * @retval Operation status
//  */
//WIFI_Status_t WIFI_SetOEMProperties(const char *name, uint8_t *Mac)
//{
//  WIFI_Status_t ret = WIFI_STATUS_ERROR; 
//  
//  if(ESP8266_SetSTMACAddress(&Esp8266WifiObj, Mac) == ESP8266_STATUS_OK)
//  {
//    ret = WIFI_STATUS_OK;
//  }
//  
//  return ret;
//}
//
///**
//  * @brief  Reset the WIFI module
//  * @retval Operation status
//  */
//WIFI_Status_t WIFI_ResetModule(void)
//{
//  WIFI_Status_t ret = WIFI_STATUS_ERROR; 
//  
//  if(ESP8266_ResetModule(&Esp8266WifiObj) == ESP8266_STATUS_OK)
//  {
//      ret = WIFI_STATUS_OK;
//  }
//  return ret;
//}
//
///**
//  * @brief  Restore module default configuration
//  * @retval Operation status
//  */
//WIFI_Status_t WIFI_SetModuleDefault(void)
//{
//  WIFI_Status_t ret = WIFI_STATUS_ERROR; 
//  
//  if(ESP8266_ResetToFactoryDefault(&Esp8266WifiObj) == ESP8266_STATUS_OK)
//  {
//      ret = WIFI_STATUS_OK;
//  }
//  return ret;
//}
//
//
///**
//  * @brief  Update module firmware
//  * @param  location : Binary Location IP address
//  * @retval Operation status
//  */
//WIFI_Status_t WIFI_ModuleFirmwareUpdate(const char *location)
//{
//  WIFI_Status_t ret = WIFI_STATUS_ERROR; 
//  
//  UNUSED(location);
//#if (ESP8266_USE_FIRMWAREUPDATE == 1) 
//  if(ESP8266_OTA_Upgrade(&Esp8266WifiObj) == ESP8266_STATUS_OK)
//  {
//      ret = WIFI_STATUS_OK;
//  }
//#else
//  ret = WIFI_STATUS_NOT_SUPPORTED;
//  
//#endif
//  return ret;  
//}
//
/**
  * @brief  Return Module firmware revision
  * @param  rev : revision string
  * @retval Operation status
  */
C2C_Ret_t C2C_GetModuleFwRevision(char *rev)
{
  C2C_Ret_t ret = C2C_RET_ERROR; 
  
  if(Ug96C2cObj.FW_Rev != NULL)
  {
    strncpy(rev, (char *)Ug96C2cObj.FW_Rev, C2C_IMEI_SIZE);
    ret = C2C_RET_OK; 
  }
  return ret;
}

/**
  * @brief  Return Product ID
  * @param  ProductId : Product ID string
  * @retval Operation status
  */
C2C_Ret_t C2C_GetModuleID(char *ProductId)
{
  C2C_Ret_t ret = C2C_RET_ERROR; 
  
  if(Ug96C2cObj.ProductID != NULL)
  {
    strncpy(ProductId, (char *)Ug96C2cObj.ProductID, C2C_PROD_ID_SIZE);
    ret = C2C_RET_OK; 
  }
  return ret;
}

/**
  * @brief  Return Module Name
  * @param  Info : Module Name string
  * @retval Operation status
  */
C2C_Ret_t C2C_GetModuleName(char *ModuleName)
{
  C2C_Ret_t ret = C2C_RET_ERROR; 
  
  if(Ug96C2cObj.Manufacturer != NULL)
  {
    strncpy(ModuleName, (char *)Ug96C2cObj.Manufacturer, C2C_MFC_SIZE);
    ret = C2C_RET_OK; 
  }
  return ret;
}


/**
  * @brief  Return Module Name
  * @param  SimId : returns the ICCID (Integrated Circuit Card Identifier) of the SIM card
  * @retval Operation status
  */
C2C_Ret_t C2C_GetSimId(char *SimId)
{
  C2C_Ret_t ret = C2C_RET_ERROR; 
  
  if(Ug96C2cObj.Manufacturer != NULL)
  {
    strncpy(SimId, (char *)Ug96C2cObj.SimInfo.ICCID, C2C_ICCID_SIZE);
    ret = C2C_RET_OK; 
  }
  return ret;
}
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

