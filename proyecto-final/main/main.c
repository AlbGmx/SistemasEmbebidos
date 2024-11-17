#include <esp_http_server.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bme280.h"
#include "bme280_defs.h"
#include "driver/gpio.h"
#include "esp_eth.h"
#include "esp_event.h"
#include "esp_http_server.h"
#include "esp_log.h"
#include "esp_mac.h"
#include "esp_netif.h"
#include "esp_netif_net_stack.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/task.h"
#include "lwip/err.h"
#include "lwip/inet.h"
#include "lwip/netdb.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include "my_functions.h"
#include "my_tcp.h"
#include "nvs_flash.h"

// Constants
// #define SSID "ESP_NET"
// #define PASS "ESP_NET_IOT"
// #define SSID "IoT_AP"
// #define PASS "12345678"
// Default AP NAME
#define AP_SSID "ESP_AP_CONFIG"
#define MAX_CHAR 32

#define LED GPIO_NUM_2
#define ADC_SELECTED GPIO_NUM_34
#define ADC1_CHANNEL ADC_CHANNEL_6
#define ADC_WIDTH ADC_BITWIDTH_12
#define ADC_ATTEN ADC_ATTEN_DB_0
#define WIFI_RETRY_MAX 20
#define BUFFER_SIZE 128
#define PORT 8266
#define RESTART_PIN GPIO_NUM_22

#define BUTTON_SEND_MESSAGE GPIO_NUM_23
#define BUTTON_BOUNCE_TIME 150
#define SECOND_IN_MILLIS 1000
#define SEND_MESSAGE_DELAY_TIME 60 * SECOND_IN_MILLIS
#define RELEASED 0
#define PRESSED 1
#define MINIMUM_DELAY_MS 10
#define FREED 1
#define PUSHED 0

#define WIFI_AP_STARTED_BIT BIT0
#define WIFI_STA_STARTED_BIT BIT1
#define WIFI_CONNECTED_TO_AP_BIT BIT2

static const char *TAG = "Proyecto";
static const char *TAG_NVS = "Proyecto";
static const char *TAG_WIFI_AP = "Proyecto";
static const char *TAG_SERVER = "Proyecto";

// Global variables
int sock = 0;
EventGroupHandle_t wifi_event_group = NULL;
EventGroupHandle_t tcp_event_group;
char deviceNumber[MAX_CHAR] = {0};
char ssid[MAX_CHAR] = {0};
char pass[MAX_CHAR] = {0};
httpd_handle_t server = NULL;
esp_err_t rslt;
bme280_data_struct_t bme280_data_struct = {
    .who_am_i = 0,
    .dev = {0},
    .sensor_data = {0},
    .settings = {0},
};

void gpio_init() {
   gpio_config_t io_conf;
   io_conf.intr_type = GPIO_INTR_DISABLE;
   io_conf.mode = GPIO_MODE_INPUT_OUTPUT;
   io_conf.pin_bit_mask = (1ULL << LED);
   io_conf.pull_down_en = GPIO_PULLDOWN_ENABLE;
   io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
   gpio_config(&io_conf);

   io_conf.intr_type = GPIO_INTR_DISABLE;
   io_conf.mode = GPIO_MODE_INPUT;
   io_conf.pin_bit_mask = (1ULL << ADC_SELECTED);
   io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
   io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
   gpio_config(&io_conf);

   io_conf.intr_type = GPIO_INTR_NEGEDGE;
   io_conf.mode = GPIO_MODE_INPUT;
   io_conf.pin_bit_mask = (1 << BUTTON_SEND_MESSAGE | 1 << RESTART_PIN);
   io_conf.pull_down_en = 0;
   io_conf.pull_up_en = 1;
   gpio_config(&io_conf);
}

int read_led() {
   int led_state = gpio_get_level(LED);
   ESP_LOGI(TAG, "LED state is: %d", led_state);
   return led_state;
}

float read_temperature() { return bme280_data_struct.sensor_data.temperature; }

float read_humidity() { return bme280_data_struct.sensor_data.humidity; }

float read_pressure() { return bme280_data_struct.sensor_data.pressure; }

