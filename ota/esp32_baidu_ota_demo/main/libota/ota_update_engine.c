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
#include "ota_update_engine.h"

#include "mbedtls/md5.h"
#include "mbedtls/sha256.h"
#include "mbedtls/platform.h"
#include "mbedtls/ecdsa.h"
#include "mbedtls/pk.h"
#include "mbedtls/base64.h"
#include "mbedtls/x509_crt.h"

#include "parson.h"
#include "iothub_mqtt_client.h"


#define 	OTA_BIN_REQ_TOPIC_FMT		"$baidu/iot/ota/esp32/%s/bin_req"
#define 	OTA_BIN_RSP_TOPIC_FMT		"$baidu/iot/ota/esp32/%s/bin_rsp"
#define		OTA_REQ_TOPIC_FMT			"$baidu/iot/ota/esp32/%s/ota_req"
#define		OTA_META_DATA_FMT			"$baidu/iot/ota/esp32/%s/meta"

struct _OtaUpdater {
    HalOtaHandler* hal_ota_handler;
	void *ota_ctx;

	/* dm5 */
	mbedtls_md5_context md5_ctx;

	/* sha256 */
	mbedtls_sha256_context sha256_ctx;

	char *cur_version;
	OtaSigMethod sig_method;
	char *signature;
	size_t fw_size;
	size_t remain_len;
	char *client_ota_cert;
	
	unsigned int expect_piece_start;
	unsigned int expect_piece_end;

	char *ota_req_topic;
	char *bin_rsp_topic;

	char *bin_req_topic;
	char *ota_meta_topic;
	
	bool in_ota_process;

	OtaUpdaterCb cb;
	void *userData;
};

typedef struct {
	unsigned int beg;
	unsigned int end;
} ReqHeader;

typedef struct {
	unsigned int beg;
	unsigned int end;
} PieceHeader;

static int processSubAckFunction(QOS_VALUE* qosReturn, size_t qosCount, void *context) 
{
    LogInfo("receive ota suback from hub server\r\n");
    for (int i =0; i< qosCount; ++i) {
        LogInfo("qos return: %d\r\n", qosReturn[i]);
    }
    return 0;
}


static char* generate_topic(const char* format, const char* device)
{
    if (!format || !device) {
        LogError("Failure: both format and device should not be NULL.");
        return NULL;
    }
    size_t size = strlen(format) + strlen(device) + 1;
    char* topic = malloc(sizeof(char) * size);
    if (NULL == topic) return NULL;
    if (snprintf(topic, size, format, device) < 0) {
        free(topic);
        topic = NULL;
    }
    
    return topic;
}


