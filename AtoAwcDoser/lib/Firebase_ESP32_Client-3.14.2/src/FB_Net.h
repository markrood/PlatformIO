/**
 * Created January 18, 2022
 */

#ifndef FB_Net_H
#define FB_Net_H

typedef enum
{
    fb_cert_type_undefined = -1,
    fb_cert_type_none = 0,
    fb_cert_type_data,
    fb_cert_type_file

} fb_cert_type;

#if defined(ESP32)
#include <Arduino.h>
#include <WiFiClient.h>
#include <ETH.h>
#include <WiFiClientSecure.h>
#if __has_include(<esp_idf_version.h>)
#include <esp_idf_version.h>
#endif
static const char esp_idf_branch_str[] PROGMEM = "release/v";
#endif


#if defined(ESP8266)

#include <WiFiClientSecure.h>
#include <CertStoreBearSSL.h>
#define FB_ESP_SSL_CLIENT BearSSL::WiFiClientSecure

#include <core_version.h>
#include <time.h>

//__GNUC__
//__GNUC_MINOR__
//__GNUC_PATCHLEVEL__

#ifdef __GNUC__
#if __GNUC__ > 4 || __GNUC__ == 10
#include <string>
#define ESP8266_CORE_SDK_V3_X_X
#endif
#endif

#ifndef ARDUINO_ESP8266_GIT_VER
#error Your ESP8266 Arduino Core SDK is outdated, please update. From Arduino IDE go to Boards Manager and search 'esp8266' then select the latest version.
#endif

//2.6.1 BearSSL bug
#if ARDUINO_ESP8266_GIT_VER == 0x482516e3
#error Due to bugs in BearSSL in ESP8266 Arduino Core SDK version 2.6.1, please update ESP8266 Arduino Core SDK to newer version. The issue was found here https:\/\/github.com/esp8266/Arduino/issues/6811.
#endif

#if defined __has_include

#if __has_include(<LwipIntfDev.h>)
#include <LwipIntfDev.h>
#endif

#if __has_include(<ENC28J60lwIP.h>)
#define INC_ENC28J60_LWIP
#include <ENC28J60lwIP.h>
#endif

#if __has_include(<W5100lwIP.h>)
#define INC_W5100_LWIP
#include <W5100lwIP.h>
#endif

#if __has_include(<W5500lwIP.h>)
#define INC_W5500_LWIP
#include <W5500lwIP.h>
#endif

#endif

#endif

#endif