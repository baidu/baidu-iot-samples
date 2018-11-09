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
#include "parson.h"

#include "ota_update_engine.h"

#define CURRENT_VER "12.34.58"

// Please set the mqtt client data and security which are shown as follow.
// The endpoint address, witch is like "xxxxxx.mqtt.iot.xx.baidubce.com".
#define         ENDPOINT                    "xxxxxx.mqtt.iot.xx.baidubce.com"

// The mqtt client username, and the format is like "xxxxxx/xxxx".
#define         USERNAME                    "xxxxxx/xxxx"

// The key (password) of mqtt client.
#define         PASSWORD                    "xxxxxxxxxxxxxxxxxxxxxx"

// The connection type is TCP, TLS or MUTUAL_TLS.
#define         CONNECTION_TYPE              "TLS"

// $puid
#define         DEVICE              "xxxxx"

//The following certificate and key should be set if CONNECTION_TYPE set to 'MUTUAL_TLS'.
static char * client_cert = "-----BEGIN CERTIFICATE-----\r\n"
        "you client cert\r\n"
        "-----END CERTIFICATE-----\r\n";

static char * client_key = "-----BEGIN RSA PRIVATE KEY-----\r\n"
        "your client key\r\n"
        "-----END RSA PRIVATE KEY-----\r\n";


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

static const char* TOPIC_NAME_A      = "msgA";
static const char* TOPIC_NAME_B      = "msgB";

static OtaUpdater *apt;

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



void on_recv_callback(MQTT_MESSAGE_HANDLE msgHandle, void* context)
{
    const char *topic_name;
    const APP_PAYLOAD* appMsg = mqttmessage_getApplicationMsg(msgHandle);
    IOTHUB_MQTT_CLIENT_HANDLE clientHandle = (IOTHUB_MQTT_CLIENT_HANDLE)context;
    unsigned char *data;
    size_t data_len;

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


    // first filter the ota data flow
    ota_updater_data_filter(apt, topic_name, data, data_len);

    // user logic here ...

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
        printf("OTA_UPDATER_FINISH\n");
        reboot = true;
        break;
    default:
        break;
    }

    ota_updater_end(apt, reboot);
    ota_updater_destroy(apt);
}

int iothub_mqtt_client_run(void)
{
    printf("APPLICATION VERSION: %s\n", CURRENT_VER);
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
            return __FAILURE__;
        }

        printf("==> iothub_mqtt_doconnect\n");
        int result = iothub_mqtt_doconnect(clientHandle, 60);

        if (result == __FAILURE__)
        {
            printf("fail to establish connection with server");
            return __FAILURE__;
        }

        printf("begin ota process\n");
        //初始化  OtaUpdater
        // OTA_BIN_DM_SHA256RSA      OTA_BIN_DM_MD5
        if (!(apt = ota_updater_create(DEVICE, CURRENT_VER, OTA_BIN_DM_SHA256RSA, client_ota_cert, ota_updater_cb, clientHandle))) {
            printf("ota_updater_create() fail\n");
            return __FAILURE__;
        }
        
        SUBSCRIBE_PAYLOAD subscribe[2];
        subscribe[0].subscribeTopic = TOPIC_NAME_A;
        subscribe[0].qosReturn = DELIVER_AT_MOST_ONCE;
        subscribe[1].subscribeTopic = TOPIC_NAME_B;
        subscribe[1].qosReturn = DELIVER_AT_MOST_ONCE;
        
        int flag = 0;
        subscribe_mqtt_topics(clientHandle, subscribe, sizeof(subscribe)/sizeof(SUBSCRIBE_PAYLOAD), processSubAckFunction, &flag);

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