OtaUpdater *ota_updater_create(const char *puid, const char *cur_ver, OtaSigMethod sig_method, const char *client_ota_cert, OtaUpdaterCb cb, void* userData)
{
    int result;
    IOTHUB_MQTT_CLIENT_HANDLE clientHandle;
    OtaUpdater *apt;

    clientHandle = (IOTHUB_MQTT_CLIENT_HANDLE)userData;
    if (!cur_ver || !cb) {
        return NULL;
    }
    if (!(apt = (OtaUpdater*)malloc(sizeof(OtaUpdater)))) {
        return NULL;
    }
    memset(apt, 0, sizeof(OtaUpdater));
    apt->cur_version        = strdup(cur_ver);
    apt->sig_method         = sig_method;
    apt->expect_piece_start = 0;
    apt->expect_piece_end   = OTA_UPDATE_PIECE_SEG - 1;
    apt->cb                 = cb;
    apt->userData           = userData;
    apt->hal_ota_handler    =  get_hal_ota_handler();

    if (!apt->hal_ota_handler) {
        goto ERR_FREE_VER;
    }
    apt->bin_rsp_topic = generate_topic(OTA_BIN_RSP_TOPIC_FMT, puid);
    if (!apt->bin_rsp_topic) {
        goto ERR_FREE_VER;
    }
    apt->ota_req_topic = generate_topic(OTA_REQ_TOPIC_FMT, puid);
    if (!apt->ota_req_topic) {
        goto ERR_FREE_BIN_RESP;
    }
    apt->bin_req_topic = generate_topic(OTA_BIN_REQ_TOPIC_FMT, puid);
    if (!apt->bin_req_topic) {
        goto ERR_FREE_OTA_REQ;
    }
    apt->ota_meta_topic = generate_topic(OTA_META_DATA_FMT, puid);
    if (!apt->ota_meta_topic) {
        goto ERR_FREE_BIN_REQ;
    }
    
    SUBSCRIBE_PAYLOAD subscribe[2];
    subscribe[0].subscribeTopic = apt->bin_rsp_topic;
    subscribe[0].qosReturn      = DELIVER_AT_LEAST_ONCE;
    subscribe[1].subscribeTopic = apt->ota_req_topic;
    subscribe[1].qosReturn      = DELIVER_AT_LEAST_ONCE;
    subscribe_mqtt_topics(clientHandle, subscribe, sizeof(subscribe)/sizeof(SUBSCRIBE_PAYLOAD), processSubAckFunction, NULL);
    
    // pub meta data
    JSON_Value* request = json_value_init_object();
    JSON_Object* root   = json_object(request);
    json_object_set_string(root, "platform", CURRENT_PLATFORM);
    json_object_set_string(root, "version", apt->cur_version);
    json_object_set_number(root, "sig_method", apt->sig_method);
    json_object_set_number(root, "seg_len", OTA_UPDATE_PIECE_SEG);

    char* encoded = json_serialize_to_string(request);
    if (!encoded) {
        goto ERR_FREE_OTA_META;
    }

    result = publish_mqtt_message(clientHandle, apt->ota_meta_topic, DELIVER_AT_LEAST_ONCE, (uint8_t*)encoded, strlen(encoded), NULL, NULL);
    if (result) {
        LogError("Failed to publish meta data");
        json_free_serialized_string(encoded);
        goto ERR_FREE_OTA_META;
    }

    json_free_serialized_string(encoded);
    if (apt->sig_method == OTA_BIN_DM_MD5) {
        mbedtls_md5_init(&apt->md5_ctx);
        if (mbedtls_md5_starts_ret(&apt->md5_ctx)) {
            LogError("mbedtls_md5_starts_ret() error");
            mbedtls_md5_free(&apt->md5_ctx);
            goto ERR_FREE_OTA_META;
        }
    } else if (apt->sig_method == OTA_BIN_DM_SHA256RSA) {
        if (!client_ota_cert) {
            LogError("client ota cert is null");
            goto ERR_FREE_OTA_META;
        }
        apt->client_ota_cert = client_ota_cert;
        mbedtls_sha256_init(&apt->sha256_ctx);
        if (mbedtls_sha256_starts_ret(&apt->sha256_ctx, 0)) {
            LogError("mbedtls_sha256_starts_ret() error");
            mbedtls_sha256_free(&apt->sha256_ctx);
            goto ERR_FREE_OTA_META;
        }
    } else {
        LogError("not implemented\n");
        goto ERR_FREE_OTA_META;
    }

    return apt;
ERR_FREE_OTA_META:
    free(apt->ota_meta_topic);
ERR_FREE_BIN_REQ:
    free(apt->bin_req_topic);
ERR_FREE_OTA_REQ:
    free(apt->ota_req_topic);
ERR_FREE_BIN_RESP:
    free(apt->bin_rsp_topic);
ERR_FREE_VER:
    free(apt->cur_version);
    free(apt);
    return NULL;
}


static int compare_version(const char *cur_ver, const char *fw_ver)
{
    char curs[10];
    int cur;
    char fws[10];
    int fw;
    char *tmp;
    
    
    int i;
    char *ver_str;
    char* cur_ver_str;
    
    ver_str = (char *)fw_ver;
    cur_ver_str = (char *)cur_ver;
    
    for(i = 0; i < 2; i++) {
        tmp = strchr(ver_str, '.');
        if(NULL == tmp) {
            LogError("don't upgrade\n");
			return 0;
        }
        memcpy(fws, ver_str, tmp - ver_str);
        fws[tmp - ver_str] = '\0';
        fw = atoi(fws);
		LogInfo("fw %d\n", fw);
        ver_str = tmp + 1;
        
        
        tmp = strchr(cur_ver_str, '.');
        if(NULL == tmp) {
            LogError("do not update\n");
			return 0;   
        }
        memcpy(curs, cur_ver_str, tmp - cur_ver_str);
        curs[tmp - cur_ver_str] = '\0';
        cur = atoi(curs);    
		LogInfo("cur %d\n", cur);
        cur_ver_str = tmp + 1;
        if(fw > cur) {
			LogInfo("update\n");
            return 1;
        } else if(fw == cur) {
			LogInfo("continue\n");
		} else {
			LogInfo("do not update\n");
            return 0;			
		}
    } 
    fw = atoi(ver_str);
    cur = atoi(cur_ver_str);  
	LogInfo("fw %d\n", fw);
	LogInfo("cur %d\n", cur);
    if(fw > cur) {
		LogInfo("update\n");
        return 1;
    } else {
		LogInfo("do not update\n");
		return 0;   		
	}
}

