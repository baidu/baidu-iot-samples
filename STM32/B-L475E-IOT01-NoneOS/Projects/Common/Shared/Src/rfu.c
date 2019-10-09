/**
  ******************************************************************************
  * @file    rfu.c
  * @author  MCD Application Team
  * @version V0.0.1
  * @date    11-August-2017
  * @brief   Remote firmare upate over TCP/IP.
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
#include <stdio.h>        /* Debug printf */
#include <string.h>
#include <stdlib.h>
#include "main.h"         /* Flash layout */
#include "rfu.h"
#include "http_util.h"
#include "msg.h"
#include "net.h"
#ifdef RFU
#include "simpleformat.h"
#endif
#include "flash.h"
#include "iot_flash_config.h"
#include "version.h"
    
/* Private defines -----------------------------------------------------------*/
#define HTTP_MAX_HOST_SIZE        80      /**< Max length of the http server hostname. */
#define HTTP_MAX_QUERY_SIZE       50      /**< Max length of the http query string. */
#define HTTP_READ_BUFFER_SIZE     1050    /**< Size of the HTTP receive buffer.
                                               Must be larger than the HTTP response header length. That is about 300-800 bytes. */

/* #define URL_TRACE */       /**< Activate the URL decoding trace. */
/* #define DL_TRACE  */       /**< Activate the HTTP file download trace. */
/* #define FF_TRACE  */       /**< Activate the firmware file format decoding trace. */
/* #define SKIP_WRITING */    /**< Prevent writing the downloaded firmware into FLASH. Used for debug. */
 
/** Will only receive the simple format headers. Not need to be as large as a Flash page. */
#define RFU_READBUF_SIZE          MAX(MAX(MAX(sizeof(sff_fileheader_t),\
                                      sizeof(sff_datarecord_t)),\
                                      sizeof(sff_entryrecord_t)),\
                                      sizeof(sff_endrecord_t))

/** Translate a ROM address to the other Flash bank. */
#define ALT_BANK(a)     ( ((a) >= (FLASH_BASE + FLASH_BANK_SIZE)) ? ((a) - FLASH_BANK_SIZE) : ((a) + FLASH_BANK_SIZE) )
                                        
#define ROUND_DOWN(a,b) ( ((a) / (b)) * (b) )
#define MIN(a,b)        ( ((a)<(b)) ? (a) : (b) )
#define MAX(a,b)        ( ((a)<(b)) ? (b) : (a) )
      

/* Private typedef -----------------------------------------------------------*/
/* Private variables ----------------------------------------------------------*/
/** Important: lFwVersion must be stored at an address which persists through the FW builds. */

#ifdef __ICCARM__  /* IAR */
const firmware_version_t lFwVersion @ "INITED_FIXED_LOC" = { FW_VERSION_NAME, FW_VERSION_MAJOR, FW_VERSION_MINOR, FW_VERSION_PATCH, __DATE__, __TIME__ };
#elif defined ( __CC_ARM   ) /* Keil / armcc */
const firmware_version_t lFwVersion __attribute__((section("INITED_FIXED_LOC"))) = { FW_VERSION_NAME, FW_VERSION_MAJOR, FW_VERSION_MINOR, FW_VERSION_PATCH, __DATE__, __TIME__ };
#elif defined ( __GNUC__ )      /*GNU Compiler */
const firmware_version_t lFwVersion __attribute__((section("INITED_FIXED_LOC"))) = { FW_VERSION_NAME, FW_VERSION_MAJOR, FW_VERSION_MINOR, FW_VERSION_PATCH, __DATE__, __TIME__ };
#endif


/* Private function prototypes -----------------------------------------------*/

#ifdef RFU
static void sum_update(uint32_t * const checksum, const void * const address, const int size);
static uint16_t getShortFromArray(const uint8_t * const data);
static uint32_t getWordFromArray(const uint8_t * const data);
static void print_fileheader(const sff_fileheader_t * const pFileHeader);
static void print_datarecord(const sff_datarecord_t * const pRecord);
static void print_entryrecord(const sff_entryrecord_t * const pRecord);
static void print_endrecord(const sff_endrecord_t * const pRecord);
#endif /* RFU */


