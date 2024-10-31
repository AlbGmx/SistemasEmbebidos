#ifndef myWifi_h
#define myWifi_h
#include <string.h>

#include "esp_event.h"
#include "esp_log.h"
#include "esp_mac.h"
#include "esp_netif.h"
#include "esp_netif_net_stack.h"
#include "esp_wifi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/task.h"
#include "lwip/err.h"
#include "lwip/inet.h"
#include "lwip/netdb.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include "nvs_flash.h"

#define EXAMPLE_ESP_WIFI_STA_SSID "ESP_NET"
#define EXAMPLE_ESP_WIFI_STA_PASSWD "ESP_NET_IOT"
#define EXAMPLE_ESP_MAXIMUM_RETRY 5

/* AP Configuration */
#define EXAMPLE_ESP_WIFI_SSID "ESP32"
#define EXAMPLE_ESP_WIFI_PASS "12345678"
#define EXAMPLE_ESP_WIFI_CHANNEL 1
#define EXAMPLE_MAX_STA_CONN 4

#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT BIT1

void wifi_event_handler(void*, esp_event_base_t, int32_t, void*);
void wifi_init_softap(void);
esp_err_t init_nvs();

#endif
