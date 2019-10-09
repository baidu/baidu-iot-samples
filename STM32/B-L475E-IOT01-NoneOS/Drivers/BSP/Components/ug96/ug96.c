/**
  ******************************************************************************
  * @file    ug96.c
  * @author  MCD Application Team
  * @version V0.0.1
  * @date    11-08-2017
  * @brief   Functions to manage the ug96 module (C2C cellular modem).
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
#include "ug96.h"

//#define UG96_DBG

#define CHARISHEXNUM(x)                 (((x) >= '0' && (x) <= '9') || \
                                         ((x) >= 'a' && (x) <= 'f') || \
                                         ((x) >= 'A' && (x) <= 'F'))

#define CHARISNUM(x)                    ((x) >= '0' && (x) <= '9')
#define CHAR2NUM(x)                     ((x) - '0')

#define CTRL_Z 26

/* Private variable ---------------------------------------------------------*/
char CmdString[UG96_CMD_SIZE];

/* Exported variable ---------------------------------------------------------*/

 /* This should be handled differently, _IO should not depend by component */ 
const UG96_Ret_t ReturnValues[] = {
  // { RET_ALREADY_CONNECTED, "LINK IS BUILDED\r\n" },
  // { RET_NOCHANGE,          "no change\r\n" },
   /* send receive related keywords */
   { RET_SENT,                "SEND OK\r\n" },
   { RET_SENTFAIL,            "SEND FAIL\r\n" },
   { RET_ARROW,               ">" },   
   { RET_URC_CLOSED,          "closed\""  },   
   { RET_URC_RECV,            "recv\""  },   
   { RET_URC_IN_FULL,         "incoming full\""  },   
   { RET_URC_INCOM,           "incoming\""  },    
   { RET_URC_PDPDEACT,        "pdpdeact\""  },    
   { RET_URC_DNS,             "dnsgip\""  },    
   { RET_READ,                "QIRD: " },
   /* errors keywords */   
   { RET_ERROR,               "ERROR\r\n" },
   { RET_SIM_ERROR_1,             "SIM NOT INSERTED\r\n" },  
   { RET_SIM_ERROR_2,             "not allowed\r\n" },   
   /* set-up keywords */   
   { RET_OK,                  "OK\r\n" },
   { RET_OPEN,                "OPEN:" },   
   { RET_READY,               "ready\r\n" },
  // { RET_CLOSED,            ",CLOSED\r\n" },
  // { RET_CONNECT,           ",CONNECT\r\n" },
   { RET_CRLF,                "\r\n" },
   { RET_NONE,                "\r\n" },
};

/* Private functions ---------------------------------------------------------*/
/**
  * @brief  Convert char in Hex format to integer.
  * @param  a: character to convert
  * @retval integer value.
  */
//static  uint8_t Hex2Num(char a) 
//{
//    if (a >= '0' && a <= '9') {                             /* Char is num */
//        return a - '0';
//    } else if (a >= 'a' && a <= 'f') {                      /* Char is lowercase character A - Z (hex) */
//        return (a - 'a') + 10;
//    } else if (a >= 'A' && a <= 'F') {                      /* Char is uppercase character A - Z (hex) */
//        return (a - 'A') + 10;
//    }
//    
//    return 0;
//}

/**
  * @brief  Extract a hex number from a string.
  * @param  ptr: pointer to string
  * @param  cnt: pointer to the number of parsed digit
  * @retval Hex value.
  */
//static uint32_t ParseHexNumber(char* ptr, uint8_t* cnt) 
//{
//    uint32_t sum = 0;
//    uint8_t i = 0;
//    
//    while (CHARISHEXNUM(*ptr)) {                    		/* Parse number */
//        sum <<= 4;
//        sum += Hex2Num(*ptr);
//        ptr++;
//        i++;
//    }
//    
//    if (cnt != NULL) {                               		/* Save number of characters used for number */
//        *cnt = i;
//    }
//    return sum;                                        		/* Return number */
//}

/**
  * @brief  Parses and returns number from string.
  * @param  ptr: pointer to string
  * @param  cnt: pointer to the number of parsed digit
  * @retval integer value.
  */
static int32_t ParseNumber(char* ptr, uint8_t* cnt) 
{
    uint8_t minus = 0, i = 0;
    int32_t sum = 0;
    
    if (*ptr == '-') {                                		/* Check for minus character */
        minus = 1;
        ptr++;
        i++;
    }
    while (CHARISNUM(*ptr)) {                        		/* Parse number */
        sum = 10 * sum + CHAR2NUM(*ptr);
        ptr++;
        i++;
    }
    if (cnt != NULL) {                                		/* Save number of characters used for number */
        *cnt = i;
    }
    if (minus) {                                    		/* Minus detected */
        return 0 - sum;
    }
    return sum;                                       		/* Return number */
}

/**
  * @brief  Parses and returns QIRQ query response.
  * @param  ptr: pointer to string
  * @param  arr: pointer to IP array
  * @retval None.
  */
static  void ParseQIRD(char* ptr, uint16_t* arr) 
{
  uint8_t hexnum = 0, hexcnt;
  
  while(* ptr) { 
    hexcnt = 1;
    if(*ptr != ',')
    {
      arr[hexnum++] = (uint16_t) ParseNumber(ptr, &hexcnt);
    }
    ptr = ptr + hexcnt;
    if((*ptr == '\r') || (hexnum == 3))
    {
      return;
    }
  }
}


/**
  * @brief  Parses and returns IP address.
  * @param  ptr: pointer to string
  * @param  arr: pointer to IP array
  * @retval None.
  */
static  void ParseIP(char* ptr, uint8_t* arr) 
{
  uint8_t hexnum = 0, hexcnt;
  
  while(* ptr) { 
    hexcnt = 1;
    if(*ptr != '.')
    {
      arr[hexnum++] = ParseNumber(ptr, &hexcnt);
    }
    ptr = ptr + hexcnt;
    if(*ptr == '"')
    {
      return;
    }
  }
}


/**
  * @brief  Execute AT command.
  * @param  Obj: pointer to module handle
  * @param  cmd: pointer to command string
  * @param  resp: expected response
  * @retval Operation Status: OK or ERROR.
  */
static int32_t AT_ExecuteCommand(Ug96Object_t *Obj, uint8_t* cmd, uint32_t resp)
{
  uint32_t timeout = Obj->Timeout;
  int32_t ret = -2;
  
  if (Obj->ForceTimeout > 0)
  {
    timeout = Obj->ForceTimeout;
  }
  if(Obj->fops.IO_Send(cmd, strlen((char*)cmd)) > 0)
  {
    ret = (Obj->fops.IO_Receive(Obj->CmdData, 0, resp, timeout));
    if (ret < 0)
    {
#if 1 //UG96_DBG
      printf("                             UG96 AT_ExecuteCommand() rcv TIMEOUT: %s \n", cmd);
#endif
    }
  }
  else 
  {
#if 1 //UG96_DBG
    printf("                             UG96 AT_ExecuteCommand() send ERROR: %s \n", cmd);
#endif
  }
  Obj->ForceTimeout = 0;  

  return ret;
}

