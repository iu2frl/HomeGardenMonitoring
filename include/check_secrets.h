#include <secrets.h>

#ifndef WIFI_SSID
#error "WIFI_SSID is not defined in include/secrets.h. Please define it before compiling."
#endif

#ifndef WIFI_PASS
#error "WIFI_PASS is not defined in include/secrets.h. Please define it before compiling."
#endif

#ifndef SENSOR_NAME
#error "SENSOR_NAME is not defined in include/secrets.h. Please define it before compiling."
#endif

#ifndef INFLUXDB_URL
#error "INFLUXDB_URL is not defined in include/secrets.h. Please define it before compiling."
#endif

#ifndef INFLUXDB_TOKEN
#error "INFLUXDB_TOKEN is not defined in include/secrets.h. Please define it before compiling."
#endif

#ifndef INFLUXDB_ORG
#error "INFLUXDB_ORG is not defined in include/secrets.h. Please define it before compiling."
#endif

#ifndef INFLUXDB_BUCKET
#error "INFLUXDB_BUCKET is not defined in include/secrets.h. Please define it before compiling."
#endif
