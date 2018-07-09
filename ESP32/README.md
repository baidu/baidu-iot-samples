# 前言
本文主要讲解如何适配baidu iot-edge-c-sdk 到esp32芯片，连接到baidu iot-hub上，实现mqtt数据收发。希望带领读者在一步步解决问题的过程中逐步理解适配的原理。


# 硬件资源
- CPU：低功耗Xtensa® LX6 32-bit单/双核处理器, 7级流水线架构，支持高达240 MHz的时钟频率。
- ROM：448 KB，用于程序启动和内核功能调用。
- RAM：520 KB 片上SRAM，用于数据和指令存储的。
- 外部Flash：最多支持4个16 MB的外部 QSPI Flash。用于存放程序固件，以及其他用户数据。

# 软件资源
## esp32 sdk 编译体系结构
ESP-IDF全称Espressif IoT Development Framework是乐鑫基于ESP32推出的新一代SDK，基于FreeRTOS系统。
其主要几个目录功能介绍：（详细请参考官方文档：https://esp-idf.readthedocs.io/zh_CN/latest/get-started/index.html）
- components目录
    该目录中每个子目录都表示一个库，每个库里都有一个 components.mk, 包含一些头文件路径，需要编译的模块等信息。makefile会根据这个文件去编译该库。之后，我们的edge sdk库就会放在该目录下。
- main目录
    用户工程代码所在的目录，之后我们的测试demo程序就会放在这个文件夹下，该文件夹可以放在任何地方。注意，用户必须定义一个app_main()函数，作为程序的入口。

## esp32 sdk 编译
### 安装 toolchain
    直接下载官方提供的toolchain包，解压即可。

### 下载 esp32 sdk
```bash
    mkdir -p /path/to/proj/dir # 存放 esp32 sdk 的工程目录，以下都用 $PROJ_DIR 指代，所有涉及的工程文件都放在该路径下
    cd $PROJ_DIR
    git clone --recursive https://github.com/espressif/esp-idf.git esp32-idf
    export PROJ_DIR=`pwd`
```

### 设置环境变量
```bash
    export IDF_PATH=$PROJ_DIR/esp32-idf # IDF_PATH: esp32 sdk 目录
    export PATH=/path/to/your/toolchain/bin/:$PATH # PATH: 加入toolchain路径: 
```

### 编译测试 hello_world
```bash
    cd $IDF_PATH/examples/get-started/hello_world
    make menuconfig # 配置烧写参数，比如：spi mode, spi flash大小, 串口打印波特率等等
    make
    make flash # 烧写到esp32模块，具体可以参考官方文档。
```

### 构建自己的库 baidu-iot

```bash
cd $IDF_PATH/components
mkdir baidu-iot
cd baidu-iot
git clone --recursive https://github.com/baidu/iot-edge-c-sdk.git 
cp iot-edge-c-sdk/build_all/esp32/components/sdk/component.mk  .  # 使用官方已经适配好的components.mk
export IOT_SDK=$IDF_PATH/components/baidu-iot/iot-edge-c-sdk
```


### 构建自己的工程
```bash
cd $PROJ_DIR
git clone https://github.com/baidu/iot-edge-sdk-samples.git # 该工程下ESP32子目录里有 pal 和 sample 两个文件夹，前者存放适配相关文件，后者存放工程文件（包含程序入口函数app_main）
export SAP_DIR=$PROJ_DIR/iot-edge-sdk-samples/ESP32
```


### 配置工程
```bash
cd $SAP_DIR/sample 
make menuconfig # 配置flash大小，连接的路由器ssid 密码等
make
```
一切就绪，开始适配工作。

# 具体适配
## 适配流程 (tls库为 openssl)
可以在解决编译中出现问题的过程中，逐步理解iot-edge-c-sdk的适配原理。