/**
  * @brief  Execute AT command with data.
  * @param  Obj: pointer to module handle
  * @param  len: binary data length
  * @param  pdata: pointer to returned data
  * @retval Operation Status.
  */
static UG96_Status_t AT_RequestSendData(Ug96Object_t *Obj, uint8_t *pdata, uint16_t len, uint32_t Timeout)
{   
  int16_t count;
   
  if(Obj->fops.IO_Send(pdata, len) == len)
  {      
      count = Obj->fops.IO_Receive(Obj->CmdData, 0, RET_SENT, Timeout);
      if(count> 0 )
      { 
        return UG96_STATUS_SENT;
      }
      else
      {  
        return UG96_STATUS_SENTFAIL;
      }  
  }

  return UG96_STATUS_ERROR;
}

/**
  * @brief  Retrive URC header and decode it
  * @param  Obj: pointer to module handle
  * @param  ParseLenght : pointer to return the length parsed from URC info
  * @param  Timeout : ms
  * @retval Operation Status.
  */

static int16_t AT_RetrieveUrc(Ug96Object_t *Obj, uint16_t *ParseLenght, UG96_AccessMode_t AccMode, uint32_t Timeout)
{
  int16_t check_resp, ret;  
  uint16_t expected_resp;
  uint8_t parse_ret;
  uint8_t parse_count, ip_count;  
  uint32_t count = 0;
  
  expected_resp = RET_URC_CLOSED | RET_URC_RECV | RET_URC_IN_FULL | RET_URC_INCOM | RET_URC_PDPDEACT | RET_URC_DNS;
  check_resp =  Obj->fops.IO_Receive(Obj->CmdData, 0, expected_resp, Timeout);

  switch (check_resp) {
    case RET_URC_CLOSED:
      Obj->fops.IO_Receive(Obj->CmdData, 0, RET_CRLF, Timeout);
      parse_ret = ParseNumber((char *) Obj->CmdData+1, &parse_count);
      break;
    case RET_URC_RECV:
      /* retrieve contextID */
      ret = Obj->fops.IO_Receive(Obj->CmdData, 3, RET_NONE, Timeout);
      if (ret > 0)
      {  
        parse_ret = ParseNumber((char *) Obj->CmdData+1, &parse_count);
        if (parse_count == 2) 
        { /* read next comma */
          ret = Obj->fops.IO_Receive(Obj->CmdData, 1, RET_NONE, Timeout);
        }
      }  
      if (AccMode == UG96_BUFFER_MODE)
      {
          ret = Obj->fops.IO_Receive(Obj->CmdData, 1, RET_NONE, Timeout);
      }
      else
      {  
        for( ;count < 6; count++)
        {
          if (ret < 0)
          { 
            break;
          } 
          ret = Obj->fops.IO_Receive(&Obj->CmdData[count], 1, RET_NONE, Timeout);
          if(ret == 1)
          {
            if(Obj->CmdData[count] == '\n')
            {
              *ParseLenght = (uint16_t) ParseNumber((char *) Obj->CmdData, &parse_count);
              break;
            }
          }
        }      
      }
      break;
      
    case RET_URC_IN_FULL:
      /* nothing to be done */
      break;

    case RET_URC_INCOM:
      /* TBD:                     to be implemented for SERVER MODE*/
      break;
      
    case RET_URC_PDPDEACT:
      Obj->fops.IO_Receive(Obj->CmdData, 0, RET_CRLF, Timeout);
      parse_ret = ParseNumber((char *) Obj->CmdData+1, &parse_count);
      break;
      
    case RET_URC_DNS:
      Obj->fops.IO_Receive(Obj->CmdData, 0, RET_CRLF, Timeout);
      parse_ret = ParseNumber((char *) Obj->CmdData+1, &parse_count);
      if (parse_ret == 0) /* means no errors */
      {  
        ip_count = ParseNumber((char *) Obj->CmdData+3, &parse_count);
        for (count = 0; count < ip_count; count++)        
        {  
          expected_resp = RET_URC_DNS;
          check_resp =  Obj->fops.IO_Receive(Obj->CmdData, 0, expected_resp, Timeout);
          Obj->fops.IO_Receive(Obj->CmdData, 0, RET_CRLF, Timeout);
        }
      }
      break;      
  }  
  
  return check_resp;
}


/* Public functions ---------------------------------------------------------*/


/**
  * @brief  Initialize UG96 module.
  * @param  Obj: pointer to module handle
  * @retval Operation Status.
  */
