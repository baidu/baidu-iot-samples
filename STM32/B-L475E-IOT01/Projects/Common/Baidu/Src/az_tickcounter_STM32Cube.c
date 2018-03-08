// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

/* STM32 platform porting:
 * <h2><center>&copy; Copyright (c) 2017 STMicroelectronics International N.V. 
 * All rights reserved.</center></h2>
 */

/* Includes ------------------------------------------------------------------*/
#include <stdlib.h>
#include <stdint.h>
#include "main.h"
#ifdef UNUSED /* Prevent the redefinition from HAL to MS shared utilities */
#undef UNUSED
#endif 
#include "azure_c_shared_utility/gballoc.h"
#include "azure_c_shared_utility/tickcounter.h"
#include "azure_c_shared_utility/optimize_size.h"
#include "azure_c_shared_utility/xlogging.h"

/* Private typedef -----------------------------------------------------------*/
typedef struct TICK_COUNTER_INSTANCE_TAG
{
	int dummy : 1;
} TICK_COUNTER_INSTANCE;

/* Private defines -----------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Functions Definition ------------------------------------------------------*/

TICK_COUNTER_HANDLE tickcounter_create(void)
{
    TICK_COUNTER_INSTANCE* result = (TICK_COUNTER_INSTANCE*)malloc(sizeof(TICK_COUNTER_INSTANCE));
    if (result == NULL)
    {
		/* add any per instance initialization (starting a timer for example) here if needed (most platforms will not need this) */
		
		LogError("Cannot create tick counter");
    }
	
    return result;
}

void tickcounter_destroy(TICK_COUNTER_HANDLE tick_counter)
{
    if (tick_counter != NULL)
    {
		/* add any per instance de-initialization here (stopping the time) if needed (most platforms will not need this) */
		
        free(tick_counter);
    }
}

   
int tickcounter_get_current_ms(TICK_COUNTER_HANDLE tick_counter, tickcounter_ms_t* current_ms)
{
    int result = 0;
	
    if (tick_counter == NULL)
    {
        LogError("tickcounter failed: Invalid Arguments.");
        result = __FAILURE__;
    }
    else
    {
		/* call here any platform/OS specific function to get the millisecond tick and perform any conversions */
    /* Important: The HAL_GetTick return one uint32_t Values .. so after 49 days it will restart */
    *current_ms = HAL_GetTick();
    }
	
    return result;
}
