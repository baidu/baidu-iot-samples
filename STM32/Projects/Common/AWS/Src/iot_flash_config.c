/**
  ******************************************************************************
  * @file    iot_flash_config.c
  * @author  MCD Application Team
  * @version V1.0.0
  * @date    11-July-2017
  * @brief   configuration in flash memory.
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
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include "main.h"
#include "rfu.h"
#ifdef USE_FIREWALL
#include "firewall_wrapper.h"
#endif
#include "flash.h"
#include "iot_flash_config.h"
#include "msg.h"

/* Private typedef -----------------------------------------------------------*/
/* Private defines -----------------------------------------------------------*/
#define PEM_READ_LINE_SIZE    120
#define PEM_READ_BUFFER_SIZE  8192  /**< Max size which can be got from the terminal in a single getInputString(). */

/* Private macros ------------------------------------------------------------*/
#ifdef ENABLE_TRACE_FLASH
#define TRACE_FLASH msg_info
#else
#define TRACE_FLASH(...)
#endif

/* Private variables ---------------------------------------------------------*/
/** Do not zero-initialize the static user configuration. 
 *  Otherwise, it must be entered manually each time the device FW is updated by STLink. 
 */

#ifdef __ICCARM__  /* IAR */
__no_init const user_config_t lUserConfig @ "UNINIT_FIXED_LOC";
#elif defined ( __CC_ARM   )/* Keil / armcc */
user_config_t lUserConfig __attribute__((section("UNINIT_FIXED_LOC"), zero_init));
#elif defined ( __GNUC__ )      /*GNU Compiler */
user_config_t lUserConfig __attribute__((section("UNINIT_FIXED_LOC")));
#endif

/* Private function prototypes -----------------------------------------------*/
bool check(char *s);
int CaptureAndFlashPem(char *pem_name,char const *flash_addr);

/* Functions Definition ------------------------------------------------------*/

/**
  * @brief  Get a line from the console (user input).
  * @param  Out:  inputString   Pointer to buffer for input line.
  * @param  In:   len           Max length for line.
  * @retval Number of bytes read from the terminal.
  */
int getInputString(char *inputString, size_t len)
{
  size_t currLen = 0;
  int c = 0;
  
  c = getchar();
  
  while ((c != EOF) && ((currLen + 1) < len) && (c != '\r') && (c != '\n') )
  {
    if (c == '\b')
    {
      if (currLen != 0)
      {
        --currLen;
        inputString[currLen] = 0;
        msg_info(" \b");
      }
    }
    else
    {
      if (currLen < (len-1))
      {
        inputString[currLen] = c;
      }
      
      ++currLen;
    }
    c = getchar();
  }
  if (currLen != 0)
  { /* Close the string in the input buffer... only if a string was written to it. */
    inputString[currLen] = '\0';
  }
  if (c == '\r')
  {
    c = getchar(); /* assume there is '\n' after '\r'. Just discard it. */
  }
  
  return currLen;
}


/**
  * @brief  Check whether the Wifi parameters are present in FLASH memory.
  *         Returns the parameters if present.
  * @param  Out:  ssid              Wifi SSID.
  * @param  Out:  psk               Wifi security password.
  * @param  Out:  security_mode     See @ref wifi_network_security_t definition.
  * @retval   0 if the parameters are present in FLASH.
  *          -1 if the parameters are not present in FLASH.
  */
int checkWiFiCredentials(const char ** const ssid, const char ** const psk, uint8_t * const security_mode)
{
  bool is_ssid_present = 0;
  
  if (lUserConfig.wifi_config.magic == USER_CONF_MAGIC)
  {
    is_ssid_present = true;
    if ((ssid == NULL) ||(psk == NULL) || (security_mode == NULL))
    {
      return -2;
    }
    *ssid = lUserConfig.wifi_config.ssid;
    *psk = lUserConfig.wifi_config.psk;
    *security_mode = lUserConfig.wifi_config.security_mode;
  }
 
  return (is_ssid_present) ? 0 : -1;
}


/**
  * @brief  Write the Wifi parameters to the FLASH memory.
  * @param  In:   ssid            Wifi SSID.
  * @param  In:   psk             Wifi security password.
  * @param  In:   security_mode   See @ref wifi_network_security_t definition.
  * @retval	Error code
  * 			0	Success
  * 			<0	Unrecoverable error
  */
