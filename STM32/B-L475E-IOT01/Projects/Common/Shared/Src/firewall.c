/**
  ******************************************************************************
  * @file    firewall.c
  * @author  MCD Application Team
  * @version V0.0.1
  * @date    11-August-2017
  * @brief   IoT Discovery Kit firewall init functions.
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
#include <stdarg.h>
#include "string.h"
#include "heap.h"
#include "flash.h"
#include "main.h"
#include "firewall.h"
#include "mbedtls/platform.h"
#include "mbedtls/ctr_drbg.h"



/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
#define LOG (*myprintf)
#define ALIGN(a,align)  (((a)+(align)-1) & (~((align)-1)))
/* Private variables ---------------------------------------------------------*/

extern void *__ICFEDIT_region_ROM_start__;
extern void *__ICFEDIT_region_ROM_end__;
extern void *__ICFEDIT_region_RAM_start__;
extern void *__ICFEDIT_region_RAM_end__;


int     (*myprintf)(char const *s,...);
static  const mbedtls_pk_info_t *pk_info = 0;

void __iar_data_init3(void);
int  ProtectedStack(int funcid,  va_list(ap));
void FireWallInitRange(void *secret,size_t secret_size);
void stack_switch(void);
void stack_restore(void);
int FireWallCallGate(funcid_t funcid,...);

/* Functions Definition ------------------------------------------------------*/

//
// WARNING : DO NOT PLACE FUNCTION OVER THIS LIMIT
//
// the function FireWallCallGate must be the first function of this source file
// this is mandatory to ensure correct placement in memory and fullfit to
// fire wall rules , having firewall routine at segemnt start address + 4 byte 
// offset 

int FireWallCallGate(funcid_t funcid,...)
{
    int res;
    va_list(ap);
    va_start(ap,funcid);
    
    if (funcid == FIREWALL_INIT_FUNC)
    {
         __iar_data_init3();
        void *secret = va_arg(ap,void *);
        size_t secret_size = va_arg(ap,size_t);
        myprintf = va_arg(ap,int (*)(char const*s,...));
        
        mbedtls_platform_set_calloc_free( heap_alloc , heap_free);
        FireWallInitRange(secret,secret_size);
        
        va_end(ap);
        return 0;
    }
    
    stack_switch();
#ifdef HEAP_DEBUG
    stack_measure_prologue();
#endif
    res = ProtectedStack(funcid,ap);
#ifdef HEAP_DEBUG
    stack_measure_epilogue();
#endif
    stack_restore();
    
    va_end(ap);
    return res;
}


uint32_t HAL_GetTick(void)
{
  return 0;
}


int     ProtectedStack(int      funcid,va_list(ap))
{
       int res;
 /* Clear the FIREWALL pre arm bit to disable fetch outside of protected code area */
    CLEAR_BIT(FIREWALL->CR, FW_CR_FPA);
    switch(funcid)
    {
      case FIREWALL_PK_PARSE_KEY_FUNC:
      {
        mbedtls_pk_context *pk = va_arg(ap,mbedtls_pk_context *);
        const unsigned char *key = va_arg(ap,const unsigned char *);
        size_t keylen = va_arg(ap,size_t);
        const unsigned char *pwd = va_arg(ap,const unsigned char *);
        size_t pwdlen = va_arg(ap,size_t);
        // cannot be computed outisde of firewall , so caller argument is ignored
        keylen = strlen((char const*) key)+1;

        res=  mbedtls_pk_parse_key(pk,key,keylen,pwd,pwdlen );
        // destroy RAM key
        memset((void*)key,0,keylen);
        pk_info = pk->pk_info;
      }
      break;
  
      case FIREWALL_CANDO_FUNC:
      {
         // va_args on int to avoid ellipsis warning parameter
          mbedtls_pk_type_t type= (mbedtls_pk_type_t) va_arg(ap,int);
          res = pk_info->can_do(type);
      }
      break;
      
      case FIREWALL_SIGN_FUNC:
      {
           void *ctx = va_arg(ap,void *);
           // va_args on int to avoid ellipsis warning parameter
           mbedtls_md_type_t md_alg = (mbedtls_md_type_t)va_arg(ap,int);
           const unsigned char *hash = va_arg(ap,const unsigned char *);
           size_t hash_len = va_arg(ap,size_t);
           unsigned char *sig = va_arg(ap,unsigned char *);
           size_t *sig_len = va_arg(ap,size_t *);
           int (*f_rng)(void *, unsigned char *, size_t) = va_arg(ap,int (*)(void *, unsigned char *, size_t));
           void *p_rng = va_arg(ap,void*);
           // patching function pointer to not call a function outside of firewall
           f_rng=mbedtls_ctr_drbg_random;
           
           res =pk_info->sign_func(ctx,md_alg,hash,hash_len,sig,sig_len,f_rng,p_rng);
      }
      break;

      case  FIREWALL_CTX_FREE_FUNC:
      {
          mbedtls_rsa_context *ctx = va_arg(ap,mbedtls_rsa_context*);
          pk_info->ctx_free_func(ctx);
      }
      break;
      
      case FIREWALL_HEAP_STAT_FUNC:
      {
        uint32_t *hmax= va_arg(ap,uint32_t *);
        uint32_t *hcurr= va_arg(ap,uint32_t *);
        uint32_t *stackmax= va_arg(ap,uint32_t *);
        *hmax=0;
        *hcurr=0;
        *stackmax=0;
#ifdef HEAP_DEBUG
        heap_stat(hmax,hcurr,stackmax);
#endif
      }
      break;
      
      case FIREWALL_FLASH_KEY_FUNC:
      {
        uint32_t dst_addr= va_arg(ap,uint32_t );
        const void *data= va_arg(ap,void *);
        uint32_t size= va_arg(ap,uint32_t);
        
        res= FLASH_update(dst_addr,data,size);
      }
      break;
   
  }
    
    /* Set the FIREWALL pre arm bit to enable fetch outside of protected code area */ 
    SET_BIT(FIREWALL->CR, FW_CR_FPA);
    return res;
}

