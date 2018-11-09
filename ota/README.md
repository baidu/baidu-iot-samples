这里提供了一个基于mqtt协议的ota方案，支持数据分片，数据校验等基本功能。资源占用很小，非常适合资源受限的嵌入式环境。

## 文件结构
ota模块代码在`esp32_baidu_ota_demo/main/libota/`中：
```
- ota_update_engine.c       # ota 主逻辑
- ota_update_engine.h       # ota 主逻辑头文件
- ota_hal.h                 # ota 硬件抽象层接口
- ota_hal_esp32.c           # ota 硬件抽象层接口实现，基于`esp32`平台
```
如果用户希望基于其他平台适配该ota模块，只需要实现`ota_hal.h`中提供的接口即可。其他代码不用任何改变。

## ota 硬件抽象层接口
参考 `ota_hal.h`。改文件抽象了各个平台进行ota需要实现的接口，包括“flash的读写” “boot 分区标志位切换”等等。这些接口的具体实现，在相应的`ota_hal_xxx.c`中。

- `HalOtaHandler* get_hal_ota_handler();`   
获取`HalOtaHandler`实例，该实例中提供了ota升级所涉及到的所有具体接口，需用户实现。以下就对这些接口作简单介绍。

    - `int (*hal_ota_begin)(void **ctx);`  
        用户实现的`hal_ota_begin`接口函数，`ctx`参数是用户传入的，在该函数中为`ctx`分配内存。因为通常在升级过程中，用户需要  
        维护一些的状态信息，缓存结构等等。这些结构都可以存放在 ctx 全局结构中。

    - `int (*hal_ota_write_data)(void *ctx, unsigned char *data, int data_len);`  
        用户实现的`hal_ota_write_data`接口函数，用来向ota partition 中写入二进制数据。可能会涉及 flash 读写操作。

    - `int (*hal_ota_end)(void *ctx, bool reboot);`  
        用户实现的`hal_ota_end`接口函数，如果判断二进制数据下载完成，同时写入操作都正确的话，就可以结束  
        ota 升级操作，释放ctx结构，相关缓存以及状态信息，第二个参数表示是否立刻重启系统。如果在ota中出现  
        错误，或者是checksum校验失败，也可以调用该接口结束ota。

## ota 硬件抽象层接口实现（基于esp32）
这里提供了在 esp32 平台下ota硬件抽象层接口的实现`ota_hal_esp32.c`，以供参考。首先，先了解一下在esp32下ota的一些基本概念。以下内容大部分内容摘录自 espressif 官方文档。

- esp32分区表：http://esp-idf-zh.readthedocs.io/zh_CN/latest/api-guides/partition-tables.html
- esp32 OTA升级接口：
http://esp-idf-zh.readthedocs.io/zh_CN/latest/api-reference/system/ota.html

### esp32分区表
单个 ESP32 flash 可以包含多个应用程序，以及多种数据（校验数据、文件系统、参数存储器等）。基于这个原因，在 flash 的偏移地址`0x8000`处烧写了一个分区表。

分区表的每个条目都包含Name(标签)、Type（app、data 等）、SubType以及在 flash 中的偏移量 Offset（分区表被加载的地址）以及分区Size。比如像下面这样：
```
# Espressif ESP32 Partition Table
# Name,   Type, SubType, Offset,  Size
nvs,      data, nvs,     0x9000,  0x6000
phy_init, data, phy,     0xf000,  0x1000
factory,  app,  factory, 0x10000, 1M
```

要生成分区表，最简单的方法是使用 make menuconfig:  
```
Partition Table
    Partition Table (Single factory app, no OTA)
        (X) Single factory app, no OTA  
        ( ) Factory app, two OTA definitions  
        ( ) Custom partition table CSV
```
- 第一个选项是默认的，不支持OTA，只有一个factory分区。也就是上面例子描述的样子。
- 第二个选项表示支持OTA升级，除了一个factory分区，还包括两个OTA分区，用于保存OTA下载下来的bin文件。
- 第三个选项表示支持用户自定义的分区表，比如你需要多余2个的OTA分区。

在前两种情况下，factory.bin必须烧写到偏移量`0x10000`处。
以下是两个例子：

#### esp32内置分区表
下面是配置 “Single factory app, no OTA” 所打印出的信息
```
# Espressif ESP32 Partition Table
# Name,   Type, SubType, Offset,  Size
nvs,      data, nvs,     0x9000,  0x6000
phy_init, data, phy,     0xf000,  0x1000
factory,  app,  factory, 0x10000, 1M
```
- flash 的偏移地址 0x10000 (64KB) 处是标记为 “factory” 的应用程序。bootloader 默认会运行这里的应用程序。
- 分区表中还定义了两个数据区域，用于存储 NVS 库分区和 PHY 初始化数据。

