/**
  ******************************************************************************
  * @file    ug96.h
  * @author  MCD Application Team
  * @version V0.0.1
  * @date    11-08-2017
  * @brief   header file for the ug96 module (C2C cellular modem).
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
#ifndef __UG96_H
#define __UG96_H

#ifdef __cplusplus
 extern "C" {
#endif  

/* Includes ------------------------------------------------------------------*/
#include "stdint.h"
#include "string.h"
#include "stdio.h"
#include "ug96_conf.h"  
 
/* Exported Constants --------------------------------------------------------*/ 
#define  RET_NONE           0x0000  /* RET_NONE shall be 0x0: don't change it */
#define  RET_CRLF           0x0001   
//#define  RET_REGISTER       0x0001  
#define  RET_OK             0x0002
#define  RET_READY          0x0004
#define  RET_ARROW          0x0008    
#define  RET_SENT           0x0010
#define  RET_OPEN           0x0020
#define  RET_ERROR          0x0040
//#define  RET_ALREADY_CONNECTED 0x0080
//#define  RET_CONNECT      0x0100
//#define  RET_CLOSED       0x0200
//#define  RET_NOCHANGE     0x0800
#define  RET_URC_CLOSED     0x0080   
#define  RET_URC_RECV       0x0100 
#define  RET_URC_IN_FULL    0x0200   
#define  RET_URC_INCOM      0x0400
#define  RET_URC_PDPDEACT   0x0800   
#define  RET_URC_DNS        0x1000   
#define  RET_SENTFAIL       0x2000
#define  RET_READ           0x4000 
#define  RET_SIM_ERROR_1    0x8000
#define  RET_SIM_ERROR_2    0x10000   
#define  NUM_RESPONSES  18

   
#define UG96_MAX_APN_NAME_SIZE                  32
#define UG96_MAX_USER_NAME_SIZE                 32
#define UG96_MAX_PSW_NAME_SIZE                  32
#define UG96_ERROR_STRING_SIZE                  40


#define UG96_TX_DATABUF_SIZE                1460
#define UG96_RX_DATABUF_SIZE                1500
   
#define UG96_TIMEOUT_SHORT                            100   /* 0,1 sec */
#define UG96_TIMEOUT_MID                             2000   /* 2 sec */
#define UG96_TIMEOUT_LONG                           20000   /* 20 sec */
#define UG96_TIMEOUT_MAX                           0xFFFF   /* 65 sec */
#define UG96_TIMEOUT_DEFAULT                         5000   /*  5 sec */
   
   
/* Exported macro-------------------------------------------------------------*/
#define MIN(a, b)  ((a) < (b) ? (a) : (b))


/* Exported typedef ----------------------------------------------------------*/   
typedef int8_t (*IO_Init_Func)( void);
typedef int8_t (*IO_DeInit_Func)( void);
typedef void (*IO_Flush_Func)(void);
typedef int16_t (*IO_Send_Func)( uint8_t *, uint16_t);
typedef int16_t (*IO_SendOne_Func)( uint8_t *, uint16_t, uint32_t);
typedef int32_t (*IO_Receive_Func)(uint8_t *, uint16_t , uint32_t, uint32_t);

typedef struct {
  uint32_t retval;
  char retstr[100];
} UG96_Ret_t;

typedef enum {
  UG96_SIM_READY       = 0x00,   
  UG96_SIM_PIN_NEEDED  = 0x01, 
  UG96_SIM_ERROR       = 0xFF,    
} UG96_SIMState_t;


typedef enum {
  UG96_AP_NOT_CONFIG  = 0x00,   
  UG96_AP_CONFIGURED  = 0x01, 
  UG96_AP_ACVTIVATED  = 0x02,   
  UG96_AP_ERROR       = 0xFF,    
} UG96_APState_t;

typedef enum {
  UG96_NRS_NOT_REGISTERED  = 0x00,   
  UG96_NRS_HOME_NETWORK    = 0x01, 
  UG96_NRS_TRYING          = 0x02,    
  UG96_NRS_REG_DENIED      = 0x03,   
  UG96_NRS_UNKNOWN         = 0x04, 
  UG96_NRS_ROAMING         = 0x05,
  UG96_NRS_ERROR           = 0xFF,    
} UG96_NetworkRegistration_Status_t;


//    typedef enum {
//      ESP8266_STATUS_OK = RET_OK,         
//      ESP8266_STATUS_READY = RET_READY,        
//      ESP8266_STATUS_LINKED = RET_LINKED,       
//      ESP8266_STATUS_SENT = RET_SENT,         
//      ESP8266_STATUS_UNLINK = RET_UNLINK,      
//      ESP8266_STATUS_ERROR = RET_ERROR,        
//      ESP8266_STATUS_ALREADY_CONNECTED = RET_ALREADY_CONNECTED,   
//      ESP8266_STATUS_CONNECT = RET_CONNECT,      
//      ESP8266_STATUS_CLOSED = RET_CLOSED,       
//      ESP8266_STATUS_NOCHANGE = RET_NOCHANGE,    
//      ESP8266_STATUS_SENTFAIL = RET_SENTFAIL,  
//      ESP8266_STATUS_ADDRESOLVED = RET_DNS,
//    }ESP8266_Status_t;

