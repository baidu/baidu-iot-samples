/**
  @page Baidu IoT Cloud application

  @verbatim
  ******************************************************************************
  * @file    readme.txt 
  * @author  MCD Application Team
  * @version V0.0.1
  * @date    22-Nov-2017
  * @brief   Description of the Baidu Cloud application.
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

The Baidu sample application illustrates the various ways for a device to interact
with an Baidu IoT hub, using the Baidu IoT edge SDK for C.

  Notes:
    - By contrast to the generic interface, the low level interface lets the application explicitly schedule the network traffic.
      This allows the user to time-slice the network interface activity, and relieves from running
      a dedicated working thread for handling the network events. It can ease with a low-power SW design.
    - The serializer helps reliably building JSON strings to be sent to the IoT hub, from the device model
      attributes. 

It is ported to the B-L475E-IOT01 and connects to the Internet through the on-board network interface.

MbedTLS secures the network link on STM32 side.

The application connects to a Baidu IoT hub thanks to the credentials provided
by the user on the code.

@par Hardware and Software environment

  - MCU board: B-L475E-IOT01 (MB1297 rev D), with FW "Inventek eS-WiFi ISM43362-M3G-L44-SPI C3.5.2.3.BETA9"
      Note: the FW version is displayed on the board console at boot time.
  
  - WiFi access point.
      * with a transparent Internet connectivity: no proxy, no firewall blocking the outgoing traffic.
      * running a DHCP server delivering the IP and DNS configuration to the board.

  - Computer for running a device management application.
      * with a transparent Internet connectivity: no proxy, no firewall blocking the outgoing traffic.
      * This can be your development PC, a virtual private server, a single-board computer...
      * It may be connected to the same router as the MCU board.

  - Baidu IoT account 
      * and create an IoT Hub.
        See https://cloud.baidu.com/product/iot.html

  - A development PC for building the application, programming through ST-Link, and running the virtual console.
  
  - Support IAR 8.2 or later
  - Support Keil 5.24 or later
  - Support SW4STM32 4.6.3 or later

@par How to use it ? 

In order to make the program work, you must follow these steps:

Baidu device creation
  - Create a Baidu IoT hub instance.
  - Create a device under your instance
  - get the instance URL, device user name and password.

Application build and flash
  - Open the project with one of the supported development tool chains (see the release note
    for detailed information about the version requirements), enter the Baidu IoT hub instance address(ENDPOINT), 
	device user name and password into the source file under iothub_mqtt_client_sample.c.
	
  - Build the project.

  - Program the firmware on the STM32 board: you can copy (or drag and drop) the generated bin
    file to the USB mass storage location created when you plug the STM32 
    board to your PC. If the host is a Linux PC, the STM32 device can be found in 
    the /media folder with the name "DIS_L4IOT". For example, if the created mass 
    storage location is "/media/DIS_L4IOT", then the command to program the board 
    with a binary file named "B-L475E-IOT01.bin" is simply: cp B-L475E-IOT01.bin 
    /media/DIS_L4IOT. 

Note: SW4STM32 provided cross platform support besides Windows, if you are using SW4STM32 for Mac to build the project
      and encounter the make error. it's due to the environment variables on SW4STM32 changed the build variables for Mac.
	  to solve it, you can right click the project, open the Properties, under Resource->Linked Resources, convert the
      absolute location to variable relative location, for example, PROJECT_LOC/../startup_stm32l475xx.c, then save.
	  right click the project, clean the project then build the project, it should solve the issue and work properly.	  
 

Application first launch
  - The board must be connected to a PC through USB (ST-LINK USB port).
    Open the console through serial terminal emulator (e.g. TeraTerm), select the ST-LINK COM port of your
    board and configure it with:
    - 8N1, 115200 bauds, no HW flow control;
    - line endings set to LF.

  - On the console:
    - On WiFi-enabled boards: Enter your Wifi SSID, encryption mode and password.
  
     - After the parameters are configured, it is possible to change them by restarting the board 
     and pushing the User button (blue button) just after boot.


Application runtime             
  - Connects to the Baidu IoT hub,
      - Publish the on-board sensor data to IoT hub;
      - Subscribe the published sensor data from IoT hub.

@par Directory contents

---> in .
Inc
  es_wifi_conf.h                              ES Wifi configuration file.
  es_wifi_io.h                                ES Wifi IO header file.  
  main.h                                      Application-specific global symbols declaration. 
  stm32l4xx_hal_conf.h                        HAL configuration file
  stm32l4xx_it.h                              STM32 interrupt handlers declaration.
  tcpsocketSTM32Cube.h						  TCP socket header file
  vl53l0x_platform.h                          Proximity sensor platform port definitions.
  vl53l0x_proximity.h                         Proximity sensor API declaration.
  wifi.h									  WiFi header file
Src
  es_wifi_io.c								  WiFi IO implementation
  iothub_mqtt_client_wifi_sample.c			  IoT Hub MQTT wiFi sample
  main.c                                      Application entry point.
  stm32l4xx_hal_msp.c                         Application-specific HAL initialization.
  stm32l4xx_it.c                              STM32 interrupt handlers.
  tcpsocketSTM32Cube						  TCP socket WiFi implementation
  vl53l0x_platform.c                          Proximity sensor platform port.
  vl53l0x_proximity.c                         Proximity sensor API implementation.
  wifi.c									  WiFi implementation

---> in Projects/Common/Baidu
comodo_baltimore.pem                          List of root CA certificates to be pasted on the board console at first launch.

Inc
  az_mbedtls_config.h                         Application-specific mbedTLS middleware configuration.
  az_tlsio_mbedtls_STM32Cube.h                Azure SDK tlsIO adapter interface declaration. 
  baidu_version.h                             X-Cube package name and version definition.
  mqtt_client_sample.h						  mqtt sample header
Src
  az_platform_STM32Cube.c                     Routing of the Azure SDK required interface. NB: platform_init() and platform_deinit() are provided by cloud.c.
  az_threadapi_STM32Cube.c                    Implementation of the Azure SDK thread adapater interface. No multithread support is currently implemented.    
  az_tickcounter_STM32Cube.c                  Implementation of the Azure SDK tick counter adapter interface.
  az_tlsio_mbedtls_STM32Cube.c                Implementation of the Azure SDK tlsIO adapter interface.
  azure_base64.c                              Avatar of the Azure SDK base64.c file (filename conflict with mbedTLS).
  azure_sha1.c                                Avatar of the Azure SDK sha1.c file (filename conflict with mbedTLS).
  baidu_certs.c								  Baidu certificate
  socketio_STM32Cube.c						  Socket implementation
  
---> in Projects/Common/Shared (Cloud-common, to be reorganized)
Inc
  cloud.h
  flash.h
  heap.h
  http_util.h
  iot_flash_config.h
  logging.h
  mbedtls_net.h
  msg.h
  net.h
  net_internal.h
  rfu.h
  sensors_data.h
  timedate.h
  timingSystem.h
  version.h

Src
  cloud.c
  entropy_hardware_poll.c                     RNG entropy source for mbedTLS.
  flash_l4.c                                  Flash programing interface for L4.
  heap.c
  http_util.c                                 Helper functions for building HTTP requests, and downloading by chunks.
  iot_flash_config.c                          Dialog and storage management utils for the user-configured settings.
  lwip_net.c                                  LwIP on ethernet init/deinit functions for NET.
  mbedtls_net.c                               Network adpater for mbedTLS on NET.  
  net.c                                       NET 
  net_tcp_lwip.c                              NET TCP / LwIP on ethernet implementation.
  net_tcp_wifi.c                              NET TCP / WiFi-LL implementation.
  net_tls_mbedtls.c                           NET TLS / mbedTLS implementation.
  rfu.c                                       Firmware versioning and change management functions.
  sensors_data.c                              B-L475E-IOT01 sensors init.
  STM32CubeRTCInterface.c                     Standard time implementation on the RTC.
  timedate.c                                  RTC init from HTTP or HTTPS requests.
  timingSystem.c                              Standard time implementation on the RTC.
  wifi_net.c                                  WiFi_LL init/deinit functions for NET (B-L475E-IOT01).


@par Caveats

  - The mbedTLS configuration parameter MBEDTLS_SSL_MAX_CONTENT_LEN is tailored down to 1024*6 bytes.
    It is sufficient for connecting to the Baidu IoT cloud, and to the HTTPS server used for retrieving
    the time and date at boot time.
    But the TLS standard may require up to 16kbytes, depending on the server configuration.
    For instance, if the server certificate is 7kbytes large, it will not fit in the device 5kbytes buffer,
    the TLS handshake will fail, and the TLS connection will not be possible.

  - Derived based on X-Cube-Azure package version 1.0.0.

 * <h3><center>&copy; COPYRIGHT STMicroelectronics</center></h3>
 */