static uint32_t u32_bytes_swap(uint32_t net)
{
    uint8_t data[4] = {};
    memcpy(&data, &net, sizeof(data));

    return ((uint32_t) data[3] << 0)
         | ((uint32_t) data[2] << 8)
         | ((uint32_t) data[1] << 16)
         | ((uint32_t) data[0] << 24);
}

static int parse_piece_header(unsigned char *data, int len, PieceHeader* header)
{
    if (len <= PIECE_HEADER_LEN) {
        LogError("invalid piece\n");
        return -1;
    }
    /* 4 Bytes: recv start offset */
    header->beg = u32_bytes_swap(*((unsigned int*)data));
    data += 4;
    /* 4 Bytes: recv end offset */
    header->end = u32_bytes_swap(*((unsigned int*)data));
    
    return 0;
}


static int pub_req_piece_ack_process(MQTT_PUB_STATUS_TYPE status, void* context)
{
    if (status == MQTT_PUB_SUCCESS) {
        LogInfo("got req piece pub ack\r\n\r\n");
    }
    else {
        LogInfo("fail to got req piece pub ack\r\n");
    }

    return 0;
}

static unsigned char * create_req_header_buf_needfree(unsigned int expect_start, unsigned int expect_end)
{
    unsigned char *buf, *p;

    if (!(buf = p = malloc(REQ_HEADER_LEN))) {
        return NULL;
    }
    memset(buf, 0, REQ_HEADER_LEN);
    
    /* 4 Bytes: expect start offset */
    *((unsigned int *)p) = u32_bytes_swap(expect_start);
    p += 4;
    /* 4 Bytes: expect end offset */
    *((unsigned int *)p) = u32_bytes_swap(expect_end);
    
    return buf;
}

static int ota_updater_req_next_piece(OtaUpdater* apt)
{
    IOTHUB_MQTT_CLIENT_HANDLE clientHandle;
    unsigned char *reqBuf;
    int result;
    
    clientHandle =  (IOTHUB_MQTT_CLIENT_HANDLE)apt->userData;
    if (!(reqBuf = create_req_header_buf_needfree(apt->expect_piece_start, apt->expect_piece_end))) {
        LogError("create_req_header_buf_needfree() failed\n");
        return -1;
    }

    result = publish_mqtt_message(clientHandle, apt->bin_req_topic, DELIVER_AT_LEAST_ONCE, (const uint8_t*)reqBuf,
                                      REQ_HEADER_LEN, pub_req_piece_ack_process , clientHandle);
    if (result) {
        LogError("publist mqtt message failed\n");
    }
    
    free(reqBuf);
    return 0;    
}


int ota_updater_run(OtaUpdater* apt)
{
    if (apt->hal_ota_handler->hal_ota_begin(&apt->ota_ctx)) {
        LogError("hal_ota_begin() error\n");
        return -1;
    }
    
    return ota_updater_req_next_piece(apt);
}

int ota_updater_consum_data(OtaUpdater* apt, uint8_t *data, size_t data_len);


