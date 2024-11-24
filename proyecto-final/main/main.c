#include "my_functions.h"
#include "my_tcp.h"

static const char *TAG = "Proyecto";

// Global variables
extern int sock;
extern EventGroupHandle_t wifi_event_group;
extern EventGroupHandle_t tcp_event_group;
extern char deviceNumber[MAX_CHAR];
extern char ssid[MAX_CHAR];
extern char pass[MAX_CHAR];
extern httpd_handle_t server;
sensors_data_struct_t sensors_data_struct = {
    .who_am_i = 0,
    .dev = {0},
    .sensor_data = {0},
    .settings = {0},
    .gyro = 0,
};

void app_main(void) {
   ESP_ERROR_CHECK(nvs_flash_init());
   wifi_init();
   gpio_init();

   server = start_webserver();
   xTaskCreate(tcp_server_task, "tcp_server", 4096 * 8, (void *)sock, 5, NULL);
   xEventGroupWaitBits(tcp_event_group, TCP_CONNECTED_BIT, pdFALSE, pdTRUE, portMAX_DELAY);
   xTaskCreate(tcp_receive_task, "tcp_receive", 4096 * 8, (void *)sock, 5, NULL);
}
