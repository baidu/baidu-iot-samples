本文档介绍了ota升级的统一接口。用户只需要添加`ota_mq.h`到自己的工程目录中，并且实现相应xxx平台的`ota_mq_xxx.c`。在该文件中  
实现下述各个接口，即可实现 ota 功能。

## ota mq API接口
参考 `ota_mq.h`

- `OtaMqHandler* get_ota_mq_handler();`   
获取 OtaMqHandler 实例，该实例中提供了 ota 升级所涉及到的所有具体接口，需用户实现。

    - `int (*ota_mq_begin)(void **ctx);`  
        用户实现的 ota_mq_begin 接口函数，ctx参数是用户传入的，通常用户需要保存一些升级过程中的状态信息，缓存结构等等。  
        可以在改函数中统一分配上述全局结构，初始化状态等等。

    - `int (*ota_mq_write_data)(void *ctx, uint8_t *data, size_t data_len);`
        用户实现的 ota_mq_write_data 接口函数，用来向ota partition 中写入二进制数据。用户必须实现相关的逻辑判断  
        当前数据是否已经下载完成，以及校验工作。

    - `int (*ota_mq_end)(void *ctx, bool reboot);`
        用户实现的 ota_mq_end 接口函数，如果判断二进制数据下载完成，并且校验通过，同时写入操作都正确的话，就可以结束  
        ota 升级操作，释放ctx结构，相关缓存以及状态信息，第二个参数表示是否重启系统。如果重启，用户必须有相关逻辑指示   
        下次启动的ota分区是哪一块。

## ota mq Sample
在 sample_esp32 目录下，提供了在 esp32 平台下 ota mq 接口的一种实现，以供参考。