```
make[1]: *** No rule to make target `sdk/c-utility/src/xlogging.o', needed by `libbaidu-iot.a'.  Stop.
make: *** [component-baidu-iot-build] Error 2
```
修改 $IDF_PATH/components/baidu-iot/component.mk 把 "sdk/..." 起始的路径改为"iot-edge-c-sdk/..."

```
In file included from $PROJ_DIR/esp32-idf/components/baidu-iot/iot-edge-c-sdk/c-utility/src/constbuffer.c:12:0:
$PROJ_DIR/esp32-idf/components/baidu-iot/iot-edge-c-sdk/c-utility/inc/azure_c_shared_utility/refcount.h:31:25: fatal error: refcount_os.h: No such file or directory
compilation terminated.
```
`refcount_os.h`用于线程安全的引用计数实现，esp32 跑的是 freeRTOS，选择 generic 实现版本就可以了，
文件路径在 $IOT_SDK/c-utility/pal/generic/refcount_os.h  
故修改 component.mk 在 COMPONENT_ADD_INCLUDEDIRS 中加入  
"iot-edge-c-sdk/c-utility/pal/generic"  


```
CC build/baidu-iot/iot-edge-c-sdk/iothub_client/src/iothub_mqtt_client.o
$PROJ_DIR/esp32-idf/components/baidu-iot/iot-edge-c-sdk/iothub_client/src/iothub_mqtt_client.c:25:19: fatal error: certs.h: No such file or directory
compilation terminated.
```
`certs.h` extern定义应用层使用的证书变量，之后我们需要在.c中定义实际的证书变量，以便在tls handshake中验证服务器证书。
证书变量的定义在： iot-edge-c-sdk/certs/certs.c，故修改 component.mk  
在 COMPONENT_ADD_INCLUDEDIRS 中加入  
"iot-edge-c-sdk/certs"  
在 COMPONENT_OBJS 中加入  
"iot-edge-c-sdk/certs/certs.o"  


```
make[1]: *** No rule to make target `iot-edge-c-sdk/c-utility/adapters/platform_openssl_compact.o', needed by `libbaidu-iot.a'.  Stop.
make: *** [component-baidu-iot-build] Error 2
```
`platform_openssl_compact.c` 是 platform适配器的具体实现，是平台适配必不可少的一部分，主要提供平台启动初始化（platform_init）和反初始化（platform_deinit）函数，以及 platform_get_default_tlsio 提供合适的 TLSIO 适配器操作接口。详细请参考：$IOT_SDK/c-utility/devdoc/platform_requirements.md  
基于esp32平台，我们选用openssl作为tls接口，故复制 $SAP_DIR/pal/src/platform_openssl_compact.c 到 $IOT_SDK/c-utility/adapters/ 中。


```
$PROJ_DIR/esp32-idf/components/baidu-iot/iot-edge-c-sdk/c-utility/adapters/platform_openssl_compact.c:5:18: fatal error: sntp.h: No such file or directory
compilation terminated.
```
在esp32系统初始化过程中，需要通过连接到ntp服务器以获取系统时间。`sntp.h`提供了sntp函数操作:  
SNTP_SetServerName() SNTP_Init() SNTP_Deinit() 的声明。  
文件位于： $IOT_SDK/c-utility/pal/inc/sntp.h  
修改 component.mk 在 COMPONENT_ADD_INCLUDEDIRS 中加入  
"iot-edge-c-sdk/c-utility/pal/inc"

```
$PROJ_DIR/esp32-idf/components/baidu-iot/iot-edge-c-sdk/c-utility/adapters/platform_openssl_compact.c:6:23: fatal error: tlsio_pal.h: No such file or directory
compilation terminated.
```
`tlsio_pal.h` 提供了 tlsio_pal_get_interface_description 函数声明。  
复制 $SAP_DIR/pal/inc/tlsio_pal.h 到 $IOT_SDK/c-utility/inc/azure_c_shared_utility/  


```
make[1]: *** No rule to make target `iot-edge-c-sdk/c-utility/adapters/threadapi_freertos.o', needed by `libbaidu-iot.a'.  Stop.
```
`threadapi_freertos.c` 提供了threadapi 和 sleep 适配器，为系统提供多线程和休眠函数。  
FreeRTOS不保证支持多线程，相关的函数都定义为空。休眠函数使用Freertos中的vTaskDelay来实现。  
详细请参考：$IOT_SDK/c-utility/devdoc/threadapi_freertos_requirements.md  
在iot_edge_c_sdk中已经提供了该适配器的实现：  
`$IOT_SDK/umqtt/deps/c-utility/pal/freertos/threadapi.c`  
故修改 component.mk COMPONENT_OBJS下的  
"iot-edge-c-sdk/c-utility/adapters/threadapi_freertos.o    \"  
改为  
"iot-edge-c-sdk/umqtt/deps/c-utility/pal/freertos/threadapi.o    \"  
同时修改 component.mk 在 COMPONENT_SRCDIRS 中加入  
"iot-edge-c-sdk/umqtt/deps/c-utility/pal/freertos"  


```
make[1]: *** No rule to make target `iot-edge-c-sdk/c-utility/adapters/tickcounter_freertos.o', needed by `libbaidu-iot.a'.  Stop.
make: *** [component-baidu-iot-build] Error 2
```
`tickcounter_freertos.c` 提供了 tickcounter 适配器，为mqtt client提供时间计数功能，比如tickcounter_get_current_ms(),返回
系统启动到此刻的毫秒计数。  
详细请参考：  
$IOT_SDK/c-utility/devdoc/tickcounter_freertos_requirement.md  
在iot_edge_c_sdk中已经提供了该适配器的实现：  
`$IOT_SDK/umqtt/deps/c-utility/pal/freertos/tickcounter.c`  
故修改 component.mk COMPONENT_OBJS下的  
"iot-edge-c-sdk/c-utility/adapters/tickcounter_freertos.o	\"  
改为  
"iot-edge-c-sdk/umqtt/deps/c-utility/pal/freertos/tickcounter.o    \"  


```
make[1]: *** No rule to make target `iot-edge-c-sdk/c-utility/adapters/sntp_lwip.o', needed by `libbaidu-iot.a'.  Stop.
make: *** [component-baidu-iot-build] Error 2
```
`sntp_lwip.c`使用esp32 lwIP库中的接口实现 sntp.h 中的三个函数 SNTP_SetServerName() SNTP_Init() SNTP_Deinit()。  
在iot_edge_c_sdk中已经提供了实现：  
`$IOT_SDK/c-utility/pal/lwip/sntp_lwip.c`  
故修改 component.mk COMPONENT_OBJS下的  
"iot-edge-c-sdk/c-utility/adapters/sntp_lwip.o    \"  
改为   
"iot-edge-c-sdk/c-utility/pal/lwip/sntp_lwip.o    \"  
同时，在 COMPONENT_SRCDIRS 中加入：  
"iot-edge-c-sdk/c-utility/pal/lwip"  

```
make[1]: *** No rule to make target `iot-edge-c-sdk/c-utility/adapters/dns_async.o', needed by `libbaidu-iot.a'.  Stop.
```
`dns_async.c`为系统提供 dns 服务的支持，根据传入的mqtt server域名，得到对应的ip。  
在iot_edge_c_sdk中已经提供了实现：  
`$IOT_SDK/c-utility/pal/dns_async.c`  
故修改 component.mk COMPONENT_OBJS下的  
"iot-edge-c-sdk/c-utility/adapters/dns_async.o    \"  
改为   
"iot-edge-c-sdk/c-utility/pal/dns_async.o   \"  
同时，在 COMPONENT_SRCDIRS 中加入：  
"iot-edge-c-sdk/c-utility/pal"  


```
$PROJ_DIR/esp32-idf/components/baidu-iot/iot-edge-c-sdk/c-utility/pal/dns_async.c:11:29: fatal error: socket_async_os.h: No such file or directory
compilation terminated.
```
在这里主要解释一下iot-edge-c-sdk框架下，tlsio适配器 和 socketio适配器的基本概念，这是适配工作的最重要组成部分。详细请参考：  
$IOT_SDK/c-utility/devdoc/porting_guide.md


### tlsio适配器介绍

tlsio适配器提供SDK可以通过标准的安全的TLS通讯方式让设备和IoT Hub交互数据。  
tlsio适配器通过xio接口暴露功能让SDK来调用，通过调用函数xio_create来创建tlsio适配器实例。  

tlsio支持的模式包括两种：直接的，串联的。  
- 直接模式，tlsio适配器直接创建自己的TCP socket，直接和远程的服务器进行TLS通讯。直接模式的好处是消耗资源会少很多，比较适合MCU，比如Arduino和ESP32等等。我们在这里就是属于这种情况。（后面会看到，是 tlsio_openssl_compact.c + socket_async.c 模式，后者只是一个"thin wrapper of standard socket api"）
- 串联模式，tlsio适配器不拥有自己的TCP socket，不直接和远程服务器通讯，但是它任然处理TLS的所有逻辑，包括加密，解密，协商，只不过通过另外的方式和远程服务器进行通讯。串联模式提供更大的灵活度，但是资源消耗也会多很多，所以主要主流的OS，比如windows，linux和Mac等等。在Mac上跑的iot demo就是属于这种情况。（即 tlsio_openssl_compact.c + socketio_berkeley.c + "standard socket api"）

### socketio适配器介绍

只针对所谓的“串联”模式，tlsio适配器必须调用xio的适配器，这个适配器可以包含一个tcp socket。在百度的IoT SDK里面，xio适配器是通过socketio管理tcp socket。具体可以参考 socketio_berkeley.c。  

在这里我们使用的是“直接”模式，也就是tlsio适配器直接创建tcp连接的方式。  
`tlsio_openssl_conpact.c` 就是直接模式tlsio适配器的一个很好的例子，如果需要适配的话，可以拷贝一份，基于这个修改来满足自己的需求。  
tlsio_openssl_conpact for ESP32提供类两个文件，这两个文件是和具体平台无关的：  
- socket_async.c
（位于 $IOT_SDK/c-utility/pal/socket_async.c）
- dns_async.c
（位于 $IOT_SDK/c-utility/pal/dns_async.c）

大部分的用户都可以直接使用这两个文件而不需要修改，对于特殊情况只需要修改socket_async_os.h就可以了。  


根据以上分析，我们直接使用上述4个文件加入工程即可。  
复制 $SAP_DIR/pal/inc/socket_async_os.h 到 $IOT_SDK/c-utility/inc/azure_c_shared_utility/  
复制 $SAP_DIR/pal/src/tlsio_openssl_compact.c 到 $IOT_SDK/c-utility/adapters/  
修改 component.mk iot-edge-c-sdk/c-utility/adapters/socket_async.o 改为 iot-edge-c-sdk/c-utility/pal/socket_async.o  
复制 dns_async.c 已经拷贝  

```
LD build/esp32.elf
$PROJ_DIR/esp32_demo/sample/build/baidu-iot/libbaidu-iot.a(iothub_mqtt_client.o):(.literal.CreateTcpConnection+0x0): undefined reference to `socketio_get_interface_description'
$PROJ_DIR/esp32_demo/sample/build/baidu-iot/libbaidu-iot.a(iothub_mqtt_client.o): In function `CreateTcpConnection':
$PROJ_DIR/esp32-idf/components/baidu-iot/iot-edge-c-sdk/iothub_client/src/iothub_mqtt_client.c:1057: undefined reference to `socketio_get_interface_description'
$PROJ_DIR/esp32_demo/sample/build/baidu-iot/libbaidu-iot.a(tlsio_openssl_compact.o):(.literal.tlsio_openssl_setoption+0x10): undefined reference to `tlsio_options_set'
$PROJ_DIR/esp32_demo/sample/build/baidu-iot/libbaidu-iot.a(tlsio_openssl_compact.o):(.literal.tlsio_openssl_destroy+0x8): undefined reference to `tlsio_options_release_resources'
$PROJ_DIR/esp32_demo/sample/build/baidu-iot/libbaidu-iot.a(tlsio_openssl_compact.o):(.literal.tlsio_openssl_create+0x14): undefined reference to `tlsio_options_initialize'
$PROJ_DIR/esp32_demo/sample/build/baidu-iot/libbaidu-iot.a(tlsio_openssl_compact.o):(.literal.tlsio_openssl_retrieveoptions+0x8): undefined reference to `tlsio_options_retrieve_options'
$PROJ_DIR/esp32_demo/sample/build/baidu-iot/libbaidu-iot.a(tlsio_openssl_compact.o): In function `tlsio_openssl_setoption':
$PROJ_DIR/esp32-idf/components/baidu-iot/iot-edge-c-sdk/c-utility/adapters/tlsio_openssl_compact.c:829: undefined reference to `tlsio_options_set'
$PROJ_DIR/esp32_demo/sample/build/baidu-iot/libbaidu-iot.a(tlsio_openssl_compact.o): In function `tlsio_openssl_destroy':
$PROJ_DIR/esp32-idf/components/baidu-iot/iot-edge-c-sdk/c-utility/adapters/tlsio_openssl_compact.c:217: undefined reference to `tlsio_options_release_resources'
$PROJ_DIR/esp32_demo/sample/build/baidu-iot/libbaidu-iot.a(tlsio_openssl_compact.o): In function `tlsio_openssl_create':
$PROJ_DIR/esp32-idf/components/baidu-iot/iot-edge-c-sdk/c-utility/adapters/tlsio_openssl_compact.c:280: undefined reference to `tlsio_options_initialize'
$PROJ_DIR/esp32_demo/sample/build/baidu-iot/libbaidu-iot.a(tlsio_openssl_compact.o): In function `tlsio_openssl_retrieveoptions':
$PROJ_DIR/esp32-idf/components/baidu-iot/iot-edge-c-sdk/c-utility/adapters/tlsio_openssl_compact.c:856: undefined reference to `tlsio_options_retrieve_options'
collect2: error: ld returned 1 exit status
```
"socketio_get_interface_description" 提供非安全的 tcp 连接，需要一个针对 lwip socket API实现一个socketio层 —— `socketio_lwip.c`   
复制 $SAP_DIR/pal/src/socketio_lwip.c 到 $IOT_SDK/c-utility/adapters/ 目录下。  
修改 component.mk COMPONENT_OBJS 加入：  
"iot-edge-c-sdk/c-utility/adapters/socketio_lwip.o   \"  

