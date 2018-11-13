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


#ifndef __OTA_UPDATER_H__
#define __OTA_UPDATER_H__

#include "ota_hal.h"
#include <inttypes.h>


#define 	CURRENT_PLATFORM			"esp32"
#define		REQ_HEADER_LEN				16
#define 	PIECE_HEADER_LEN 			16
#define		OTA_UPDATE_PIECE_SEG		8192

typedef struct _OtaUpdater OtaUpdater;

typedef enum {
	OTA_UPDATER_CB_ERR = -1,
	OTA_UPDATER_CB_FW_ALREADY_NEW,
	OTA_UPDATER_CB_CHECKSUM_ERR,
	OTA_UPDATER_FINISH,
} OtaUpdaterCbSt;

typedef enum {
	OTA_BIN_DM_MD5,
	OTA_BIN_DM_SHA256RSA,
} OtaSigMethod;

typedef void(*OtaUpdaterCb)(OtaUpdater* apt, OtaUpdaterCbSt st, void* userData);


OtaUpdater * ota_updater_create(const char *puid, const char *cur_ver, OtaSigMethod sig_method, const char *client_ota_cert, OtaUpdaterCb cb, void* userData);
int ota_updater_data_filter(OtaUpdater* apt, const char *topic, uint8_t *bin_data, size_t data_len);
int ota_updater_end(OtaUpdater* apt, bool reboot);
void ota_updater_destroy(OtaUpdater* apt);

#endif