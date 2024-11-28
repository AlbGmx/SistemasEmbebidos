#include "my_functions.h"
#include "my_tcp.h"

static const char *TAG = "Proyecto";

// Global variables
extern EventGroupHandle_t wifi_event_group;
extern EventGroupHandle_t tcp_event_group;
extern char deviceNumber[MAX_CHAR];
extern char ssid[MAX_CHAR];
extern char pass[MAX_CHAR];
extern httpd_handle_t server;

sensors_data_struct_t sensors_data_struct[2] = {
    {.device_id = 0},
    {.device_id = 1},
};

sensors_packet_history_t history[2] = {
    {.data = {}, .head = -1, .count = 0},
    {.data = {}, .head = -1, .count = 0},
};

void app_main(void) {
   ESP_ERROR_CHECK(nvs_flash_init());
   wifi_init();
   gpio_init();
   init_history(&history[0]);
   init_history(&history[1]);

   server = start_webserver();
   xTaskCreate(tcp_server_task, "tcp_server", 4096 * 8, NULL, 5, NULL);
}
