#ifndef _PLATFORM_H_
#define _PLATFORM_H_

#define STRHELPER(x) #x
#define STR(x) STRHELPER(x) // stringifier

// form version string
#define VERSION_STR(MAJOR, MINOR, PATCH) STR(MAJOR) "." STR(MINOR) "." STR(PATCH)

#if defined(ESP8266)
# include <core_version.h>
# define INFLUXDB_CLIENT_PLATFORM "ESP8266"
# define INFLUXDB_CLIENT_PLATFORM_VERSION  STR(ARDUINO_ESP8266_GIT_DESC)
#elif defined(ESP32)
# include <esp_arduino_version.h>
# define INFLUXDB_CLIENT_PLATFORM "ESP32"
# define INFLUXDB_CLIENT_PLATFORM_VERSION VERSION_STR(ESP_ARDUINO_VERSION_MAJOR, ESP_ARDUINO_VERSION_MINOR, ESP_ARDUINO_VERSION_PATCH)
#endif

#endif //_PLATFORM_H_