typedef enum {
  UG96_STATUS_OK = RET_OK,         
  UG96_STATUS_READY = RET_READY,        
  UG96_STATUS_OPEN = RET_OPEN,
  UG96_STATUS_SENT = RET_SENT,   
  UG96_STATUS_ERROR = RET_ERROR,    
  UG96_STATUS_SENTFAIL = RET_SENTFAIL,  
}UG96_Status_t;



//typedef enum {
//  ESP8266_MODE_SINGLE           = 0,
//  ESP8266_MODE_MULTI            = 1,
//}ESP8266_ConnMode_t;


typedef enum {
  UG96_TCP_CONNECTION           = 0,
  UG96_UDP_CONNECTION           = 1,
  UG96_TCP_LISTENER_CONNECTION  = 2,
  UG96_UDP_SERVER_CONNECTION    = 3,     
}UG96_ConnType_t;

typedef enum {
  UG96_BUFFER_MODE           = 0,
  UG96_DIRECT_PUSH           = 1,
  UG96_TRANSPARENT_MODE      = 2,    
}UG96_AccessMode_t;


/**
 * \brief  Authentication settings for C2C network
 */
typedef enum {
  UG96_AUTHENT_NONE     = 0x00,
  UG96_AUTHENT_PAP      = 0x01,
  UG96_AUTHENT_CHAP     = 0x02,
  UG96_AUTHENT_PAP_CHAP = 0x03
}UG96_Authent_t;



typedef enum {
  UG96_UART_FLW_CTL_NONE     = 0x00,   
  UG96_UART_FLW_CTL_RTS      = 0x01, 
  UG96_UART_FLW_CTL_CTS      = 0x02,  
  UG96_UART_FLW_CTL_RTS_CTS  = 0x03,    
} UG96_UART_FLW_CTL_t;

//typedef enum {
//  ESP8266_AutoConnect_Off = 0x00, /*!< Disables automatic connection to AP on power up */
//  ESP8266_AutoConnect_On = 0x01   /*!< Enables automatic connection to AP on power up */
//} ESP8266_AutoConnect_t;

typedef struct
{
  uint32_t BaudRate;          
  uint32_t FlowControl;   
}UG96_UARTConfig_t;



typedef struct {
  uint8_t ContextID;  /*!< range is 1-20 */
  uint8_t ContextType;     /*!< shall be 1 (IpV */
  uint8_t ApnString[25];  /*!< access point name, string of chars */
  uint8_t Username[25];   /*!< user name, string of chars */
  uint8_t Password[25];   /*!< passoword, string of chars */
  UG96_Authent_t Authentication;
} UG96_APConfig_t;


typedef struct {
  UG96_ConnType_t    Type;     
  UG96_AccessMode_t  AccessMode;  
  uint8_t            ConnectID;             
  uint16_t           RemotePort;         
  uint16_t           LocalPort;          
//  uint8_t            RemoteIP[4];    /* UG96 uses just the string */    
  char*              Url;  
} UG96_Conn_t;

typedef struct {
  UG96_ConnType_t    Type;     
  UG96_AccessMode_t  AccessMode;
  uint16_t           ComulatedQirdData;
  uint16_t           HaveReadLenght;
  uint16_t           UnreadLenght;
  int16_t            UartRemaining; /* if Timeout respects UART speed this should always be 0 */  
} UG96_Socket_t;

typedef struct {
  IO_Init_Func       IO_Init;  
  IO_DeInit_Func     IO_DeInit;
  IO_Flush_Func      IO_FlushBuffer;  
  IO_Send_Func       IO_Send;
  IO_SendOne_Func    IO_SendOne;
  IO_Receive_Func    IO_Receive;  
} UG96_IO_t;

typedef struct {
  UG96_SIMState_t   SimStatus;  
  uint8_t           RegistStatusString[3];  
  uint8_t           IMSI [C2C_IMSI_SIZE];  
  uint8_t           ICCID [C2C_ICCID_SIZE];
} UG96_SIMInfo_t;



typedef struct {
  uint8_t                Manufacturer[C2C_MFC_SIZE];  
  uint8_t                ProductID[C2C_PROD_ID_SIZE];  
  uint8_t                FW_Rev[C2C_FW_REV_SIZE];  
  uint8_t                Imei[C2C_IMEI_SIZE];  /*International Mobile Equipment Identity*/
  UG96_SIMInfo_t         SimInfo;   
  uint8_t                APsActive;
  uint8_t                APContextState[UG96_MAX_CONTEXTS];  /* to decide if keeping all UG96_APConfig_t info. maybe at c2c SW level*/  
  UG96_Socket_t          SocketInfo[UG96_MAX_SOCKETS];  /* to decide if keeping all UG96_Conn_t info. maybe at c2c SW level*/
  UG96_UARTConfig_t      UART_Config;  
  UG96_IO_t              fops;
  uint8_t                CmdData[UG96_CMD_SIZE];
  uint32_t               Timeout;
  uint32_t               ForceTimeout; /* if !+ 0 this value is taken */
  uint32_t               RemRevdData;
  uint32_t               BufferSize; 
}Ug96Object_t;