int updateWiFiCredentials(const char ** const ssid, const char ** const psk, uint8_t * const security_mode)
{
  wifi_config_t wifi_config;
  int ret = 0;

  if ((ssid == NULL) ||(psk == NULL) || (security_mode == NULL))
  {
    return -1;
  }
  
  memset(&wifi_config, 0, sizeof(wifi_config_t));
    
  msg_info("\nEnter SSID: ");
  
  getInputString(wifi_config.ssid, USER_CONF_WIFI_SSID_MAX_LENGTH);
  msg_info("You have entered %s as the ssid.\n", wifi_config.ssid);
  
  
  msg_info("\n");
  char c;
  do
  {
      msg_info("\rEnter Security Mode (0 - Open, 1 - WEP, 2 - WPA, 3 - WPA2): \b");
      c = getchar();
  }
  while ( (c < '0')  || (c > '3'));
  wifi_config.security_mode = c - '0';
  msg_info("\nYou have entered %d as the security mode.\n", wifi_config.security_mode);
  
  
  if (wifi_config.security_mode != 0)
  {
    msg_info("\nEnter password: "); 
    getInputString(wifi_config.psk, sizeof(wifi_config.psk));
  }
  
  wifi_config.magic = USER_CONF_MAGIC;


  ret = FLASH_update((uint32_t)&lUserConfig.wifi_config, &wifi_config, sizeof(wifi_config_t));  

  if (ret < 0)
  {
    msg_error("Failed updating the wifi configuration in FLASH.\n");
  }

  msg_info("\n");
  return ret;
}


/**
  * @brief  Get one PEM string (ASCII format of TLS certificates and keys) from the console (user input).
  * @param  Out: key_read_buffer    Destimatation buffer.
  * @param  In:  max_len            Maximum length to be written to the destination buffer.
  * @retval Number of characters read into the output buffer.
  */
int enterPemString(char * read_buffer, size_t max_len)
{
  int i = 0;
  int read_len = 0;
  bool eof = false;
  read_len = getInputString(&read_buffer[i], max_len);
  
  while ( (read_len >= 0) && (i < max_len) && !eof )
  {
    i += read_len;
    read_buffer[i++] = '\n';
    read_len = getInputString(&read_buffer[i], max_len);
    eof = (strncmp(&read_buffer[i], "-----END",8) == 0);
    if (eof)
    {
        i += read_len;
        read_buffer[i++] = '\n';
        read_len = getInputString(&read_buffer[i], max_len);
        if (read_len != 0) eof =false;
    }
  }
  
  if (i >= max_len)
  {
   msg_error("Certificate is too long , allocated size is %d\n",max_len);
   return 0;
  }
  read_buffer[++i] = '\0';
  return i;
}

  
/**
  * @brief  Ask user and write the TLS certificates and key to the FLASH memory.
  * @param  none 
  * @retval	Error code
  * 			0	Success
  * 			<0	Unrecoverable error
  */
  
 int   CaptureAndFlashPem(char *pem_name,char const *flash_addr)
 {  
  char * key_read_buffer = NULL;
  int		ret=0;
  key_read_buffer = malloc(PEM_READ_BUFFER_SIZE);
  if (key_read_buffer == NULL)
  {
	msg_error("Could not allocate %d bytes for the console readbuffer.\n", PEM_READ_BUFFER_SIZE);
	return -1;
  }

  memset(key_read_buffer, 0, PEM_READ_BUFFER_SIZE);
  msg_info("\nEnter your %s certificate: \n",pem_name);
  enterPemString(key_read_buffer, PEM_READ_BUFFER_SIZE);

  msg_info("read: --->\n%s\n<---\n", key_read_buffer);
  
  /* Write to FLASH. */
  TRACE_FLASH("writing to %lx\n", flash_addr);
  ret = FLASH_update((uint32_t)flash_addr, key_read_buffer, strlen(key_read_buffer) + 1);  /* Append the closing \0*/
  free(key_read_buffer);
  return ret;
 }
   

