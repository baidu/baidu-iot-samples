/*
* Copyright (c) 2018 Baidu, Inc. All Rights Reserved.
*
* Licensed to the Apache Software Foundation (ASF) under one or more
* contributor license agreements.  See the NOTICE file distributed with
* this work for additional information regarding copyright ownership.
* The ASF licenses this file to You under the Apache License, Version 2.0
* (the "License"); you may not use this file except in compliance with
* the License.  You may obtain a copy of the License at
*
*    http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/
#include "azure_c_shared_utility/xlogging.h"

#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event_loop.h"
#include "esp_log.h"
#include "esp_ota_ops.h"
#include "esp_partition.h"
#include "lwip/inet.h"


#include "ota_hal.h"


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
    ctx = *_ctx = (esp32_ota_ctx*)malloc(sizeof(esp32_ota_ctx));
    if (NULL == ctx) {
        LogError("malloc esp32 ctx error\n");
        return -1;
    }

    ctx->update_partition = esp_ota_get_next_update_partition(NULL);
    LogInfo("Writing to partition subtype %d at offset 0x%x\n",
            ctx->update_partition->subtype, ctx->update_partition->address);
    assert(ctx->update_partition != NULL);

    err = esp_ota_begin(ctx->update_partition, OTA_SIZE_UNKNOWN, &ctx->update_handle);
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
    err = esp_ota_write(ctx->update_handle, (const void *)data, data_len);
    if (err != ESP_OK) {
        LogError("Error: esp_ota_write failed! err=0x%x", err);
        if (esp_ota_end(ctx->update_handle) != ESP_OK) {
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
    if ((err = esp_ota_end(ctx->update_handle)) != ESP_OK) {
        LogError("esp_ota_end failed!\n");
        free(ctx);
        return -1;
    }
    err = esp_ota_set_boot_partition(ctx->update_partition);
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