int ota_updater_data_filter(OtaUpdater* apt, const char *topic_name, uint8_t *data, size_t data_len)
{
    JSON_Value *req;
    JSON_Object *root;
    const char *ver_str, *signature;
    int total_len;

    if (!apt || !topic_name) {
        return -1;
    }
    
    if (!strcmp(topic_name, apt->ota_req_topic)) {
        LogInfo("got ota req message!\n");
        if (apt->in_ota_process) {
            LogError("aready in ota, pass\n");
            return 0;
        }

        if (!(req = json_parse_string((const char *)data))) {
            LogError("json_parse_string() error\n");
            return -1;
        }
        if (!(root = json_object(req))) {
            LogError("json_object() error\n");
            json_value_free(req);
            return -1;
        }

        total_len = json_object_get_number(root, "size");
        ver_str   = json_object_get_string(root, "version");
        signature = json_object_get_string(root, "signature");
        if (!total_len || !ver_str || !signature) {
            json_value_free(req);
            return -1;
        }
        if (compare_version(apt->cur_version, ver_str) == 0) {
            json_value_free(req);
            apt->cb(apt, OTA_UPDATER_CB_FW_ALREADY_NEW, apt->userData);
            return -1;
        }
        apt->remain_len = apt->fw_size = total_len;
        apt->signature  = strdup(signature);
        json_value_free(req);

        // start ota!
        if (ota_updater_run(apt) < 0) {
            LogError("ota_updater_run() fail\n");
            apt->cb(apt, OTA_UPDATER_CB_ERR, apt->userData);
            return -1;
        }
        apt->in_ota_process = 1;

    } else if (!strcmp(topic_name, apt->bin_rsp_topic)) {
        if (!apt->in_ota_process) {
            return 0;
        }
        if (ota_updater_consum_data(apt, data, data_len) < 0) {
            LogError("ota_updater_consum_data() fail\n");
            return -1;
        }
        
    } else {
        //bypass
    }
    return 0;
}


static void hex_to_decimal(const char *hex_string, unsigned char *arr, size_t arr_len)
{
    const char *pos = hex_string;
    int count;

    for (count = 0; count < arr_len; count++) {
        sscanf(pos, "%2hhx", &arr[count]);
        pos += 2;
    }
}


static int Base64Decode(char* b64message, unsigned char** buffer, size_t* length) 
{
    *buffer = malloc(sizeof(char) * 1000);
    unsigned char* tmp = *buffer;
    int ret = mbedtls_base64_decode(*buffer, 1000, length, (const unsigned char *)b64message, strlen(b64message));
    if (ret != 0) {
        free(*buffer);
        return -1;
    }
    tmp[*length] = '\0';
    return 0;
}