int updateTLSCredentials(void)
{
  int ret = 0;
  
  msg_info("\nUpdating TLS security credentials.\n");
  msg_info("\nEnter keys  as per the following format:\n");
  msg_info("-----BEGIN CERTIFICATE-----\n");
  msg_info("YMPGn8u67GB9t+aEMr5P+1gmIgNb1LTV+/Xjli5wwOQuvfwu7uJBVcA0Ln0kcmnL\n");
  msg_info("R7EUQIN9Z/SG9jGr8XmksrUuEvmEF/Bibyc+E1ixVA0hmnM3oTDPb5Lc9un8rNsu\n");
  msg_info("KNF+AksjoBXyOGVkCeoMbo4bF6BxyLObyavpw/LPh5aPgAIynplYb6LVAgMBAAGj\n");
  msg_info(".......\n");
  msg_info("-----END CERTIFICATE-----\n");
  msg_info("-----BEGIN CERTIFICATE-----\n");
  msg_info("YMPGn8u67GB9t+aEMr5P+1gmIgNb1LTV+/Xjli5wwOQuvfwu7uJBVcA0Ln0kcmnL\n");
  msg_info(".......\n");
  msg_info("-----END CERTIFICATE-----\n");
  msg_info("\\n.......\n");
  
  
  if (check("Do you want to update the root CA certificate(s) ? [y/n]\n")) 
  {
	  ret = CaptureAndFlashPem("root CA",lUserConfig.tls_root_ca_cert);
  }
  
 
  
 #if defined(BLUEMIX) || defined(AWS) 

  if (check("Do you want to update the device certificate? [y/n]\n"))
  {
	  ret |= CaptureAndFlashPem("device certificate",lUserConfig.tls_device_cert);
  }
  
  if (check("Do you want to update the device key ? [y/n]\n"))
  {
	 ret |= CaptureAndFlashPem("device key",lUserConfig.tls_device_key);
  }
#endif
	 
  if (ret == 0)
  {
    uint64_t magic = USER_CONF_MAGIC;
    ret = FLASH_update((uint32_t)&lUserConfig.tls_magic, &magic, sizeof(uint64_t));
  }
  
  if (ret < 0)
  {
    msg_error("Failed updating the TLS configuration in FLASH.\n");
  }
    return ret;
}


/**
  * @brief  Check if the TLS credentials are present in FLASH memory.
  * @param  void
  * @retval 0 TLS credentials are configured,
           -1 No TLS credentials found.
  */
int checkTLSCredentials()
{
  return (lUserConfig.tls_magic == USER_CONF_MAGIC) ? 0 : -1;
}


/**
  * @brief  Ask user and write in FLASH memory the MQTT broker address (AWS endpoint),
  *          and the device name.
  * @retval	Error code
  * 			0	Success
  * 			<0	Unrecoverable error
  */
int AWSServerDeviceConfig(void)
{
  iot_config_t iot_config;
  int ret = 0;

  memset(&iot_config, 0, sizeof(iot_config_t));
    
  msg_info("\nEnter server address: (example: xxx.iot.region.amazonaws.com) \n");
  getInputString(iot_config.server_name, USER_CONF_SERVER_NAME_LENGTH);
  msg_info("read: --->\n%s\n<---\n", iot_config.server_name);
  
  msg_info("\nEnter device name: (example: mything1) \n");
  getInputString(iot_config.device_name, USER_CONF_DEVICE_NAME_LENGTH);
  msg_info("read: --->\n%s\n<---\n", iot_config.device_name);

  iot_config.magic = USER_CONF_MAGIC;
   
  ret = FLASH_update((uint32_t)&lUserConfig.iot_config, &iot_config, sizeof(iot_config_t));
  
  if (ret < 0)
  {
    msg_error("Failed programming the IOT server / device config into Flash.\n");
  }
  return ret;
}


/**
  * @brief  Ask user and write in FLASH memory the MQTT broker address (Baidu endpoint),
  *          and username, password pair.
  * @retval	Error code
  * 			0	Success
  * 			<0	Unrecoverable error
  */
int BaiduServerDeviceConfig(void)
{
  iot_config_t iot_config;
  int ret = 0;

  memset(&iot_config, 0, sizeof(iot_config_t));

  msg_info("\nEnter server address: (example: xxxxx.mqtt.iot.yy.baidubce.com) \n");
  getInputString(iot_config.server_name, USER_CONF_SERVER_NAME_LENGTH);
  msg_info("read: --->\n%s\n<---\n", iot_config.server_name);

  msg_info("\nEnter user name: (example: xxxx/yyy) \n");
  getInputString(iot_config.user_name, USER_CONF_DEVICE_NAME_LENGTH);
  msg_info("read: --->\n%s\n<---\n", iot_config.user_name);

  msg_info("\nEnter password: (example: xxxxxxxxxxxxxx) \n");
  getInputString(iot_config.password, USER_CONF_DEVICE_NAME_LENGTH);
  msg_info("read: --->\n%s\n<---\n", iot_config.password);

  iot_config.magic = USER_CONF_MAGIC;

  ret = FLASH_update((uint32_t)&lUserConfig.iot_config, &iot_config, sizeof(iot_config_t));

  if (ret < 0)
  {
    msg_error("Failed programming the IOT server / device config into Flash.\n");
  }
  return ret;
}

