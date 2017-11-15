### 教程概述
*** 
- ST和百度云天工基于ST新近推出的一款物联网开发套件B-L475E-IOT01A，提供了连接百度云天工的软件扩展包，内含连接到百度云天工物接入（IoT Hub），进行消息订阅和发布的应用程序。
- 用户基于本教程，使用ST的B-L475E-IOT01A可以快速上手搭建自己的百度云物联网应用程序。

### 硬件概述
*** 
- B-L475E-IOT01A是ST新近推出一款物联网套件，支持WIFI，蓝牙，SubG, NFC等多种无线连接方式。 而且还支持Arduino接口，所以使用者也可以很方便的通过Arduino接口来扩展其他的无线连接模块，比如GSM模块。
- 这块开发板使用的是ST低功耗MCU系列中的STM32L475这款芯片，拥有1Mbytes的Flash和128Kbytes的SRAM。板上还集成了64Mbits的Quad-SPI Flash。而且板上搭载了多种传感器，比如温湿度传感器，高性能3轴磁力计，加速度传感器，陀螺仪，接近传感器和压力传感器等。便于开发者用来进行多种应用的演示。
- 板子上还有两个数字全角度麦克风，所以用这块开发板，还可以实现语音控制的功能。
- 配合这个套件，ST和百度共同推出了连接百度天工的软件扩展包。提供了连接到百度IoT hub, 进行消息定阅和发布的应用例程，客户可以基于这个例程快速的上手搭建自己的应用程序。
- 从[ST的官网](http://www.st.com/content/st_com/en/products/evaluation-tools/product-evaluation-tools/mcu-eval-tools/stm32-mcu-eval-tools/stm32-mcu-discovery-kits/b-l475e-iot01a.html)可以下载到关于该开发套件的详细介绍文档和硬件设计资料。
### 硬件环境
*** 
- B-L475E-IOT01A板子上已经集成了WIFI模块和ST-LINK，所以不需要额外的模块和调试工具，只需要一根micro的USB线给板子提供电源就可以开始使用了。当然，如果需要更新程序的话，还需要一台带USB接口的电脑。
    - 一块B-L475E-IOT01A开发板（集成了WIFI模块和ST-LINK）
    - 一根micro接口的 USB线（给板子供电，提供调试和程序下载接口）
    - 一个WIFI热点（WIFI路由器或者手机热点）
    - 一台电脑（编译和烧录程序，配置WIFI密码）

### 软件环境
***
![](https://github.com/baidu/iot-edge-sdk-samples/blob/master/STM32/B-L475E-IOT01/_htmresc/1.png)