UG96_Status_t  UG96_Init(Ug96Object_t *Obj)
{
  UG96_Status_t fret = UG96_STATUS_ERROR; 
  int32_t ret = RET_ERROR; 
  int16_t cmp_ret = 0;
  int8_t i, received_string[16];
  char *align_ptr;
  
  Obj->Timeout = UG96_TIMEOUT_DEFAULT;
  Obj->ForceTimeout = 0;
  Obj->APsActive = 0;
  for (i = 0; i < UG96_MAX_SOCKETS; i++)
  {  
    Obj->SocketInfo[i].Type = UG96_TCP_CONNECTION;
    Obj->SocketInfo[i].AccessMode = UG96_BUFFER_MODE;
    Obj->SocketInfo[i].ComulatedQirdData = 0;
    Obj->SocketInfo[i].HaveReadLenght = 0;
    Obj->SocketInfo[i].UnreadLenght = 0;
  }
  
  Obj->fops.IO_FlushBuffer();
  
  if (Obj->fops.IO_Init() == 0)
  {
    ret = AT_ExecuteCommand(Obj, (uint8_t *)"AT\r\n", RET_OK | RET_ERROR);

    /* Retrieve Quectel UART baud rate and flow control*/
    /* If not alligned to the UART of MCU (_io.h), already previous AT command will fail */
    if(ret == RET_OK)
    { 
      ret = UG96_GetUARTConfig(Obj, &Obj->UART_Config);
    }
    
    if(ret == RET_OK)
    { 
      /* Use ATV1 to set the response format */
      ret = AT_ExecuteCommand(Obj,(uint8_t *)"ATV1\r\n", RET_OK | RET_ERROR);
    }  
    
    /* Use ATE1 to enable or  ATE0 to disable echo mode */
    if(ret == RET_OK)
    { 
      ret = AT_ExecuteCommand(Obj,(uint8_t *)"ATE0\r\n", RET_OK | RET_ERROR);    
    }
    
    /* Use AT+CMEE=2 to enable result code and use verbose values */ 
    if(ret == RET_OK)
    { 
      ret = AT_ExecuteCommand(Obj,(uint8_t *)"AT+CMEE=2\r\n", RET_OK | RET_ERROR);
    }      

    /* Use AT+QCFG to enable/disable URC on port UART1 */
    if(ret == RET_OK)
    { 
      ret = AT_ExecuteCommand(Obj,(uint8_t *)"AT+QCFG=\"URC/PORT\",1,\"uart1\"\r\n", RET_OK | RET_ERROR);        
    }  
    
    /* retrive module info */       
    if(ret == RET_OK)
    { 
        AT_ExecuteCommand(Obj,(uint8_t *)"AT+GMI\r\n", RET_OK | RET_ERROR);
        align_ptr = strstr((char *) Obj->CmdData,"\r\n") + 2;
        strncpy((char *)Obj->Manufacturer,  align_ptr, C2C_MFC_SIZE); 
          
        AT_ExecuteCommand(Obj,(uint8_t *)"AT+GMM\r\n", RET_OK | RET_ERROR);
        align_ptr = strstr((char *) Obj->CmdData,"\r\n") + 2;
        strncpy((char *)Obj->ProductID,  align_ptr, C2C_PROD_ID_SIZE); 
          
        AT_ExecuteCommand(Obj,(uint8_t *)"AT+GMR\r\n", RET_OK | RET_ERROR);
        align_ptr = strstr((char *) Obj->CmdData,"\r\n") + 2;
        strncpy((char *)Obj->FW_Rev,  align_ptr, C2C_FW_REV_SIZE); 

        /* Use AT+GSN to query the IMEI (International Mobile Equipment Identity) of module */
        ret = AT_ExecuteCommand(Obj,(uint8_t *)"AT+GSN\r\n", RET_OK | RET_ERROR);
        align_ptr = strstr((char *) Obj->CmdData,"\r\n") + 2;
        strncpy((char *)Obj->Imei,  align_ptr, C2C_FW_REV_SIZE); 
    }  
      
      /* retrive SIM info */ 
    if(ret == RET_OK)
    { 
        Obj->ForceTimeout = UG96_TIMEOUT_LONG;
        ret = AT_ExecuteCommand(Obj,(uint8_t *)"AT+CPIN?\r\n", RET_OK | RET_ERROR | RET_SIM_ERROR_1 | RET_SIM_ERROR_2);
        align_ptr = strstr((char *) Obj->CmdData,"\r\n+C") + 2;
        strncpy((char *)received_string,  align_ptr, 15); 
        cmp_ret = strncmp((char *)received_string,  "+CPIN: READY", 12);      
        if(cmp_ret != 0)
        { 
          cmp_ret = strncmp((char *)received_string,  "+CME ERROR", 13);
          if(cmp_ret != 0)
          {
            Obj->SimInfo.SimStatus = UG96_SIM_ERROR;
            return UG96_STATUS_ERROR;
          }
          else
          {
            Obj->SimInfo.SimStatus = UG96_SIM_PIN_NEEDED;
          }
        }
        else
        {  
          Obj->SimInfo.SimStatus = UG96_SIM_READY;
          fret = UG96_STATUS_READY;
            
          AT_ExecuteCommand(Obj,(uint8_t *)"AT+CIMI\r\n", RET_OK | RET_ERROR);
          align_ptr = strstr((char *) Obj->CmdData,"\r\n") + 2;
          strncpy((char *)Obj->SimInfo.IMSI,  align_ptr, C2C_IMSI_SIZE); 
        
          AT_ExecuteCommand(Obj,(uint8_t *)"AT+QCCID\r\n", RET_OK | RET_ERROR);
          align_ptr = strstr((char *) Obj->CmdData,"+QCCID:") + sizeof("+QCCID:");
          strncpy((char *)Obj->SimInfo.ICCID,  align_ptr, C2C_ICCID_SIZE - 1); 
        }

        /* Signal Quality level: useful info during debuggin */
        AT_ExecuteCommand(Obj,(uint8_t *)"AT+CSQ\r\n", RET_OK | RET_ERROR);
           
        AT_ExecuteCommand(Obj,(uint8_t *)"AT+CREG?\r\n", RET_OK | RET_ERROR);
        align_ptr = strstr((char *) Obj->CmdData,"+CREG:") + sizeof("+CREG:");
        strncpy((char *)Obj->SimInfo.RegistStatusString,  align_ptr, 3); 
    }

  }
  return fret;
}

/**
  * @brief  Get Signal Quality value
  * @param  Obj: pointer to module handle
  * @retval value.
  */
UG96_Status_t  UG96_GetSignalQualityStatus(Ug96Object_t *Obj, int32_t *Qvalue)
{
  UG96_Status_t ret = UG96_STATUS_ERROR; 
  uint8_t parse_count;  
  char *align_ptr;
  
  ret = (UG96_Status_t) AT_ExecuteCommand(Obj,(uint8_t *)"AT+CSQ\r\n", RET_OK | RET_ERROR);  
  align_ptr = strstr((char *) Obj->CmdData,"+CSQ:") + sizeof("+CSQ:");
  *Qvalue = ParseNumber(align_ptr, &parse_count);

  /* TO BE DONE: converted to dB */
  
  return ret;  
}


/**
  * @brief  Get Registration Status
  * @param  Obj: pointer to module handle
  * @retval Operation Status.
  */
UG96_NetworkRegistration_Status_t  UG96_GetNetworkRegistrationStatus(Ug96Object_t *Obj)
{
  int8_t received_string[3];  
  int16_t val;  
  UG96_NetworkRegistration_Status_t ret;
  char *align_ptr;

  if (Obj->SimInfo.SimStatus != UG96_SIM_READY)
  {
    ret = UG96_NRS_ERROR;
  }  
  else
  {  
    AT_ExecuteCommand(Obj,(uint8_t *)"AT+CREG?\r\n", RET_OK | RET_ERROR);  
    align_ptr = strstr((char *) Obj->CmdData,"+CREG:") + sizeof("+CREG:");
    strncpy((char *)received_string,  align_ptr+2, 1); 
    strncpy((char *)received_string+1,  "\r\n", 2);
    val = atoi((char *)received_string);
    ret = (UG96_NetworkRegistration_Status_t) val;  
  }  
  return ret;
}

/**
  * @brief  Get the list of Network Operator available in the area
  * @param  Obj: pointer to module handle
  * @param  Operator: pointer to a string
  * @retval Operation Status.
  */
