/**
  ******************************************************************************
  * @file    ug96_io.c
  * @author  MCD Application Team
  * @version V0.0.1
  * @date    11-August-2017
  * @brief   This file implments the IO operations to deal with the C2C 
  *          module. It mainly Inits and Deinits the UART interface. Send and
  *          receive data over it.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright � 2017 STMicroelectronics International N.V. 
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
#include "ug96_io.h"
#include "ug96.h"
#include "string.h"
/* Private define ------------------------------------------------------------*/
#define RING_BUFFER_SIZE                                    UG96_RX_DATABUF_SIZE


/* Private typedef -----------------------------------------------------------*/
typedef struct
{
  uint8_t  data[RING_BUFFER_SIZE];
  uint16_t tail; 				
  uint16_t head;
}RingBuffer_t;

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
RingBuffer_t UART_RxData;
UART_HandleTypeDef hUART_c2c;

extern UG96_Ret_t ReturnValues[];

/* Private function prototypes -----------------------------------------------*/
void C2C_ResetSIMConnection(void);

/* Private functions ---------------------------------------------------------*/

/**
  * @brief UART MSP Initialization 
  *        This function configures the hardware resources used in this example: 
  *           - Peripheral's clock enable
  *           - Peripheral's GPIO Configuration  
  *           - DMA configuration for transmission request by peripheral 
  *           - NVIC configuration for DMA interrupt request enable
  * @param hUART_c2c: UART handle pointer
  * @retval None
  */
static void UART_C2C_MspInit(UART_HandleTypeDef *hUART_c2c)
{
  static DMA_HandleTypeDef hdma_tx;
  GPIO_InitTypeDef  GPIO_Init;

  /* Enable the GPIO clock */
//  C2C_RST_GPIO_CLK_ENABLE();

  
  /* Set the GPIO pin configuration parametres */
  GPIO_Init.Pin       = C2C_RST_PIN;
  GPIO_Init.Mode      = GPIO_MODE_OUTPUT_PP;
  GPIO_Init.Pull      = GPIO_PULLUP;
  GPIO_Init.Speed     = GPIO_SPEED_HIGH;

  /* Configure the RST IO */
  HAL_GPIO_Init(C2C_RST_GPIO_PORT, &GPIO_Init);
  
  /* Enable DMA clock */
  DMAx_CLK_ENABLE();
  
  /*##-1- Enable peripherals and GPIO Clocks #################################*/
  /* Enable GPIO TX/RX clock */
  HAL_PWREx_EnableVddIO2(); /* needed for GPIO PGxx on L496AG*/
  USARTy_TX_GPIO_CLK_ENABLE();
  USARTy_RX_GPIO_CLK_ENABLE();

  /* Enable USARTy clock */
  USARTy_CLK_ENABLE(); 
 
  /* Enable DMA clock */
  DMAx_CLK_ENABLE();
  
  /*##-2- Configure peripheral GPIO ##########################################*/  
  /* UART TX GPIO pin configuration  */
  GPIO_Init.Pin       = USARTy_TX_PIN;
  GPIO_Init.Mode      = GPIO_MODE_AF_PP;
  GPIO_Init.Pull      = GPIO_PULLUP;
  GPIO_Init.Speed     = GPIO_SPEED_HIGH;
  GPIO_Init.Alternate = USARTy_TX_AF;

  HAL_GPIO_Init(USARTy_TX_GPIO_PORT, &GPIO_Init);

  /* UART RX GPIO pin configuration  */
  GPIO_Init.Pin = USARTy_RX_PIN;
  GPIO_Init.Alternate = USARTy_RX_AF;
  
  HAL_GPIO_Init(USARTy_RX_GPIO_PORT, &GPIO_Init);

  
  /*##-3- Configure the NVIC for UART ########################################*/   
  HAL_NVIC_SetPriority(USARTy_IRQn, 1, 0);
  HAL_NVIC_EnableIRQ(USARTy_IRQn);
  
  /*##-4- Configure the DMA ##################################################*/
  /* Configure the DMA handler for Transmission process */
  hdma_tx.Instance                 = USARTy_TX_DMA_CHANNEL;
  hdma_tx.Init.Direction           = DMA_MEMORY_TO_PERIPH;
  hdma_tx.Init.PeriphInc           = DMA_PINC_DISABLE;
  hdma_tx.Init.MemInc              = DMA_MINC_ENABLE;
  hdma_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
  hdma_tx.Init.MemDataAlignment    = DMA_MDATAALIGN_BYTE;
  hdma_tx.Init.Mode                = DMA_NORMAL;
  hdma_tx.Init.Priority            = DMA_PRIORITY_LOW;
  hdma_tx.Init.Request             = USARTy_TX_DMA_REQUEST;

  HAL_DMA_Init(&hdma_tx);

  /* Associate the initialized DMA handle to the UART handle */
  __HAL_LINKDMA(hUART_c2c, hdmatx, hdma_tx);

  /*##-4- Configure the NVIC for DMA #########################################*/
  /* NVIC configuration for DMA transfer complete interrupt (USARTy_TX) */
  HAL_NVIC_SetPriority(USARTy_DMA_TX_IRQn, 2, 0);
  HAL_NVIC_EnableIRQ(USARTy_DMA_TX_IRQn);
  
}