"tlsio_options_xxx"函数缺少定义。  
修改component.mk COMPONENT_OBJS, 加入 "iot-edge-c-sdk/c-utility/pal/tlsio_options.o   \"  
修改component.mk COMPONENT_SRCDIRS, 加入 "iot-edge-c-sdk/c-utility/pal "  


最后，编译通过。生成esp32.bin，烧写测试。发现可以正常启动。但是无法连接mqtt server。  
```
iothub_mqtt_dowork Line:1175 fail to establish connection with server
```
还需要修改 `iothub_mqtt_client_sample.c` 中的服务器域名以及用户名密码等等信息。这些信息的获取，可以登陆 `https://cloud.baidu.com` 创建百度云物接入实例，从而生成对应的服务器账号密码等信息。参考：`https://cloud.baidu.com/doc/IOT/GettingStarted.html`

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

编译，烧写，测试，发现可以连上服务器了，只是 tls 握手依旧有问题：
```
...
Info: SNTP initialization complete
Error: tlsio_options.c Func:tlsio_options_set Line:156 Trusted certs option not supported
Error: tlsio_openssl_compact.c Func:tlsio_openssl_setoption Line:832 Failed tlsio_options_set
Error: iothub_mqtt_client.c Func:CreateTlsConnection Line:893 Fail to assign trusted cert chain
Error: iothub_mqtt_client.c Func:publish_mqtt_message Line:525 Does not support qos = DELIVER_EXACTLY_ONCE
call publish at most once handle
...
```
是因为该demo 还不支持验证服务器证书的功能，我们可以自己添加该功能。修改 `tlsio_openssl_compact.c`
```c
static CONCRETE_IO_HANDLE tlsio_openssl_create(void* io_create_parameters)
{
    ...
    //tlsio_options_initialize(&result->options, TLSIO_OPTION_BIT_NONE);
    tlsio_options_initialize(&result->options, TLSIO_OPTION_BIT_TRUSTED_CERTS); //支持 trusted_certs 功能
    ...
}
/**
 * @brief add CA client certification into the CTX
 */
static int ssl_ctx_load_verify_buffer(SSL_CTX *ctx, const unsigned char* buffer, long len)
{
    if (!ctx || !buffer) {
        return -1;
    }

    X509* cacrt = d2i_X509(NULL, buffer, len);

    if (cacrt) {
        SSL_CTX_add_client_CA(ctx, cacrt);
        return 1;
    } else {
        return 0;
    }
}
static int create_ssl(TLS_IO_INSTANCE* tls_io_instance)
{
    ...
    // openssl 添加 CA 证书，用于验证服务器。
    trusted_certs = tls_io_instance->options.trusted_certs;
    if (trusted_certs && trusted_certs[0]) 
    {
        //add baidu cert
        printf("load ca crt ......");
        ret = ssl_ctx_load_verify_buffer(tls_io_instance->ssl_context, (const unsigned char*)trusted_certs, strlen(trusted_certs));
        printf("set verify mode verify peer\n");
        SSL_CTX_set_verify(tls_io_instance->ssl_context, SSL_VERIFY_PEER, NULL); //SSL_VERIFY_PEER SSL_VERIFY_NONE

    }
    ...
}
```
编译，烧写，测试，用MQTT fx工具测试数据收发正常，适配结束。