/* Functions Definition ------------------------------------------------------*/

/**
 * @brief   Get the version info of the running firmware.
 * @param   Out: version    Allocated by the caller.
 * @param   In:  alt        Get the version of the FW in the alternate bank.
 *                          Note: No check is made that this is a valid version.                  
 * @retval  Error code
 *            RFU_OK   (0)  Success
 *            RFU_ERR (<0)  Failure
 */
int rfu_getVersion(firmware_version_t const ** const version, bool alt)
{
  int ret = RFU_OK;
  
  if (alt == true)
  {
#ifdef RFU
    *version = (firmware_version_t *) ALT_BANK( (uint32_t) &lFwVersion );
#else
    ret = RFU_ERR;
#endif
  }
  else
  {
    *version = &lFwVersion;
  }
  
  return ret;
}

#ifdef RFU
/**
 * @brief   Update the cumulated sum of the firmware file being downloaded.
 * @note    The checksum initialization is the caller responsibility.
 * @param   In/Out: checksum    Cumulated byte sum.
 */
static void sum_update(uint32_t * const checksum, const void * const address, const int size)
{
  uint8_t * buffer = (uint8_t *) address;
  for (int i = 0; i < size; i++)
  {
    *checksum += buffer[i];
  }
}


/**
 * @brief   Download a firmware image from an HTTP server into the alternate Flash bank.
 * @note    The current program, as well as the update, must fit in a single bank of the embedded FLASH:
 *          [0x08000000 - 0x08080000] on STM32L745.
 * @note    The HTTP server must support the "Range:" request header. This is the case with HTTP/1.1.
 * @param   In: url    Location of the new firmware (HTTP url: "http://<hostname>:<port>/<path>")
 * @retval  Error code
 *             RFU_OK (0) Success.
 *             <0         Failure.
 *                          RFU_ERR_HTTP  Error downloading over HTTP.
 *                          RFU_ERR_FF    Error decoding the simple code file format.
 *                          RFU_ERR_FLASH Error erasing or programmig the Flash memory.
 */
