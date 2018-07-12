/*
* Copyright (c) 2017 Baidu, Inc. All Rights Reserved.
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

#include <azure_c_shared_utility/platform.h>
#include <azure_c_shared_utility/utf8_checker.h>
#include <azure_c_shared_utility/threadapi.h>
#include "iothub_mqtt_client.h"
#include "iothub_mqtt_client_sample.h"

#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event_loop.h"
#include "esp_log.h"
#include "esp_ota_ops.h"
#include "esp_partition.h"

#include "nvs.h"
#include "nvs_flash.h"
#include "parson.h"
#include "ota_updater.h"

#define CURRENT_VER "12.34.57"

// Please set the mqtt client data and security which are shown as follow.
// The endpoint address, witch is like "xxxxxx.mqtt.iot.xx.baidubce.com".
#define         ENDPOINT                    "xxxxxx.mqtt.iot.xx.baidubce.com"

// The mqtt client username, and the format is like "xxxxxx/xxxx".
#define         USERNAME                    "xxxxxx/xxxx"

// The key (password) of mqtt client.
#define         PASSWORD                    "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"

// The connection type is TCP, TLS or MUTUAL_TLS.
#define         CONNECTION_TYPE              "TLS"

//The following certificate and key should be set if CONNECTION_TYPE set to 'MUTUAL_TLS'.
static char * client_cert = "-----BEGIN CERTIFICATE-----\r\n"
        "you client cert\r\n"
        "-----END CERTIFICATE-----\r\n";

static char * client_key = "-----BEGIN RSA PRIVATE KEY-----\r\n"
        "your client key\r\n"
        "-----END RSA PRIVATE KEY-----\r\n";

static const char* TOPIC_NAME_A      = "msgA";
static const char* TOPIC_NAME_B      = "msgB";
static int expect_piece = 0;

static OtaUpdater apt;
static int recved_profile = 0;

static const char* QosToString(QOS_VALUE qosValue)
{
    switch (qosValue)
    {
        case DELIVER_AT_LEAST_ONCE: return "Deliver_At_Least_Once";
        case DELIVER_EXACTLY_ONCE: return "Deliver_Exactly_Once";
        case DELIVER_AT_MOST_ONCE: return "Deliver_At_Most_Once";
        case DELIVER_FAILURE: return "Deliver_Failure";
    }
    return "";
}


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
        //test
        printf("current heap size: %d\n", system_get_free_heap_size());
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

int pub_least_ack_process(MQTT_PUB_STATUS_TYPE status, void* context)
{
    IOTHUB_MQTT_CLIENT_HANDLE clientHandle = (IOTHUB_MQTT_CLIENT_HANDLE)context;

    printf("call publish at least once handle\r\n");
    if (clientHandle->mqttClientStatus == MQTT_CLIENT_STATUS_CONNECTED)
    {
        printf("hub is connected\r\n");
    }

    if (status == MQTT_PUB_SUCCESS)
    {
        printf("received publish ack from mqtt server when deliver at least once message\r\n");
    }
    else
    {
        printf("fail to publish message to mqtt server\r\n");
    }

    return 0;
}

int pub_most_once_process(MQTT_PUB_STATUS_TYPE status, void* context)
{
    printf("call publish at most once handle\r\n");

    IOTHUB_MQTT_CLIENT_HANDLE clientHandle = (IOTHUB_MQTT_CLIENT_HANDLE)context;

    if (clientHandle->mqttClientStatus == MQTT_CLIENT_STATUS_CONNECTED)
    {
        printf("hub is connected\r\n");
    }

    if (status == MQTT_PUB_SUCCESS)
    {
        printf("received publish ack from mqtt server when deliver at most once message\r\n");
    }
    else
    {
        printf("fail to publish message to mqtt server\r\n");
    }
    return 0;
}

static int processSubAckFunction(QOS_VALUE* qosReturn, size_t qosCount, void *context) {
    printf("receive suback from hub server\r\n");
    for (int i =0; i< qosCount; ++i) {
        printf("qos return: %d\r\n", qosReturn[i]);
    }

    int *flag = (int *)context;
    *flag = 1;

    return 0;
}

int iothub_mqtt_client_run(void)
{
    if (platform_init() != 0)
    {
        (void)printf("platform_init failed\r\n");
        return __FAILURE__;
    }
    else
    {
        MQTT_CLIENT_OPTIONS options = { 0 };
        options.clientId = "iotclient2017";
        options.willMessage = NULL;
        options.willTopic = NULL;
        options.username = USERNAME;
        options.password = PASSWORD;
        options.keepAliveInterval = 10;
        options.useCleanSession = true;
        options.qualityOfServiceValue = DELIVER_AT_MOST_ONCE;

        const char *endpoint = ENDPOINT;

        MQTT_CONNECTION_TYPE type;
        if (strcmp(CONNECTION_TYPE, "TCP") == 0)
        {
            type = MQTT_CONNECTION_TCP;
        }
        else if (strcmp(CONNECTION_TYPE, "TLS") == 0)
        {
             type = MQTT_CONNECTION_TLS;
        }
        else if (strcmp(CONNECTION_TYPE, "MUTUAL_TLS") == 0)
        {
             type = MQTT_CONNECTION_MUTUAL_TLS;
        }



        IOTHUB_CLIENT_RETRY_POLICY retryPolicy = IOTHUB_CLIENT_RETRY_EXPONENTIAL_BACKOFF;

        size_t retryTimeoutLimitInSeconds = 1000;

        IOTHUB_MQTT_CLIENT_HANDLE clientHandle = initialize_mqtt_client_handle(&options, endpoint, type, on_recv_callback,
                                                                               retryPolicy, retryTimeoutLimitInSeconds);

        if (strcmp(CONNECTION_TYPE, "MUTUAL_TLS") == 0)
        {
            set_client_cert(clientHandle, client_cert, client_key);
        }

        if (clientHandle == NULL)
        {
            printf("Error: fail to initialize IOTHUB_MQTT_CLIENT_HANDLE");
            return 0;
        }

        int result = iothub_mqtt_doconnect(clientHandle, 60);

        if (result == __FAILURE__)
        {
            printf("fail to establish connection with server");
            return __FAILURE__;
        }

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

        const char* publishData = "publish message to topic /china/sh.";

        result = publish_mqtt_message(clientHandle, "/china/sh", DELIVER_EXACTLY_ONCE, (const uint8_t*)publishData,
                             strlen(publishData), pub_least_ack_process , clientHandle);

        if (result == __FAILURE__) {
            printf("Does not support DELIVER_EXACTLY_ONCE\r\n");
        }

        result = publish_mqtt_message(clientHandle, "/china/bj", DELIVER_AT_MOST_ONCE, (const uint8_t*)publishData,
                             strlen(publishData), pub_most_once_process , clientHandle);

        result = publish_mqtt_message(clientHandle, "/china/bj", DELIVER_AT_MOST_ONCE, (const uint8_t*)publishData,
                                      strlen(publishData), NULL , NULL);

        result = publish_mqtt_message(clientHandle, "/china/sh", DELIVER_AT_LEAST_ONCE, (const uint8_t*)publishData,
                                      strlen(publishData), pub_least_ack_process , clientHandle);

        TICK_COUNTER_HANDLE tickCounterHandle = tickcounter_create();
        tickcounter_ms_t currentTime, lastSendTime;
        tickcounter_get_current_ms(tickCounterHandle, &lastSendTime);
        bool needSubscribeTopic = false;
        do
        {
            iothub_mqtt_dowork(clientHandle);
            tickcounter_get_current_ms(tickCounterHandle, &currentTime);
            if (clientHandle->isConnectionLost && !needSubscribeTopic)
            {
                needSubscribeTopic = true;
            }

            if (clientHandle->mqttClientStatus == MQTT_CLIENT_STATUS_CONNECTED && needSubscribeTopic)
            {
                needSubscribeTopic = false;
                subscribe_mqtt_topics(clientHandle, subscribe, sizeof(subscribe)/sizeof(SUBSCRIBE_PAYLOAD), NULL, NULL);
            }

            // send a publish message every 5 seconds
            if (!clientHandle->isConnectionLost && (currentTime - lastSendTime) / 1000 > 5)
            {
                result = publish_mqtt_message(clientHandle, "/china/sh", DELIVER_AT_LEAST_ONCE, (const uint8_t*)publishData,
                                              strlen(publishData), pub_least_ack_process , clientHandle);
                lastSendTime = currentTime;
            }
            ThreadAPI_Sleep(10);
        } while (!clientHandle->isDestroyCalled && !clientHandle->isDisconnectCalled);

        iothub_mqtt_destroy(clientHandle);
        tickcounter_destroy(tickCounterHandle);
        return 0;
    }

#ifdef _CRT_DBG_MAP_ALLOC
    _CrtDumpMemoryLeaks();
#endif
}

