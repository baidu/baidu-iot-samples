/**
  @page Baidu Cloud application

  @verbatim
  ******************************************************************************
  * @file    readme.txt 
  * @author  MCD Application Team
  * @version V0.0.1
  * @date    30-Nov-2017
  * @brief   Description of Baidu Cloud application.
  ******************************************************************************
  *
  * Copyright (c) 2017 STMicroelectronics International N.V. All rights reserved.
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
  @endverbatim

@par Application Description 


The sample application implements an Baidu Cloud IoT client for STM32L496G-Discovery
board using external UG96 Quectel modem connected via STmod+ connector, and mbedTLS 
for secure network communication.

The application connects to Baidu IoT Cloud with the credentials provided
by the user.


@par Hardware and Software environment

  - This example runs on STM32L496G-Discovery board equipped with a Quectel UG96
    daughterboard connected on its IO expander: Modem package upside, SIM card slot downside.

  - An active cellular SIM card must be fit in the slot.

    In order to activate the EMnify SIM card:
    * Follow the EMnify Setup Guide available in the Knowledge Base section of https://support.emnify.com

      Note for corporate users: Your card may have already been registered by the person who received the batch.
      You can skip the account creation, and:
      * get your email address added to the list of users of the EMnify corporate account;
      * click on the activation link you will receive by email;
      * login and activate your card from its IccID;
      * create your endpoint.
      See the details in "Enabling SIMs" section and below of the EMnify Setup Guide.

    If unsure that the IccID printed on your card is valid, you can cross-check with the "SIM Id (IccID):" value 
    displayed on the console during the application start up.
    Alternatively, you can use the AT+QCCID command in the QNavigator on the Brisk card.
  

  - A Cellular access point is required.

  - Baidu IoT account 
      * An Baidu account is required to use Baidu IoT hub application

  - A Baidu IoT hub application
      * This provides a way to interact with the cloud

  - A development PC for building the application, programming through ST-Link, and running the virtual console.
  
  - Support IAR 8.2 or later
  - Support Keil 5.24 or later
  - Support SW4STM32 4.6.3 or later


@par How to use it ? 

In order to make the program work, you must follow these steps:

- Device creation

- Application build and flash
  
  - WARNING: before opening the project with any toolchain be sure your folder
    installation path is not too in-depth since the toolchain may report errors
    after building.

  - Open, fill the device credentials when you do device creation in Baidu IoT hub and build the project with one of the supported development toolchains (see the release note
    for detailed information about the version requirements).

  - Program the firmware on the STM32 board: you can copy (or drag and drop) the 
    binary file under Projects\STM32L496G-Discovery\Applications\Cloud\Baidu\EWARM\STM32L496G-Discovery\Exe\
    to the USB mass storage location created when you plug the STM32 
    board to your PC. If the host is a Linux PC, the STM32 device can be found in 
    the /media folder with the name "DIS_L4IOT". For example, if the created mass 
    storage location is "/media/DIS_L4IOT", then the command to program the board 
    with a binary file named "Project.bin" is simply: cp Project.bin 
    /media/DIS_L4IOT. 

  Alternatively, you can program the STM32 board directly through one of the supported development toolchains.
 

Application first launch   
  - The board must be connected to a PC through USB (ST-LINK USB port).
    Open the console through serial terminal emulator (e.g. TeraTerm), select the ST-LINK COM port of your
    board and configure it with:
    - 8N1, 115200 bauds, no HW flow control;
    - line endings set to LF.

  - On the console:
     - C2C network configuration (SIM operator access point code, username and password).
       Example: 
         with Emnify SIM:  access point: "EM",     username: "", password: ""
         with Baidu SIM:   access point: "lnsyszhx01.schj.njm2mapn", username: "", password: ""
  
  - After the parameters are configured, it is possible to change them by restarting the board 
    and pushing the User button (blue button) just after boot.


Application runtime 
  - Device to cloud connection: the device connects to the Baidu IoT hub
      The application publish a test message text

@par Directory contents

---> in .
Inc

  c2c.h							     C2C header
  main.h                             Header containing config parameters for the application
  stm32l4xx_hal_conf.h               HAL configuration file
  stm32l4xx_it.h                     STM32 interrupt handlers header file
  tcpsocketSTM32Cube.h				 TCP socket for C2C header file
  ug96_conf.h						 Cellular modem config header file
  ug96_io.h							 Cellular modem IO header file	

Src
  c2c.c                              C2C interface file
  iothub_mqtt_client_sample.c		 C2C application sample
  main.c                             Main application file
  stm32l4xx_hal_msp.c                specific initializations
  stm32l4xx_it.c                     STM32 interrupt handlers
  tcpsocketSTM32Cube.c				 TCP socket for C2C implementation
  ug96_io.c							 Cellular modem IO file

---> in Projects/Common/Baidu
Inc
  az_mbedtls_config.h
  az_tlsio_mbedtls_STM32Cube.h
  baidu_version.h
  mqtt_client_sample.h
Src
  az_platform_STM32Cube.c
  az_threadapi_STM32Cube.c
  az_tickcounter_STM32Cube.c
  az_tlsio_mbedtls_STM32Cube.c
  azure_base64.c
  azure_sha1.c
  baidu_certs.c
  socketio_STM32Cube.c
  
---> in Projects/Common/Shared
Inc
  cloud.h
  flash.h
  heap.h
  http_util.h
  iot_flash_config.h
  mbedtls_config.h
  mbedtls_net.h
  msg.h
  net.h
  net_internal.h
  rfu.h
  simpleformat.h
  test_macro.h
  timedate.h
  timer.h
  timingSystem.h
  version.h

Src
  cloud.c                                     Cloud device aplication
  entropy_hardware_poll.c                     RNG entropy source for mbedTLS.
  flash_l4.c                         		  Flash programing interface for L4  
  heap.c                                      Heap check functions
  http_util.c                                 Helper functions for building HTTP requests, and downloading by chunks.
  iot_flash_config.c                          Dialog and storage management utils for the user-configured settings.
  mbedtls_net.c                               Network adpater for mbedTLS on NET.  
  net.c                                       NET 
  net_tcp_c2c.c                               NET TCP / C2C-LL implementation (STM32L496G-DISCO).
  net_tls_mbedtls.c                           NET TLS / mbedTLS implementation.
  rfu.c                                       Firmware versioning and change management functions.
  STM32CubeRTCInterface.c                     Standard time implementation on the RTC.
  timedate.c                                  RTC init from HTTP or HTTPS requests.
  timer.c                                     Timer interface implementation
  c2c_net.c                                   C2C_LL init/deinit functions for NET (STM32L496G-DISCO).


@par Caveats

  - Baidu SIM Card is node-locked, i.e. the Baidu APN will lock the SIM card to the node device(here ST STM32L496G-DISCO board)used it first time. 
    Switch the SIM card to other node devices without permission from Baidu will be failed to retrieve APN and IP address during initialization stage. 

  - The mbedTLS configuration parameter MBEDTLS_SSL_MAX_CONTENT_LEN is tailored down to 8 kbytes.
    It is sufficient for connecting to the Baidu IoT cloud, and to the HTTPS server used for retrieveing
    the time and date at boot time.
    But the TLS standard may require up to 16kbytes, depending on the server configuration.
    For instance, if the server certificate is 10 kbytes large, it will not fit in the device buffer,
    the TLS handshake will fail, and the TLS connection will not be possible.

 * <h3><center>&copy; COPYRIGHT STMicroelectronics</center></h3>
 */