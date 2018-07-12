# 前言
本文介绍如何在esp32平台上通过消息订阅/发布的方式实现 OTA 升级。大部分内容摘录自 espressif 官方文档。

- esp32分区表：http://esp-idf-zh.readthedocs.io/zh_CN/latest/api-guides/partition-tables.html
- esp32 OTA升级接口：
http://esp-idf-zh.readthedocs.io/zh_CN/latest/api-reference/system/ota.html

# 概述
## 分区表
单个 ESP32 flash 可以包含多个应用程序，以及多种数据（校验数据、文件系统、参数存储器等）。基于这个原因，在 flash 的偏移地址`0x8000`处烧写了一个分区比表。

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

#### **内置分区表**
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

#### **自定义分区表**
如果你在配置菜单中选择 “Custom partition table CSV”，你需要输入用于保存你的分区表的 CSV 文件的名字（在工程目录中）。CSV 可以根据你的需要描述任意数量的定义。具体的格式以及编写方法，请参考官方文档。

## OTA升级
#### OTA 过程概述
OTA 升级机制允许常规固件在运行时基于它所接收的数据对设备进行升级（通过 WiFI 或者蓝牙）。

OTA 需要配置设备的 Partition Table，且至少需要两个 “OTA app” 分区（即 ota_0 和 ota_1）和一个 OTA 数据分区。

OTA 会将新的 app 固件镜像写到当前未用于启动程序的那个 OTA app 分区。当镜像校验完成后，OTA 数据分区会被更新，表示下一次启动时将使用该镜像。

#### OTA 数据分区
使用 OTA 功能的产品必须在 Partition Table 中包含一个 OTA 数据分区。

`对于工厂启动设置，OTA 数据分区应当不包含数据（所有的字节被擦除为 0xFF）。`在这种情况下，如果分区表中存在工厂 app，esp-idf 软件的 bootloader 会启动工厂 app。如果分区表中不存在工厂 app，则会启动第一个有效的 OTA 分区（通常是 ota_0）。

当第一次 OTA 更新后，OTA 数据分区将会被更新，表示表示下一次启动时将使用哪个 OTA app 分区。

OTA 数据分区是两个 flash 扇区（0x2000 字节），以消除正在写时供电失败的问题。如果没有计数字段表明哪个扇区在最近被写过，则两个扇区会被独立擦除并写入匹配的数据。


# 实现ota_mq API接口
```c
// ota_mq_esp32.c
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event_loop.h"
#include "esp_log.h"
#include "esp_ota_ops.h"
#include "esp_partition.h"

#include <ota_mq.h>


typedef struct{
    esp_partition_t *update_partition;
    esp_ota_handle_t update_handle;
} esp32_ota_ctx;


static int esp32_ota_mq_begin(void **_ctx)
{
    esp32_ota_ctx *ctx;
    int err;

    if (!_ctx) {
        return -1;
    }
    ctx = *_ctx = (esp32_ota_ctx*)malloc(sizeof(esp32_ota_ctx)); //升级过程中的全局变量，用于保存状态信息
    if (NULL == ctx) {
        printf("malloc esp32 ctx error\n");
        return -1;
    }

    ctx->update_partition = esp_ota_get_next_update_partition(NULL);//获得下一个 ota partition 对象
    printf("Writing to partition subtype %d at offset 0x%x\n",
            ctx->update_partition->subtype, ctx->update_partition->address);
    assert(ctx->update_partition != NULL);

    printf("5\n");
    /* update handle : set by esp_ota_begin(), must be freed via esp_ota_end() */
    err = esp_ota_begin(ctx->update_partition, OTA_SIZE_UNKNOWN, &ctx->update_handle);//开始 ota 升级
    if (err != ESP_OK) {
        printf("esp_ota_begin failed, error=%d\n", err);
        free(ctx);
        return -1;
    }
    printf("esp_ota_begin succeeded\n");
    return 0;
}

static int esp32_ota_mq_write_data(void *_ctx, uint8_t *data, size_t data_len)
{
    esp32_ota_ctx *ctx;
    int err;

    ctx = (esp32_ota_ctx *)_ctx;
    err = esp_ota_write(ctx->update_handle, (const void *)data, data_len);//写 ota 二进制数据到 ota partition
    if (err != ESP_OK) {
        printf("Error: esp_ota_write failed! err=0x%x", err);
        if (esp_ota_end(ctx->update_handle) != ESP_OK) { //如果写出错，也结束 ota 升级
            printf("esp_ota_end failed!\n");
        }
        free(ctx);
        return -1;
    }
    printf("free heap size: %d\n", system_get_free_heap_size());
    return data_len;
}

static int esp32_ota_mq_end(void *_ctx, bool reboot)
{
    esp32_ota_ctx *ctx;
    int err;

    ctx = (esp32_ota_ctx *)_ctx;
    if (esp_ota_end(ctx->update_handle) != ESP_OK) { //结束 ota 升级，释放缓存
        printf("esp_ota_end failed!\n");
        free(ctx);
        return -1;
    }
    err = esp_ota_set_boot_partition(ctx->update_partition);//设置下一个 ota partition 为下一次启动的分区
    if (err != ESP_OK) {
        printf("esp_ota_set_boot_partition failed! err=0x%x", err);
        free(ctx);
        return -1;
    }
    if (reboot) {
        printf("Prepare to restart system!\n");
        esp_restart();
    }
    printf("esp32_ota_mq_end success!\n");
    free(ctx);
    return 0;
}


OtaMqHandler esp32_ota_mq_handler = {
    esp32_ota_mq_begin,
    esp32_ota_mq_write_data,
    esp32_ota_mq_end,
};


OtaMqHandler* get_ota_mq_handler()
{
    return &esp32_ota_mq_handler;
}
```