int rfu_update(const char * const url)
{
  int rc = RFU_OK;
  int ret = 0;
  
  uint32_t sum = 0;
  
  http_sock_handle_t sockHnd;
  uint8_t readbuffer[RFU_READBUF_SIZE];
  memset(readbuffer, 0, RFU_READBUF_SIZE);

#ifdef SKIP_WRITING
  msg_debug("\n==========WARNING: SKIP_WRITING was defined, this is a RFU dry run."
         "\n==========The new version is downloaded and checked, but it is not written to the flash memory.\n\n");
#endif
  
  ret = http_sock_open(&sockHnd, url);
  if (RFU_OK != ret) {
    msg_error("Could not open %s\n", url);
    rc = RFU_ERR_HTTP;
  }
  else
  { 
    /* Read the file header and the first record header */
    if ( sizeof(sff_fileheader_t) != http_sock_rcv(readbuffer, sizeof(sff_fileheader_t), sockHnd) )  
    {
      rc = RFU_ERR_HTTP;
      msg_error("Could not read %d bytes from HTTP\n", RFU_READBUF_SIZE);
    }
    else
    { /* Has read as many bytes as sent by the server. */
      sff_fileheader_t *file_header = (sff_fileheader_t *) readbuffer;
#ifdef FF_TRACE
      print_fileheader(file_header);
#endif /* FF_TRACE */
      sum_update(&sum, file_header, sizeof(sff_fileheader_t));

      uint32_t word = getWordFromArray(file_header->magic_number);
      if (word != 0x7f494152)
      {
        msg_error("Wrong file format: Does not start with the simple-code magic number 0x7f494152\n");
        print_fileheader(file_header);
        rc = RFU_ERR_FF;
      }
    }
    
    if (RFU_OK == rc)
    { /* This is an IAR simple-code binary file. */
      bool b_EOF = false; /* Denotes when the HTTP server reaches the end of the file.*/
      do {  
        /* Walk through the record headers.
         * Read the record type, then the record header, and - if applicable - the record data.
         * Write the record data to the FLASH.
         * Keep the checksum updated.
         * Note: The record header contents are stored in network byte order (big endian).
         * Leave the loop upon error, or once the end record and end of file is reached.
         */
        if ( 1 != http_sock_rcv(readbuffer, 1, sockHnd) )
        {
          msg_error("Could not read the record type\n");
          rc = RFU_ERR_HTTP;
          break;
        }
        
        switch ( readbuffer[0] )
        {
          case SFF_DATA_RECORD:
            {
              if ( (sizeof(sff_datarecord_t) - 1) != http_sock_rcv(readbuffer + 1, sizeof(sff_datarecord_t) - 1, sockHnd) ) 
              {
                msg_error("Could not read a data record header\n");
                rc = RFU_ERR_HTTP;
                break;
              }
              sff_datarecord_t *record = (sff_datarecord_t *) readbuffer;
              
              sum_update(&sum, record, sizeof(sff_datarecord_t));
#ifdef FF_TRACE              
              print_datarecord(record);
#else
              (void)print_datarecord; /* avoid warning */
#endif /* FF_TRACE */           

              uint32_t dst_size = getWordFromArray(record->number_of_program_bytes);
              uint32_t dst_address = getWordFromArray(record->record_start_address);
              
              /* Load and update the Flash pages from: record_start_address to: record_start_address + number_of_program_bytes.
               * Loop on the page count, with an HTTP ranged GET each.
               * Keep the checksum updated. */
              
              /* Allocate a page buffer as the output of the HTTP download.
               * It must be aligned on 64b boundaries to match the Flash programming alignment constraints. */
              uint64_t page_cache[FLASH_PAGE_SIZE/sizeof(uint64_t)];  

              /* Align the Flash programming on page boundaries to prevent an additional erase-write cycle. */
              /* After the boot bank switch, at reset time, the 0x00000000 alias will point to the alternate bank. So we write there. */
              int ret = 0;
              int remaining = dst_size;
   
              do {
                uint32_t user_addr = ALT_BANK(dst_address);                 /* Program address in the destination bank. */
                uint32_t fl_addr = ROUND_DOWN(user_addr, FLASH_PAGE_SIZE);  /* Start address of the page. */
                int fl_offset = user_addr - fl_addr;                        /* Offset to the start of the chunk to be updated in this page. */
                int len = MIN(FLASH_PAGE_SIZE - fl_offset, remaining);      /* Bytes to be updated in this page. */
#ifdef FL_TRACE
                msg_debug("fl_offset:%d, len:%d\n", fl_offset, len);
#endif /* FL_TRACE */
                
                /* Load from the flash into the cache */
                memcpy(page_cache, (void *) fl_addr, FLASH_PAGE_SIZE);  
                
                /* Update the cache from the source */
#ifdef DL_TRACE
                msg_debug("Going to read %d bytes, into 0x%08x.\n", len, (uint8_t *)fl_addr + fl_offset);
#endif /* DL_TRACE */
                int hret = http_sock_rcv((uint8_t *)page_cache + fl_offset, len, sockHnd);
                if ( len != hret )
                {
                  msg_error("Could not read as many program bytes as requested (%d != %d)\n", hret, len);
                  rc = RFU_ERR_HTTP;
                  break;
                }
                sum_update(&sum, (uint8_t *)page_cache + fl_offset, len);
                
                /* Erase the page, and write the cache */
#ifndef SKIP_WRITING
#ifdef FL_TRACE
                msg_debug("Erasing at 0x%08x\n", fl_addr);
#endif /* FL_TRACE */
                ret = FLASH_unlock_erase(fl_addr, FLASH_PAGE_SIZE);
                if (ret != 0)
                {
                  msg_error("Could not erase the Flash at: 0x%08lx\n", fl_addr);
                  rc = RFU_ERR_FLASH;
                }
                else
                {
                  ret = FLASH_write_at(fl_addr, page_cache, FLASH_PAGE_SIZE);
                }
#endif /* SKIP_WRITING */                  
                if (ret != 0)
                {
                  msg_error("Error when programming the Flash at: 0x%08lx\n", fl_addr);
                  rc = RFU_ERR_FLASH;
                }
                else
                {
                  remaining -= len;
                  dst_address += len;
                }
              } while ((ret == 0) && (remaining > 0));
              break;
            }
          case SFF_ENTRY_RECORD:
            { 
              if ( (sizeof(sff_entryrecord_t) - 1) != http_sock_rcv(readbuffer + 1, sizeof(sff_entryrecord_t) - 1, sockHnd) ) 
              {
                rc = RFU_ERR_HTTP;
                break;
              }
              sff_entryrecord_t *record = (sff_entryrecord_t *) readbuffer;
              sum_update(&sum, record, sizeof(sff_entryrecord_t));
#ifdef FF_TRACE
              print_entryrecord(record);
#else
              (void)print_entryrecord; /* avoid warning */
#endif /* FF_TRACE */
              /* Assuming that there is nothing to do with the entry record. The boot address is fixed to 0x08000000. */
              break;
            }
          case SFF_END_RECORD:
            {
              if ( (sizeof(sff_endrecord_t) - 1) != http_sock_rcv(readbuffer + 1, sizeof(sff_endrecord_t) - 1, sockHnd) )
              {
                rc = RFU_ERR_HTTP;
                break;
              }
              sff_endrecord_t *record = (sff_endrecord_t *) readbuffer;
              sum_update(&sum, record, &record->checksum[0] - &record->record_tag);  /* Exclude the checksum field from the computation. */
#ifdef FF_TRACE
              print_endrecord(record);
#else
              (void)print_endrecord; /* avoid warning */
#endif /* FF_TRACE */
              uint32_t word = getWordFromArray(record->checksum);
              
              if ((sum + word) != 0)
              {
                msg_error("File checksum test failed (%lu)\n", sum + word);
                rc = RFU_ERR_FF;
              }
              b_EOF = true;
              break;
            }
          case SFF_INVALID_RECORD:
          default:
            msg_error("File format parsing error: unknown record tag %x\n", readbuffer[0]);
            rc = RFU_ERR_FF;
        }
      } while ( (rc == RFU_OK) && (! b_EOF ) );
    }
    
    if (rc == RFU_OK)
    {
      const firmware_version_t  * fw_version, * alt_fw_version;
      rfu_getVersion(&fw_version, false);
      rfu_getVersion(&alt_fw_version, true);
#ifdef FL_TRACE
      msg_info("Cur FW Version: %u.%u.%u %s %s\n", fw_version->major, fw_version->minor, fw_version->patch, fw_version->build_date, fw_version->build_time);
      msg_info("Alt FW Version: %u.%u.%u %s %s\n", alt_fw_version->major, alt_fw_version->minor, fw_version->patch, alt_fw_version->build_date, alt_fw_version->build_time);
#endif /* FL_TRACE */

       
      /* Copy the static user settings to the alternate bank.
       * The user settings location and contents format MUST be the same in the current FW and in the new FW. */
#ifndef SKIP_WRITING
      FLASH_update(ALT_BANK((uint32_t)&lUserConfig), &lUserConfig, sizeof(user_config_t)); 
#endif /* SKIP_WRITING */
    }
  }

  HAL_FLASH_Lock();
  
  http_sock_close(sockHnd);
  return rc;
}