/**
  * @brief UART MSP De-Initialization
  *        This function frees the hardware resources used in this example:
  *          - Disable the Peripheral's clock
  *          - Revert GPIO, DMA and NVIC configuration to their default state
  * @param hUART_c2c: UART handle pointer
  * @retval None
  */
static void UART_C2C_MspDeInit(UART_HandleTypeDef *hUART_c2c)
{
  static DMA_HandleTypeDef hdma_tx;
  static DMA_HandleTypeDef hdma_rx;
  
  /*##-1- Reset peripherals ##################################################*/
  USARTy_FORCE_RESET();
  USARTy_RELEASE_RESET();

  /*##-2- Disable peripherals and GPIO Clocks #################################*/
  /* Configure UART Tx as alternate function  */
  HAL_GPIO_DeInit(USARTy_TX_GPIO_PORT, USARTy_TX_PIN);
  /* Configure UART Rx as alternate function  */
  HAL_GPIO_DeInit(USARTy_RX_GPIO_PORT, USARTy_RX_PIN);
  
  /*##-3- Disable the DMA Channels ###########################################*/
  /* De-Initialize the DMA Channel associated to transmission process */
  HAL_DMA_DeInit(&hdma_tx);
  /* De-Initialize the DMA Channel associated to reception process */
  HAL_DMA_DeInit(&hdma_rx);

  /*##-4- Disable the NVIC for DMA ###########################################*/
  HAL_NVIC_DisableIRQ(USARTy_DMA_TX_IRQn);
  HAL_NVIC_DisableIRQ(USARTy_DMA_RX_IRQn);
}


/**
  * @brief Set the pins related to the SIM to input nopull and enable the clocks
  * @retval None
  */
void C2C_ResetSIMConnection()
{
  GPIO_InitTypeDef  GPIO_InitStructSimInputs;

  __HAL_RCC_GPIOA_CLK_ENABLE();  
  __HAL_RCC_GPIOB_CLK_ENABLE();  
  __HAL_RCC_GPIOC_CLK_ENABLE();  
  
  /* SIM RESET setting (PC7)*/
  GPIO_InitStructSimInputs.Pin       = GPIO_PIN_7;
  GPIO_InitStructSimInputs.Mode      = GPIO_MODE_INPUT;
  GPIO_InitStructSimInputs.Pull      = GPIO_NOPULL;
  GPIO_InitStructSimInputs.Speed     = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStructSimInputs);
  
  /* SIM CLK setting (PA4)*/
  GPIO_InitStructSimInputs.Pin       = GPIO_PIN_4;
  GPIO_InitStructSimInputs.Mode      = GPIO_MODE_INPUT;
  GPIO_InitStructSimInputs.Pull      = GPIO_NOPULL;
  GPIO_InitStructSimInputs.Speed     = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStructSimInputs);
  
  /* SIM DATA setting (PB12) */
  GPIO_InitStructSimInputs.Pin       = GPIO_PIN_12;
  GPIO_InitStructSimInputs.Mode      = GPIO_MODE_INPUT;
  GPIO_InitStructSimInputs.Pull      = GPIO_NOPULL;
  GPIO_InitStructSimInputs.Speed     = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStructSimInputs);
}

/* Public functions ---------------------------------------------------------*/

/**
  * @brief Select which SIM to use
  * @param sim: SLOT_B1: exteranl slot, SLOT_B2: embedded slot
  * @retval None
  */