# OTA demo 代码流程详细分析
与mqtt server 成功tls握手之后，成功连接到服务器。分别订阅`ota_for_test_profile`和`ota_for_test_bin`这两个主题，前者用来接收升级通知，后者用来接收ota数据。代码如下：
```c
    ...
    SUBSCRIBE_PAYLOAD subscribe[4];
    subscribe[0].subscribeTopic = TOPIC_NAME_A;
    subscribe[0].qosReturn = DELIVER_AT_MOST_ONCE;
    subscribe[1].subscribeTopic = TOPIC_NAME_B;
    subscribe[1].qosReturn = DELIVER_AT_MOST_ONCE;
    subscribe[2].subscribeTopic = "ota_for_test_profile";
    subscribe[2].qosReturn = DELIVER_AT_LEAST_ONCE; //至少一次
    subscribe[3].subscribeTopic = "ota_for_test_bin";
    subscribe[3].qosReturn = DELIVER_AT_LEAST_ONCE; //至少一次

    int flag = 0;
    subscribe_mqtt_topics(clientHandle, subscribe, sizeof(subscribe)/sizeof(SUBSCRIBE_PAYLOAD), processSubAckFunction, &flag);
    ...

```

在接收到pub消息的回调函数中，过滤 ota 相关的topic。注意，这块代码是应用层相关的。  
首先，应用程序需要获取 ota bin 相关的信息，比如 md5校验码，bin 大小，版本号等等。  
在实际应用中，不限于用 mqtt 订阅的方式获取 ota bin 相关的信息，比如，可以通过 http 的方式获取。  
另一方面，由于固件一般都比较大，如果采用分片发送的方式，每一片中可能还需要加入用户数据，比如，  
该片的序号等等。在这个例子中，使用开始的4个字节，代表片的序号。从第5个字节开始，才是真正的数据。   
鉴于此，另外封装一层 ota_updater 层。用来统一各种操作。该层是通用的，可以满足大部分用户的升级需求。用户可以直接引用 ota_updater.c ota_updater.h到自己的工程中编译。  
使用方法如下：
- 当用户通过某种途径得到固件的md5校验码，bin 大小，版本号等信息，可以依次调用以下函数序列，开始ota升级：
    - ota_updater_init(...)           # 初始化 ota_updater 层，提供一个回调函数，用来接收在 ota 升级过程中的各种错误通知。
    - ota_updater_set_fw_version(...) # 设置固件版本号，格式如“xx.xx.xx”形式，如果设置的版本号小于等于当前版本，则会通过回调函数通知应用层。
    - ota_updater_set_fw_size(...)    # 设置固件大小
    - ota_updater_set_md5_chksum(...) # 设置固件md5 checksum 字符串，即用命令`md5sum`得到的输出。
                                      # 如果校验失败，则会通过回调函数通知应用层。
                                      # 本程序默认提供md5校验算法，如果用户使用的是其他算法，只需要将  
                                      # 传入的参数，即md5 checksum 字符串，设置为 NULL即可  
                                      # 用户便可以在上层做自己的校验。  
