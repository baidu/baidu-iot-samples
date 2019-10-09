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

#ifndef IOTDM_CALLBACK_H
#define IOTDM_CALLBACK_H

#ifdef __cplusplus
#include <cstdint>
extern "C" {
#else
#include <stdint.h>
#endif // __cplusplus

#include "parson.h"

typedef enum SHADOW_CALLBACK_TYPE_TAG
{
    SHADOW_CALLBACK_TYPE_DELTA
} SHADOW_CALLBACK_TYPE;

typedef struct SHADOW_MESSAGE_CONTEXT_TAG
{
    const char* requestId;
    char* device;
} SHADOW_MESSAGE_CONTEXT;

typedef void (*SHADOW_DELTA_CALLBACK) (const SHADOW_MESSAGE_CONTEXT* messageContext, const JSON_Object* desired, void* callbackContext);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // IOTDM_CALLBACK_H