下面是配置 “Factory app, two OTA definitions” 所打印出的信息
```
# Espressif ESP32 Partition Table
# Name,   Type, SubType, Offset,  Size
nvs,      data, nvs,     0x9000,  0x4000
otadata,  data, ota,     0xd000,  0x2000
phy_init, data, phy,     0xf000,  0x1000
factory,  0,    0,       0x10000, 1M
ota_0,    0,    ota_0,   ,        1M
ota_1,    0,    ota_1,   ,        1M
```
- 存在三个应用程序分区的定义。
- 这三个的类型都是 app，但是`子类型`不同，工厂 app 位于 0x10000 处，剩余两个是 “OTA” app。
- `这里还有一个新的 otadata，即OTA 数据分区，它用于保存 OTA 更新的一些信息。bootloader 会使用这些数据来判断指定哪个应用程序。如果 “otadata” 是空的，它会执行工厂应用程序。`

#### esp32自定义分区表
如果你在配置菜单中选择 “Custom partition table CSV”，你需要输入用于保存你的分区表的 CSV 文件的名字（在工程目录中）。CSV 可以根据你的需要描述任意数量的定义。具体的格式以及编写方法，请参考官方文档。

### esp32 OTA 过程概述
OTA 升级机制允许常规固件在运行时基于它所接收的数据对设备进行升级（通过 WiFI 或者蓝牙）。

OTA 需要配置设备的 Partition Table，且至少需要两个 “OTA app” 分区（即 ota_0 和 ota_1）和一个 OTA 数据分区。

OTA 会将新的 app 固件镜像写到当前未用于启动程序的那个 OTA app 分区。当镜像校验完成后，OTA 数据分区会被更新，表示下一次启动时将使用该镜像。

使用 OTA 功能的产品必须在 Partition Table 中包含一个 OTA 数据分区。

`对于工厂启动设置，OTA 数据分区应当不包含数据（所有的字节被擦除为 0xFF）。`在这种情况下，如果分区表中存在工厂 app，esp-idf 软件的 bootloader 会启动工厂 app。如果分区表中不存在工厂 app，则会启动第一个有效的 OTA 分区（通常是 ota_0）。

当第一次 OTA 更新后，OTA 数据分区将会被更新，表示表示下一次启动时将使用哪个 OTA app 分区。

OTA 数据分区是两个 flash 扇区（0x2000 字节），以消除正在写时供电失败的问题。如果没有计数字段表明哪个扇区在最近被写过，则两个扇区会被独立擦除并写入匹配的数据。

### esp32 OTA 接口实现代码分析
```c
typedef struct{
    esp_partition_t *update_partition;
    esp_ota_handle_t update_handle;
} esp32_ota_ctx;


static int esp32_hal_ota_begin(void **_ctx)
{
    esp32_ota_ctx *ctx;
    int err;

    if (!_ctx) {
        return -1;
    }
    ctx = *_ctx = (esp32_ota_ctx*)malloc(sizeof(esp32_ota_ctx));//升级过程中的全局变量，用于保存状态信息
    if (NULL == ctx) {
        LogError("malloc esp32 ctx error\n");
        return -1;
    }

    ctx->update_partition = esp_ota_get_next_update_partition(NULL);//获得下一个 ota partition 对象
    LogInfo("Writing to partition subtype %d at offset 0x%x\n",
            ctx->update_partition->subtype, ctx->update_partition->address);
    assert(ctx->update_partition != NULL);

    err = esp_ota_begin(ctx->update_partition, OTA_SIZE_UNKNOWN, &ctx->update_handle);//开始 ota 升级
    if (err != ESP_OK) {
        LogError("esp_ota_begin failed, error=%d\n", err);
        free(ctx);
        return -1;
    }
    
    return 0;
}

static int esp32_hal_ota_write_data(void *_ctx, unsigned char *data, int data_len)
{
    esp32_ota_ctx *ctx;
    int err;

    ctx = (esp32_ota_ctx *)_ctx;
    err = esp_ota_write(ctx->update_handle, (const void *)data, data_len);//写 ota 二进制数据到 ota partition
    if (err != ESP_OK) {
        LogError("Error: esp_ota_write failed! err=0x%x", err);
        if (esp_ota_end(ctx->update_handle) != ESP_OK) {//如果写出错，也结束 ota 升级
            LogError("esp_ota_end failed!\n");
        }
        free(ctx);
        return -1;
    }
    return data_len;
}

static int esp32_hal_ota_end(void *_ctx, bool reboot)
{
    esp32_ota_ctx *ctx;
    int err;

    ctx = (esp32_ota_ctx *)_ctx;
    if ((err = esp_ota_end(ctx->update_handle)) != ESP_OK) {//结束 ota 升级，释放缓存
        LogError("esp_ota_end failed!\n");
        free(ctx);
        return -1;
    }
    err = esp_ota_set_boot_partition(ctx->update_partition);//设置下一个 ota partition 为下一次启动的分区
    if (err != ESP_OK) {
        LogError("esp_ota_set_boot_partition failed! err=0x%x", err);
        free(ctx);
        return -1;
    }
    if (reboot) {
        esp_restart();
    }
    
    free(ctx);
    return 0;
}


HalOtaHandler esp32_hal_ota_handler = {
    esp32_hal_ota_begin,
    esp32_hal_ota_write_data,
    esp32_hal_ota_end,
};


HalOtaHandler* get_hal_ota_handler()
{
    return &esp32_hal_ota_handler;
}
```