void C2C_SimSelect(C2C_SimSlot_t sim)
{
  GPIO_InitTypeDef  GPIO_InitStructSimSelect;

  __HAL_RCC_GPIOI_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();  

  C2C_ResetSIMConnection();
  HAL_Delay(50);
  
  /* SIM selection */
  switch( sim )
  {
  case SLOT_B1 :
    
    /* Sim_select0 */
    /* S */
    /* LOW */
    /* STMOD+, pin 18 (PC2) */
    GPIO_InitStructSimSelect.Pin       = GPIO_PIN_2;
    GPIO_InitStructSimSelect.Mode      = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStructSimSelect.Pull      = GPIO_PULLUP;
    GPIO_InitStructSimSelect.Speed     = GPIO_SPEED_FREQ_HIGH;

    HAL_GPIO_Init(GPIOC, &GPIO_InitStructSimSelect);
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_2, GPIO_PIN_RESET);

    /* Sim_select1 */
    /* OE */
    /* LOW */    
    /* STMOD+, pin 8 (PI3) */
    GPIO_InitStructSimSelect.Pin       = GPIO_PIN_3;
    GPIO_InitStructSimSelect.Mode      = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStructSimSelect.Pull      = GPIO_PULLUP;
    GPIO_InitStructSimSelect.Speed     = GPIO_SPEED_FREQ_HIGH;

    HAL_GPIO_Init(GPIOI, &GPIO_InitStructSimSelect);
    HAL_GPIO_WritePin(GPIOI, GPIO_PIN_3, GPIO_PIN_RESET);
    break;
    
   case SLOT_B2 :
  
    /* Sim_select0 */
    /* S */
    /* HIGH */
    /* STMOD+, pin 18 (PC2)*/
    GPIO_InitStructSimSelect.Pin       = GPIO_PIN_2;
    GPIO_InitStructSimSelect.Mode      = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStructSimSelect.Pull      = GPIO_PULLUP;
    GPIO_InitStructSimSelect.Speed     = GPIO_SPEED_FREQ_HIGH;

    HAL_GPIO_Init(GPIOC, &GPIO_InitStructSimSelect);
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_2, GPIO_PIN_SET);

    /* Sim_select1 */
    /* OE */
    /* LOW */    
    /* STMOD+, pin 8 (PI3) */
    GPIO_InitStructSimSelect.Pin       = GPIO_PIN_3;
    GPIO_InitStructSimSelect.Mode      = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStructSimSelect.Pull      = GPIO_PULLUP;
    GPIO_InitStructSimSelect.Speed     = GPIO_SPEED_FREQ_HIGH;

    HAL_GPIO_Init(GPIOI, &GPIO_InitStructSimSelect);
    HAL_GPIO_WritePin(GPIOI, GPIO_PIN_3, GPIO_PIN_RESET);
    break;
  }
  
}


/**
  * @brief Hw reset sequence and power up of the modem (it takes about 3 seconds)
  * @retval None
  */
void C2C_HwResetAndPowerUp(void)
{ 
 GPIO_InitTypeDef  GPIO_InitStructReset, GPIO_InitStructPwr;  

  C2C_RST_GPIO_CLK_ENABLE();
  C2C_PWRKEY_GPIO_CLK_ENABLE(); 
  
  // RESET sequence
  // STMOD+ IO12
  // LOW 
  GPIO_InitStructReset.Pin       = C2C_RST_PIN;
  GPIO_InitStructReset.Mode      = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStructReset.Pull      = GPIO_PULLUP;
  GPIO_InitStructReset.Speed     = GPIO_SPEED_FREQ_HIGH;

  HAL_GPIO_Init(C2C_RST_GPIO_PORT, &GPIO_InitStructReset);

  HAL_GPIO_WritePin(C2C_RST_GPIO_PORT, C2C_RST_PIN, GPIO_PIN_SET);
  HAL_Delay(200);
  HAL_GPIO_WritePin(C2C_RST_GPIO_PORT, C2C_RST_PIN, GPIO_PIN_RESET);
  HAL_Delay(150);
  
  // PWRKEY
  // STMOD+ IO9
  // at least LOW during 100 ms
  GPIO_InitStructPwr.Pin       = C2C_PWRKEY_PIN;
  GPIO_InitStructPwr.Mode      = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStructPwr.Pull      = GPIO_PULLUP;
  GPIO_InitStructPwr.Speed     = GPIO_SPEED_FREQ_HIGH;

  HAL_GPIO_Init(C2C_PWRKEY_GPIO_PORT,&GPIO_InitStructPwr);
  
  HAL_GPIO_WritePin(C2C_PWRKEY_GPIO_PORT, C2C_PWRKEY_PIN, GPIO_PIN_SET);
  HAL_Delay(150);
  HAL_GPIO_WritePin(C2C_PWRKEY_GPIO_PORT, C2C_PWRKEY_PIN, GPIO_PIN_RESET);
  HAL_Delay(2300);
  
}