float read_gyroscope() { return 0.0; }

esp_err_t set_nvs_creds_and_name(const char *ssid, const char *pass, char *deviceNumber) {
   nvs_handle_t nvs_handle;
   ESP_LOGI(TAG_NVS, "Setting %s, %s, %s", ssid, pass, deviceNumber);
   esp_err_t err = nvs_open("storage", NVS_READWRITE, &nvs_handle);
   if (err != ESP_OK) {
      ESP_LOGE(TAG_NVS, "Error (%s) opening NVS handle", esp_err_to_name(err));
      return err;
   }
   if (nvs_set_str(nvs_handle, "deviceNumber", deviceNumber) != ESP_OK) {
      ESP_LOGE(TAG_NVS, "Error setting device name");
      return ESP_ERR_NVS_NOT_FOUND;
   }
   if (nvs_set_str(nvs_handle, "ssid", ssid) != ESP_OK) {
      ESP_LOGE(TAG_NVS, "Error setting SSID");
      return ESP_ERR_NVS_NOT_FOUND;
   }
   if (nvs_set_str(nvs_handle, "pass", pass) != ESP_OK) {
      ESP_LOGE(TAG_NVS, "Error setting password");
      return ESP_ERR_NVS_NOT_FOUND;
   }
   nvs_close(nvs_handle);
   return ESP_OK;
}

void url_decode(char *str) {
   char *source = str;
   char *destination = str;
   while (*source) {
      if (*source == '%') {
         if (isxdigit((unsigned char)source[1]) && isxdigit((unsigned char)source[2])) {
            char hex[3] = {source[1], source[2], '\0'};
            *destination = (char)strtol(hex, NULL, 16);
            source += 3;
         } else {
            *destination = *source++;
         }
      } else if (*source == '+') {
         *destination = ' ';
         source++;
      } else {
         *destination = *source++;
      }
      destination++;
   }
   *destination = '\0';
}

esp_err_t info_get_handler(httpd_req_t *req) {
   extern unsigned char info_start[] asm("_binary_info_html_start");
   extern unsigned char info_end[] asm("_binary_info_html_end");
   size_t info_len = info_end - info_start;
   char infoHtml[info_len];

   memcpy(infoHtml, info_start, info_len);

   // ESP_LOGI(TAG_SERVER, "'%s'", infoHtml);
   snprintf(infoHtml, info_len, infoHtml, read_gyroscope(), read_temperature(), read_pressure(), read_humidity(),
            read_gyroscope(), read_temperature(), read_pressure(), read_humidity());

   httpd_resp_set_type(req, "text/html");
   httpd_resp_send(req, infoHtml, info_len);

   if (httpd_req_get_hdr_value_len(req, "Host") == 0) {
      ESP_LOGI(TAG_SERVER, "Request headers lost");
   }

   return ESP_OK;
}

static const httpd_uri_t infoSite = {
    .uri = "/info",
    .method = HTTP_GET,
    .handler = info_get_handler,
};

httpd_handle_t start_webserver(void) {
   httpd_handle_t server = NULL;
   httpd_config_t config = HTTPD_DEFAULT_CONFIG();
   config.stack_size = 4096 * 8;

   ESP_LOGI(TAG_SERVER, "Iniciando el servidor en el puerto: '%d'", config.server_port);
   if (httpd_start(&server, &config) == ESP_OK) {
      ESP_LOGI(TAG_SERVER, "Registrando manejadores de URI");
      httpd_register_uri_handler(server, &infoSite);
      return server;
   }

   ESP_LOGE(TAG_SERVER, "Error starting server!");
   return NULL;
}

