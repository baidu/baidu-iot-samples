/**
  ******************************************************************************
  * @file    c2c.h
  * @author  MCD Application Team
  * @version V0.0.1
  * @date    11-August-2017
  * @brief   This file contains the diffrent c2c core resources definitions.
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
#ifndef __C2C_H_
#define __C2C_H_

#ifdef __cplusplus
 extern "C" {
#endif
/* Includes ------------------------------------------------------------------*/   
#include "ug96.h"
#include "ug96_io.h"

/* Exported constants --------------------------------------------------------*/

#define C2C_OPERATOR_STRING           20
#define C2C_OPERATORS_LIST            6*C2C_OPERATOR_STRING
#define C2C_IPADDR_STRING             35
#define C2C_IPADDR_LIST               3*C2C_IPADDR_STRING
#define C2C_ERROR_STRING              UG96_ERROR_STRING_SIZE


#define C2C_MAX_NOS                  100  /* network operators */
#define C2C_MAX_CONNECTIONS          10 // TODO: Double-check from the C2C module datasheet
#if (UG96_RX_DATABUF_SIZE > 1500)
#define C2C_PAYLOAD_SIZE             1500  
#else   
#define C2C_PAYLOAD_SIZE             UG96_RX_DATABUF_SIZE  
#endif
   
/* Exported types ------------------------------------------------------------*/
typedef enum {
  C2C_IPV4 = 0x01,   
  C2C_IPV6 = 0x02,   
} C2C_IPVer_t;

typedef enum {
  C2C_AUTHENT_NONE     = 0x00,
  C2C_AUTHENT_PAP      = 0x01,
  C2C_AUTHENT_CHAP     = 0x02,
  C2C_AUTHENT_PAP_CHAP = 0x03  
}C2C_Authent_t;


typedef enum {
  C2C_TCP_PROTOCOL = 0,
  C2C_UDP_PROTOCOL = 1,
//  C2C_TCP_LISTENER_PROTOCOL = 2, /* do I need ? */
//  C2C_UDP_SERVICE_PROTOCOL = 3   /* do I nned ? */
}C2C_Protocol_t;


//typedef enum {
//  WIFI_SERVER = 0,
//  WIFI_CLIENT = 1,
//}WIFI_Type_t;


typedef enum {
  C2C_REGSTATUS_NOT_REGISTERED  = 0x00,   
  C2C_REGSTATUS_HOME_NETWORK    = 0x01, 
  C2C_REGSTATUS_TRYING          = 0x02,    
  C2C_REGSTATUS_REG_DENIED      = 0x03,   
  C2C_REGSTATUS_UNKNOWN         = 0x04, 
  C2C_REGSTATUS_ROAMING         = 0x05,   
  C2C_REGSTATUS_ERROR           = 0xFF,  
} C2C_RegStatus_t;

typedef enum {
  C2C_RET_OK = 0x00,         
  C2C_RET_ERROR = 0xFF        
} C2C_Ret_t;

typedef enum {
  C2C_SEND_OK = 0x00,         
  C2C_SEND_FAIL = 0x01,         
  C2C_SEND_ERROR = 0x02        
} C2C_SendStatus_t;

  /**
 * \brief  Connection structure
 */



/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
C2C_RegStatus_t  C2C_Init(uint8_t registration_timeout_sec);
C2C_Ret_t        C2C_GetSignalQualityStatus(int32_t *Qvalue);
C2C_Ret_t        C2C_ListOperators(char *Operators);
C2C_Ret_t        C2C_GetCurrentOperator(char *Operator);
C2C_Ret_t        C2C_ForceOperator(int32_t OperatorCode);

C2C_Ret_t       C2C_ConfigureAP(uint8_t ContextType,
                                const char* Apn, 
                                const char* Username,
                                const char* Password,
                                uint8_t AuthenticationMode);
C2C_Ret_t       C2C_Connect(void);
C2C_Ret_t       C2C_Disconnect(void);

C2C_Ret_t       C2C_GetActiveIpAddresses(char *IPaddr_string, uint8_t* IPaddr_int);


C2C_Ret_t       C2C_Ping(char *host_addr_string, uint16_t count, uint16_t timeout_sec);
C2C_Ret_t       C2C_GetHostAddress(char* location, uint8_t* ipaddr);

C2C_Ret_t       C2C_OpenClientConnection(uint32_t socket, C2C_Protocol_t type, const char* host_url, uint8_t* ipaddr, uint16_t port, uint16_t local_port);
C2C_Ret_t       C2C_CloseClientConnection(uint32_t socket);

//WIFI_Status_t       WIFI_StartServer(uint32_t socket, WIFI_Protocol_t type, const char* name, uint16_t port);
//WIFI_Status_t       WIFI_StopServer(uint32_t socket);
//
//WIFI_Status_t       WIFI_StartClient(void);
//WIFI_Status_t       WIFI_StopClient(void);

C2C_SendStatus_t C2C_SendData(uint32_t socket, uint8_t *pdata, uint16_t Reqlen, uint16_t *SentDatalen, uint32_t Timeout);
C2C_Ret_t        C2C_ReceiveData(uint8_t socket, uint8_t *pdata, uint16_t Reqlen, uint16_t *RcvDatalen, uint32_t Timeout);

C2C_Ret_t C2C_RetrieveLastErrorDetails(char *ErrorString);

//WIFI_Status_t       WIFI_SetOEMProperties(const char *name, uint8_t *Mac);
//WIFI_Status_t       WIFI_ResetModule(void);
//WIFI_Status_t       WIFI_SetModuleDefault(void);
//WIFI_Status_t       WIFI_ModuleFirmwareUpdate(const char *url);

C2C_Ret_t C2C_GetModuleID(char *Id);
C2C_Ret_t C2C_GetModuleFwRevision(char *rev);
C2C_Ret_t C2C_GetModuleName(char *ModuleName);
C2C_Ret_t C2C_GetSimId(char *SimId);

#ifdef __cplusplus
}
#endif

#endif /* __C2C_H_ */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
