@echo off


if "%1"=="" (
	echo 用法：copy_src_to_keil_stm32.bat [iot-sdk-c的路径] [拷贝目标文件夹路径]
	echo 例：copy_src_to_keil_stm32.bat .\iot-sdk-c .\Demo\
	exit /b
)
if "%2"=="" (
	echo 用法：copy_src_to_keil_stm32.bat [iot-sdk-c的路径] [拷贝目标文件夹路径]
	echo 例：copy_src_to_keil_stm32.bat .\iot-sdk-c .\Demo\
	exit /b
)

if not exist "%1%\" (
	echo %1%\ 不存在！
	exit /b
)


if exist "%2%\BAIDU" (
	echo %2%\BAIDU 已经存在！
	exit /b
)

md %2%\BAIDU
md %2%\BAIDU\COMMON
md %2%\BAIDU\ADAPTOR

md %2%\BAIDU\COMMON\certs
md %2%\BAIDU\COMMON\c-utility
md %2%\BAIDU\COMMON\c-utility\inc
md %2%\BAIDU\COMMON\c-utility\inc\azure_c_shared_utility
md %2%\BAIDU\COMMON\c-utility\src
md %2%\BAIDU\COMMON\iothub_client
md %2%\BAIDU\COMMON\iothub_client\inc
md %2%\BAIDU\COMMON\iothub_client\src
md %2%\BAIDU\COMMON\iothub_client\samples\
md %2%\BAIDU\COMMON\iothub_client\samples\iotdm_client_sample
md %2%\BAIDU\COMMON\parson
md %2%\BAIDU\COMMON\serializer
md %2%\BAIDU\COMMON\serializer\inc
md %2%\BAIDU\COMMON\serializer\src
md %2%\BAIDU\COMMON\umqtt
md %2%\BAIDU\COMMON\umqtt\inc
md %2%\BAIDU\COMMON\umqtt\inc\azure_umqtt_c
md %2%\BAIDU\COMMON\umqtt\src


set src=%1%
set dst=%2%

copy %src%\certs\certs.c %dst%\BAIDU\COMMON\certs\
copy %src%\certs\certs.h %dst%\BAIDU\COMMON\certs

::goto han

copy %src%\c-utility\inc\azure_c_shared_utility\agenttime.h			%dst%\BAIDU\COMMON\c-utility\inc\azure_c_shared_utility
copy %src%\c-utility\inc\azure_c_shared_utility\buffer_.h			%dst%\BAIDU\COMMON\c-utility\inc\azure_c_shared_utility
copy %src%\c-utility\inc\azure_c_shared_utility\consolelogger.h		%dst%\BAIDU\COMMON\c-utility\inc\azure_c_shared_utility
copy %src%\c-utility\inc\azure_c_shared_utility\crt_abstractions.h	%dst%\BAIDU\COMMON\c-utility\inc\azure_c_shared_utility
copy %src%\c-utility\inc\azure_c_shared_utility\doublylinkedlist.h	%dst%\BAIDU\COMMON\c-utility\inc\azure_c_shared_utility
copy %src%\c-utility\inc\azure_c_shared_utility\gballoc.h			%dst%\BAIDU\COMMON\c-utility\inc\azure_c_shared_utility
copy %src%\c-utility\inc\azure_c_shared_utility\macro_utils.h		%dst%\BAIDU\COMMON\c-utility\inc\azure_c_shared_utility
copy %src%\c-utility\inc\azure_c_shared_utility\optimize_size.h		%dst%\BAIDU\COMMON\c-utility\inc\azure_c_shared_utility
copy %src%\c-utility\inc\azure_c_shared_utility\optionhandler.h		%dst%\BAIDU\COMMON\c-utility\inc\azure_c_shared_utility
copy %src%\c-utility\inc\azure_c_shared_utility\platform.h			%dst%\BAIDU\COMMON\c-utility\inc\azure_c_shared_utility
copy %src%\c-utility\inc\azure_c_shared_utility\refcount.h			%dst%\BAIDU\COMMON\c-utility\inc\azure_c_shared_utility
copy %src%\c-utility\pal\generic\refcount_os.h						%dst%\BAIDU\COMMON\c-utility\inc\azure_c_shared_utility
copy %src%\c-utility\inc\azure_c_shared_utility\sha-private.h		%dst%\BAIDU\COMMON\c-utility\inc\azure_c_shared_utility
copy %src%\c-utility\inc\azure_c_shared_utility\singlylinkedlist.h	%dst%\BAIDU\COMMON\c-utility\inc\azure_c_shared_utility
copy %src%\c-utility\inc\azure_c_shared_utility\socketio.h			%dst%\BAIDU\COMMON\c-utility\inc\azure_c_shared_utility
copy %src%\c-utility\inc\azure_c_shared_utility\strings.h			%dst%\BAIDU\COMMON\c-utility\inc\azure_c_shared_utility
copy %src%\c-utility\inc\azure_c_shared_utility\strings_types.h		%dst%\BAIDU\COMMON\c-utility\inc\azure_c_shared_utility
copy %src%\c-utility\inc\azure_c_shared_utility\threadapi.h			%dst%\BAIDU\COMMON\c-utility\inc\azure_c_shared_utility
copy %src%\c-utility\inc\azure_c_shared_utility\tickcounter.h		%dst%\BAIDU\COMMON\c-utility\inc\azure_c_shared_utility
copy %src%\c-utility\inc\azure_c_shared_utility\tlsio.h				%dst%\BAIDU\COMMON\c-utility\inc\azure_c_shared_utility
copy %src%\c-utility\inc\azure_c_shared_utility\umock_c_prod.h		%dst%\BAIDU\COMMON\c-utility\inc\azure_c_shared_utility
copy %src%\c-utility\inc\azure_c_shared_utility\utf8_checker.h		%dst%\BAIDU\COMMON\c-utility\inc\azure_c_shared_utility
copy %src%\c-utility\inc\azure_c_shared_utility\uuid.h				%dst%\BAIDU\COMMON\c-utility\inc\azure_c_shared_utility
copy %src%\c-utility\inc\azure_c_shared_utility\vector.h				%dst%\BAIDU\COMMON\c-utility\inc\azure_c_shared_utility
copy %src%\c-utility\inc\azure_c_shared_utility\vector_types.h		%dst%\BAIDU\COMMON\c-utility\inc\azure_c_shared_utility
copy %src%\c-utility\inc\azure_c_shared_utility\vector_types_internal.h		%dst%\BAIDU\COMMON\c-utility\inc\azure_c_shared_utility
copy %src%\c-utility\inc\azure_c_shared_utility\xio.h				%dst%\BAIDU\COMMON\c-utility\inc\azure_c_shared_utility
copy %src%\c-utility\inc\azure_c_shared_utility\xlogging.h			%dst%\BAIDU\COMMON\c-utility\inc\azure_c_shared_utility

