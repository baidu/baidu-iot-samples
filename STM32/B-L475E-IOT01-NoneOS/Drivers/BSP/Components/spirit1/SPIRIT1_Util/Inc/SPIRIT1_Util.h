/**
* @file    SPIRIT1_Util.h
* @author  High End Analog & RF BU - AMS / ART Team IMS-Systems Lab
* @version V3.0.1
* @date    November 19, 2012
* @brief   Identification functions for SPIRIT DK.
* @attention
*
* <h2><center>&copy; COPYRIGHT(c) 2015 STMicroelectronics</center></h2>
*
* Redistribution and use in source and binary forms, with or without modification,
* are permitted provided that the following conditions are met:
*   1. Redistributions of source code must retain the above copyright notice,
*      this list of conditions and the following disclaimer.
*   2. Redistributions in binary form must reproduce the above copyright notice,
*      this list of conditions and the following disclaimer in the documentation
*      and/or other materials provided with the distribution.
*   3. Neither the name of STMicroelectronics nor the names of its contributors
*      may be used to endorse or promote products derived from this software
*      without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
* FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
* SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
* CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
* OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
* OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
******************************************************************************
*/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __SPIRIT1_UTIL_H
#define __SPIRIT1_UTIL_H


/* Includes ------------------------------------------------------------------*/
#include "SPIRIT_Config.h"
#include "stm32l475_discovery_subg.h"

#ifdef __cplusplus
  "C" {
#endif


/**
 * @addtogroup ST_SPIRIT1
 * @{
 */
    
typedef struct 
{
  uint16_t nSpiritVersion;
  SpiritVersion xSpiritVersion;
}SpiritVersionMap;
   
#define CUT_MAX_NO 3
#define CUT_2_1v3 0x0103
#define CUT_2_1v4 0x0104
#define CUT_3_0   0x0130

/**
 * @brief  Range extender type
 */
typedef enum
{
  RANGE_EXT_NONE = 0x00,
  RANGE_EXT_SKYWORKS_169,
  RANGE_EXT_SKYWORKS_868
} RangeExtType;

/**
 * @addgroup SPIRIT1_Util_FUNCTIONS
 * @{
 */
void SpiritManagementIdentificationRFBoard(void);

RangeExtType SpiritManagementGetRangeExtender(void);
void SpiritManagementSetRangeExtender(RangeExtType xRangeType);
void SpiritManagementRangeExtInit(void);
void SpiritManagementSetBand(uint8_t value);
uint8_t SpiritManagementGetBand(void);

uint8_t SdkEvalGetHasEeprom(void);

void Spirit1InterfaceInit(void);
void Spirit1GpioIrqInit(SGpioInit *pGpioIRQ);
void Spirit1RadioInit(SRadioInit *pRadioInit);
void Spirit1SetPower(uint8_t cIndex, float fPowerdBm);
void Spirit1PacketConfig(void);
void Spirit1SetPayloadlength(uint8_t length);
void Spirit1SetDestinationAddress(uint8_t address);
void Spirit1EnableTxIrq(void);
void Spirit1EnableRxIrq(void);
void Spirit1DisableIrq(void);
void Spirit1SetRxTimeout(float cRxTimeOut);
void Spirit1EnableSQI(void);
void Spirit1SetRssiTH(int dbmValue);
float Spirit1GetRssiTH(void);
void Spirit1ClearIRQ(void);
void Spirit1StartRx(void);
void Spirit1GetRxPacket(uint8_t *buffer, uint8_t *size );
void Spirit1StartTx(uint8_t *buffer, uint8_t size);

/**
* @}
*/

/**
* @}
*/

/**
* @}
*/

#ifdef __cplusplus
}
#endif


#endif


 /******************* (C) COPYRIGHT 2015 STMicroelectronics *****END OF FILE****/

