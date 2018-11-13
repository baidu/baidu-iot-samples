#
# "main" pseudo-component makefile.
#
# (Uses default behaviour of compiling all source files in directory, adding 'include' to include path.)


COMPONENT_ADD_INCLUDEDIRS :=  \
libota \
\


COMPONENT_OBJS =  \
main.o \
iothub_mqtt_client_sample.o \
libota/ota_hal_esp32.o \
libota/ota_update_engine.o \
\



COMPONENT_SRCDIRS :=  \
libota \
. \


