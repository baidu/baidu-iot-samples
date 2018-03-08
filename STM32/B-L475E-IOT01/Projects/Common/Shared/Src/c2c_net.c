/**
  ******************************************************************************
  * @file    c2c_net.c
  * @author  MCD Application Team
  * @version V0.0.1
  * @date    11-August-2017
  * @brief   C2C-specific network initialization.
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
#include "c2c.h"
#include "iot_flash_config.h"

/* Private defines -----------------------------------------------------------*/
#define  C2C_CONNECT_MAX_ATTEMPT_COUNT  3
#define MAX(a,b) ((a) < (b) ? (b) : (a))

/* Private typedef -----------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
static char OperatorsString[C2C_OPERATORS_LIST + 1]; 

/* Private function prototypes -----------------------------------------------*/
int net_if_init(void * if_ctxt);
int net_if_deinit(void * if_ctxt);

/* Functions Definition ------------------------------------------------------*/
int net_if_init(void * if_ctxt)
{
  C2C_Ret_t ret = C2C_RET_ERROR;
  C2C_RegStatus_t reg_status = C2C_REGSTATUS_UNKNOWN;
  int32_t quality_level = 0;
  static uint8_t c2cConnectCounter = 0;
  const char *oper_ap_code;
  const char *username;
  const char *password;
  
  char moduleinfo[MAX(MAX(MAX(MAX(C2C_FW_REV_SIZE, C2C_MFC_SIZE), C2C_PROD_ID_SIZE), C2C_IMEI_SIZE), C2C_ICCID_SIZE)];

  while (checkC2cCredentials(&oper_ap_code, &username, &password) != HAL_OK)
  {
    printf("Your C2C parameters need to be entered to proceed\n");
    updateC2cCredentials(&oper_ap_code, &username, &password);  
  }
  
  /* SIM SLOT will be choosen via console in future*/
  C2C_SimSelect(SLOT_B1); // sim in slot
  //C2C_SimSelect(SLOT_B2); // embedded sim
  HAL_Delay(50);
  
  printf("\n*** C2C connection ***\n\n"); 
  
  /*  C2C Module power up and initialization */
  printf("Power Up the C2C module (be patient: it takes about 5 second)\n");
  C2C_HwResetAndPowerUp();
  HAL_Delay(2000);  /* wait additional 2 sec before SIM Init */

  printf("Initializing the C2C module\n");
  reg_status =  C2C_Init(10);
  
  switch (reg_status) {                            		
    case C2C_REGSTATUS_HOME_NETWORK: 
    case C2C_REGSTATUS_ROAMING: 
      BSP_LED_On(LED_GREEN);
      printf("C2C module registered\n");
      break;
    case C2C_REGSTATUS_TRYING: 
      printf("C2C registreation tying\n");
      break;
    case C2C_REGSTATUS_REG_DENIED: 
      printf("C2C registreation denied\n");
      break;
    case C2C_REGSTATUS_ERROR:
      BSP_LED_On(LED_ORANGE);
      printf("C2C registration error\n");
      break;
    default:
      printf("C2C not registered\n");
      break;
  } 

  /* Retrieve the quality level of the connectio  */
  C2C_GetSignalQualityStatus(&quality_level);
  printf("Signal Quality Level: %d \n",quality_level);
  
#if GRENOBLE
  printf(" ---- Force Operator Grenoble --- \n");
  ret = C2C_ForceOperator(20820);  /* Bouygues Telecom */
  /* Retrieve the quality level of the connectio  */
  C2C_GetSignalQualityStatus(&quality_level);
  printf("Signal Quality Level Bouygues Telecom: %d \n",quality_level);
#endif
  
  /* Retrieve the C2C module info  */
  C2C_GetModuleName(moduleinfo);
  printf("Module initialized successfully: %s \n",moduleinfo);
  
  C2C_GetModuleID(moduleinfo);
  printf("ProductID: %s \n",moduleinfo);
  
  C2C_GetModuleFwRevision(moduleinfo);
  printf("Version: %s\n",moduleinfo);

  C2C_GetSimId(moduleinfo);
  printf("SIM Id (IccID): %s\n",moduleinfo);


  /* Retrieve the C2C current cellularoperator to confirm that it is detected and communicating. */
  printf("Retrieving the cellular operator: ");
  ret = C2C_GetCurrentOperator(OperatorsString);
  if (ret == C2C_RET_OK)
  {
    printf(" %s\n", OperatorsString);

  }
  else
  {
    printf("Failed to get current cellular operator name\n");
  }
  /* Connect to the specified SSID. */

  printf("\n");
  ret = C2C_ConfigureAP(1, oper_ap_code, username, password,0);           /* Emnify SIM */
  //ret = C2C_ConfigureAP(1,"ESEYE1","","",0);     /* Eseye SIM */
  printf("\rConnecting to AP: be patient ...\n");

  HAL_Delay(1000);    

  do
  {
    ret = C2C_Connect();

    c2cConnectCounter++;

    if (ret == C2C_RET_OK)
    { 
      BSP_LED_Off(LED_GREEN);
      c2cConnectCounter = C2C_CONNECT_MAX_ATTEMPT_COUNT;
      printf(" AP connected\n");
    } 
    else
    { 
      BSP_LED_Off(LED_GREEN);
      printf(" Connectio try nr %d of %d failed", c2cConnectCounter, C2C_CONNECT_MAX_ATTEMPT_COUNT);
    }
  } 
  while (c2cConnectCounter < C2C_CONNECT_MAX_ATTEMPT_COUNT);
  
  if (ret == C2C_RET_OK)
  {
    printf("\nConnected to AP \n");
  }
  else
  {
    printf("\nFailed to connect to AP \n");
  }
  
  return (ret == C2C_RET_OK) ? 0 : -1;
}



int net_if_deinit(void * if_ctxt)
{
  // TODO: Anything to do here?
  return 0;
}


int network_check_credential(void)
{
  //return checkWiFiCredentials(&ssid, &psk, &security_mode);
  return 0;
}



void network_enter_credential(void)
{
  const char *oper_ap_code;
  const char *username;
  const char *password;
  
  updateC2cCredentials(&oper_ap_code, &username, &password);
}


/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