/**
  * @brief  Ask user and write in FLASH memory the MQTT broker address (AWS endpoint),
  *          and the device name.
  * @retval	Error code
  * 			0	Success
  * 			<0	Unrecoverable error
  */
int AzureDeviceConfig(void)
{
    iot_config_t iot_config;
    int ret = 0;

    memset(&iot_config, 0, sizeof(iot_config_t));
      
    msg_info("\nEnter the Azure connection string of your device: (template: HostName=xxx;DeviceId=xxx;SharedAccessKey=xxxx=) \n");
    getInputString(iot_config.device_name, USER_CONF_DEVICE_NAME_LENGTH);
    msg_info("read: --->\n%s\n<---\n", iot_config.device_name);

    iot_config.magic = USER_CONF_MAGIC;
     
    ret = FLASH_update((uint32_t)&lUserConfig.iot_config, &iot_config, sizeof(iot_config_t));
    
    if (ret != 0)
    {
      msg_error("Error: Failed programming the IOT server / device config into FLASH.\n");
      return -1;
    }
   
  return 0;
}

/**
  * @brief  Get the MQTT broker address (AWS endpoint) from FLASH memory.
  * @param  Out:  address   Pointer to location of the server address.
  * @retval HAL_OK      The server address is configured and returned to the caller. 
  *         HAL_ERROR   No server address is configured.
  */
int getServerAddress(const char ** const address)
{
  int ret = HAL_ERROR;
  
  if (address != NULL)
  {
    if (lUserConfig.iot_config.magic == USER_CONF_MAGIC)
    {
      *address = lUserConfig.iot_config.server_name;
      ret = HAL_OK;
    } else {
      *address = NULL;
    }
  }
  return ret;
}


/**
  * @brief  Get the device name from FLASH memory.
  * @param  Out:  name    Pointer to location of the device name.
  * @retval   0   The device name is configured and returned to the caller. 
  *          -1   No device name is configured.
  
  */
int getDeviceName(const char ** const name)
{
  int ret = -1;

  if (name != NULL)
  {
    if (lUserConfig.iot_config.magic == USER_CONF_MAGIC)
    {
      *name = lUserConfig.iot_config.device_name;
      ret = 0;
    } else {
      *name = NULL;
    }
  }
  return ret;
}


/**
  * @brief  Check if the MQTT broker address and the device name are present in FLASH memory.
  * @retval 0:  The server name and the device name are configured.
  *        -1:  The server name and the device name are not configured.
  */
int checkServerDevice()
{
  return (lUserConfig.iot_config.magic == USER_CONF_MAGIC) ? 0 : -1;
}

/**
  * @brief  Get the TLS cerificates and private key addresses in the FLASH memory.
  * @param  Out: ca_cert        CA certificate / trust chain (PEM format: string)
  * @param  Out: device_cert    Device certificate (PEM format: string)
  * @param  Out: private_key    Device private key (PEM format: string)
  * @retval 0:    TLS credentials found, and passed back to the caller.
*          -1:    TLS credentials not found.
  */
int getTLSKeys(const char ** const root_ca_cert, const char ** const device_cert, const char ** const private_key)
{
  int rc = -1;
  if (lUserConfig.tls_magic == USER_CONF_MAGIC)
  {
    if (root_ca_cert != NULL)    *root_ca_cert = lUserConfig.tls_root_ca_cert;
    if (device_cert !=NULL)      *device_cert = lUserConfig.tls_device_cert;
    if (private_key!= NULL)      *private_key = lUserConfig.tls_device_key;
    rc = 0;
  }
  else
  {
    if (root_ca_cert != NULL)    *root_ca_cert = NULL;
    if (device_cert !=NULL)      *device_cert = NULL;
    if (private_key!= NULL)      *private_key = NULL;
  }
  return rc;
}

#ifdef RFU
/**
  * @brief  Firmware version management dialog.
  *         Allows: 
  *             - Selecting a different FW version for the next boot, if already programmed in the other FLASH bank.
  *             - Download a FW file from an HTTP URL and program it to the other FLASH bank.
  */
