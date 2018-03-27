#include "iot_configs.h"

#include <AzureIoTHub.h>
#include <AzureIoTProtocol_MQTT.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiAP.h>
#include <ESP8266WiFiGeneric.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266WiFiScan.h>
#include <ESP8266WiFiSTA.h>
#include <ESP8266WiFiType.h>
#include <WiFiClient.h>
#include <WiFiClientSecure.h>
#include <WiFiServer.h>
#include <WiFiUdp.h>
 
#include "sample.h"
#include "esp8266/sample_init.h"

static char ssid[] = IOT_CONFIG_WIFI_SSID;
static char pass[] = IOT_CONFIG_WIFI_PASSWORD;

// PIN DEFINE
#define BUILTIN_LED 2     // ESP-12E module's onboard LED, used as status indicator
#define BUILTIN_LED_OFF() digitalWrite(BUILTIN_LED, HIGH)
#define BUILTIN_LED_ON()  digitalWrite(BUILTIN_LED, LOW)

void setup() {
    // sample_init(ssid, pass);
    Serial.begin(115200);
    pinMode(BUILTIN_LED, OUTPUT);
    Serial.println("");

    // setup WiFi
    // no need to config SSID and password
    // turn on Auto Connect
    WiFi.mode(WIFI_STA);
    if(WiFi.getAutoConnect() == false){
        WiFi.setAutoConnect(true);
    }
    // WiFi.disconnect(true);
    // delay(2000);
    // ESP.reset();
    // WiFi.setAutoReconnect(true);
 
    // start WiFi connecting
    int cnt = 0;
    while (WiFi.status() != WL_CONNECTED) {
        // onboard LED flash twice quickly
        // UART output "..." 
        // ESP8266 is trying to connect to a known WiFi
        status_blink();
        delay(100);
        status_blink();
        delay(750);
        Serial.print(".");
 
        // if no connection available after timeout
        if (cnt++ >= 10) {
            Serial.println("");
            WiFi.beginSmartConfig();
            while (1) {
                // onboard LED flash slowly
                // UART output "|||" 
                // ESP8266 is in smartconfig loop
                // use app to finish the smartconfig
                status_blink();
                delay(900);
                
                if (WiFi.smartConfigDone()) {
                    Serial.println();
                    Serial.println(F("[WIFI] SmartConfig: Success"));
                    break;
                }
                Serial.print(F("|"));
            }
        }
        BUILTIN_LED_OFF();
    }
 
    Serial.println("");
    Serial.println(F("WiFi connected"));  
    Serial.print(F("IP address: "));
    Serial.println(WiFi.localIP());
 
    WiFi.printDiag(Serial);
 
    Serial.print(F("RSSI: "));
    Serial.println(WiFi.RSSI());
    Serial.print(F("BSSID: "));
    Serial.println(WiFi.BSSIDstr());
}

// Azure IoT samples contain their own loops, so only run them once
static bool done = false;
void loop() {
    if (!done)
    {
        // Run the sample
        // You must set the device id, device key, IoT Hub name and IotHub suffix in
        // iot_configs.h
        sample_run();
        done = true;
    }
    else
    {
      delay(500);
    }
}

void status_blink(){
    BUILTIN_LED_ON();
    delay(50);
    BUILTIN_LED_OFF();
}