/**
  * @brief  C2C IO Initalization.
  *         This function inits the UART interface to deal with the C2C,
  *         then starts asynchronous listening on the RX port.
  * @param None
  * @retval 0 on success, -1 otherwise.
  */
int8_t UART_C2C_Init(void)
{
  /* Set the C2C USART configuration parameters on MCU side */
  /* Attention: make sure the module uart is configured with the same values */
  hUART_c2c.Instance        = USARTy;
  hUART_c2c.Init.BaudRate   = 115200;  
  hUART_c2c.Init.WordLength = UART_WORDLENGTH_8B;
  hUART_c2c.Init.StopBits   = UART_STOPBITS_1;
  hUART_c2c.Init.Parity     = UART_PARITY_NONE;
  hUART_c2c.Init.HwFlowCtl  = UART_HWCONTROL_NONE;
  hUART_c2c.Init.Mode       = UART_MODE_TX_RX;
  hUART_c2c.Init.OverSampling = UART_OVERSAMPLING_16;

  UART_C2C_MspInit(&hUART_c2c);
  /* Configure the USART IP */
  if(HAL_UART_Init(&hUART_c2c) != HAL_OK)
  {
    return -1;
  }
 
  /* Once the C2C UART is intialized, start an asynchrounous recursive 
   listening. the HAL_UART_Receive_IT() call below will wait until one char is
   received to trigger the HAL_UART_RxCpltCallback(). The latter will recursively
   call the former to read another char.  */
  UART_RxData.head = 0;
  UART_RxData.tail = 0;
  HAL_UART_Receive_IT(&hUART_c2c, (uint8_t *)&UART_RxData.data[UART_RxData.tail], 1);
  
  return 0;
}


/**
  * @brief  C2C IO Deinitialization.
  *         This function Deinits the UART interface of the C2C. When called
  *         the C2C commands can't be executed anymore.
  * @param  None.
  * @retval None.
  */
int8_t UART_C2C_DeInit(void)
{
  /* Reset USART configuration to default */
  HAL_UART_DeInit(&hUART_c2c);
  UART_C2C_MspDeInit(&hUART_c2c);
  
  return 0;
}


/**
  * @brief  Flush Ring Buffer
  * @param  None
  * @retval None.
  */
void UART_C2C_FlushBuffer(void)
{
  memset(UART_RxData.data, 0, RING_BUFFER_SIZE);
  UART_RxData.head = UART_RxData.tail = 0;
}
/**
  * @brief  Send Data to the C2C module over the UART interface.
  *         This function allows sending data to the  C2C Module, the
  *          data can be either an AT command or raw data to send over 
             a pre-established C2C connection.
  * @param pData: data to send.
  * @param Length: the data length.
  * @retval 0 on success, -1 otherwise.
  */
int16_t UART_C2C_SendData(uint8_t* pData, uint16_t Length)
{
  if (HAL_UART_Transmit_DMA(&hUART_c2c, (uint8_t*)pData, Length) != HAL_OK)
  {
     return -1;
  }

  return Length;
}

/**
  * @brief  Send Data to the C2C module over the UART interface.
  *         This function allows sending only one char data to the  C2C Module, 
  * @param pData: data to send.
  * @param Length: the data length.
  * @param Timeout: transmit Timeout.
  * @retval 0 on success, -1 otherwise.
  */
int16_t UART_C2C_SendOneChar(uint8_t* pData, uint16_t Length, uint32_t Timeout)
{
  HAL_Delay(10);
  if (HAL_UART_Transmit(&hUART_c2c, (uint8_t*)pData, Length, Timeout) != HAL_OK)
  {
     return -1;
  }

  return Length;
}

/**
  * @brief  Receive Data from the C2C module over the UART interface.
  *         This function receives data from the  C2C module, the
  *         data is fetched from a ring buffer that is asynchonously and continuously
            filled with the received data.
  * @param  pData: a buffer inside which the data will be read.
  * @param  Length: Size of the data to receive.
  * @param  ScanVals: when param Lenght = 0 : values to be retrieved in the coming data in order to exit.
  * @retval int32_t: if param Lenght = 0 : the actual RET_CODE found, 
                     if param Lenght > 0 : the actual data size that has been received
                     if error (timeout): return -1.
  */
