/**
  ******************************************************************************
  * @file    timer.ch
  * @author  MCD Application Team
  * @version V0.0.1
  * @date    11-August-2017
  * @brief   main application header file.
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
 
/**
 * @file timer.c
 * @brief Linux implementation of the timer interface.
 */
#include "main.h"
#include "timer.h"  

void sleep(int DelaySec);
void TimerCountdownMS(Timer* timer, unsigned int timeout_ms);
void TimerCountdown(Timer* timer, unsigned int timeout);
int TimerLeftMS(Timer* timer);
char TimerIsExpired(Timer* timer);
void TimerInit(Timer* timer);
  
void sleep(int DelaySec)
{
 HAL_Delay(DelaySec*1000);
}  
  
bool has_timer_expired(Timer *timer) {
  uint32_t tickstart = 0;

  __disable_irq();

  tickstart = HAL_GetTick();
  long left = timer->end_time - tickstart;

  __enable_irq();

  return (left < 0);
}

void countdown_ms(Timer *timer, uint32_t timeout) {
  uint32_t tickstart = 0;

  __disable_irq();

  tickstart = HAL_GetTick();
  timer->end_time = tickstart + timeout;

  __enable_irq();
}

uint32_t left_ms(Timer *timer) {
  uint32_t tickstart = 0;

  __disable_irq();

  tickstart = HAL_GetTick();
  long left = timer->end_time - tickstart;

  __enable_irq();

  return (left < 0) ? 0 : left;
}

void countdown_sec(Timer *timer, uint32_t timeout) {
  uint32_t tickstart = 0;

  __disable_irq();

  tickstart = HAL_GetTick();
  timer->end_time = tickstart + (timeout * 1000);

  __enable_irq();
}

void init_timer(Timer *timer) {
  timer->end_time = 0;
}
void TimerCountdownMS(Timer* timer, unsigned int timeout_ms)
{
  timer->init_tick = HAL_GetTick();
  timer->end_time = timeout_ms;
}


void TimerCountdown(Timer* timer, unsigned int timeout)
{
  TimerCountdownMS(timer, timeout * 1000);
}


int TimerLeftMS(Timer* timer)
{
  int ret = 0;
  uint32_t cur_tick = HAL_GetTick();  // The HAL tick period is 1 millisecond.
  if (cur_tick < timer->init_tick)
  { // Timer wrap-around detected
    // printf("Timer: wrap-around detected from %d to %d\n", timer->init_tick, cur_tick);
    timer->end_time -= 0xFFFFFFFF - timer->init_tick;
    timer->init_tick = 0;
  }
  ret = timer->end_time - (cur_tick - timer->init_tick);

  return (ret >= 0) ? ret : 0;
}


char TimerIsExpired(Timer* timer)
{
  return (TimerLeftMS(timer) > 0) ? 0 : 1;
}


void TimerInit(Timer* timer)
{
  timer->init_tick = 0;
  timer->end_time = 0;
}

/**
  * @brief   fills a string buffer with a timestamp:date + UTC time. The format is yyyy:mm:ddThh:mm:ssZ
  * @param  In: buff       string buffer address
  * @param  In: buffSize   string buffer size
  * @retval O the date and time have been retrieved
  *        -1 otherwise 
  */
int getTimestamp(char *buff, int buffSize)
{
  int ret = 0;
  int n = 0;
  RTC_DateTypeDef date;
  RTC_TimeTypeDef time;
  
  if ( (HAL_RTC_GetTime(&hrtc,&time,0) != HAL_OK) || (HAL_RTC_GetDate(&hrtc,&date,0) != HAL_OK) )
  {
    ret = -1; 
  } 
  else 
  {
    n = snprintf(buff,buffSize,"%04d-%02d-%02dT%02d:%02d:%02dZ", 2000 + date.Year, date.Month, date.Date, time.Hours, time.Minutes, time.Seconds);
  
    if (n > buffSize)
    {
      ret = -1;
    }
  }
  
  return ret;
}

