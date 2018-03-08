/**
  ******************************************************************************
  * @file    mbedtls_patch.c
  * @author  MCD Application Team
  * @version V0.0.1
  * @date    11-August-2017
  * @brief   IoT Discovery Kit wrapper to firewall protected functions.
  *
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
#include "stdio.h"
#include "stdlib.h"
#include "firewall_wrapper.h"
#include "msg.h"

static size_t xxx_get_bitlen( const void *ctx )
{
  msg_info("xxx_get_bitlen : Not Implemented\n");
  abort();
  return 0;
}

static int xxx_verify_wrap( void *ctx, mbedtls_md_type_t md_alg,
                   const unsigned char *hash, size_t hash_len,
                   const unsigned char *sig, size_t sig_len )
{
  msg_info("xx1x_verify_wrap : Not Implemented\n");
  abort();
  return( 0 );
}


static int xxx_decrypt_wrap( void *ctx,
                    const unsigned char *input, size_t ilen,
                    unsigned char *output, size_t *olen, size_t osize,
                    int (*f_rng)(void *, unsigned char *, size_t), void *p_rng )
{
  msg_info("xxx_decrypt_wrap : Not Implemented\n");
  abort();
  return( 0 );
}

static int xxx_encrypt_wrap( void *ctx,
                    const unsigned char *input, size_t ilen,
                    unsigned char *output, size_t *olen, size_t osize,
                    int (*f_rng)(void *, unsigned char *, size_t), void *p_rng )
{
   msg_info("xxx_encrypt_wrap : Not Implemented\n");
   abort();
   return( 0 );
}

static int xxx_check_pair_wrap( const void *pub, const void *prv )
{
   msg_info("xxx_check_pair_wrap : Not Implemented\n");
   abort();
   return( 0 );
}

static void *xxx_alloc_wrap( void )
{
  msg_info("xxx_alloc_wrap : Not Implemented\n");
  abort();
}

static void xxx_debug( const void *ctx, mbedtls_pk_debug_item *items )
{
  msg_info("xxx_debug : Not Implemented\n");
  abort();
}

mbedtls_pk_info_t mbedtls_firewall_info = {
    MBEDTLS_PK_NONE,
    "FIREWALL",
    xxx_get_bitlen,
    firewall_can_do,
    xxx_verify_wrap,
    firewall_sign_wrap,
    xxx_decrypt_wrap,
    xxx_encrypt_wrap,
    xxx_check_pair_wrap,
    xxx_alloc_wrap,
    firewall_free_wrap,
    xxx_debug,
};


/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
