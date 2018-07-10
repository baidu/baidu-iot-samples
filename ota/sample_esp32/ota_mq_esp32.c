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
    ctx = *_ctx = (esp32_ota_ctx*)malloc(sizeof(esp32_ota_ctx));
    if (NULL == ctx) {
        printf("malloc esp32 ctx error\n");
        return -1;
    }

    ctx->update_partition = esp_ota_get_next_update_partition(NULL);//获得下一个 ota partition 对象
    printf("Writing to partition subtype %d at offset 0x%x\n",
            ctx->update_partition->subtype, ctx->update_partition->address);
    assert(ctx->update_partition != NULL);

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