int updateFirmwareVersion()
{
  char console_yn = 'n';
  const firmware_version_t  * fw_version;
  uint32_t cur_bank = FLASH_get_bank(FLASH_BASE);
  uint32_t alt_bank = (cur_bank == FLASH_BANK_1) ? FLASH_BANK_2 : FLASH_BANK_1;
  
  rfu_getVersion(&fw_version, false);
  msg_info("\n*** Firmware version management ***\n");
  msg_info("\nPress the BLUE user button within the next 5 seconds\nto change the firmware version from %s %d.%d.%d %s %s, running from bank #%lu.\n",
         fw_version->name, fw_version->major, fw_version->minor, fw_version->patch, fw_version->build_date, fw_version->build_time, cur_bank);
  msg_info("\n");

  if (Button_WaitForPush(5000))
  {    
    msg_info("Current FW Version: %s %d.%d.%d %s %s, running from bank #%lu\n", fw_version->name, fw_version->major, fw_version->minor, fw_version->patch, fw_version->build_date, fw_version->build_time, cur_bank);

    /* Check whether an alternative firmware version is available in FLASH memory. */
    uint32_t *sp_candidate = (uint32_t *) (FLASH_BASE + FLASH_BANK_SIZE);
    if ((*sp_candidate < SRAM1_BASE) || (*sp_candidate > (SRAM1_BASE + SRAM1_SIZE_MAX)))
    { /* No alternative. */
      msg_info("Warning: The FLASH bank #%lu bank does not contain a valid boot image. Bank %lu will keep being used at next reset.\n", alt_bank, cur_bank);
    }
    else
    { /* An alternative exists. */
      firmware_version_t alt_fw_version = { "", 0, 0, 0, "", "" };
      const firmware_version_t *p_alt_fw_version;
      rfu_getVersion(&p_alt_fw_version, true);
      /* Guard the version strings against a read overflow in case the alt FLASH bank is inconsistent. */
      memcpy(&alt_fw_version, p_alt_fw_version, sizeof(firmware_version_t));
      alt_fw_version.build_date[FWVERSION_DATE_SIZE-1] = '\0';
      alt_fw_version.build_time[FWVERSION_TIME_SIZE-1] = '\0';
      
      msg_info("Altern. FW Version: %s %d.%d.%d %s %s, available in bank #%lu\n",
             alt_fw_version.name,
             alt_fw_version.major,
             alt_fw_version.minor,
             alt_fw_version.patch,
             alt_fw_version.build_date,
             alt_fw_version.build_time, alt_bank);
   
     do
     {
       msg_info("\rDo you want to switch to the alternative firmware version? (You will have to reset the board.) (y/n)  \b");
       console_yn = getchar();
     }
     while((console_yn != 'y') && (console_yn != 'n') && (console_yn != '\n'));
     if (console_yn != '\n') msg_info("\n");

     if (console_yn == 'y')
      {
        if (0 == FLASH_set_boot_bank(FLASH_BANK_BOTH))
        {
          msg_info("Befault boot bank switched successfully.\n");
          return 0;
        }
        else
        {
          msg_info("Error: failed changing the boot configuration\n");
          return -1;
        }
      }
    }
    
    do
    {
      msg_info("\rDo you want to download and program a new firmare version into FLASH bank #%lu? (y/n) \b", alt_bank);
      console_yn = getchar();
    }
    while((console_yn != 'y') && (console_yn != 'n') && (console_yn != '\n'));   
    if (console_yn != '\n') msg_info("\n");
   
    if (console_yn != 'y')
    {
      return 0;
    }

#define DEFAULT_FW_URL      "http://192.168.3.100:1080/B-L475E-IOT01_Cloud_AWS.sim"
#define MAX_FW_URL_LENGTH   100
    char fw_url[MAX_FW_URL_LENGTH];
    strncpy(fw_url, DEFAULT_FW_URL, sizeof(fw_url));

    msg_info("\nEnter the URL of the new firmware file:(By default: %s) :", fw_url);
    getInputString(fw_url, sizeof(fw_url));
    msg_info("read: --->\n%s\n<---\n", fw_url);
    
    msg_info("Downloading and programming the new firmware into the alternate FLASH bank.\n");
    
    int ret = rfu_update(fw_url);
    switch (ret)
    {
      case RFU_OK:
        msg_info("\nProgramming done. Now you can reset the board.\n\n");
        break;
      case RFU_ERR_HTTP:
        msg_info("\nError: Programming failed. Reason: HTTP error - check your network connection, "
               "and that the HTTP server supports HTTP/1.1 and the progressive download.\n\n");
        break;
      case RFU_ERR_FF:
        msg_info("\nError: Programming failed. Reason: Invalid firmware fileformat - check that the IAR simple-code format is used.\n\n");
        break;
      case RFU_ERR_FLASH:
        msg_info("\nError: Programming failed. Reason: FLASH memory erase/write - check that the firmware file matches the SoC FLASH memory mapping"
               "and write protection settings. Double check that there is no illegal write to the FLASH address range.\n\n");
        break;
      default:
        msg_info("\nError: Programming failed. Unknown reason.\n\n");
    }
  }
  
  return 0;
}
#endif /* RFU support */
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