UG96_Status_t  UG96_ListOperators(Ug96Object_t *Obj, char *Operators)
{
  UG96_Status_t ret = UG96_STATUS_ERROR; 
  char *align_ptr;

  Obj->ForceTimeout = UG96_TIMEOUT_MAX;
  ret = (UG96_Status_t) AT_ExecuteCommand(Obj,(uint8_t *)"AT+COPS=?\r\n", RET_OK | RET_ERROR);  
  align_ptr = strstr((char *) Obj->CmdData,"+COPS:") + sizeof("+COPS:");
  strncpy((char *)Operators,  align_ptr, 100); 
  return ret;  
}

/**
  * @brief  Get current Network Operator (by string descriptor).
  * @param  Obj: pointer to module handle
  * @param  Operator: pointer to a string
  * @retval Operation Status.
  */
UG96_Status_t  UG96_GetCurrentOperator(Ug96Object_t *Obj, char *Operator)
{
  UG96_Status_t ret = UG96_STATUS_ERROR; 
  int16_t cmp_ret = 0;
  char *align_ptr;
  
  Obj->ForceTimeout = UG96_TIMEOUT_LONG;
  ret = (UG96_Status_t) AT_ExecuteCommand(Obj,(uint8_t *)"AT+COPS?\r\n", RET_OK | RET_ERROR);  
  align_ptr = strstr((char *) Obj->CmdData,"+COPS:") + sizeof("+COPS:");
  strncpy((char *)Operator,  align_ptr, 20); 
  cmp_ret = strncmp((char *)(Operator+1),  ",", 1);   
  if (cmp_ret != 0)
  {  
    strncpy((char *)(Operator+1),  " Info not available", 19);   
  }
  return ret;  
}

/**
  * @brief  Force registration to specific Network Operator (by operator code).
  * @param  Obj: pointer to module handle
  * @param  OperatorCode: http://www.imei.info/operator-codes/
  * @retval Operation Status.
  */
UG96_Status_t  UG96_ForceOperator(Ug96Object_t *Obj, int32_t OperatorCode)
{
  UG96_Status_t ret = UG96_STATUS_ERROR; 

  Obj->ForceTimeout = UG96_TIMEOUT_LONG;
  snprintf(CmdString, 24,"AT+COPS=1,2,\"%d\"\r\n", OperatorCode);
  ret = (UG96_Status_t) AT_ExecuteCommand(Obj, (uint8_t *)CmdString, RET_OK | RET_ERROR); 
  return ret;
}

/**
  * @brief  Return Manufacturer.
  * @param  Obj: pointer to module handle
  * @param  Manufacturer: pointer to Manufacturer
  * @retval Operation Status.
  */
UG96_Status_t  UG96_GetManufacturer( Ug96Object_t *Obj, uint8_t *Manufacturer)
{
  strncpy((char *)Manufacturer, (char *)Obj->Manufacturer, C2C_MFC_SIZE);
  return UG96_STATUS_OK;
}

/**
  * @brief  Return Model.
  * @param  Obj: pointer to module handle
  * @param  Model: pointer to Model
  * @retval Operation Status.
  */
UG96_Status_t  UG96_GetProductID(Ug96Object_t *Obj, uint8_t *ProductID)
{
  strncpy((char *)ProductID, (char *)Obj->ProductID, C2C_PROD_ID_SIZE);
  return UG96_STATUS_OK;
}

/**
  * @brief  Return FW revision.
  * @param  Obj: pointer to module handle
  * @param  Model: pointer to FW revision
  * @retval Operation Status.
  */
UG96_Status_t  UG96_GetFWRevID(Ug96Object_t *Obj, uint8_t *Fw_ver)
{
  strncpy((char *)Fw_ver, (char *)Obj->FW_Rev, C2C_FW_REV_SIZE);
  return UG96_STATUS_OK;
}


/**
  * @brief  Initialize UG96 module.
  * @param  Obj: pointer to module handle
  * @retval Operation Status.
  */
UG96_Status_t  UG96_RegisterBusIO(Ug96Object_t *Obj, IO_Init_Func IO_Init,
                                                              IO_DeInit_Func    IO_DeInit,
                                                              IO_Send_Func      IO_Send,
                                                              IO_SendOne_Func   IO_SendOne,
                                                              IO_Receive_Func   IO_Receive,
                                                              IO_Flush_Func     IO_Flush)
{
  if(!Obj || !IO_Init || !IO_DeInit || !IO_Send || !IO_Receive || !IO_Flush)
  {
    return UG96_STATUS_ERROR;
  }

  Obj->fops.IO_Init = IO_Init;
  Obj->fops.IO_Send = IO_Send;
  Obj->fops.IO_SendOne = IO_SendOne;  
  Obj->fops.IO_Receive = IO_Receive;
  Obj->fops.IO_FlushBuffer = IO_Flush;  
  
  return UG96_STATUS_OK;
}

/**
  * @brief  Change default Timeout.
  * @param  Obj: pointer to module handle
  * @param  Timeout: Timeout in mS
  * @retval Operation Status.
  */
UG96_Status_t  UG96_SetTimeout(Ug96Object_t *Obj, uint32_t Timeout)
{
  Obj->Timeout = Timeout;
  return UG96_STATUS_OK;
}



///**
//  * @brief  Enable Automatic connection to saved AP on power up
//  * @param  Obj: pointer to module handle
//  * @param  Autoconn: Auto connection Enable/Disable
//  * @retval Operation Status.
//  */
//ESP8266_Status_t ESP8266_SetAutoConnect(ESP8266Object_t* Obj, ESP8266_AutoConnect_t Autoconn) 
//{
//  sprintf((char*)Obj->CmdData,"AT+CWAUTOCONN=%c\r\n", (char)Autoconn + '0');
//  return AT_ExecuteCommand(Obj,Obj->CmdData, RET_OK | RET_ERROR);
//}


/**
  * @brief  Configure a PDP Access point.
  * @param  Obj: pointer to module handle
  * @param  ContextID : range is 1-20
  * @param  Apn : access point name
  * @param  Username : Got IP Address
  * @param  Password : Network IP mask
  * @param  AuthenticationMode : 0: none, 1: PAP
  * @retval Operation Status.
  */
UG96_Status_t UG96_ConfigureAP(Ug96Object_t *Obj,
                               UG96_APConfig_t *ApConfig)
{
  UG96_Status_t ret = UG96_STATUS_ERROR; 
  
  snprintf(CmdString,UG96_CMD_SIZE,"AT+QICSGP=%d,1,\"%s\",\"%s\",\"%s\",%d\r\n", ApConfig->ContextID, ApConfig->ApnString, ApConfig->Username, ApConfig->Password, ApConfig->Authentication);
  ret = (UG96_Status_t) AT_ExecuteCommand(Obj, (uint8_t *)CmdString, RET_OK | RET_ERROR);
  if(ret == UG96_STATUS_OK)
  {
     Obj->APContextState[ApConfig->ContextID-1] = UG96_AP_CONFIGURED;
  }
  return ret;  
}


