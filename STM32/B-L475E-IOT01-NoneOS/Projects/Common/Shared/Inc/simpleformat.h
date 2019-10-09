/**
  ******************************************************************************
  * @file    simpleformat.h
  * @author  MCD Application Team
  * @version V0.0.1
  * @date    11-August-2017
  * @brief   Type definitions for parsing the IAR simple-code binary firmware 
  *          file format. 
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
#ifndef __simpleformat_H
#define __simpleformat_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>

/* Specification here: http://supp.iar.com/FilesPublic/UPDINFO/007283/430/doc/simple_code.htm */

typedef enum {
  SFF_INVALID_RECORD = 0,
  SFF_DATA_RECORD = 1,
  SFF_ENTRY_RECORD = 2,
  SFF_END_RECORD = 3
} sff_record_tag;

typedef enum {
  SFF_SEG_INVALID = 0,
  SFF_SEG_CODE = 1,
  SFF_SEG_DATA = 2,
  SFF_SEG_XDATA = 3,
  SFF_SEG_IDATA = 4,
  SFF_SEG_BIT = 5
} sff_segmenttype_t;
  

typedef struct {
  uint8_t magic_number[4];
  uint8_t program_flags[4];
  uint8_t number_of_program_bytes[4];
  uint8_t version_information[2];
} sff_fileheader_t;

typedef struct {
  uint8_t record_tag;
  uint8_t segment_type;
  uint8_t record_flags[2];
  uint8_t record_start_address[4];
  uint8_t number_of_program_bytes[4];
} sff_datarecord_t;

typedef struct {
  uint8_t record_tag;
  uint8_t entry_address[4];
  uint8_t segment_type;
} sff_entryrecord_t;

typedef struct {
  uint8_t record_tag;
  uint8_t checksum[4];
} sff_endrecord_t;



#ifdef __cplusplus
}
#endif

#endif /* __simpleformat_H */


/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