/* Exported functions --------------------------------------------------------*/

/* ==== Init ==== */
UG96_Status_t  UG96_Init(Ug96Object_t *Obj);
UG96_Status_t  UG96_GetSignalQualityStatus(Ug96Object_t *Obj, int32_t *Qvalue);
UG96_NetworkRegistration_Status_t  UG96_GetNetworkRegistrationStatus(Ug96Object_t *Obj);

UG96_Status_t  UG96_GetManufacturer( Ug96Object_t *Obj, uint8_t *Manufacturer);
UG96_Status_t  UG96_GetProductID(Ug96Object_t *Obj, uint8_t *ProductID);
UG96_Status_t  UG96_GetFWRevID(Ug96Object_t *Obj, uint8_t *Fw_ver);

UG96_Status_t  UG96_SetTimeout(Ug96Object_t *Obj, uint32_t Timeout);
UG96_Status_t  UG96_ListOperators(Ug96Object_t *Obj, char *Operators);
UG96_Status_t  UG96_GetCurrentOperator(Ug96Object_t *Obj, char *Operator);
UG96_Status_t  UG96_ForceOperator(Ug96Object_t *Obj, int32_t OperatorCode);

/* ==== AP Connection ==== */


UG96_Status_t UG96_ConfigureAP(Ug96Object_t *Obj, UG96_APConfig_t *ApConfig);
UG96_Status_t  UG96_Activate(Ug96Object_t *Obj, uint8_t ContextID);
UG96_Status_t  UG96_Deactivate(Ug96Object_t *Obj, uint8_t ContextID);
UG96_APState_t  UG96_IsActivated(Ug96Object_t *Obj, uint8_t ContextID);

/* ====IP Addr ==== */

UG96_Status_t  UG96_GetActiveIpAddresses(Ug96Object_t *Obj, char *IPaddr_string, uint8_t* IPaddr_int);

//ESP8266_Status_t  ESP8266_DNS_LookUp(ESP8266Object_t *Obj, const char *url, uint8_t *ipaddress);

/* To be done */

/* ==== Reset Settings ==== */

//ESP8266_Status_t  ESP8266_ResetToFactoryDefault(ESP8266Object_t *Obj);
//ESP8266_Status_t  ESP8266_ResetModule(ESP8266Object_t *Obj);
//ESP8266_Status_t  ESP8266_SetProductName(ESP8266Object_t *Obj, uint8_t *ProductName);


/* To be done */

/* ==== Ping ==== */

#if (UG96_USE_PING == 1)
UG96_Status_t  UG96_Ping(Ug96Object_t *Obj, uint8_t ContextID, char *host_addr_string, uint16_t count, uint16_t rep_delay_sec);
#endif

/* ==== Client connection ==== */

UG96_Status_t  UG96_ResolveHostIPviaDNS(Ug96Object_t *Obj, uint8_t ContextID, char *IPaddr_string, uint8_t* IPaddr_int);

  // ESP8266_Status_t  ESP8266_StartServer(ESP8266Object_t *Obj, uint32_t Port);
  // ESP8266_Status_t  ESP8266_StopServer(ESP8266Object_t *Obj);

UG96_Status_t  UG96_OpenClientConnection(Ug96Object_t *Obj, uint8_t ContextID, UG96_Conn_t *conn);
UG96_Status_t  UG96_CloseClientConnection(Ug96Object_t *Obj, UG96_Conn_t *conn);

UG96_Status_t UG96_SendData(Ug96Object_t *Obj, uint8_t Socket, uint8_t *pdata, uint16_t Reqlen , uint16_t *SentLen , uint32_t Timeout);
UG96_Status_t UG96_ReceiveData(Ug96Object_t *Obj, uint8_t Socket, uint8_t *pdata, uint16_t Reqlen, uint16_t *Receivedlen, uint32_t Timeout);

UG96_Status_t  UG96_RetrieveLastErrorDetails(Ug96Object_t *Obj, char *error_string);

//#if (ESP8266_USE_FIRMWAREUPDATE == 1)
//ESP8266_Status_t  ESP8266_OTA_Upgrade(ESP8266Object_t *Obj);
//#endif

UG96_Status_t  UG96_SetUARTConfig(Ug96Object_t *Obj, UG96_UARTConfig_t *pconf, int8_t bd_or_fc);
UG96_Status_t  UG96_GetUARTConfig(Ug96Object_t *Obj, UG96_UARTConfig_t *pconf);

UG96_Status_t  UG96_RegisterBusIO(Ug96Object_t *Obj, IO_Init_Func IO_Init,
                                                              IO_DeInit_Func  IO_DeInit,
                                                              IO_Send_Func    IO_Send,
                                                              IO_SendOne_Func    IO_SendOne,
                                                              IO_Receive_Func  IO_Receive,
                                                              IO_Flush_Func   IO_Flush);
#ifdef __cplusplus
}
#endif
#endif /*__UG96_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/ 