/**
  * @brief  Join a PDP Access point.
  * @param  Obj: pointer to module handle
  * @param  ContextID : range is 1-20 (max three can be connected simultaneusly)
  * @retval Operation Status.
  */
UG96_Status_t UG96_Activate(Ug96Object_t *Obj, uint8_t ContextID)
{
  UG96_Status_t ret = UG96_STATUS_ERROR; 
  
  if (Obj->APContextState[ContextID-1] == UG96_AP_CONFIGURED)
  {  
    if (Obj->APsActive <3 )
    {  
      //Obj->ForceTimeout = UG96_TIMEOUT_LONG/2;
      Obj->ForceTimeout = UG96_TIMEOUT_LONG;
      snprintf(CmdString, 24,"AT+QIACT=%d\r\n", ContextID);
      ret = (UG96_Status_t) AT_ExecuteCommand(Obj, (uint8_t *)CmdString, RET_OK | RET_ERROR);
      if(ret == UG96_STATUS_OK)
      {
         Obj->APContextState[ContextID-1] = UG96_AP_ACVTIVATED;
         Obj->APsActive++; 
      }
    }
  }
  return ret;  
}


/**
  * @brief  Leave a PDP Access point.
  * @param  Obj: pointer to module handle
  * @param  ContextID : range is 1-20 (max three are connected simultaneusly)
  * @retval Operation Status.
  */
UG96_Status_t UG96_Deactivate(Ug96Object_t *Obj, uint8_t ContextID)
{
  UG96_Status_t ret = UG96_STATUS_ERROR; 
  
  Obj->ForceTimeout = UG96_TIMEOUT_LONG;
  snprintf(CmdString, 24,"AT+QIDEACT=%d\r\n", ContextID);
  ret = (UG96_Status_t) AT_ExecuteCommand(Obj, (uint8_t *)CmdString, RET_OK | RET_ERROR);
  if(ret == UG96_STATUS_OK)
  {
     Obj->APContextState[ContextID-1] = UG96_AP_CONFIGURED;
     Obj->APsActive--;
  }
  return ret;  
}


/**
  * @brief  Get the list of the current activated context and its IP addesses
  * @param  Obj: pointer to module handle
  * @param  IPaddr_string: pointer where to retrive the string with all active IP info 
  * @param  IPaddr_int: pointer where to retrive the first active IP adr in int_array[] format  
  * @retval Operation Status.
  */
UG96_Status_t  UG96_GetActiveIpAddresses(Ug96Object_t *Obj, char *IPaddr_string, uint8_t* IPaddr_int)
{
  UG96_Status_t ret = UG96_STATUS_ERROR; 
  int32_t cmdret;
  char *align_ptr;
  uint8_t exit =0;
  
  Obj->ForceTimeout = UG96_TIMEOUT_LONG;
  cmdret = AT_ExecuteCommand(Obj,(uint8_t *)"AT+QIACT?\r\n", RET_OK | RET_ERROR);  
  if (cmdret == RET_OK)
  { 
    ret = UG96_STATUS_OK;
    align_ptr = strstr((char *) Obj->CmdData,"+QIACT:") + sizeof("+QIACT:");
    strncpy((char *)IPaddr_string,  align_ptr, 40); 
    align_ptr = IPaddr_string+2;
    while(!exit) {   /* find where number start because it is not always same position */
      align_ptr++;
      if(*align_ptr == '"')
      {
        exit =1;
      }
    }
    ParseIP(align_ptr+1, IPaddr_int); 
  }  
  return ret;  
}


/**
  * @brief  Check whether the contextID is connected to an access point.
  * @retval Operation Status.
  */
UG96_APState_t  UG96_IsActivated(Ug96Object_t *Obj, uint8_t ContextID)
{
  return (UG96_APState_t) Obj->APContextState[ContextID-1]; 
}


#if (UG96_USE_PING == 1)
/**
  * @brief  Test the Internet Protocol reachability of a host
  * @param  Obj: pointer to module handle
  * @param  ContextID : range is 1-20 (max three are connected simultaneusly)
  * @param  host_addr_string: domain name (e.g. www.amazon.com) or dotted decimal IP addr
  * @param  count: PING repetitions (default 4) (max 10)
  * @param  rep_delay_sec: timeout for each repetition in seconds 
  * @retval Operation Status.
  */
UG96_Status_t  UG96_Ping(Ug96Object_t *Obj, uint8_t ContextID, char *host_addr_string, uint16_t count, uint16_t rep_delay_sec)
{
  UG96_Status_t ret = UG96_STATUS_ERROR; 
  
  if (count > 10)
  {
    count = 10;
  }  
  Obj->ForceTimeout = UG96_TIMEOUT_LONG;
  snprintf(CmdString, UG96_CMD_SIZE,"AT+QPING=%d,\"%s\",%d,%d\r\n", ContextID, host_addr_string, rep_delay_sec, count);
  ret = (UG96_Status_t) AT_ExecuteCommand(Obj, (uint8_t *)CmdString, RET_OK | RET_ERROR);  

  return ret;   
} 
#endif


/**
  * @brief  Reset To factory defaults.
  * @param  Obj: pointer to module handle
  * @retval Operation Status.
  */
//ESP8266_Status_t ESP8266_ResetToFactoryDefault(ESP8266Object_t *Obj)
//{
//  ESP8266_Status_t ret ;
// 
//  sprintf((char*)Obj->CmdData,"AT+RESTORE\r\n");
//  ret = AT_ExecuteCommand(Obj, Obj->CmdData, RET_OK | RET_ERROR);       
//  return ret;
//}

/**
  * @brief  Reset the module.
  * @param  Obj: pointer to module handle
  * @retval Operation Status.
  */
//ESP8266_Status_t ESP8266_ResetModule(ESP8266Object_t *Obj)
//{
//  ESP8266_Status_t ret ;
//  
//  sprintf((char*)Obj->CmdData,"AT+RST\r\n");
//  ret = AT_ExecuteCommand(Obj, Obj->CmdData, RET_OK | RET_ERROR);       
//  return ret;
//}


/**
  * @brief  Set UART Configuration on the QUectel modem (MCU should be adapted accordingly).
  * @param  Obj: pointer to module handle
  * @param  pconf: pointer to UART config structure
  * @param  bd_or_fc: change 1 param at the time (in between UART on MCU should be adapted accordingly)
  * @retval Operation Status.
  */