## 适配流程（tls库选用 mbedtls）
esp32 sdk中已经提供了mbedtls的支持，用户只需做很少的修改即可使用。
- 仿照 platfomr_openssl_compact.c，添加一个 platform_mbedtls_compact.c，内容基本一致，适当修改头文件
```c
...
//#include "tlsio_pal.h"
//改为
#include "tlsio_mbedtls.h"
...

/*const IO_INTERFACE_DESCRIPTION* platform_get_default_tlsio(void)
{
    return tlsio_pal_get_interface_description();
}*/
//改为
const IO_INTERFACE_DESCRIPTION* platform_get_default_tlsio(void)
{
    return tlsio_mbedtls_get_interface_description();
}

```
- 修改 component.mk  
    - 添加 CFLAGS += -DUSE_MBED_TLS   
    - 修改 COMPONENT_OBJS，去掉 openssl 相关的文件，加入以下文件：
    sdk/c-utility/adapters/tlsio_mbedtls.o	\
    sdk/c-utility/adapters/platform_mbedtls_compact.o	\

- 编译报错
```
undefined reference to `mbedtls_debug_set_threshold'
```
make menuconfig
加入mbedtls 的 debug选项

编译，烧写，测试，用MQTT fx工具测试数据收发正常，适配结束。



# changelog
- 2018-6-26         初版   1.0.0
- 2018-6-26               1.0.1
    - 实现基于lwip socket api的 socketio层适配函数 socketio_lwip.c，用于非tls连接。
    - 文档润色。
- 2018-6-28               1.0.2
    - 消除文档中用户敏感信息
    - 添加 overview
- 2018-7-1                1.0.3
    - 添加 mbedtls 适配的支持
- 2018-7-8                1.0.4
    - 修改所引用开源库的链接
    - 文档润色。
- 2018-7-9                1.0.5
    - 修改设置环境变量命令
    - 去掉文档中的一些空白字符