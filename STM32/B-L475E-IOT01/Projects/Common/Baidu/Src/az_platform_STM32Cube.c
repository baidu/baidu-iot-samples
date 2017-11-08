// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

/* STM32 platform porting:
 * <h2><center>&copy; Copyright (c) 2017 STMicroelectronics International N.V. 
 * All rights reserved.</center></h2>
 */

/* Includes ------------------------------------------------------------------*/
#include "msg.h"
#include "timedate.h"
#include "sensors_data.h"
#include "az_tlsio_mbedtls_STM32Cube.h"
#include "azure_c_shared_utility/strings.h"
#include "azure_c_shared_utility/platform.h"

extern const IO_INTERFACE_DESCRIPTION* tlsio_mbedtls_STM32Cube_get_interface_description(void);

/* Private typedef -----------------------------------------------------------*/
/* Private defines -----------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Functions Definition ------------------------------------------------------*/

/* Note: platform_init() and platform_deinit() are implemented in the Cloud-common module. */


const IO_INTERFACE_DESCRIPTION* platform_get_default_tlsio(void)
{
  return tlsio_mbedtls_get_interface_description();
}


STRING_HANDLE platform_get_platform_info(void)
{
  return STRING_construct("(STM32Cube)");
}
