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



#ifndef __BAIDU_IOT_HAL_OTA_H___
#define __BAIDU_IOT_HAL_OTA_H___

#include "stdbool.h"


typedef struct {
    /**
     * Begin ota update
     * @param[in, out] ctx user defined structure, used to stroing some state info
     *                  that will be used in ota operations.
     * @retval 0: success, -1: fail
     */
    int (*hal_ota_begin)(void **ctx);

    /**
     * Write binary data to ota partition in flash
     * @param[in] ctx      user defined structure, used to stroing some state info
     *                      that will be used in ota operations.
     * @param[in] data     ota binary data to write
     * @param[in] data_len data length need to write
     * @retval 0: success, -1: fail
     */
    int (*hal_ota_write_data)(void *ctx, unsigned char *data, int data_len);

    /**
     * End this ota operation, usually you need to do some cleaning here
     * @param[in] ctx      user defined structure, used to stroing some state info
     *                      that will be used in ota operations.
     * @param[in] reboot   flag indicates whether or not reboot system after finishing ota operation
     * @retval 0: success, -1: fail
     */
    int (*hal_ota_end)(void *ctx, bool reboot);
} HalOtaHandler;


/**
 * Get hal ota handler.
 * @retval HalOtaHandler if success, or NULL indicate failure
 */
HalOtaHandler* get_hal_ota_handler();

#endif