int ota_updater_consum_data(OtaUpdater* apt, uint8_t *data, size_t data_len)
{
    int len, ret;
    unsigned char md5_cur[16];
    unsigned char sha256_cur[32];
    unsigned char md5_fw[16];
    PieceHeader header;
    unsigned char* sig_buf;
    size_t sig_len;
    
    if (!apt->cb) {
        return -1;
    }
    
    if (parse_piece_header(data, data_len, &header)) {
        return -1;
    }
    if (header.beg != apt->expect_piece_start || header.end != apt->expect_piece_end) {
        LogError("got piece start or end don't match expect\n");
        return 0;
    }
    
    data     += PIECE_HEADER_LEN;
    data_len -= PIECE_HEADER_LEN;

    len = apt->expect_piece_end - apt->expect_piece_start + 1;
    if (data_len < len) {
        LogError("not enough remaining length\n");
        return 0;
    }
    
    if (apt->sig_method == OTA_BIN_DM_MD5) {
        if (mbedtls_md5_update_ret(&apt->md5_ctx, data, len)) {
            LogError("mbedtls_md5_update_ret() failed\n");
            return -1;
        }
    } else if (apt->sig_method == OTA_BIN_DM_SHA256RSA) {
        if (mbedtls_sha256_update_ret(&apt->sha256_ctx, data, len)) {
            LogError("mbedtls_sha256_update_ret() failed\n");
            return -1;
        }
    } else {
        LogError("not implemented\n");
        return -1;
    }

    if (apt->hal_ota_handler->hal_ota_write_data(apt->ota_ctx, data, len) < 0) {
        LogError("hal_ota_write_data() error\n");
        apt->cb(apt, OTA_UPDATER_CB_ERR, apt->userData);
        return -1;
    }
    LogInfo("esp_ota_write success, progress: %d%%\n", 100 - 100 * apt->remain_len / apt->fw_size);
    apt->remain_len -= len;
    apt->expect_piece_start += OTA_UPDATE_PIECE_SEG;
    if (apt->remain_len > 0) {
        if (apt->remain_len >= OTA_UPDATE_PIECE_SEG) {
            apt->expect_piece_end += OTA_UPDATE_PIECE_SEG;
        } else {
            apt->expect_piece_end += apt->remain_len;
        }
        
        return ota_updater_req_next_piece(apt);
    } else if (apt->remain_len == 0) {
        LogInfo("ota download finish!\n");
        if (apt->sig_method == OTA_BIN_DM_MD5) {
            if (mbedtls_md5_finish_ret(&apt->md5_ctx, md5_cur)) {
                LogError("mbedtls_md5_finish_ret() error\n");
                apt->cb(apt, OTA_UPDATER_CB_CHECKSUM_ERR, apt->userData);
            }
            //hex to decimal array
            hex_to_decimal(apt->signature, md5_fw, sizeof(md5_fw));
            // verify
            if (!memcmp(md5_fw, md5_cur, sizeof(md5_fw))) { 
                LogInfo("md5 check sum successfylly!\n");
                apt->cb(apt, OTA_UPDATER_FINISH, apt->userData);
            } else {
                LogInfo("md5 check sum fail!\n");
                apt->cb(apt, OTA_UPDATER_CB_CHECKSUM_ERR, apt->userData);
            }
        } else if (apt->sig_method == OTA_BIN_DM_SHA256RSA) {
            if (mbedtls_sha256_finish_ret(&apt->sha256_ctx, sha256_cur)) {
                LogError("mbedtls_sha256_finish_ret() error\n");
                apt->cb(apt, OTA_UPDATER_CB_CHECKSUM_ERR, apt->userData);
            }
            // Base64 decode
            if (Base64Decode((char *)apt->signature, &sig_buf, &sig_len)) {
                LogError("Base64Decode() error");
                apt->cb(apt, OTA_UPDATER_CB_CHECKSUM_ERR, apt->userData);
            }

            // verify
            mbedtls_pk_context pk;
            mbedtls_pk_init( &pk );
            mbedtls_x509_crt chain;
            mbedtls_x509_crt_init( &chain );

            if (mbedtls_x509_crt_parse( &chain, (const unsigned char *)apt->client_ota_cert, strlen(apt->client_ota_cert) + 1)) {
                LogError("mbedtls_x509_crt_parse() fail");
                free(sig_buf);
                mbedtls_x509_crt_free(&chain);
                mbedtls_pk_free(&pk);
                apt->cb(apt, OTA_UPDATER_CB_CHECKSUM_ERR, apt->userData);
            }

            mbedtls_rsa_context* rsa = mbedtls_pk_rsa(chain.pk);

            ret = mbedtls_rsa_pkcs1_verify( rsa, NULL, NULL, MBEDTLS_RSA_PUBLIC,
                                                MBEDTLS_MD_SHA256, 20, sha256_cur, sig_buf );
            if (ret == 0) { 
                LogInfo("SHA256RSA check sum successfylly!\n");
                apt->cb(apt, OTA_UPDATER_FINISH, apt->userData);
            } else {
                LogInfo("SHA256RSA check sum fail!\n");
                apt->cb(apt, OTA_UPDATER_CB_CHECKSUM_ERR, apt->userData);
            }

            free(sig_buf);
            mbedtls_x509_crt_free(&chain);
            mbedtls_pk_free(&pk);
            mbedtls_rsa_free( rsa );

        } else {
            LogError("not implemented\n");
            apt->cb(apt, OTA_UPDATER_CB_CHECKSUM_ERR, apt->userData);
        }

    
    } else {
        LogError("downloading errorl!\n");
        apt->cb(apt, OTA_UPDATER_CB_CHECKSUM_ERR, apt->userData);
    }
    
    return 0;
}

int ota_updater_end(OtaUpdater* apt, bool reboot)
{
    if (!apt->cb) {
        return -1;
    }
    if (apt->in_ota_process) {
        if (apt->hal_ota_handler->hal_ota_end(apt->ota_ctx, reboot) < 0) {
            LogError("hal_ota_end() error\n");
        }
    }
    
    return 0;
}


void ota_updater_destroy(OtaUpdater* apt)
{
    if (apt->sig_method == OTA_BIN_DM_MD5) {
        mbedtls_md5_free(&apt->md5_ctx);
    } else if (apt->sig_method == OTA_BIN_DM_SHA256RSA) {
        mbedtls_sha256_free(&apt->sha256_ctx);
    }
    if (apt->cur_version) {
        free(apt->cur_version);
    }
    if (apt->signature) {
        free(apt->signature);
    }
    if (apt->ota_req_topic) {
        free(apt->ota_req_topic);
    }
    if (apt->bin_rsp_topic) {
        free(apt->bin_rsp_topic);
    }
    if (apt->bin_req_topic) {
        free(apt->bin_req_topic);
    }
    if (apt->ota_meta_topic) {
        free(apt->ota_meta_topic);
    }
    free(apt);
}