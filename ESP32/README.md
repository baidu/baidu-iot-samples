# 前言
本文主要讲解如何适配baidu iot-edge-c-sdk 到esp32芯片，连接到baidu iot-hub上，实现mqtt数据收发。希望以esp32这个典型的例子，向读者展示适配的一般性原理和步骤，为读者之后适配该sdk到其他各种硬件平台提供便利。


# 硬件资源
- CPU：低功耗Xtensa® LX6 32-bit单/双核处理器, 7级流水线架构，支持高达240 MHz的时钟频率。
- ROM：448 KB，用于程序启动和内核功能调用。
- RAM：520 KB 片上SRAM，用于数据和指令存储。
- 外部Flash：最多支持4个16 MB的外部 QSPI Flash。用于存放程序固件，以及其他用户数据。


# esp32 编译环境搭建
## 安装 toolchain
参考[官方文档](https://docs.espressif.com/projects/esp-idf/zh_CN/latest/get-started/index.html#get-started-setup-toolchain)

```bash
    cd iot-edge-sdk-samples/ESP32
    sudo easy_install pip
    wget https://dl.espressif.com/dl/xtensa-esp32-elf-osx-1.22.0-80-g6c4433a-5.2.0.tar.gz # 直接下载官方提供的toolchain包，解压。这里以 MacOS 版本为例。
    mkdir esp32_toolchain
    tar zxvf xtensa-esp32-elf-osx-1.22.0-80-g6c4433a-5.2.0.tar.gz -C esp32_toolchain
```

## 下载 esp32 sdk
```bash
    git clone --recursive https://github.com/espressif/esp-idf.git esp32-idf
```

## 下载 baidu sdk
```bash
    git clone --recursive https://github.com/baidu/iot-edge-c-sdk.git iot-edge-c-sdk
```

## 设置环境变量
```bash
    export IDF_PATH=`pwd`/esp32-idf                 # IDF_PATH  : esp32 sdk 目录
    export PATH=`pwd`/esp32_toolchain/xtensa-esp32-elf/bin/:$PATH  # PATH : esp32 toolchain 路径
    export IOT_SDK=`pwd`/iot-edge-c-sdk             # IOT_SDK   : baidu iot-edge-c-sdk 目录
```

# 编译&测试
```bash
    cd sample # 这就是我们工程目录
```
该目录下主要包含了编译所需要的`Makefile`，以及两个文件夹：
- `components`  
首先，需要先了解一下esp32的编译框架`ESP-IDF`。`ESP-IDF`全称`Espressif IoT Development Framework`是乐鑫基于ESP32推出的新一代SDK，基于FreeRTOS系统。详细请参考[官方文档](https://esp-idf.readthedocs.io/zh_CN/latest/get-started/index.html)。在该目录中每个子目录都包含了一个库。程序在编译的时候，会进入每个子目录编译出每个库的静态.a文件，最后把所有的.a文件链接成最终的可执行文件。每个库目录中都有一个`components.mk`,包含一些头文件路径，需要编译的文件等信息。makefile会根据这个文件去编译该库。对于我们的`iot-edge-c-sdk`，主要的工作就是从中抽取适用于`ESP32`平台的代码文件。这里，我们提供好了适配完毕的`components.mk`，可以直接编译。
- `main`  
用户工程代码所在的目录，之后我们的测试demo程序就会放在这个文件夹下。

在`components.mk`中有一些文件是通用的，有一些是与esp32平台相关的，在这里我们称与平台相关的模块为 `adapter` (适配器)。关于适配器，将在下面章节详细介绍。现在，我们就可以直接编译了。

```bash
make menuconfig # 配置flash大小，连接的路由器ssid 密码等
make
```
编译成功！在`build`目录下生成烧写需要的bin文件，可以烧写测试了。

# 适配原理介绍
关于适配原理部分，建议读者先阅读`$IOT_SDK/PortingGuide.md`，该文档介绍了适配的一般性原理，而本文是该文档在`ESP32`这款平台上适配的一个具体例子，进一步阐述相关的概念。

## component.mk 结构
esp32是在所谓的`ESP IDF`框架下编译的，一个模块所有需要编译的文件都会被罗列在一个`component.mk`的文件中。如果读者用的是自己构建的Makefile工程或者是CMake工程，方法原理都是一样的。重点是如何从sdk中抽取各个`适配器`的代码。以下就对`component.mk`文件结构详细分析一下。
```Makefile
#
# Component Makefile
#


# 我们当前使用的是`openssl`库，如果使用`mbedtls`，相应地改为`CFLAGS += -DUSE_MBED_TLS`
CFLAGS += -DUSE_OPENSSL

# 指定 edge sdk 路径，pal 中实现了 openssl 的tlsio适配器
EDGE_SDK_PATH = ../../../iot-edge-c-sdk
ESP_PAL_PATH = ../../../pal

# 所需要的包含的头文件路径，也需要添加到你相应工程的`INCLUDE_PATH`中
COMPONENT_ADD_INCLUDEDIRS :=  \
$(ESP_PAL_PATH)/inc \
$(EDGE_SDK_PATH)/c-utility/inc  \
$(EDGE_SDK_PATH)/c-utility/inc/azure_c_shared_utility \
$(EDGE_SDK_PATH)/iothub_client/inc \
$(EDGE_SDK_PATH)/umqtt/inc  \
$(EDGE_SDK_PATH)/umqtt/inc/azure_umqtt_c 	\
$(EDGE_SDK_PATH)/iothub_client/samples/iothub_client_sample \
$(EDGE_SDK_PATH)/c-utility/pal/generic \
$(EDGE_SDK_PATH)/certs \
$(EDGE_SDK_PATH)/c-utility/pal/inc

# 所需要编译的文件源文件，有些是通用的上层逻辑代码，有些是平台相关的`适配器`
COMPONENT_OBJS =  \
\
# sdk用到的一些工具类模块，比如链表，log打印等等
# 通用代码，可以直接包含在你的代码中
$(EDGE_SDK_PATH)/c-utility/src/xlogging.o	\
$(EDGE_SDK_PATH)/c-utility/src/consolelogger.o	\
$(EDGE_SDK_PATH)/c-utility/src/buffer.o	\
$(EDGE_SDK_PATH)/c-utility/src/crt_abstractions.o	\
$(EDGE_SDK_PATH)/c-utility/src/singlylinkedlist.o	\
$(EDGE_SDK_PATH)/c-utility/src/doublylinkedlist.o	\
$(EDGE_SDK_PATH)/c-utility/src/utf8_checker.o	\
$(EDGE_SDK_PATH)/c-utility/src/map.o	\
$(EDGE_SDK_PATH)/c-utility/src/optionhandler.o	\
$(EDGE_SDK_PATH)/c-utility/src/strings.o	\
$(EDGE_SDK_PATH)/c-utility/src/vector.o	\
$(EDGE_SDK_PATH)/c-utility/src/xio.o	\
\
\
# mqtt client上层封装模块，通用代码，可以直接包含在你的代码中
$(EDGE_SDK_PATH)/iothub_client/src/iothub_client_persistence.o	\
$(EDGE_SDK_PATH)/iothub_client/src/iothub_mqtt_client.o	\
\
\# mqtt 库文件，通用代码，可以直接包含在你的代码中
$(EDGE_SDK_PATH)/umqtt/src/mqtt_client.o	\
$(EDGE_SDK_PATH)/umqtt/src/mqtt_codec.o	\
$(EDGE_SDK_PATH)/umqtt/src/mqtt_message.o	\
\
\
\# 这部分就是各种`适配器`了，用户需要根据自己的平台OS类型，所使用的SSL库，硬件平台的资源大小，选择不同的适配文件。这部分会在下面章节详细介绍。
$(ESP_PAL_PATH)/src/tlsio_openssl_compact.o	\
$(ESP_PAL_PATH)/src/platform_esp.o	\
$(ESP_PAL_PATH)/src/socketio_lwip.o \
$(EDGE_SDK_PATH)/c-utility/adapters/agenttime.o	\
$(EDGE_SDK_PATH)/umqtt/deps/c-utility/pal/freertos/threadapi.o	\
$(EDGE_SDK_PATH)/umqtt/deps/c-utility/pal/freertos/tickcounter.o	\
$(EDGE_SDK_PATH)/c-utility/pal/lwip/sntp_lwip.o	\
$(EDGE_SDK_PATH)/c-utility/pal/dns_async.o	\
$(EDGE_SDK_PATH)/c-utility/pal/socket_async.o	\
$(EDGE_SDK_PATH)/certs/certs.o \
$(EDGE_SDK_PATH)/c-utility/pal/tlsio_options.o \
\
\
# 上层应用的代码，也就是`app_main`调用的demo入口函数就在该模块。通用，可以直接包含在你的代码中
$(EDGE_SDK_PATH)/iothub_client/samples/iothub_client_sample/iothub_mqtt_client_sample.o	\

# 源文件所在的目录路径。比如，你用Makefile构建的工程，就可以添加到`vpath`路径中。
COMPONENT_SRCDIRS :=  \
$(EDGE_SDK_PATH)/c-utility/src \
$(EDGE_SDK_PATH)/c-utility/adapters  \
$(EDGE_SDK_PATH)/umqtt/src	\
$(EDGE_SDK_PATH)/iothub_client/src  \
$(EDGE_SDK_PATH)/iothub_client/samples/iothub_client_sample  \
$(EDGE_SDK_PATH)/certs \
$(EDGE_SDK_PATH)/umqtt/deps/c-utility/pal/freertos \
$(EDGE_SDK_PATH)/c-utility/pal/lwip \
$(EDGE_SDK_PATH)/c-utility/pal \
$(ESP_PAL_PATH)/src \

```
## iot-edge-c-sdk 适配器
`iot-edge-c-sdk`可以很方便地适配到各种平台，只有几个系统相关的接口需要根据特定平台作特别适配。所以sdk抽象出一个`PAL`层来屏蔽各个平台的差异性。开发者必须实现以下适配器：
- `platform` 适配器
- `tick_counter`适配器
- `agenttime`适配器
- `tlsio`适配器
- `socketio`适配器
- `sleep`适配器
- `threadapi` 和 `lock`适配器

以下对各个适配器做详细介绍。

### platform 适配器
主要是用来提供系统初始化，一次性的工作。在这里是通过`platform_esp.c`实现的。在ESP32上电初始化时，必须先初始化sntp服务获取当前系统时间等等。`platform_esp.c` 是`platform适配器`的具体实现，是平台适配必不可少的一部分，主要提供平台启动初始化（platform_init）和反初始化（platform_deinit）函数，以及 platform_get_default_tlsio 提供合适的 TLSIO 适配器操作接口。详细请参考：`$IOT_SDK/c-utility/devdoc/platform_requirements.md`。基于esp32平台，可以使用`openssl`或`mbedtls`作为tls接口，这里以openssl为例。

### tick_counter适配器
主要用来做超时，还有时间间隔的计量。在这里是通过`iot-edge-c-sdk/umqtt/deps/c-utility/pal/freertos/tickcounter.c`实现的。`tickcounter_freertos.c` 提供了`tickcounter`适配器，为mqtt client提供时间计数功能，比如`tickcounter_get_current_ms`函数,返回系统启动到此刻的毫秒计数。详细请参考：`$IOT_SDK/c-utility/devdoc/tickcounter_freertos_requirement.md`  

### agenttime适配器
主要用来做时间戳以及时间间隔的计量。大部分的平台和操作系统可以使用标准`agenttime.c`文件，将此文件放到你的编译环境。这个适配器只是简单的调用c语言的的time, difftime, ctime 函数等等。如果这个文件在你的平台无法工作的，你可以拷贝一份，做适当的修改。ESP32是FreeRTOS系统，移植了标准的C库，故可以直接使用标准的`agenttime.c`，无须任何修改。

### tlsio适配器
提供标准的基于TLS之上的通讯方式。  
tlsio适配器提供SDK可以通过标准的安全的TLS通讯方式让设备和IoT Hub交互数据。  
tlsio适配器通过`xio接口`暴露功能让SDK来调用，通过调用函数`xio_create`来创建tlsio适配器实例，`xio_dowork`进行数据收发。  

tlsio支持的模式包括两种：
- 直接模式  
tlsio适配器直接创建自己的TCP socket，直接和远程的服务器进行TLS通讯。直接模式的好处是消耗资源会少很多，比较适合MCU，比如Arduino和ESP32等等。我们在这里就是属于这种情况。  
`tlsio_openssl_compact.c`就是直接模式tlsio适配器的一个很好的例子，如果需要适配的话，可以拷贝一份，基于这个修改来满足自己的需求。  
同时tlsio_openssl_conpact for ESP32提供两个文件，这两个文件是和具体平台无关的：  
  - socket_async.c  
位于 `$IOT_SDK/c-utility/pal/socket_async.c`
  - dns_async.c  
位于 `$IOT_SDK/c-utility/pal/dns_async.c`
大部分的用户都可以直接使用这两个文件而不需要修改，对于特殊情况只需要修改`socket_async_os.h`就可以了。  
- 串联模式  
tlsio适配器不拥有自己的TCP socket，不直接和远程服务器通讯，但是它仍然处理TLS的所有逻辑，包括加密，解密，协商，只不过通过另外的方式和远程服务器进行通讯。串联模式提供更大的灵活度，比如数据可以直接从socket上来，也可以来自其他不同的协议，比如http等等。但是资源消耗也会多很多，在一些硬件资源充足的平台上，比如一些嵌入式linux平台就可以采用这种方式。可以使用tlsio_openssl.c + socketio_berkeley.c 模式，前者是一个`tlsio`适配器，后者是一个`socketio`适配器。

### socketio适配器
提供非安全的tcp通讯方式，和tlsio一样，符合`xio接口`。上层通过`xio_create` `xio_open` `xio_dowork`等函数去创建实例、数据收发等等。所以，我们可以看到，socketio可以在tlsio之下，这种就是所谓的“串连方式”，也可以不存在，tlsio直接调用socket接口，也就是所谓的“直连方式”。
### sleep适配器
主要提供了睡眠函数，和平台相关。SDK需要使用 ThreadAPI_Sleep这个接口提供毫秒级的延时。
比如在linux平台上，最后调用nanosleep进行休眠，将进程状态设置成TASK_INTERRUPTIBLE状态，
放入等待队列。比如在esp32平台上，调用 vTaskDelay 进行休眠。其相关实现在`threadapi_freertos.c`中。详细请参考：`$IOT_SDK/c-utility/devdoc/threadapi_freertos_requirements.md`  

### threadapi and lock适配器
threadapi和lock这个两个适配器是可选的。就目前来说，用的比较少。
esp32 平台使用但FreeRTOS 本身不支持多线程。所以相关但函数都设置为空。因为创建线程
需要为每个线程分为独立的stack，这样对于某些嵌入式设备来说，就比较困难了。
### 证书变量
`certs.h` 声明了应用层使用的证书变量，该证书的作用是客户端在ssl握手时，用来验证服务器发过来的证书。我们还需要具体定义这些变量，在`iot-edge-c-sdk/certs/certs.c`中

烧写测试，发现可以正常启动，但是无法连接mqtt server。  
```
iothub_mqtt_dowork Line:1175 fail to establish connection with server
```
还需要修改 `iothub_mqtt_client_sample.c` 中的服务器域名以及用户名密码等等信息。这些信息的获取，可以登陆[百度云](https://cloud.baidu.com) ，创建百度云物接入实例，从而生成对应的服务器账号密码等信息。详细参考[官方文档](https://cloud.baidu.com/doc/IOT/GettingStarted.html)

```c
// Please set the mqtt client data and security which are shown as follow.
// The endpoint address, witch is like "xxxxxx.mqtt.iot.xx.baidubce.com".
#define         ENDPOINT                    "xxxxxx.mqtt.iot.xx.baidubce.com"

// The mqtt client username, and the format is like "xxxxxx/xxxx".
#define         USERNAME                    "xxxxxx/xxxx"

// The key (password) of mqtt client.
#define         PASSWORD                    "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"

// The connection type is TCP, TLS or MUTUAL_TLS.
#define         CONNECTION_TYPE              "TLS"
```

编译，烧写，测试，用MQTT fx工具测试数据收发正常，适配结束。

## 适配流程（tls库选用 mbedtls）
esp32 sdk中已经提供了mbedtls的支持，用户只需做很少的修改即可使用。

- 修改 component.mk  
    - 去掉 ```CFLAGS += -DUSE_OPENSSL```，添加 ```CFLAGS += -DUSE_MBED_TLS ```  
    - 修改 COMPONENT_OBJS，去掉 openssl tlsio适配器 加入 mbedtls tlsio适配器
        ```
        $(EDGE_SDK_PATH)/c-utility/pal/dns_async.o	\
        $(EDGE_SDK_PATH)/c-utility/pal/socket_async.o	\
        $(ESP_PAL_PATH)/src/tlsio_openssl_compact.o	\	
        ```
        加入以下文件：
        ```
        $(EDGE_SDK_PATH)/c-utility/adapters/tlsio_mbedtls.o	\
        ```

    - 编译报错
        ```
        undefined reference to `mbedtls_debug_set_threshold'
        ```
        make menuconfig
加入mbedtls 的 debug选项即可

编译，烧写，测试，用MQTT fx工具测试数据收发正常，适配结束。
