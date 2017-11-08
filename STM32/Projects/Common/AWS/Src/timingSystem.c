 /**
  ******************************************************************************
  * @file    timingSystem.c
  * @author  Central LAB
  * @version V1.0.0
  * @date    11-July-2017
  * @brief   Wrapper to STM32 timing
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
#include <time.h>
#include "timingSystem.h"

extern const int yytab[2][12];
/* Private typedef -----------------------------------------------------------*/
/* Private defines -----------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Functions Definition ------------------------------------------------------*/

#if defined (__IAR_SYSTEMS_ICC__)
#if _DLIB_TIME_USES_64
__time64_t __time64(__time64_t * pointer)
{
  return (__time64_t)TimingSystemGetSystemTime();
}
#else
__time32_t __time32(__time32_t * pointer)
{
  return (__time32_t)TimingSystemGetSystemTime();
}
#endif
#elif defined (__CC_ARM)

time_t time(time_t * pointer)
{
  return (time_t)TimingSystemGetSystemTime();
}

/** @brief gmtime conversion for ARM compiler
* @param  time_t *timer : File pointer to time_t structure
* @retval struct tm * : date in struct tm format
 */
struct tm *gmtimeMDK(register const time_t *timer)
{
  static struct tm br_time;
  register struct tm *timep = &br_time;
  time_t time = *timer;
  register unsigned long dayclock, dayno;
  int year = EEPOCH_YR;

  dayclock = (unsigned long)time % SSECS_DAY;
  dayno = (unsigned long)time / SSECS_DAY;

  timep->tm_sec = dayclock % 60;
  timep->tm_min = (dayclock % 3600) / 60;
  timep->tm_hour = dayclock / 3600;
  timep->tm_wday = (dayno + 4) % 7;       /* day 0 was a thursday */
  while (dayno >= YYEARSIZE(year)) {
    dayno -= YYEARSIZE(year);
    year++;
  }
  timep->tm_year = year - YYEAR0;
  timep->tm_yday = dayno;
  timep->tm_mon = 0;
  while (dayno >= yytab[LLEAPYEAR(year)][timep->tm_mon]) {
    dayno -= yytab[LLEAPYEAR(year)][timep->tm_mon];
    timep->tm_mon++;
  }
  timep->tm_mday = dayno + 1;
  timep->tm_isdst = 0;
  return timep;
}

struct tm * gmtime(const time_t *p)
{
  return gmtimeMDK(p);
}


#elif defined (__GNUC__)
time_t time(time_t * pointer)
{
  return (time_t)TimingSystemGetSystemTime();
}
#endif

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
