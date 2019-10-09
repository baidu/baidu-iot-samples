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

#include "main.h"
#include "serializer.h"
#include "jsondecoder.h"
#include "mqtt_client_sample.h"

/*
* this demo will send sensor data to cloud throught mqtt protocol
* you can start a mqtt client to subscibe topic /china/sh to get data send by board.
* mqtt desktop application could be downloaded from http://mqttfx.bceapp.com/
* for more information about how to use baidu tiangong produce could be found at https://cloud.baidu.com/doc/IOT/Quickstart-new.html#.E7.9B.AE.E5.BD.95
*/

/*
* Please set the mqtt client data and security which are shown as follow.
* The endpoint address, witch is like "xxxxxx.mqtt.iot.yy.baidubce.com".
* please access website https://cloud.baidu.com/ to create a mqtt endpoint
* replace xxxxx with endpoint name and yy with region name
*/
#define         ENDPOINT                    "stm32.mqtt.iot.gz.baidubce.com"  //"xxxxxx.mqtt.iot.yy.baidubce.com"

/*
* The mqtt client username, and the format is like "xxxxxx/yyyy".
*/
#define         USERNAME                    "stm32/node-1"  //"xxxxxx/yyyy"

/*
* The key (password) of mqtt client.
*/
#define         PASSWORD                    "AH5kABst757B1I93xp5RA4saKlQPwDEafPCX+RCy3vM="  //"xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"

/*
* The the topic of mqtt client
*/
static const char* TOPIC_NAME_A = "node-topic-1";  //"xxxxxx";
//static const char* TOPIC_NAME_B = "node-topic-2";  //"yyyyyy";


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
    const APP_PAYLOAD* appMsg = mqttmessage_getApplicationMsg(msgHandle);
    IOTHUB_MQTT_CLIENT_HANDLE clientHandle = (IOTHUB_MQTT_CLIENT_HANDLE)context;

    (void)printf("\r\n");
    (void)printf("Incoming Msg: Packet Id: %d\r\nQOS: %s\r\nTopic Name: %s\r\nIs Retained: %s\r\nIs Duplicate: %s\r\nApp Msg: ", mqttmessage_getPacketId(msgHandle),
                 QosToString(mqttmessage_getQosType(msgHandle) ),
                 mqttmessage_getTopicName(msgHandle),
                 mqttmessage_getIsRetained(msgHandle) ? "true" : "fale",
                 mqttmessage_getIsDuplicateMsg(msgHandle) ? "true" : "fale"
    );

    // bool isValidUtf8 = utf8_checker_is_valid_utf8((unsigned char *)appMsg->message, appMsg->length);
    bool isValidUtf8 = true;
    printf("content is valid UTF8:%s\r\nMessage length:%d\r\n", isValidUtf8? "true": "false", (int)appMsg->length);
    
    printf("\r\n");
    for (size_t index = 0; index < appMsg->length; index++)
    {
        if (isValidUtf8)
        {
            (void)printf("%c", appMsg->message[index]);
        }
        else
        {
            (void)printf("0x%x", appMsg->message[index]);
        }
    }

    (void)printf("\r\n");

    if (strncmp((const char *)appMsg->message, "led_off", strlen("led_off")) == 0)
    {
        printf("\r\n"); 
        printf("set led off\r\n");
        Led_Off();
    }
    
    if (strncmp((const char *)appMsg->message, "led_on", strlen("led_on")) == 0)
    {
        printf("\r\n");
        printf("set led on\r\n");
        Led_On();
    }      
    
    // when receive message is "stop", call destroy method to exit
    // trigger stop by send a message to topic "msgA" and payload with "stop"
    if (strcmp((const char *)appMsg->message, "stop") == 0)
    {
        iothub_mqtt_disconnect(clientHandle);
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

int iothub_mqtt_client_run(void)
{
    if (platform_init() != 0)
    {
        (void)printf("platform_init failed\r\n");
        return __FAILURE__;
    }
    else
    {
        printf("Platform Inited - Green LED ON\r\n");
        printf("\r\n");
        printf("********************************************\r\n");
        Led_On();

        MQTT_CLIENT_OPTIONS options = { 0 };
        options.clientId = "iotclient1234";
        options.willMessage = NULL;
        options.willTopic = NULL;
        options.username = USERNAME;
        options.password = PASSWORD;
        options.keepAliveInterval = 10;
        options.useCleanSession = true;
        options.qualityOfServiceValue = DELIVER_AT_LEAST_ONCE;

        const char *endpoint = ENDPOINT;

        MQTT_CONNECTION_TYPE type = MQTT_CONNECTION_TCP;

        IOTHUB_CLIENT_RETRY_POLICY retryPolicy = IOTHUB_CLIENT_RETRY_EXPONENTIAL_BACKOFF;

        size_t retryTimeoutLimitInSeconds = 1000;

        IOTHUB_MQTT_CLIENT_HANDLE clientHandle = initialize_mqtt_client_handle(&options, endpoint, type, on_recv_callback,
                                                                               retryPolicy, retryTimeoutLimitInSeconds);

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

        SUBSCRIBE_PAYLOAD subscribe[1];

        subscribe[0].subscribeTopic = TOPIC_NAME_A;
        subscribe[0].qosReturn = DELIVER_AT_LEAST_ONCE;
//        subscribe[1].subscribeTopic = TOPIC_NAME_B;
//        subscribe[1].qosReturn = DELIVER_AT_LEAST_ONCE;

        subscribe_mqtt_topics(clientHandle, subscribe, sizeof(subscribe)/sizeof(SUBSCRIBE_PAYLOAD));

//        const char* publishDataA = "publishing message to topic A.";
//        result = publish_mqtt_message(clientHandle, TOPIC_NAME_A, DELIVER_AT_LEAST_ONCE, (const uint8_t*)publishDataA,
//                                      strlen(publishDataA), pub_least_ack_process , clientHandle);
//
//        const char* publishDataB = "publishing message to topic B.";
//        result = publish_mqtt_message(clientHandle, TOPIC_NAME_B, DELIVER_AT_MOST_ONCE, (const uint8_t*)publishDataB,
//                                      strlen(publishDataB), NULL , NULL);

        TICK_COUNTER_HANDLE tickCounterHandle = tickcounter_create();
        tickcounter_ms_t currentTime, lastSendTime;
        tickcounter_get_current_ms(tickCounterHandle, &lastSendTime);
        bool needSubscribeTopic = false;

        char destination[256] = "*** test message start from here *** ";
        uint32_t destinationSize = strlen(destination);
        
        uint32_t  messageCounter=1;

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
                subscribe_mqtt_topics(clientHandle, subscribe, sizeof(subscribe)/sizeof(SUBSCRIBE_PAYLOAD));
            }

            // send a publish message every 10 seconds
            if (!clientHandle->isConnectionLost && (currentTime - lastSendTime) / 1000 > 10)
            {
              printf("\r\n********************************************\r\n");

              result = publish_mqtt_message(clientHandle, TOPIC_NAME_A, DELIVER_AT_LEAST_ONCE, (uint8_t const*)destination,
                    destinationSize, pub_least_ack_process , clientHandle);

              messageCounter++;

              //memset((unsigned char *)destination, 0, destinationSize);

              destinationSize = sprintf((char *)destination, "*** this is test message :  %u", messageCounter);

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