int32_t UART_C2C_ReceiveData(uint8_t* pData, uint16_t Length, uint32_t ScanVals, uint32_t Timeout)
{
  uint32_t tickstart = HAL_GetTick();
  int16_t ReadData = 0;
  uint16_t x;
  uint16_t index[NUM_RESPONSES];
  uint16_t lens[NUM_RESPONSES];  
  uint8_t c;
  int32_t  min_requested_time;

  min_requested_time = 2*Length*8*1000/hUART_c2c.Init.BaudRate;

  if (Timeout < min_requested_time)       /* UART speed 115200 bits per sec */ 
  { 
     Timeout = min_requested_time; 
#ifdef C2C_IO_DBG
     printf("                UART_C2C: Timeout forced to respect UART speed: %d \n", min_requested_time);
#endif 
  }
  
  for (x = 0; x < NUM_RESPONSES; x++)
  {
    index[x] = 0;
    lens[x] = strlen(ReturnValues[x].retstr);
  }

#ifdef C2C_IO_DBG
   printf("\n UART_C2C enter: Data.head %d, Data.tail %d, Length %d \n", UART_RxData.head, UART_RxData.tail, Length);
#endif 

  if ((Length == 0) && (ScanVals == RET_NONE))
  {
     return 0;  /* to avoid waiting a RET_VAL in case the parsed_lenth of payload is zero */
                /* but no code needs to be retrieved */
  }
        
  while ((HAL_GetTick() - tickstart ) < Timeout)
  {
    if(UART_RxData.head != UART_RxData.tail)
    {
      /* serial data available, so return data to user */
      c = pData[ReadData++] = UART_RxData.data[UART_RxData.head++];
      
      /* check for ring buffer wrap */
      if (UART_RxData.head >= RING_BUFFER_SIZE)
      {
        /* ring buffer wrap, so reset head pointer to start of buffer */
        UART_RxData.head = 0;
      }
      
      if (Length == 0) 
      {      
        /* Check whether we hit an ESP return values */
        for(x = 0; x < NUM_RESPONSES; x++)
        {
          if (c != ReturnValues[x].retstr[index[x]])  
          {  
            index[x] = 0;
          }  
         
          if (c == ReturnValues[x].retstr[index[x]])
          {
            if(++(index[x]) >= lens[x])
            {
              if (ScanVals & ReturnValues[x].retval)
              {
                return ReturnValues[x].retval;
              }
            }
          }
        }
      }
      else  /* end (Length > 0) */
      {  
        if (ReadData < Length)  /* nothing to do except keep reading */
        {
#ifdef C2C_IO_DBG 
          /* DEBUG: search for "recv" sequence */
//          x = 4; /* position of RET_URC_RECV in the ReturnValues matrix */
//          if (c != ReturnValues[x].retstr[index[x]])  
//          {  
//            index[x] = 0;
//          }  
//          
//          if (c == ReturnValues[x].retstr[index[x]])
//          {
//            printf("%c", c);
//            if(++(index[x]) >= lens[x])
//            {
//              if (ScanVals & ReturnValues[x].retval)
//              {
//                printf("\n UART_C2C ERRRRROOOORRR: this shouldnd be part of the payload \n ");
//              }
//            }
//          }
//          printf("%c", c);
#endif 
        }
        else /* ReadData >= Length */
        {
#ifdef C2C_IO_DBG
           printf("\n UART_C2C exit: Data.head %d, Data.tail %d, ReadData %d \n", UART_RxData.head, UART_RxData.tail, ReadData);

#endif 
          return ReadData;
        }
      } /* end (Length == 0) */
    }
  }

  if ((Length > 0) && (ReadData > 0))
  {
#ifdef C2C_IO_DBG
    printf("\n UART_C2C: Warning: timeout occurred before all data was read (%d/%d)\n ", ReadData, Length);
    printf("\n UART_C2C exit: Data.head %d, Data.tail %d, ReadData %d \n", UART_RxData.head, UART_RxData.tail, ReadData);
#endif 
    return ReadData;
  }

#ifdef C2C_IO_DBG
    printf("\n UART_C2C: RET_CODE not found or non a single byte has been read at all \n ");
#endif

  return -1; 
}

/**
  * @brief  Rx Callback when new data is received on the UART.
  * @param  UartHandle: Uart handle receiving the data.
  * @retval None.
  */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *UartH)
{
  /* If ring buffer end is reached reset tail pointer to start of buffer */
  if(++UART_RxData.tail >= RING_BUFFER_SIZE)
  {
    UART_RxData.tail = 0;   
  }
  
  HAL_UART_Receive_IT(UartH, (uint8_t *)&UART_RxData.data[UART_RxData.tail], 1);
}



/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