- 当用户通过某种途径得到固件的二进制数据，直接调用`ota_updater_eat_data(...)`即可。
- 其他细节，可以参考具体源代码。

```c
void ota_updater_cb(OtaUpdater* apt, OtaUpdaterCbSt st, void* userData)
{
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
        printf("OTA_UPDATER_FINISH\n");
        ota_updater_end(apt, true);
        return;
    default:
        break;
    }

    expect_piece = 0;
    recved_profile = 0;
    ota_updater_end(apt, false);
}

static int prepare_ota(const char *ver_str, size_t total_len, const char *md5_str)
{
    //初始化  OtaUpdater
    if (ota_updater_init(&apt, CURRENT_VER, ota_updater_cb, &apt) < 0) {
        printf("ota_updater_init() fail\n");
        goto ERR_OUT;
    }

    if (ota_updater_set_fw_version(&apt, ver_str) < 0) {
        printf("ota_updater_set_fw_version() fail\n");
        goto ERR_DINIT_OTA;
    }

    if (ota_updater_set_fw_size(&apt, total_len) < 0) {
        printf("ota_updater_set_fw_size() fail\n");
        goto ERR_DINIT_OTA;
    }

    if (ota_updater_set_md5_chksum(&apt, md5_str) < 0) {
        printf("ota_updater_set_md5_chksum() fail\n");
        goto ERR_DINIT_OTA;
    }

    return 0;
ERR_DINIT_OTA:
    ota_updater_end(&apt, false);
ERR_OUT:
    return -1;
}

void on_recv_callback(MQTT_MESSAGE_HANDLE msgHandle, void* context)
{
    const char *topic_name, *ver_str, *md5_str;
    const APP_PAYLOAD* appMsg = mqttmessage_getApplicationMsg(msgHandle);
    IOTHUB_MQTT_CLIENT_HANDLE clientHandle = (IOTHUB_MQTT_CLIENT_HANDLE)context;
    STRING_HANDLE profile_str;
    JSON_Value* profile_jv;
    JSON_Object* root;
    unsigned char *data;
    size_t data_len;
    int total_len, index;

    topic_name = mqttmessage_getTopicName(msgHandle); //获取 topic 名字
    data = appMsg->message;         //获取 数据
    data_len = appMsg->length;      //获取 数据长度

    (void)printf("Incoming Msg: Packet Id: %d\r\nQOS: %s\r\nTopic Name: %s\r\nIs Retained: %s\r\nIs Duplicate: %s\r\nApp Msg: ", mqttmessage_getPacketId(msgHandle),
                 QosToString(mqttmessage_getQosType(msgHandle) ),
                 topic_name,
                 mqttmessage_getIsRetained(msgHandle) ? "true" : "fale",
                 mqttmessage_getIsDuplicateMsg(msgHandle) ? "true" : "fale"
    );

    (void)printf("\r\n");

    // when receive message is "stop", call destroy method to exit
    // trigger stop by send a message to topic "msgA" and payload with "stop"
    if (appMsg->length > 0 && strcmp((const char *)appMsg->message, "stop") == 0)
    {
        iothub_mqtt_disconnect(clientHandle);
    }
            
    if (!strcmp(topic_name, "ota_for_test_profile")) { //如果收到 OTA 升级通知
        if (recved_profile) { // 如果当前已经处于ota升级中，直接返回
            printf("aready in ota, pass profile\n");
            return;
        }
        if (!(profile_str = STRING_from_byte_array(data, data_len))) {
            printf("STRING_from_byte_array() error\n");
            return ;
        }
        // {"version":"1.2.3", "size":701342, "checksum": "9c7402e9c1968cac76566f9619b08068"}
        if (!(profile_jv = json_parse_string(STRING_c_str(profile_str)))) { //解析 json 字符串
            printf("json_parse_string() error\n");
            STRING_delete(profile_str);
            return;
        }
        root = json_object(profile_jv);
        if (!root) {
            printf("json_object() error\n");
            STRING_delete(profile_str);
            json_value_free(profile_jv);
            return;
        }

        printf("get profile: %s, %lf, %s\n", json_object_get_string(root, "version"),
            json_object_get_number(root, "size"), json_object_get_string(root, "checksum"));
        // 分别得到固件的“版本号” “大小” “checksum”
        total_len = json_object_get_number(root, "size");
        ver_str = json_object_get_string(root, "version");
        md5_str = json_object_get_string(root, "checksum");

        if (!total_len || !ver_str) {
            STRING_delete(profile_str);
            json_value_free(profile_jv);
            return;
        }
        

        if (prepare_ota(ver_str, total_len, md5_str) < 0) {
            printf("prepare_ota() fail\n");
            STRING_delete(profile_str);
            json_value_free(profile_jv);
            return;
        }

        STRING_delete(profile_str);
        json_value_free(profile_jv);

        recved_profile = 1;
        expect_piece = 0;
    } else if (!strcmp(topic_name, "ota_for_test_bin")) {
        if (!recved_profile) { //还没收到 ota 升级通知，忽略所有的ota数据
            printf("no ota profile, pass bin data\n");
            return;
        }
        // 获取开始的4字节数据，代表第几片
        index = (*data & 0xff) << 24 |  (*(data+1) & 0xff) << 16 |  (*(data+2) & 0xff) << 8 | (*(data+3) & 0xff);
        if (index != expect_piece) {
            printf("piece index don't match: %d %d\n", index, expect_piece);
            return;
        }
        printf("got piece index: %d\n", index);
        expect_piece++;
        data += 4;
        data_len -= 4;

        
        //写数据
        if (ota_updater_eat_data(&apt, data, data_len) < 0) {
            printf("ota_updater_eat_data() fail\n");
            ota_updater_end(&apt, false);
            expect_piece = 0;
            recved_profile = 0;
        }
        
    } else {
        //bypass
    }

}
```


