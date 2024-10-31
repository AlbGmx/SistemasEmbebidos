#include "myServer.h"
#include "myWifi.h"

static const char *TAG = "HTTP Server";

void app_main(void) {
   static httpd_handle_t server = NULL;
   ESP_ERROR_CHECK(init_nvs());
   wifi_init_softap();

   /*
    * Register event handlers to stop the server when Wi-Fi or Ethernet is disconnected,
    * and re-start it upon connection.
    */
   ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &connect_handler, &server));
   ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, &disconnect_handler, &server));

   server = start_webserver();
   while (server) {
      sleep(5);
   }
}
