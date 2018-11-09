#
# Component Makefile
#

# Component configuration in preprocessor defines
CFLAGS += -DUSE_OPENSSL
EDGE_SDK_PATH = ../../../../../iot-edge-c-sdk
ESP_PAL_PATH = ../../../pal

COMPONENT_ADD_INCLUDEDIRS :=  \
$(ESP_PAL_PATH)/inc \
$(EDGE_SDK_PATH)/c-utility/inc  \
$(EDGE_SDK_PATH)/c-utility/inc/azure_c_shared_utility \
$(EDGE_SDK_PATH)/iothub_client/inc \
$(EDGE_SDK_PATH)/umqtt/inc  \
$(EDGE_SDK_PATH)/umqtt/inc/azure_umqtt_c 	\
$(EDGE_SDK_PATH)/iothub_client/samples/iothub_client_sample \
$(EDGE_SDK_PATH)/c-utility/pal/generic \
$(EDGE_SDK_PATH)/certs \
$(EDGE_SDK_PATH)/c-utility/pal/inc

COMPONENT_OBJS =  \
$(EDGE_SDK_PATH)/c-utility/src/xlogging.o	\
$(EDGE_SDK_PATH)/c-utility/src/consolelogger.o	\
$(EDGE_SDK_PATH)/c-utility/src/buffer.o	\
$(EDGE_SDK_PATH)/c-utility/src/crt_abstractions.o	\
$(EDGE_SDK_PATH)/c-utility/src/singlylinkedlist.o	\
$(EDGE_SDK_PATH)/c-utility/src/doublylinkedlist.o	\
$(EDGE_SDK_PATH)/c-utility/src/utf8_checker.o	\
$(EDGE_SDK_PATH)/c-utility/src/map.o	\
$(EDGE_SDK_PATH)/c-utility/src/optionhandler.o	\
$(EDGE_SDK_PATH)/c-utility/src/strings.o	\
$(EDGE_SDK_PATH)/c-utility/src/vector.o	\
$(EDGE_SDK_PATH)/c-utility/src/xio.o	\
\
\
$(EDGE_SDK_PATH)/iothub_client/src/iothub_client_persistence.o	\
$(EDGE_SDK_PATH)/iothub_client/src/iothub_mqtt_client.o	\
\
\
$(EDGE_SDK_PATH)/umqtt/src/mqtt_client.o	\
$(EDGE_SDK_PATH)/umqtt/src/mqtt_codec.o	\
$(EDGE_SDK_PATH)/umqtt/src/mqtt_message.o	\
\
\
\
$(EDGE_SDK_PATH)/c-utility/pal/dns_async.o	\
$(EDGE_SDK_PATH)/c-utility/pal/socket_async.o	\
$(ESP_PAL_PATH)/src/tlsio_openssl_compact.o	\
$(ESP_PAL_PATH)/src/platform_esp.o	\
$(ESP_PAL_PATH)/src/socketio_lwip.o \
$(EDGE_SDK_PATH)/c-utility/adapters/agenttime.o	\
$(EDGE_SDK_PATH)/umqtt/deps/c-utility/pal/freertos/threadapi.o	\
$(EDGE_SDK_PATH)/umqtt/deps/c-utility/pal/freertos/tickcounter.o	\
$(EDGE_SDK_PATH)/c-utility/pal/lwip/sntp_lwip.o	\
$(EDGE_SDK_PATH)/certs/certs.o \
$(EDGE_SDK_PATH)/c-utility/pal/tlsio_options.o \
\
\
$(EDGE_SDK_PATH)/iothub_client/samples/iothub_client_sample/iothub_mqtt_client_sample.o	\


COMPONENT_SRCDIRS :=  \
$(EDGE_SDK_PATH)/c-utility/src \
$(EDGE_SDK_PATH)/c-utility/adapters  \
$(EDGE_SDK_PATH)/umqtt/src	\
$(EDGE_SDK_PATH)/iothub_client/src  \
$(EDGE_SDK_PATH)/iothub_client/samples/iothub_client_sample  \
$(EDGE_SDK_PATH)/certs \
$(EDGE_SDK_PATH)/umqtt/deps/c-utility/pal/freertos \
$(EDGE_SDK_PATH)/c-utility/pal/lwip \
$(EDGE_SDK_PATH)/c-utility/pal \
$(ESP_PAL_PATH)/src \
















