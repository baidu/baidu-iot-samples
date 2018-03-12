/**
  ******************************************************************************
  * @file    c2c_io.h
  * @author  MCD Application Team
  * @version V0.0.1
  * @date    11-August-2017
  * @brief   This file contains the common defines and functions prototypes for
  *          the C2C IO operations.
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
#ifndef __C2C_IO__
#define __C2C_IO__

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32l4xx_hal.h"

/* Exported types ------------------------------------------------------------*/
typedef enum {
  SLOT_B1,
  SLOT_B2
} C2C_SimSlot_t;

/* Exported constants --------------------------------------------------------*/
/* This section can be used to tailor USARTy instance used and associated
   resources */
#define USARTy                           USART1
#define USARTy_CLK_ENABLE()              __HAL_RCC_USART1_CLK_ENABLE();
#define DMAx_CLK_ENABLE()                __HAL_RCC_DMA1_CLK_ENABLE()
#define USARTy_RX_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOG_CLK_ENABLE()
#define USARTy_TX_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOB_CLK_ENABLE()

#define USARTy_FORCE_RESET()             __HAL_RCC_USART1_FORCE_RESET()
#define USARTy_RELEASE_RESET()           __HAL_RCC_USART1_RELEASE_RESET()

/* Definition for USARTy Pins */
#define USARTy_TX_PIN                    GPIO_PIN_6
#define USARTy_TX_GPIO_PORT              GPIOB
#define USARTy_TX_AF                     GPIO_AF7_USART1

/* PortG on L696AG device needs Independent I/O supply rail;
   It can be enabled by setting the IOSV bit in the PWR_CR2 register, 
   when the VDDIO2 supply is present (depends by the package).*/
#define USARTy_RX_PIN                    GPIO_PIN_10
#define USARTy_RX_GPIO_PORT              GPIOG
#define USARTy_RX_AF                     GPIO_AF7_USART1

#define USARTy_RTS_PIN                    GPIO_PIN_12
#define USARTy_RTS_GPIO_PORT              GPIOG
#define USARTy_RTS_AF                     GPIO_AF7_USART1

#define USARTy_CTS_PIN                    GPIO_PIN_11
#define USARTy_CTS_GPIO_PORT              GPIOG
#define USARTy_CTS_AF                     GPIO_AF7_USART1
   
/* Definition for USARTy's NVIC IRQ and IRQ Handlers */
#define USARTy_IRQn                      USART1_IRQn
#define USARTy_IRQHandler                USART1_IRQHandler

/* Definition for USARTy's DMA */
#define USARTy_TX_DMA_CHANNEL             DMA1_Channel4
#define USARTy_RX_DMA_CHANNEL             DMA1_Channel5

/* Definition for USARTy's DMA Request */
#define USARTy_TX_DMA_REQUEST             DMA_REQUEST_2
#define USARTy_RX_DMA_REQUEST             DMA_REQUEST_2

/* Definition for USARTy's NVIC */
#define USARTy_DMA_TX_IRQn                DMA1_Channel4_IRQn
#define USARTy_DMA_RX_IRQn                DMA1_Channel5_IRQn
#define USARTy_DMA_TX_IRQHandler          DMA1_Channel4_IRQHandler
#define USARTy_DMA_RX_IRQHandler          DMA1_Channel5_IRQHandler

/* C2C module Reset pin definitions */
#define C2C_RST_PIN                     GPIO_PIN_2
#define C2C_RST_GPIO_PORT               GPIOB
#define C2C_RST_GPIO_CLK_ENABLE()       __HAL_RCC_GPIOB_CLK_ENABLE()

/* C2C module PowerKey pin definitions */
#define C2C_PWRKEY_PIN                  GPIO_PIN_3
#define C2C_PWRKEY_GPIO_PORT            GPIOD
#define C2C_PWRKEY_GPIO_CLK_ENABLE()    __HAL_RCC_GPIOD_CLK_ENABLE()
     
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
void C2C_SimSelect(C2C_SimSlot_t sim);
void C2C_HwResetAndPowerUp(void);
     
     
int8_t  UART_C2C_Init(void);
int8_t  UART_C2C_DeInit(void);
void    UART_C2C_FlushBuffer(void);
int16_t UART_C2C_SendData(uint8_t* Buffer, uint16_t Length);
int16_t UART_C2C_SendOneChar(uint8_t* pData, uint16_t Length, uint32_t Timeout);
int32_t UART_C2C_ReceiveData(uint8_t* pData, uint16_t Length, uint32_t RetVals, uint32_t Timeout);

#ifdef __cplusplus
}
#endif

#endif /* __C2C_IO__ */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