/**
 * @brief   Get a short from a 2 byte network-endian array.
 * @param   In: Byte array.
 * @return  half word
 */
static uint16_t getShortFromArray(const uint8_t * const data)
{
  uint16_t half = 0;
  if (data != NULL)
  {
    half = data[0] << 8 | data[1];
  }
  return half;
}

/**
 * @brief   Get a word from a 4 byte network-endian array.
 * @param   In: Byte array.
 * @return	word
 */
static uint32_t getWordFromArray(const uint8_t * const data)
{
  uint32_t word = 0;
  if (data != NULL)
  {
    word = data[0] << 24 | data[1] << 16 | data[2] << 8 | data[3];
  }
  return word;
}

/**
 * @brief   Print a simple file format file header.
 * @param   In: pFileHeader   File header.
 */
static void print_fileheader(const sff_fileheader_t * pFileHeader) 
{
  if (pFileHeader != NULL)
  {
    uint32_t word = getWordFromArray(pFileHeader->magic_number);
    printf("magic_number: 0x%08lx\n", word);

    word = getWordFromArray(pFileHeader->program_flags);
    printf("program_flags: 0x%08lx\n", word);

    word = getWordFromArray(pFileHeader->number_of_program_bytes);
    printf("number_of_program_bytes: %lu\n", word);

    uint16_t half = getShortFromArray(pFileHeader->version_information);
    printf("version_information: 0x%04x\n", half);
  }
}