void FireWallInitRange(void *secret,size_t secret_size)
{
  uint32_t CodeRomStart = (uint32_t) & __ICFEDIT_region_ROM_start__;
  uint32_t CodeRomEnd   = (uint32_t) & __ICFEDIT_region_ROM_end__;
  uint32_t CodeRomLen= CodeRomEnd-CodeRomStart;
  CodeRomLen= ALIGN(CodeRomLen,256);
  LOG(" CodeRom [0x%x-0x%x] size 0x%x (%d)\n",CodeRomStart,CodeRomEnd,CodeRomLen,CodeRomLen);
  
  uint32_t DataRamStart = (uint32_t) & __ICFEDIT_region_RAM_start__;
  uint32_t DataRamEnd   = (uint32_t) & __ICFEDIT_region_RAM_end__;
  uint32_t DataRamLen = DataRamEnd - DataRamStart;
  DataRamLen= ALIGN(DataRamLen,64);
  LOG(" DataRam [0x%x-0x%x] size 0x%x (%d)\n",DataRamStart,DataRamEnd,DataRamLen,DataRamLen);
   
  
  uint32_t DataRomStart = (uint32_t) secret;
  uint32_t DataRomLen= ALIGN(secret_size,256);
  uint32_t DataRomEnd   = DataRomStart+ DataRomLen;
  LOG(" DataRom [0x%x-0x%x] size 0x%x (%d)\n",DataRomStart,DataRomEnd,DataRomLen,DataRomLen);

  
   /* Enabling the clock for the Firewall peripheral */
  __HAL_RCC_FIREWALL_CLK_ENABLE();
  
  /* Code segment address initialization , page size is 256*/
  (FIREWALL->CSSA) = CodeRomStart;
  /* Code segment length */
  (FIREWALL->CSL) = CodeRomLen-1;

  /* NVM data address initialization ,data rom with pagesize 256*/
  (FIREWALL->NVDSSA) = DataRomStart;
  /* NVM data segment length */
  (FIREWALL->NVDSL) = DataRomLen;

  /* Volatile data segment address initialization, data ram with pagesize of 64 */
  (FIREWALL->VDSSA) = DataRamStart;
  /* Volatile data segment length */
  (FIREWALL->VDSL) = DataRamLen-1;

  /* SRAM1 protected area is not shared with non-protected code and it is not
       executable , so no data access , no code access from unsecure code */

  CLEAR_BIT(FIREWALL->CR,FW_CR_VDS);
  CLEAR_BIT(FIREWALL->CR,FW_CR_VDE);
  
  /* Set the FIREWALL pre arm bit to enable fetch outside of protected code area */
  CLEAR_BIT(FIREWALL->CR, FW_CR_FPA);

}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