copy %src%\c-utility\adapters\agenttime.c	%dst%\BAIDU\COMMON\c-utility\src
copy %src%\c-utility\src\buffer.c			%dst%\BAIDU\COMMON\c-utility\src
copy %src%\c-utility\src\consolelogger.c		%dst%\BAIDU\COMMON\c-utility\src
copy %src%\c-utility\src\crt_abstractions.c	%dst%\BAIDU\COMMON\c-utility\src
copy %src%\c-utility\src\doublylinkedlist.c	%dst%\BAIDU\COMMON\c-utility\src
copy %src%\c-utility\src\optionhandler.c		%dst%\BAIDU\COMMON\c-utility\src
copy %src%\c-utility\src\singlylinkedlist.c	%dst%\BAIDU\COMMON\c-utility\src
copy %src%\c-utility\src\strings.c			%dst%\BAIDU\COMMON\c-utility\src
copy %src%\c-utility\src\vector.c			%dst%\BAIDU\COMMON\c-utility\src
copy %src%\c-utility\src\xio.c				%dst%\BAIDU\COMMON\c-utility\src
copy %src%\c-utility\src\xlogging.c			%dst%\BAIDU\COMMON\c-utility\src

copy %src%\iothub_client\inc\iotdm_callback.h			%dst%\BAIDU\COMMON\iothub_client\inc
copy %src%\iothub_client\inc\iotdm_client.h				%dst%\BAIDU\COMMON\iothub_client\inc
copy %src%\iothub_client\inc\iothub_client.h				%dst%\BAIDU\COMMON\iothub_client\inc
copy %src%\iothub_client\inc\iothub_client_persistence.h	%dst%\BAIDU\COMMON\iothub_client\inc
copy %src%\iothub_client\inc\iothub_mqtt_client.h		%dst%\BAIDU\COMMON\iothub_client\inc

copy %src%\iothub_client\src\iotdm_client.c				%dst%\BAIDU\COMMON\iothub_client\src
copy %src%\iothub_client\src\iothub_client_persistence.c	%dst%\BAIDU\COMMON\iothub_client\src
copy %src%\iothub_client\src\iothub_mqtt_client.c		%dst%\BAIDU\COMMON\iothub_client\src

copy %src%\iothub_client\samples\iotdm_client_sample\iotdm_client_sample.c		%dst%\BAIDU\COMMON\iothub_client\samples\iotdm_client_sample
copy %src%\iothub_client\samples\iotdm_client_sample\iotdm_client_sample.h		%dst%\BAIDU\COMMON\iothub_client\samples\iotdm_client_sample

copy %src%\parson\parson.c	%dst%\BAIDU\COMMON\parson\
copy %src%\parson\parson.h	%dst%\BAIDU\COMMON\parson\

copy %src%\serializer\inc\*				%dst%\BAIDU\COMMON\serializer\inc\
copy %src%\serializer\src\*				%dst%\BAIDU\COMMON\serializer\src

copy %src%\umqtt\inc\azure_umqtt_c\*		%dst%\BAIDU\COMMON\umqtt\inc\azure_umqtt_c\
copy %src%\umqtt\src\*					%dst%\BAIDU\COMMON\umqtt\src

echo 源文件复制完成！

:han
