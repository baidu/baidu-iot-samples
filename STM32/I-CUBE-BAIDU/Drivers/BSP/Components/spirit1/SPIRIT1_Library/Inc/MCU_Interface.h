/**
 * @file    MCU_Interface.h
 * @author  VMA division - AMS
 * @version V2.0.2
 * @date    Febrary 7, 2015
 * @brief   Header file for low level SPIRIT SPI driver.
 * @details
 *
 * This header file constitutes an interface to the SPI driver used to
 * communicate with Spirit.
 * It exports some function prototypes to write/read registers and FIFOs
 * and to send command strobes.
 * Since the Spirit libraries are totally platform independent, the implementation
 * of these functions are not provided here. The user have to implement these functions
 * taking care to keep the exported prototypes.
 *
 * These functions are:
 *
 * <ul>
 * <li>SpiritSpiInit</i>
 * <li>SpiritSpiWriteRegisters</i>
 * <li>SpiritSpiReadRegisters</i>
 * <li>SpiritSpiCommandStrobes</i>
 * <li>SpiritSpiWriteLinearFifo</i>
 * <li>SpiritSpiReadLinearFifo</i>
 * </ul>
 *
 * @note An example of SPI driver implementation is available in the <i>Sdk_Eval</i> library.
 *
 * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
 * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
 * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
 * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
 * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
 * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
 *
 * THIS SOURCE CODE IS PROTECTED BY A LICENSE.
 * FOR MORE INFORMATION PLEASE CAREFULLY READ THE LICENSE AGREEMENT FILE LOCATED
 * IN THE ROOT DIRECTORY OF THIS FIRMWARE PACKAGE.
 *
 * <h2><center>&copy; COPYRIGHT 2015 STMicroelectronics</center></h2>
 */


/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MCU_INTERFACE_H
#define __MCU_INTERFACE_H


/* Includes ------------------------------------------------------------------*/
#include "SPIRIT_Types.h"


#ifdef __cplusplus
extern "C" {
#endif


/** @addtogroup SPIRIT_Libraries
 * @{
 */


/** @defgroup SPIRIT_SPI_Driver         SPI Driver
 * @brief Header file for low level SPIRIT SPI driver.
 * @details See the file <i>@ref MCU_Interface.h</i> for more details.
 * @{
 */



/** @defgroup SPI_Exported_Types        SPI Exported Types
 * @{
 */

/**
 * @}
 */



/** @defgroup SPI_Exported_Constants    SPI Exported Constants
 * @{
 */

/**
 * @}
 */



/** @defgroup SPI_Exported_Macros       SPI Exported Macros
 * @{
 */

/**
 * @}
 */



/** @defgroup SPI_Exported_Functions    SPI Exported Functions
 * @{
 */

typedef SpiritStatus StatusBytes;

void RadioSpiInit(void);
StatusBytes RadioSpiWriteRegisters(uint8_t cRegAddress, uint8_t cNbBytes, uint8_t* pcBuffer);
StatusBytes RadioSpiReadRegisters(uint8_t cRegAddress, uint8_t cNbBytes, uint8_t* pcBuffer);
StatusBytes RadioSpiCommandStrobes(uint8_t cCommandCode);
StatusBytes RadioSpiWriteFifo(uint8_t cNbBytes, uint8_t* pcBuffer);
StatusBytes RadioSpiReadFifo(uint8_t cNbBytes, uint8_t* pcBuffer);

void EnterShutdown(void);
void ExitShutdown(void);
SpiritFlagStatus CheckShutdown(void);

#define SpiritEnterShutdown                                  RadioEnterShutdown
#define SpiritExitShutdown                                   RadioExitShutdown
#define SpiritCheckShutdown                                  (SpiritFlagStatus)RadioCheckShutdown


#define SpiritSpiInit                                                  RadioSpiInit
#define SpiritSpiWriteRegisters(cRegAddress, cNbBytes, pcBuffer)       RadioSpiWriteRegisters(cRegAddress, cNbBytes, pcBuffer)
#define SpiritSpiReadRegisters(cRegAddress, cNbBytes, pcBuffer)        RadioSpiReadRegisters(cRegAddress, cNbBytes, pcBuffer)
#define SpiritSpiCommandStrobes(cCommandCode)                          RadioSpiCommandStrobes(cCommandCode)
#define SpiritSpiWriteLinearFifo(cNbBytes, pcBuffer)                   RadioSpiWriteFifo(cNbBytes, pcBuffer)
#define SpiritSpiReadLinearFifo(cNbBytes, pcBuffer)                    RadioSpiReadFifo(cNbBytes, pcBuffer)

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
