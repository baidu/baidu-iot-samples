本文档介绍了ota升级的统一接口。用户只需要添加`ota_mq.h`到自己的工程目录中，并且实现相应xxx平台的`ota_mq_xxx.c`。在该文件中  
实现下述各个接口，即可实现 ota 功能。

## ota mq API接口
参考 `ota_mq.h`

- `OtaMqHandler* get_ota_mq_handler();`   
获取 OtaMqHandler 实例，该实例中提供了 ota 升级所涉及到的所有具体接口，需用户实现。

    - `int (*ota_mq_begin)(void **ctx);`  
        用户实现的 ota_mq_begin 接口函数，ctx参数是用户传入的，在该函数中为 ctx 分配内存。因为通常在升级过程中，用户需要  
        维护一些的状态信息，缓存结构等等。这些结构都可以存放在 ctx 全局结构中。

    - `int (*ota_mq_write_data)(void *ctx, uint8_t *data, size_t data_len);`
        用户实现的 ota_mq_write_data 接口函数，用来向ota partition 中写入二进制数据。可能会涉及 flash 读写操作。

    - `int (*ota_mq_end)(void *ctx, bool reboot);`
        用户实现的 ota_mq_end 接口函数，如果判断二进制数据下载完成，同时写入操作都正确的话，就可以结束  
        ota 升级操作，释放ctx结构，相关缓存以及状态信息，第二个参数表示是否立刻重启系统。

## ota mq 示例
在 esp32 目录下有一个`ota_mq_esp32.c`。提供了在 esp32 平台下 ota mq 接口的一种实现，以供参考。同时提供了一个通过连接到 baidu iot hub  
实现 ota 升级的具体demo.