## OTA 模块使用接口
接口定义在`esp32_baidu_ota_demo/main/libota/ota_update_engine.h`
```h
#ifndef __OTA_UPDATER_H__
#define __OTA_UPDATER_H__

#include "ota_hal.h"
#include <inttypes.h>

/* 当前平台代号 */
#define 	CURRENT_PLATFORM			"esp32"
/* ota请求头长度 */
#define		REQ_HEADER_LEN				16
/* 请求分片头长度 */
#define 	PIECE_HEADER_LEN 			16
/* 分片大小，根据平台内存资源情况，合理设置 */
#define		OTA_UPDATE_PIECE_SEG		8192

typedef struct _OtaUpdater OtaUpdater;

typedef enum {
	OTA_UPDATER_CB_ERR = -1,
	OTA_UPDATER_CB_FW_ALREADY_NEW,
	OTA_UPDATER_CB_CHECKSUM_ERR,
	OTA_UPDATER_FINISH,
} OtaUpdaterCbSt;

/**
 * ota 暂时支持的摘要算法：
 * - OTA_BIN_DM_MD5： 会对下载下来的ota文件做MD5校验。
 * - OTA_BIN_DM_SHA256RSA：会对下载下来的ota文件做SHA256摘要签名校验。此时用户需要提供证书文件。
 */
typedef enum {
	OTA_BIN_DM_MD5,
	OTA_BIN_DM_SHA256RSA,
} OtaSigMethod;

typedef void(*OtaUpdaterCb)(OtaUpdater* apt, OtaUpdaterCbSt st, void* userData);


/**
 * 创建 OtaUpdater 对象
 * 
 * @param[in] puid          设备的PUID
 * @param[in] cur_ver       程序当前版本号
 * @param[in] sig_method    选用的摘要算法
 * @param[in] client_ota_cert  如果用户选择的调度算法是`OTA_BIN_DM_SHA256RSA`，则需要在此提供证书文件。
 * @param[in] cb            ota升级回调函数
 * @param[in] userData      user data
 * 
 * @retval not NULL: success, NULL: fail
 */
OtaUpdater * ota_updater_create(const char *puid, const char *cur_ver, OtaSigMethod sig_method, const char *client_ota_cert, OtaUpdaterCb cb, void* userData);

/**
 * ota数据处理函数
 * 
 * @param[in] apt       OtaUpdater 对象
 * @param[in] topic     收到数据所属的 topic, 模块会自动过滤ota相关的数据
 * @param[in] bin_data  数据指针
 * @param[in] data_len  数据长度
 * 
 * @retval 0: success, -1: fail
 */
int ota_updater_data_filter(OtaUpdater* apt, const char *topic, uint8_t *bin_data, size_t data_len);

/**
 * 结束ota升级
 * 
 * @param[in] apt       OtaUpdater 对象
 * @param[in] reboot    是否需要重启系统，对于升级成功的情况，需要重启。而对于升级出错的情况，无需重启。
 * 
 * @retval 0: success, -1: fail
 */
int ota_updater_end(OtaUpdater* apt, bool reboot);

/**
 * 清理OtaUpdater 对象
 * 
 * @param[in] apt       OtaUpdater 对象
 * 
 * @retval void
 */
void ota_updater_destroy(OtaUpdater* apt);

#endif
```