UG96_Status_t  UG96_SetUARTConfig(Ug96Object_t *Obj, UG96_UARTConfig_t *pconf, int8_t bd_or_fc)
{
  int32_t cmdret;

  /* After one param is changed UART communication with modem will fail until 
     same parameter is updated accordingly on MCU */
  if (bd_or_fc == 0) /* change baud rate */
  {
    snprintf(CmdString, 17,"AT+IPR=%d\r\n", pconf->BaudRate);
    cmdret = AT_ExecuteCommand(Obj, (uint8_t *)CmdString, RET_OK | RET_ERROR); 
  }
  else               /* change flow control */
  {  
    switch (pconf->FlowControl) {
      case UG96_UART_FLW_CTL_NONE:
        snprintf(CmdString, 16,"AT+IFC=0,0\r\n");
        break;
      case UG96_UART_FLW_CTL_RTS:
        snprintf(CmdString, 16,"AT+IFC=2,0\r\n");
        break;
      case UG96_UART_FLW_CTL_CTS:
        snprintf(CmdString, 16,"AT+IFC=0,2\r\n");
        break;
      case UG96_UART_FLW_CTL_RTS_CTS:
        snprintf(CmdString, 16,"AT+IFC=2,2\r\n");
        break;               
    }           
    cmdret = AT_ExecuteCommand(Obj, (uint8_t *)CmdString, RET_OK | RET_ERROR);              
  }

  return (UG96_Status_t) cmdret; 
}

/**
  * @brief  Get UART Configuration.
  * @param  Obj: pointer to module handle
  * @param  pconf: pointer to UART config structure
  * @retval Operation Status.
  */
UG96_Status_t  UG96_GetUARTConfig(Ug96Object_t *Obj, UG96_UARTConfig_t *pconf)
{ 
  char *align_ptr;
  uint8_t rts, cts;
  
  AT_ExecuteCommand(Obj,(uint8_t *)"AT+IPR?\r\n", RET_OK | RET_ERROR);
  align_ptr = strstr((char *) Obj->CmdData,"+IPR:") + sizeof("+IPR:");
  Obj->UART_Config.BaudRate = ParseNumber(align_ptr, NULL);

  AT_ExecuteCommand(Obj,(uint8_t *)"AT+IFC?\r\n", RET_OK | RET_ERROR);
  align_ptr = strstr((char *) Obj->CmdData,"+IFC:") + sizeof("+IPR:");
  rts = ParseNumber(align_ptr, NULL);   
  cts = ParseNumber(align_ptr+2, NULL);   

  if (rts == 2)
  {
    if (cts == 2)
    {
      pconf->FlowControl = UG96_UART_FLW_CTL_RTS_CTS;  
    }
    else
    {
      pconf->FlowControl = UG96_UART_FLW_CTL_RTS;  
    }
  }
  else
  {
    if (cts == 2)
    {
      pconf->FlowControl = UG96_UART_FLW_CTL_CTS;  
    }
    else
    {
      pconf->FlowControl = UG96_UART_FLW_CTL_NONE;  
    }
  }    

  return UG96_STATUS_OK;
}


/**
  * @brief  Get the last IP addesses associated to the host name via Google DNS service
  * @param  Obj: pointer to module handle
  * @param  ContextID : range is 1-20 (max three are connected simultaneusly)
  * @param  IPaddr_string: host name (e.g. www.amazon.com) 
  * @param  IPaddr_int: pointer where to retrive the first active IP adr in int_array[] format  
  * @retval Operation Status.
  */
UG96_Status_t  UG96_ResolveHostIPviaDNS(Ug96Object_t *Obj, uint8_t ContextID, char *IPaddr_string, uint8_t* IPaddr_int)
{
  UG96_Status_t ret = UG96_STATUS_ERROR; 
  int16_t  urc_retval = 0;
  uint16_t parsedlen = 0;
  int32_t cmdret;
  char *align_ptr;
  
  Obj->ForceTimeout = UG96_TIMEOUT_LONG;
  
  /* force to use google DNS service : "8.8.8.8" */    
  snprintf(CmdString, 30,"AT+QIDNSCFG=%d,\"%s\"\r\n", ContextID, "8.8.8.8");
  cmdret = (UG96_Status_t) AT_ExecuteCommand(Obj, (uint8_t *)CmdString, RET_OK | RET_ERROR);
        
  /* inquire the DNS service */    
  if (cmdret == RET_OK)
  {
    snprintf(CmdString, 255,"AT+QIDNSGIP=%d,\"%s\"\r\n", ContextID, IPaddr_string);
    cmdret = (UG96_Status_t) AT_ExecuteCommand(Obj, (uint8_t *)CmdString, RET_OK | RET_ERROR);
  }

  if (cmdret == RET_OK)
  { 
    urc_retval = AT_RetrieveUrc(Obj, &parsedlen, UG96_BUFFER_MODE, UG96_TIMEOUT_LONG);  
    if (urc_retval == RET_URC_DNS)
    {
      align_ptr = (char *) Obj->CmdData+2;
      ParseIP(align_ptr, IPaddr_int); 
      ret = UG96_STATUS_OK;    
    }
  }  

#ifdef UG96_DBG 
  char error_string[40];
  if (ret == UG96_STATUS_OK)
  {
    printf("                Host addr from DNS: %d.%d.%d.%d \n", IPaddr_int[0],IPaddr_int[1],IPaddr_int[2],IPaddr_int[3]);  
  }
  else
  {
    UG96_RetrieveLastErrorDetails(Obj, error_string);
    printf("                UG96_ResolveHostIPviaDNS error: %s\n", error_string);  
  }
#endif
  return ret;  
}

/**
  * @brief  Configure and Start a Client connection.
  * @param  Obj: pointer to module handle
  * @param  ContextID : range is 1-20 (max three are connected simultaneusly)
  * @param  conn: pointer to the connection structure
  * @retval Operation Status.
  */
