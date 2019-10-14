@echo off

echo show %1%

if exist ".\%1%\BAIDU" (
	echo ".\%1%\BAIDU 已经存在！"
	exit /b
)

if not exist ".\iot-edge-c-sdk" (
	echo ".\iot-edge-c-sdk 不存在！"
	exit /b
)

md .\%1%\BAIDU
md .\%1%\BAIDU\COMMON
md .\%1%\BAIDU\ADAPTOR

md .\%1%\BAIDU\COMMON\certs
md .\%1%\BAIDU\COMMON\c-utility
md .\%1%\BAIDU\COMMON\c-utility\inc
md .\%1%\BAIDU\COMMON\c-utility\inc\azure_c_shared_utility
md .\%1%\BAIDU\COMMON\c-utility\src
md .\%1%\BAIDU\COMMON\iothub_client
md .\%1%\BAIDU\COMMON\iothub_client\inc
md .\%1%\BAIDU\COMMON\iothub_client\src
md .\%1%\BAIDU\COMMON\parson
md .\%1%\BAIDU\COMMON\serializer
md .\%1%\BAIDU\COMMON\serializer\inc
md .\%1%\BAIDU\COMMON\serializer\src
md .\%1%\BAIDU\COMMON\umqtt
md .\%1%\BAIDU\COMMON\umqtt\inc
md .\%1%\BAIDU\COMMON\umqtt\inc\azure_umqtt_c
md .\%1%\BAIDU\COMMON\umqtt\src


copy .\iot-edge-c-sdk\certs\certs.c .\%1%\BAIDU\COMMON\certs
copy .\iot-edge-c-sdk\certs\certs.h .\%1%\BAIDU\COMMON\certs

copy .\iot-edge-c-sdk\c-utility\inc\azure_c_shared_utility\agenttime.h			.\%1%\BAIDU\COMMON\c-utility\inc\azure_c_shared_utility
copy .\iot-edge-c-sdk\c-utility\inc\azure_c_shared_utility\buffer_.h			.\%1%\BAIDU\COMMON\c-utility\inc\azure_c_shared_utility
copy .\iot-edge-c-sdk\c-utility\inc\azure_c_shared_utility\consolelogger.h		.\%1%\BAIDU\COMMON\c-utility\inc\azure_c_shared_utility
copy .\iot-edge-c-sdk\c-utility\inc\azure_c_shared_utility\crt_abstractions.h	.\%1%\BAIDU\COMMON\c-utility\inc\azure_c_shared_utility
copy .\iot-edge-c-sdk\c-utility\inc\azure_c_shared_utility\doublylinkedlist.h	.\%1%\BAIDU\COMMON\c-utility\inc\azure_c_shared_utility
copy .\iot-edge-c-sdk\c-utility\inc\azure_c_shared_utility\gballoc.h			.\%1%\BAIDU\COMMON\c-utility\inc\azure_c_shared_utility
copy .\iot-edge-c-sdk\c-utility\inc\azure_c_shared_utility\macro_utils.h		.\%1%\BAIDU\COMMON\c-utility\inc\azure_c_shared_utility
copy .\iot-edge-c-sdk\c-utility\inc\azure_c_shared_utility\optimize_size.h		.\%1%\BAIDU\COMMON\c-utility\inc\azure_c_shared_utility
copy .\iot-edge-c-sdk\c-utility\inc\azure_c_shared_utility\optionhandler.h		.\%1%\BAIDU\COMMON\c-utility\inc\azure_c_shared_utility
copy .\iot-edge-c-sdk\c-utility\inc\azure_c_shared_utility\platform.h			.\%1%\BAIDU\COMMON\c-utility\inc\azure_c_shared_utility
copy .\iot-edge-c-sdk\c-utility\inc\azure_c_shared_utility\refcount.h			.\%1%\BAIDU\COMMON\c-utility\inc\azure_c_shared_utility
copy .\iot-edge-c-sdk\c-utility\pal\generic\refcount_os.h						.\%1%\BAIDU\COMMON\c-utility\inc\azure_c_shared_utility
copy .\iot-edge-c-sdk\c-utility\inc\azure_c_shared_utility\sha-private.h		.\%1%\BAIDU\COMMON\c-utility\inc\azure_c_shared_utility
copy .\iot-edge-c-sdk\c-utility\inc\azure_c_shared_utility\singlylinkedlist.h	.\%1%\BAIDU\COMMON\c-utility\inc\azure_c_shared_utility
copy .\iot-edge-c-sdk\c-utility\inc\azure_c_shared_utility\socketio.h			.\%1%\BAIDU\COMMON\c-utility\inc\azure_c_shared_utility
copy .\iot-edge-c-sdk\c-utility\inc\azure_c_shared_utility\strings.h			.\%1%\BAIDU\COMMON\c-utility\inc\azure_c_shared_utility
copy .\iot-edge-c-sdk\c-utility\inc\azure_c_shared_utility\strings_types.h		.\%1%\BAIDU\COMMON\c-utility\inc\azure_c_shared_utility
copy .\iot-edge-c-sdk\c-utility\inc\azure_c_shared_utility\threadapi.h			.\%1%\BAIDU\COMMON\c-utility\inc\azure_c_shared_utility
copy .\iot-edge-c-sdk\c-utility\inc\azure_c_shared_utility\tickcounter.h		.\%1%\BAIDU\COMMON\c-utility\inc\azure_c_shared_utility
copy .\iot-edge-c-sdk\c-utility\inc\azure_c_shared_utility\tlsio.h				.\%1%\BAIDU\COMMON\c-utility\inc\azure_c_shared_utility
copy .\iot-edge-c-sdk\c-utility\inc\azure_c_shared_utility\umock_c_prod.h		.\%1%\BAIDU\COMMON\c-utility\inc\azure_c_shared_utility
copy .\iot-edge-c-sdk\c-utility\inc\azure_c_shared_utility\utf8_checker.h		.\%1%\BAIDU\COMMON\c-utility\inc\azure_c_shared_utility
copy .\iot-edge-c-sdk\c-utility\inc\azure_c_shared_utility\uuid.h				.\%1%\BAIDU\COMMON\c-utility\inc\azure_c_shared_utility
copy .\iot-edge-c-sdk\c-utility\inc\azure_c_shared_utility\vector.h				.\%1%\BAIDU\COMMON\c-utility\inc\azure_c_shared_utility
copy .\iot-edge-c-sdk\c-utility\inc\azure_c_shared_utility\vector_types.h		.\%1%\BAIDU\COMMON\c-utility\inc\azure_c_shared_utility
copy .\iot-edge-c-sdk\c-utility\inc\azure_c_shared_utility\vector_types_internal.h		.\%1%\BAIDU\COMMON\c-utility\inc\azure_c_shared_utility
copy .\iot-edge-c-sdk\c-utility\inc\azure_c_shared_utility\xio.h				.\%1%\BAIDU\COMMON\c-utility\inc\azure_c_shared_utility
copy .\iot-edge-c-sdk\c-utility\inc\azure_c_shared_utility\xlogging.h			.\%1%\BAIDU\COMMON\c-utility\inc\azure_c_shared_utility



