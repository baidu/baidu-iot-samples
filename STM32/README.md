**Release Notes for**

**X-CUBE-BAIDU**

 

Copyright � 2017 STMicroelectronics

Microcontrollers Division - Application Team

![](./_htmresc/st_logo.png)

 

**Overview**

  

•  The **X-CUBE-BAIDU** (STM32 Baidu IoT software expansion for
STM32Cube) provides ready-to-run firmware examples to support quick
evaluation and development of Baidu IoT Cloud applications on
B-L475E-IOT01 boards.

•  3 IDE are supported: IAR, Keil and System Workbench.

•  It contains BSP compatible Sensor drivers for B-L475E-IOT01.

•  It connects to Baidu IoT hubs through an MQTT on TLS connection.

 

**Update History**

  

**V1.0.0 / 11-July-2017**

 

**Main Changes**

º  First official release.

º  Note: 32F413H-DISCO and 32F769I-DISCO are not equipped with the
B-L475E-IOT01 set of sensors. The telemetry messages they send therefore
contain a timestamp, but no measurements.

 

**Content**

º  Drivers.

–  STM32L4xx HAL V1.7.0 ([release
notes](Drivers/STM32L4xx_HAL_Driver/Release_Notes.html)).

–  BSP B-L475E-IOT01 V1.1.0 ([release
notes](Drivers/BSP/B-L475E-IOT01/Release_Notes.html)).

–  BSP Common V5.0.0 ([release
notes](Drivers/BSP/Components/Common/Release_Notes.html)).

–  BSP Components V3.0.0:

–  HTS221 V1.0.0 ([release
notes](Drivers/BSP/Components/hts221/Release_Notes.html)).

–  LIS3MDL V1.0.0 ([release
notes](Drivers/BSP/Components/lis3mdl/Release_Notes.html)).

–  LPS22HB V1.0.0 ([release
notes](Drivers/BSP/Components/lps22hb/Release_Notes.html)).

–  LSM6DSL V1.0.0 ([release
notes](Drivers/BSP/Components/lsm6dsl/Release_Notes.html)).

º  Middleware.

–  Microsoft Baidu SDK for C 2017-06-02 ([release
notes](Middlewares/Third_Party/baidu-iot-c-sdk/readme.md)) ST modified
2017-07-07 ([release
notes](Middlewares/Third_Party/baidu-iot-c-sdk/st_readme.txt)).

–  FreeRTOS V9.0.0 ([release
notes](Middlewares/Third_Party/FreeRTOS/Source/readme.txt)) ST modified
2017-03-03 ([release
notes](Middlewares/Third_Party/FreeRTOS/Source/st_readme.txt)).

–  LwIP V2.0.0 ([release notes](Middlewares/Third_Party/LwIP/CHANGELOG))
ST modified 2016-12-23 + PACK\_STRUCT\_BEGIN definition fix for armcc
--gnu in system/arch/cc.h ([release
notes](Middlewares/Third_Party/LwIP/st_readme.txt)).

–  mbedTLS V2.4.0 ([release
notes](Middlewares/Third_Party/mbedTLS/ChangeLog)) ST modified
2016-12-23 ([release
notes](Middlewares/Third_Party/mbedTLS/st_readme.txt)).

º  Application.

–  Baidu IoT hub application.

 

**Development Toolchains and Compilers**

•  The package has been tested with:

º  IAR Embedded Workbench for ARM (EWARM). Version 7.80.4.

º  Keil Microcontroller Development Kit (MDK-ARM) Version 5.22.

º  System Workbench for STM32. Version 2.0.1.

 

**Supported Devices and EVAL boards**

•  B-L475E-IOT01 board (MB1297 rev D).

 

**Known Limitations**

•  Baidu SDK: Disabled some compilation warnings in the project files.

º  IAR.

–  Do not require C functions prototypes.

–  Pa084: pointless integer comparison, the result is always %s.

–  Pa089: enumerated type mixed with another enumerated type.

–  Pe186: pointless comparison of unsigned integer with zero.

–  Pe188: enumerated type mixed with another type.

–  Pe177: %n was declared but never referenced.

º  Keil.

–  --gnu.

–  --diag\_suppress=186,177,188,191.

º  SystemWorkbench.

–  -Wno-unused-variable -Wno-unused-function -Wno-maybe-uninitialized
-Wno-incompatible-pointer-types -Wno-enum-compare.

•  Application using the Azure SDK serializer: Disabled some compilation
warnings in the source.

º  IAR.

–  \#pragma diag\_suppress=Pe177.

º  Keil.

–  \#pragma diag\_suppress 177.

º  SystemWorkbench.

–  \#pragma GCC diagnostic ignored "-Wunused-variable".

–  \#pragma GCC diagnostic ignored "-Wunused-function".

•  mbedTLS compilation warnings.

º  2 false positive "variable is used before being set".

•  Network connection instability on B-L475E-IOT01 and STM32F413H-DISCO:

º  The board may not connect to the WiFi access point at first trial.

º  The resolution of the remote host name sometimes fails - it seems to
depend on the cache status of the DNS server of the local network.

º  Sporadic AT protocol errors on the serial link between the STM32 host
and the WiFi module (control/data unsync).

º  Remote server deconnection reported as a system error by the WiFi
module.

–  -\> In any of those cases, a board reset is required.

 

**License**

 

This software package is licensed under ST license SLA0048, (the
"License"). You may not use this package except in compliance with the
License. You may obtain a copy of the License at:

[http://www.st.com/SLA0048](http://www.st.com/SLA0048)

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

**    **

**---**

**    **

For complete documentation on **STM32** microcontrollers please visit
[http://www.st.com/stm32](http://www.st.com/stm32)

**   **

Generated by RNcreator v2.1