/**
 * @brief   Print a simple file format data record header.
 * @param   In: pRecord     Data record header.
 */
static void print_datarecord(const sff_datarecord_t * pRecord) 
{
  if (pRecord != NULL)
  {
    int record_match = 0;
    printf("\nrecord_tag: 0x%x", pRecord->record_tag);
    switch (pRecord->record_tag)
    {
      case SFF_DATA_RECORD:
        printf(" - DATA_RECORD\n");
        record_match = 1;
        break;
      case SFF_ENTRY_RECORD:
        printf(" - ENTRY_RECORD\n");
        break;
      case SFF_END_RECORD:
        printf(" - END_RECORD\n");
        break;
      case SFF_INVALID_RECORD:
      default:
        printf(" - INVALID_RECORD\n");
    }
    if (record_match == 1)
    {
      printf("segment_type: %d\n", pRecord->segment_type);

      uint16_t half = getShortFromArray(pRecord->record_flags);
      printf("record_flags: 0x%04x\n", half);

      uint32_t word = getWordFromArray(pRecord->record_start_address);
      printf("record_start_address: 0x%08lx\n", word);

      word = getWordFromArray(pRecord->number_of_program_bytes);
      printf("number_of_program_bytes: %lu 0x%lx\n", word, word);
    } else
    {
      msg_error("Mismatching record type.\n"); 
    }
  }
}

/**
 * @brief   Print a simple file format entry record header.
 * @param   In: pRecord     Entry record header.
 */
static void print_entryrecord(const sff_entryrecord_t * pRecord) 
{
  if (pRecord != NULL)
  {
    int record_match = 0;
    printf("\nrecord_tag: 0x%x", pRecord->record_tag);
    switch (pRecord->record_tag)
    {
      case SFF_DATA_RECORD:
        printf(" - DATA_RECORD\n");
        break;
      case SFF_ENTRY_RECORD:
        printf(" - ENTRY_RECORD\n");
        record_match = 1;
        break;
      case SFF_END_RECORD:
        printf(" - END_RECORD\n");
        break;
      case SFF_INVALID_RECORD:
      default:
        printf(" - INVALID_RECORD\n");
    }

    if (record_match == 1)
    {
      uint32_t word = getWordFromArray(pRecord->entry_address);
      printf("entry_address: 0x%08lx\n", word);

      printf("segment_type: %d\n", pRecord->segment_type);
    } else
    {
      msg_error("Mismatching record type.\n"); 
    }
  }
}

/**
 * @brief   Print a simple file format end record header.
 * @param   In: pRecord     End record header.
 */
static void print_endrecord(const sff_endrecord_t * pRecord) 
{
  if (pRecord != NULL)
  {
    int record_match = 0;
    printf("\nrecord_tag: 0x%x", pRecord->record_tag);
    switch (pRecord->record_tag)
    {
      case SFF_DATA_RECORD:
        printf(" - DATA_RECORD\n");
        break;
      case SFF_ENTRY_RECORD:
        printf(" - ENTRY_RECORD\n");
        break;
      case SFF_END_RECORD:
        printf(" - END_RECORD\n");
        record_match = 1;
        break;
      case SFF_INVALID_RECORD:
      default:
        printf(" - INVALID_RECORD\n");
    }
    if (record_match == 1)
    {
      uint32_t word = getWordFromArray(pRecord->checksum);
      printf("checksum: 0x%08lx\n", word);
    } else
    {
      msg_error("Mismatching record type.\n"); 
    }
  }
}
#endif /* RFU */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