copy .\iot-edge-c-sdk\c-utility\adapters\agenttime.c	.\%1%\BAIDU\COMMON\c-utility\src
copy .\iot-edge-c-sdk\c-utility\src\buffer.c			.\%1%\BAIDU\COMMON\c-utility\src
copy .\iot-edge-c-sdk\c-utility\src\consolelogger.c		.\%1%\BAIDU\COMMON\c-utility\src
copy .\iot-edge-c-sdk\c-utility\src\crt_abstractions.c	.\%1%\BAIDU\COMMON\c-utility\src
copy .\iot-edge-c-sdk\c-utility\src\doublylinkedlist.c	.\%1%\BAIDU\COMMON\c-utility\src
copy .\iot-edge-c-sdk\c-utility\src\optionhandler.c		.\%1%\BAIDU\COMMON\c-utility\src
copy .\iot-edge-c-sdk\c-utility\src\singlylinkedlist.c	.\%1%\BAIDU\COMMON\c-utility\src
copy .\iot-edge-c-sdk\c-utility\src\strings.c			.\%1%\BAIDU\COMMON\c-utility\src
copy .\iot-edge-c-sdk\c-utility\src\vector.c			.\%1%\BAIDU\COMMON\c-utility\src
copy .\iot-edge-c-sdk\c-utility\src\xio.c				.\%1%\BAIDU\COMMON\c-utility\src
copy .\iot-edge-c-sdk\c-utility\src\xlogging.c			.\%1%\BAIDU\COMMON\c-utility\src


copy .\iot-edge-c-sdk\iothub_client\inc\iotdm_callback.h			.\%1%\BAIDU\COMMON\iothub_client\inc
copy .\iot-edge-c-sdk\iothub_client\inc\iotdm_client.h				.\%1%\BAIDU\COMMON\iothub_client\inc
copy .\iot-edge-c-sdk\iothub_client\inc\iothub_client.h				.\%1%\BAIDU\COMMON\iothub_client\inc
copy .\iot-edge-c-sdk\iothub_client\inc\iothub_client_persistence.h	.\%1%\BAIDU\COMMON\iothub_client\inc
copy .\iot-edge-c-sdk\iothub_client\inc\iothub_mqtt_client.h		.\%1%\BAIDU\COMMON\iothub_client\inc


copy .\iot-edge-c-sdk\iothub_client\src\iotdm_client.c				.\%1%\BAIDU\COMMON\iothub_client\src
copy .\iot-edge-c-sdk\iothub_client\src\iothub_client_persistence.c	.\%1%\BAIDU\COMMON\iothub_client\src
copy .\iot-edge-c-sdk\iothub_client\src\iothub_mqtt_client.c		.\%1%\BAIDU\COMMON\iothub_client\src


copy .\iot-edge-c-sdk\parson\parson.c	.\%1%\BAIDU\COMMON\parson\
copy .\iot-edge-c-sdk\parson\parson.h	.\%1%\BAIDU\COMMON\parson\


copy .\iot-edge-c-sdk\serializer\inc\*				.\%1%\BAIDU\COMMON\serializer\inc\
copy .\iot-edge-c-sdk\serializer\src\*				.\%1%\BAIDU\COMMON\serializer\src


copy .\iot-edge-c-sdk\umqtt\inc\azure_umqtt_c\*		.\%1%\BAIDU\COMMON\umqtt\inc\azure_umqtt_c\
copy .\iot-edge-c-sdk\umqtt\src\*					.\%1%\BAIDU\COMMON\umqtt\src

echo 源文件复制完成！