UG96_Status_t  UG96_OpenClientConnection(Ug96Object_t *Obj, uint8_t ContextID, UG96_Conn_t *conn)
{
  UG96_Status_t fret = UG96_STATUS_ERROR; 
  char  type_string[12];
  int16_t recv;
  int32_t ret, retr_errcode;
  
  Obj->fops.IO_FlushBuffer();
  Obj->SocketInfo[conn->ConnectID].Type = conn->Type;
  Obj->SocketInfo[conn->ConnectID].AccessMode = conn->AccessMode;
  Obj->SocketInfo[conn->ConnectID].ComulatedQirdData = 0;
  Obj->SocketInfo[conn->ConnectID].HaveReadLenght = 0;
  Obj->SocketInfo[conn->ConnectID].UnreadLenght = 0;
    
  if (conn->Type == UG96_TCP_CONNECTION)
  {
    strcpy(type_string, "TCP");
  }
  else if (conn->Type == UG96_UDP_CONNECTION)
  {
    strcpy(type_string, "UDP");
  }
  else if (conn->Type == UG96_TCP_LISTENER_CONNECTION)
  {
    strcpy(type_string, "TCP_LISTENER");
  }
  else if (conn->Type == UG96_UDP_SERVER_CONNECTION)
  {
    strcpy(type_string, "UDP_SERVER");
  }

#ifdef UG96_DBG 
        printf("                UG96_OpenClientConnection() \n");
#endif      
  
  snprintf(CmdString, UG96_CMD_SIZE, "AT+QIOPEN=%d,%d,\"%s\",\"%s\",%d,%d,%d\r\n", ContextID, conn->ConnectID, type_string, conn->Url, conn->RemotePort, conn->LocalPort, conn->AccessMode);
  Obj->ForceTimeout = 80000; /* The maximum timeout of TCP connect is 75 seconds */
  ret = AT_ExecuteCommand(Obj, (uint8_t *)CmdString, RET_OPEN | RET_ERROR); 
  if (ret == RET_OPEN)
  {  
    recv = Obj->fops.IO_Receive(Obj->CmdData, 6, RET_NONE, UG96_TIMEOUT_SHORT);
    if (recv > 0)
    {  
      retr_errcode = ParseNumber((char *) Obj->CmdData+3, NULL);
      if (  retr_errcode != 0) /* read the remaining 2 characters if error nr 3 chiphers*/
      {
         recv = Obj->fops.IO_Receive(Obj->CmdData, 2, RET_NONE, UG96_TIMEOUT_SHORT);
#ifdef UG96_DBG 
         printf("                UG96_OpenClientConnection() retr_errcode: %d \n", retr_errcode);
#endif      
      }
      else
      {
#ifdef UG96_DBG 
        snprintf(CmdString, UG96_CMD_SIZE, "AT+QISTATE=1,%d\r\n", conn->ConnectID);
        ret = AT_ExecuteCommand(Obj, (uint8_t *)CmdString, RET_OK | RET_ERROR); 
#endif      
        fret = UG96_STATUS_OPEN;
      }
    }
    else
    {
#ifdef UG96_DBG 
        printf("                UG96_OpenClientConnection() missing part of the response  \n");
#endif      
    }
  } 
   
  return fret;   
}

/**
  * @brief  Stop Client connection.
  * @param  Obj: pointer to module handle
  * @param  conn: pointer to the connection structure
  * @retval Operation Status.
  */
UG96_Status_t  UG96_CloseClientConnection(Ug96Object_t *Obj, UG96_Conn_t *conn)
{
  UG96_Status_t ret = UG96_STATUS_ERROR; 
  
  Obj->fops.IO_FlushBuffer();
  snprintf(CmdString, 24, "AT+QICLOSE=%d,%d\r\n", conn->ConnectID, 15);
  ret = (UG96_Status_t) AT_ExecuteCommand(Obj, (uint8_t *)CmdString, RET_OK | RET_ERROR);  

  return ret;   
}


/**
  * @brief  Send an amount data over C2C.
  * @param  Obj: pointer to module handle
  * @param  Socket: number of the socket
  * @param  pdata: pointer to data
  * @param  Reqlen : nr of bytes to be sent
  * @param  SentLen : ptr to return the nr of bytes actually sent
  * @param  Timeout ms: timeout used by each internal Cmd, depending on the path, delay can be longer
  * @retval Operation Status.
  */
UG96_Status_t UG96_SendData(Ug96Object_t *Obj, uint8_t Socket, uint8_t *pdata, uint16_t Reqlen , uint16_t *SentLen , uint32_t Timeout)
{
  UG96_Status_t fret = UG96_STATUS_ERROR;  
  int32_t ret = 0;
  
  *SentLen = 0;
  if(Reqlen <= UG96_TX_DATABUF_SIZE ) 
  {
    snprintf(CmdString, 24, "AT+QISEND=%d,%d\r\n", Socket, Reqlen);
    ret = AT_ExecuteCommand(Obj, (uint8_t *) CmdString, RET_ERROR | RET_ARROW); /* uses Obj->Timeout */
    if(ret == RET_ARROW)
    {
      fret = AT_RequestSendData(Obj, pdata, Reqlen, Timeout); 
      if(fret == RET_SENT)
      {
        *SentLen =  Reqlen;
      }
    }
  }
  return fret;  
}


/**
  * @brief  Receive an amount data over C2C.
  * @param  Obj: pointer to module handle
  * @param  Socket: number of the socket
  * @param  pdata: pointer to data
  * @param  Requestedlen : in UG96_BUFFER_MODE the req len, in UG96_DIRECT_PUSH is the max leng available in pdata[] buffer
  * @param  Receivedlen : pointer to return the length of the data received
  * @param  Timeout ms: timeout used by each internal Cmd, depending on the path, delay can be up to 5 times longer
  * @retval Operation Status.
  */

