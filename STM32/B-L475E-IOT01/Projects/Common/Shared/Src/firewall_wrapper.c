/**
  ******************************************************************************
  * @file    firewall_wrapper.c
  * @author  MCD Application Team
  * @version V0.0.1
  * @date    11-August-2017
  * @brief   IoT Discovery Kit wrapper to firewall protected functions.
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
#if defined (STM32L475xx)|| defined (STM32L496xx)
#include "stm32l4xx_hal.h"
#endif
#include "firewall.h"
#include "firewall_wrapper.h"
#include "iot_flash_config.h"

#ifdef   FIREWALL_MBEDLIB
#if defined ( __ICCARM__ )
extern char  __firewall_ROM_start;
#elif defined ( __CC_ARM )
#error  "Firewall is not supported on Keil/armcc in this package." */
// char  __firewall_ROM_start __attribute__((at(0x08066D00)));
#elif defined   (  __GNUC__  )
#error  "Firewall is not supported on Keil/armcc in this package." */
//char  __firewall_ROM_start __attribute__((section(".firewall_section")));
#endif

extern  const user_config_t lUserConfig;
#endif


/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define FIREWALL_ENTRY_OFFSET   5

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
int (*FireWallCallGatePtr)(funcid_t funcid,...);

/* Functions Definition ------------------------------------------------------*/

void firewall_init(void)
{
#ifdef   FIREWALL_MBEDLIB
  FireWallCallGatePtr = ( int(*)(funcid_t,...)) (&__firewall_ROM_start+FIREWALL_ENTRY_OFFSET);
  printf("FireWAll entry point : %p\n",(void*) FireWallCallGatePtr);
  (*FireWallCallGatePtr)(FIREWALL_INIT_FUNC,(void *) &lUserConfig.tls_device_key[0],USER_CONF_TLS_OBJECT_MAX_SIZE, printf);
   //enabling firewall
  CLEAR_BIT(SYSCFG->CFGR1,SYSCFG_CFGR1_FWDIS);  
  printf("FireWAll is on\n");
#ifdef CHECK_FIREWALL
   for(int i=0;i<20;i++)
  {
    volatile char *p= (char *)&lUserConfig.tls_device_key[i-10];
    printf("Testing at %x\n",p);
    printf("Reading at %d\n",*p);
    printf("Testing OK %x\n",p);
  }
  for(int i=USER_CONF_TLS_OBJECT_MAX_SIZE+256+10;i>0;i--)
  {
    volatile char *p= (char *)&lUserConfig.tls_device_key[i];
    printf("Testing at %x\n",p);
    printf("Reading at %d\n",*p);
    printf("Testing OK %x\n",p);
  }
#endif
#endif /* FIREWALL_MBEDLIB */
}

int mbedtls_firewall_pk_parse_key(  mbedtls_pk_context *pk,
                  const unsigned char *key, size_t keylen,
                  const unsigned char *pwd, size_t pwdlen )
{
    int ret;
   __disable_irq();
    ret = (*FireWallCallGatePtr)(FIREWALL_PK_PARSE_KEY_FUNC,pk,key,keylen,pwd,pwdlen );
    __enable_irq();
    return ret;
}

int firewall_can_do( mbedtls_pk_type_t type )
{
  int ret;
  __disable_irq();
  ret =  (*FireWallCallGatePtr)(FIREWALL_CANDO_FUNC,type);
  __enable_irq();
  return ret; 
}

int firewall_sign_wrap( void *ctx, mbedtls_md_type_t md_alg,
                      const unsigned char *hash, size_t hash_len,
                      unsigned char *sig, size_t *sig_len,
                      int (*f_rng)(void *, unsigned char *, size_t),
                      void *p_rng )
{
  int ret;
  __disable_irq();
  ret =  (*FireWallCallGatePtr)(FIREWALL_SIGN_FUNC,ctx,md_alg,hash,hash_len,sig,sig_len,f_rng,p_rng); 
  __enable_irq();

  return ret;
}

void firewall_free_wrap( void *ctx )
{
    __disable_irq();
    (*FireWallCallGatePtr)(FIREWALL_CTX_FREE_FUNC,ctx );
    __enable_irq();
}

void heap_firewall_stat(uint32_t *hmax,uint32_t *hcurr, uint32_t *stacksize)
{
  __disable_irq();
      (*FireWallCallGatePtr)(FIREWALL_HEAP_STAT_FUNC,hmax,hcurr,stacksize);
  __enable_irq();
}

int FLASH_firewall_update(uint32_t dst_addr, const void *data, uint32_t size)
{
  int ret;
  __disable_irq();
  ret = (*FireWallCallGatePtr)(FIREWALL_FLASH_KEY_FUNC,dst_addr,data,size);
  __enable_irq();
  return ret;
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
