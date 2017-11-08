/**
  ******************************************************************************
  * @file    heap.c
  * @author  MCD Application Team
  * @version V1.0.0
  * @date    11-July-2017
  * @brief   Heap check functions.
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

#include "heap.h"
#include "stdlib.h"
#include "msg.h"

#ifndef CODE_UNDER_FIREWALL
#include "stdio.h"
#define heaplog      msg_info
#define MAGIC_NUMBER 0xDEADFA56
#else
#define MAGIC_NUMBER 0x12345678
// msg_info is not working under firewall code
#define heaplog(...)
#endif

#include "string.h"
static  uint32_t heap_allocated=0;
static  uint8_t *pmax=0;
static  uint8_t *pmin =(uint8_t*) 0xffffffff;

static size_t stack_max_size=0;
static unsigned char *stack_start;

#if defined (__ICCARM__)
extern void *CSTACK$$Base;
static unsigned char *stack_init = (unsigned char*) &CSTACK$$Base;
#elif defined ( __CC_ARM   ) 
extern void *__initial_sp;
static unsigned char *stack_init = (unsigned char*) &__initial_sp;
#elif defined (__GNUC__)
extern void *_Min_Stack_Size;
static unsigned char *stack_init = (unsigned char*) &_Min_Stack_Size;
#endif

void stack_measure_prologue(void);
void stack_measure_epilogue(void);


void stack_measure_prologue()
{
  unsigned char p;
  stack_start = &p;
  memset(stack_init, 0xC8, stack_start-stack_init-10);
}


void stack_measure_epilogue()
{
  unsigned char * p = stack_init;
  while(*p == 0xC8) p++;
  size_t stack_size = stack_start - p;
  if (stack_size > stack_max_size)
  {
    stack_max_size=stack_size;
  }
}


void heap_stat(uint32_t *heap_max,uint32_t *heap_current,uint32_t *stack_size)
{
  *heap_max = pmax-pmin;
  *heap_current = heap_allocated;
  *stack_size = stack_max_size;
}

#ifdef HEAP_DEBUG
static  void heap_abort(void)
{
  heaplog("HEAP is corrupted \n");
  while(1);
}


void *heap_alloc(size_t a,size_t b)
{
  uint8_t *p;
  unsigned int n = a*b;
  
  n = (n + 3) & ~3; /* ensure alignment for magic number */
  n += 8;           /* add 2 x 32 bit for size and magic number */
  
  p = (uint8_t*)calloc(n,1);
  
  if (p == NULL)
  {
    heaplog("HEAP run out of memory\n");
    while(1);
    /* return 0;*/
  }
  if ((p+n) > pmax) pmax = (p+n);
  if (p < pmin) pmin = p;
  
  heap_allocated += n;

  *((uint32_t*)p) = n; /* remember size */
  *((uint32_t*)(p - 4 + n)) = MAGIC_NUMBER; /* write magic number  after user allocation */
  
  return p+4;
}

void heap_free(void  *p)
{
   if (p==0) return;
  
   p = (uint8_t*)p - 4;
   int n = *((uint32_t*)p);
   
   heap_allocated-=n;
  
   /* check that magic number is not corrupted */
   if (*((uint32_t*)((uint8_t*) p + n - 4)) != MAGIC_NUMBER)
   {
    heap_abort(); /* error: data overflow or freeing already freed memory */
   }
   /* remove magic number to be able to detect freeing already freed memory */
  *((uint32_t*)((uint8_t*) p + n -4)) = 0;
  free(p);
}
#endif

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