static const char *format_mac_address(const uint8_t mac[6]) {
   static char mac_str[18];  // Enough space for "XX:XX:XX:XX:XX:XX\0"
   snprintf(mac_str, sizeof(mac_str), "%02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
   return mac_str;
}

static void wifi_event_handler(void *event_handler_arg, esp_event_base_t event_base, int32_t event_id,
                               void *event_data) {
   switch (event_id) {
      case WIFI_EVENT_AP_START:
         xEventGroupSetBits(wifi_event_group, WIFI_AP_STARTED_BIT);
         ESP_LOGI(TAG_WIFI_AP, "Access Point started");
         break;
      case WIFI_EVENT_AP_STOP:
         ESP_LOGI(TAG_WIFI_AP, "Access Point stopped");
         break;
      case WIFI_EVENT_AP_STACONNECTED:
         wifi_event_ap_staconnected_t *event_ap_staconnected = (wifi_event_ap_staconnected_t *)event_data;
         ESP_LOGI(TAG_WIFI_AP, "%s connected to AP, ID: %d", format_mac_address(event_ap_staconnected->mac),
                  event_ap_staconnected->aid);
         break;
      case WIFI_EVENT_AP_STADISCONNECTED:
         wifi_event_ap_stadisconnected_t *event_ap_stadisconnected = (wifi_event_ap_stadisconnected_t *)event_data;
         ESP_LOGI(TAG_WIFI_AP, "%s disconnected from AP, ID: %d", event_ap_stadisconnected->mac,
                  event_ap_stadisconnected->aid);
         break;
      default:
         ESP_LOGW(TAG, "Unhandled event ID: %ld", event_id);
         break;
   }
}

esp_err_t getWifiCredentials() {
   nvs_handle_t nvs_handle;
   esp_err_t err = nvs_open("storage", NVS_READWRITE, &nvs_handle);
   if (err != ESP_OK) {
      ESP_LOGE(TAG_NVS, "Error (%s) opening NVS handle", esp_err_to_name(err));
      return ESP_ERR_NVS_NOT_FOUND;
   }
   size_t device_size = sizeof(deviceNumber);
   size_t ssid_size = sizeof(ssid);
   size_t pass_size = sizeof(pass);
   if (nvs_get_str(nvs_handle, "deviceNumber", deviceNumber, &device_size) != ESP_OK) {
      ESP_LOGW(TAG_NVS, "Error getting device number");
      return ESP_ERR_NVS_NOT_FOUND;
   }
   if (nvs_get_str(nvs_handle, "ssid", ssid, &ssid_size) != ESP_OK) {
      ESP_LOGE(TAG_NVS, "Error getting SSID");
      return ESP_ERR_NVS_NOT_FOUND;
   }
   if (nvs_get_str(nvs_handle, "pass", pass, &pass_size) != ESP_OK) {
      ESP_LOGE(TAG_NVS, "Error getting password");
      return ESP_ERR_NVS_NOT_FOUND;
   }
   ESP_LOGW(TAG_NVS, "Device number: %s, SSID: %s, Password: %s", deviceNumber, ssid, pass);
   nvs_close(nvs_handle);
   return ESP_OK;
}

void wifi_init() {
   wifi_event_group = xEventGroupCreate();
   esp_netif_init();
   esp_event_loop_create_default();

   wifi_init_config_t wifi_initiation = WIFI_INIT_CONFIG_DEFAULT();
   esp_wifi_init(&wifi_initiation);

   esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, wifi_event_handler, NULL);
   esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, wifi_event_handler, NULL);

   esp_netif_create_default_wifi_ap();
   wifi_config_t wifi_config = {.ap = {.ssid = AP_SSID, .max_connection = 4, .authmode = WIFI_AUTH_OPEN}};
   esp_wifi_set_mode(WIFI_MODE_AP);
   esp_wifi_set_config(ESP_IF_WIFI_AP, &wifi_config);
   esp_wifi_start();
   ESP_LOGW(TAG_WIFI_AP, "Wi-Fi AP started as SSID: %s", AP_SSID);
}

void app_main(void) {
   ESP_ERROR_CHECK(nvs_flash_init());
   wifi_init();
   gpio_init();

   server = start_webserver();
   xTaskCreate(tcp_server_task, "tcp_server", 4096 * 8, (void *)sock, 5, NULL);
   xEventGroupWaitBits(tcp_event_group, TCP_CONNECTED_BIT, pdFALSE, pdTRUE, portMAX_DELAY);
   xTaskCreate(tcp_receive_task, "tcp_receive", 4096 * 8, (void *)sock, 5, NULL);
}