# OTA demo 测试脚本
用户可以通过 mosquitto_pub 工具测试该demo程序，向指定的 topic 发送 pub 消息，消息的payload就是ota二进制文件的分片。在发送二进制数据之前，我们必须先pub  
一个profile信息，告诉客户端ota bin的具体信息，比如文件版本，大小，md5 checksum等等。
```bash
#!/bin/bash

SLEEP_TIME=0.05  # send interval: 0.05s
PIECE_SIZE=10k   # packet size: 10K


echo "prepare profile and pieces bins..."
BIN_FILE=/path/to/your/ota/bin/file/esp32.bin
mkdir -p pieces
rm -rf pieces/*

echo "{\"version\":\"12.34.57\", \"size\":$(wc -c < $BIN_FILE), \"checksum\": \"$(md5 $BIN_FILE | awk {'print $4'})\"}" > ota_profile
cat ota_profile
split -a 4  -b ${PIECE_SIZE}  $BIN_FILE  pieces/spl_

echo "sending profile..."
# 修改服务器域名以及用户名密码等等信息。这些信息的获取，可以登陆 `https://cloud.baidu.com` 创建百度云物接入实例，从而生成对应的服务器账号密码等信息。参考：`https://cloud.baidu.com/doc/IOT/GettingStarted.html`
mosquitto_pub -h "xxxxxx.mqtt.iot.xx.baidubce.com" -p 1883 \
    -t "ota_for_test_profile" \
    -f ota_profile \
    -u "xxxxxx/xxxx" -P "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx" \
    -d -i "xxxxxxxxxxxxxxxxxxxxxxxxxx" \
    -q 1


read -p "Press enter to start sending pieces..."

cnt=0
for filename in pieces/spl*; do
    echo "processing ${filename}"
    # 数据分片，在每一片开始的4个字节存放当前 ota piece的序号
    printf "%.8x" $cnt | xxd -r -p > tmp.bin
    cat ${filename} >> tmp.bin
    mosquitto_pub -h "xxxxxx.mqtt.iot.xx.baidubce.com" -p 1883 -t "ota_for_test_bin"  -u "xxxxxx/xxxx" \
                    -P "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx" -d \
                    -i "xxxxxxxxxxxxxxxxxxxxxxxxxx" \
                    -f tmp.bin \
                    -q 1

    cnt=$(expr $cnt + 1)
    sleep ${SLEEP_TIME}
    rm -rf tmp.bin
done
```