用户层使用ota模块非常简单，示例代码在`esp32_baidu_ota_demo/main/iothub_mqtt_client_sample.c`：
```c
...
/* 这里使用了SHA256摘要签名校验，所以需要提供证书 */
static const char *client_ota_cert = "-----BEGIN CERTIFICATE-----\r\n"
        "MIIDejCCAmICCQC+9aCa0neVljANBgkqhkiG9w0BAQsFADB/MQswCQYDVQQGEwJD\r\n"
        "TjERMA8GA1UECAwIc2hhbmdoYWkxETAPBgNVBAcMCHNoYW5naGFpMQwwCgYDVQQK\r\n"
        "DANhYWExDDAKBgNVBAsMA2JiYjEQMA4GA1UEAwwHY2NjLmNvbTEcMBoGCSqGSIb3\r\n"
        "DQEJARYNY2NjQGJhaWR1LmNvbTAeFw0xODEwMjkwNzI1MDNaFw0xOTEwMjkwNzI1\r\n"
        "MDNaMH8xCzAJBgNVBAYTAkNOMREwDwYDVQQIDAhzaGFuZ2hhaTERMA8GA1UEBwwI\r\n"
        "c2hhbmdoYWkxDDAKBgNVBAoMA2FhYTEMMAoGA1UECwwDYmJiMRAwDgYDVQQDDAdj\r\n"
        "Y2MuY29tMRwwGgYJKoZIhvcNAQkBFg1jY2NAYmFpZHUuY29tMIIBIjANBgkqhkiG\r\n"
        "9w0BAQEFAAOCAQ8AMIIBCgKCAQEA+NN8s4UcunTywnSzNURVV1BslZPMWjERGbbe\r\n"
        "gF5fTZhFgLgKF96QQvWe88v9+VcI38T1jE0TDOfHRKxg2f3kGhGvvTbSpfEaDJb3\r\n"
        "bbAGbqn2ie7FNXh4h2JXWDmLOl4OYEGcvDZlXPAQrGznWMb0r3WHXaIDs3akTimT\r\n"
        "QEM3xSaP0B90PwpmYY/1OEMnL09C0TOxQ64aguLz6hJOGpr+nR/5w6cpSEtFUH6K\r\n"
        "emjEsxS5CQcP8Mq5xiLTxy0VyjwVFOM+y+9yi3M6AcN/XhG/mvD2q8gd0zAiFghR\r\n"
        "rZJp5eCTH6oBwP7/Izofh5DSR0OidjNZ6Z4ih4IaC/xS49Q6iwIDAQABMA0GCSqG\r\n"
        "SIb3DQEBCwUAA4IBAQBs06nY6onfPXg1gU2/7VIiqD7VOxVogYPwySGqWo84VyXi\r\n"
        "DhVr056X6LGMPJUtMVRZqFYJrWlxR9sZ7icFadNxMsJpJf/ZYXZVCM7wvp443Jm+\r\n"
        "LTcTyC6oqalgeMAEUBLC+iYxa4yb0uT0IVRuMQe/RlfR3wKI8ML2FhxThS0ZiPKS\r\n"
        "I/vtMrdGHKulGJDk+Z+lTjHVjHFgDsUeQfr/dCWtPyRV1I6M7dYsDd2zIcAIrNX4\r\n"
        "qU3selIIpH5WKlEi8uluTxB8Wy3vW3STHvFtn/PkWkW9utSkJ3uSHWqG0T1lGRtI\r\n"
        "0OcE5PA4o5hl5jkGYyhq10L3IpOahhTBUUVvh3BQ\r\n"
        "-----END CERTIFICATE-----\r\n";

...

void ota_updater_cb(OtaUpdater* apt, OtaUpdaterCbSt st, void* userData)
{
    bool reboot = false;

    switch (st) {
    case OTA_UPDATER_CB_ERR:
        printf("OTA_UPDATER_CB_ERR!\n");
        break;
	case OTA_UPDATER_CB_FW_ALREADY_NEW:
        printf("OTA_UPDATER_CB_FW_ALREADY_NEW\n");
        break;
    case OTA_UPDATER_CB_CHECKSUM_ERR:
        printf("OTA_UPDATER_CB_CHECKSUM_ERR\n");
        break;
	case OTA_UPDATER_FINISH:
/* 升级成功 会在此回调出来 */
        printf("OTA_UPDATER_FINISH\n");
        reboot = true;
        break;
    default:
        break;
    }

/* 对于各种正常/异常退出的情况，我们都结束升级 */
    ota_updater_end(apt, reboot);
    ota_updater_destroy(apt);
}

int iothub_mqtt_client_run(void)
{
    ...
/* 创建 OtaUpdater 对象 */
    if (!(apt = ota_updater_create(DEVICE, CURRENT_VER, OTA_BIN_DM_SHA256RSA, client_ota_cert, ota_updater_cb, clientHandle))) {
        printf("ota_updater_create() fail\n");
        return __FAILURE__;
    }
    ...
}


void on_recv_callback(MQTT_MESSAGE_HANDLE msgHandle, void* context)
{
/* 这个函数是mqtt接收数据的总入口函数，在这里做过滤。过滤出ota数据。 */
    // first filter the ota data flow
    ota_updater_data_filter(apt, topic_name, data, data_len);

    // user logic here ...
}
```