UG96_Status_t UG96_ReceiveData(Ug96Object_t *Obj, uint8_t Socket, uint8_t *pdata, uint16_t Requestedlen, uint16_t *Receivedlen, uint32_t Timeout)
{

  UG96_Status_t ret = UG96_STATUS_ERROR;  
  int16_t  urc_retval = 0, rcvlen = 0;
  uint8_t *ptr = pdata;
  uint16_t parsedlen = 0;
  uint8_t parse_count;  
  int16_t  qird_retval = 0;

 
        
  *Receivedlen = 0;

  if (Obj->SocketInfo[Socket].Type != UG96_TCP_CONNECTION)
  {
      return ret; /* currently only TCP connection is implemented */
  }


  
  if(Requestedlen <= UG96_RX_DATABUF_SIZE )
  {  
    switch (Obj->SocketInfo[Socket].AccessMode) {
      
      case UG96_BUFFER_MODE:
#ifdef UG96_DBG 
        printf("                UG96_ReceiveData() Requestedlen: %d \n", Requestedlen);
        printf("                UG96_ReceiveData() UnreadLenght: %d \n", Obj->SocketInfo[Socket].UnreadLenght);
        printf("                UG96_ReceiveData() ComulatedQirdData: %d \n", Obj->SocketInfo[Socket].ComulatedQirdData);
#endif
     
        if(Obj->SocketInfo[Socket].UartRemaining > 0)  /* if Timeout respects UART speed this should never happen*/
        {
#ifdef UG96_DBG 
            printf("       **** UG96_ReceiveData() UartRemaining %d \n", Obj->SocketInfo[Socket].UartRemaining);
#endif
            rcvlen = Obj->fops.IO_Receive(ptr, Obj->SocketInfo[Socket].UartRemaining, RET_NONE, Timeout);
            Obj->SocketInfo[Socket].UartRemaining -= rcvlen;
            *Receivedlen = rcvlen;
            if (Obj->SocketInfo[Socket].UartRemaining == 0)
            {
                snprintf(CmdString,24,"AT+QIRD=%d,0\r\n", Socket);
                AT_ExecuteCommand(Obj, (uint8_t *) CmdString, RET_ERROR | RET_READ); /* uses Obj->Timeout */  
                Obj->fops.IO_Receive(Obj->CmdData, 0, RET_CRLF, Obj->Timeout);
                ParseQIRD((char *) Obj->CmdData, &Obj->SocketInfo[Socket].ComulatedQirdData);          
            }
            ret =  UG96_STATUS_OK;
            break;
        }
          
        if(Obj->SocketInfo[Socket].UnreadLenght > 0)          /* some data remaining from previous operation */
        {
          if(Requestedlen > Obj->SocketInfo[Socket].UnreadLenght)   
          {
            Requestedlen = Obj->SocketInfo[Socket].UnreadLenght;    /* Just take the one to end remaning data */
          } 
        }
        else
        {
          /* Wait for parsing URC */
          urc_retval = AT_RetrieveUrc(Obj, &parsedlen, UG96_BUFFER_MODE, Timeout);
#ifdef UG96_DBG 
          if (urc_retval < 0)  
          {
            printf("                             UG96_ReceiveData() URC not received\n");
          }
          else
          {
            printf("                UG96_ReceiveData() URC OK\n");
          }
#endif
        }
            
        snprintf(CmdString,24,"AT+QIRD=%d,%d\r\n", Socket, Requestedlen);
        qird_retval = AT_ExecuteCommand(Obj, (uint8_t *) CmdString, RET_ERROR | RET_READ); /* uses Obj->Timeout */
        if ((qird_retval < 0) || (qird_retval == RET_ERROR)) 
        {
#ifdef UG96_DBG 
          printf("                             UG96_ReceiveData() QIRD issue\n");
#endif             
          return UG96_STATUS_ERROR;
        }            
        /* lenght parsing */
        Obj->fops.IO_Receive(Obj->CmdData, 0, RET_CRLF, Obj->Timeout);
        parsedlen = (uint16_t) ParseNumber((char *) Obj->CmdData, &parse_count);
#ifdef UG96_DBG 
        if ((Obj->SocketInfo[Socket].UnreadLenght > 0) && (parsedlen != Requestedlen))  
        {
          printf("                             UG96_ReceiveData() unexpected behaviour parsedlen != Requestedlen (%d)\n", Requestedlen);
        }   
        printf("                UG96_ReceiveData() parsedlen is %d\n", parsedlen);
#endif             
        /* Retriving data */
        rcvlen = Obj->fops.IO_Receive(ptr, parsedlen, RET_NONE, Timeout);
        if (rcvlen < 0)
        {  
          *Receivedlen = 0;
           break; /* return error */
        }   
        if(rcvlen != parsedlen) /* if Timeout respects UART speed this should never happen*/
        {
#ifdef UG96_DBG 
          printf("                             UG96_ReceiveData() Received lenght missmatch!!!\n");
#endif  
          Obj->SocketInfo[Socket].UartRemaining = parsedlen - rcvlen;    
          Obj->SocketInfo[Socket].UnreadLenght -= rcvlen;
          *Receivedlen = rcvlen; 
        }
        else
        {
#ifdef UG96_DBG 
          printf("                UG96_ReceiveData() Received  OK %d \n", rcvlen);
#endif  
          *Receivedlen = rcvlen;          
          Obj->SocketInfo[Socket].UartRemaining = 0; 
          /* Update buffer state for next itaration*/
          snprintf(CmdString,24,"AT+QIRD=%d,0\r\n", Socket);
          AT_ExecuteCommand(Obj, (uint8_t *) CmdString, RET_ERROR | RET_READ); /* uses Obj->Timeout */  
          Obj->fops.IO_Receive(Obj->CmdData, 0, RET_CRLF, Obj->Timeout);
          ParseQIRD((char *) Obj->CmdData, &Obj->SocketInfo[Socket].ComulatedQirdData);            
        }
        
        ret =  UG96_STATUS_OK;
        break;
        
    case UG96_DIRECT_PUSH:   /* Carefull: not fully tested */
 
        if(Obj->RemRevdData > 0)   /* some data remaining from previous operation */
        {
          if(Requestedlen <= Obj->RemRevdData)       /* in UG96_DIRECT_PUSH Requestedlen is the max the applic can receive */
          {
            Obj->RemRevdData -= Requestedlen;
            if(Obj->fops.IO_Receive(ptr, Requestedlen, RET_NONE, Timeout) != Requestedlen)
            {
              return ret; 
            }
            else
            {
              return UG96_STATUS_OK;
            }
          }
          else                              /* all remaing data can be retried */
          {
            Requestedlen -= Obj->RemRevdData;
         
            if( Obj->fops.IO_Receive(ptr, Obj->RemRevdData, RET_NONE, Timeout) == Obj->RemRevdData)
            {
              ptr += Obj->RemRevdData;
              Obj->RemRevdData = 0;
            }
            else
            {
              /* error */
              return ret; 
            }
          }
        }
    
        urc_retval = AT_RetrieveUrc(Obj, &parsedlen, UG96_DIRECT_PUSH, Timeout);
    
        if( urc_retval == RET_URC_RECV)
        {            
          if(parsedlen > 0)
          {
            if(Requestedlen > parsedlen)    /* retrive all data */
            {
              Obj->RemRevdData = 0;
              Requestedlen = parsedlen;
            }
            else                      /* not enought space in the applic buffer */
            {
              Obj->RemRevdData = parsedlen - Requestedlen;
            }
            
            *Receivedlen = Obj->fops.IO_Receive(ptr, Requestedlen, RET_NONE, Timeout);
            if(*Receivedlen == Requestedlen)
            {
              ret = UG96_STATUS_OK;  
            }
          }
        }
        break;

      case UG96_TRANSPARENT_MODE:
        /* TBD */
        break;

    } /*end swithc case */
  }
  return ret;   
}




/**
  * @brief  Retrieve last IP error code
  * @param  Obj: pointer to module handle
  * @param  error_string: 
  * @param  error_code
  * @retval Operation Status.
  */ 
UG96_Status_t  UG96_RetrieveLastErrorDetails(Ug96Object_t *Obj, char *error_string)
{
  UG96_Status_t ret = UG96_STATUS_ERROR; 
  char *align_ptr;
   
  ret = (UG96_Status_t) AT_ExecuteCommand(Obj, "AT+QIGETERROR\r\n", RET_OK | RET_ERROR);  
  align_ptr = strstr((char *) Obj->CmdData,"+QIGETERROR:") + sizeof("+QIGETERROR:");
  strncpy((char *)error_string,  align_ptr, UG96_ERROR_STRING_SIZE);  

  return ret;   
} 


/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
