#include "my_tcp.h"

int sock = 0;
EventGroupHandle_t tcp_event_group = NULL;
static const char *TAG_TCP = "TCP";

void tcp_send(int sock, const void *data, size_t data_size) {
   xEventGroupWaitBits(tcp_event_group, TCP_CONNECTED_BIT || SENSOR_DATA_READ_BIT, pdFALSE, pdTRUE, portMAX_DELAY);
   xEventGroupClearBits(tcp_event_group, SENSOR_DATA_READ_BIT);
   int err = send(sock, data, data_size, 0);
   if (err < 0) {
      ESP_LOGE(TAG_TCP, "Error occurred during sending: errno %d", errno);
   } else {
      ESP_LOGI(TAG_TCP, "Message sent, size: %d bytes", data_size);
   }
}

void tcp_receive_task(void *pvParameters) {
   if (sock < 0) {
      ESP_LOGE(TAG_TCP, "Socket is invalid or closed");
      return;
   }

   while (true) {
      ESP_LOGI(TAG_TCP, "Waiting for data");

      xEventGroupWaitBits(tcp_event_group, TCP_CONNECTED_BIT, pdFALSE, pdTRUE, portMAX_DELAY);

      sensors_packet_t received_packet;

      struct timeval timeout;
      timeout.tv_sec = portMAX_DELAY;
      timeout.tv_usec = 0;
      setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));

      int bytes = recv(sock, (uint8_t *)&received_packet, sizeof(received_packet), 0);
      if (bytes < 0) {
         ESP_LOGE(TAG_TCP, "Error receiving data: errno %d", errno);
         break;
      }
      if (bytes == 0) {
         ESP_LOGW(TAG_TCP, "Connection closed by peer");
         break;
      }
      ESP_LOGI(TAG_TCP, "Received %d bytes sensor data: who_am_i=0x%x, temp=%.1f, press=%.1f, humid=%.2f, gyro=%.2f",
               bytes, received_packet.who_am_i, received_packet.temperature, received_packet.pressure,
               received_packet.humidity, received_packet.gyro);
      sensors_data_struct.who_am_i = received_packet.who_am_i;
      sensors_data_struct.sensor_data.temperature = received_packet.temperature;
      sensors_data_struct.sensor_data.pressure = received_packet.pressure;
      sensors_data_struct.sensor_data.humidity = received_packet.humidity;
      sensors_data_struct.gyro = received_packet.gyro;
      delaySeconds(1);
   }
}

void tcp_client_task(void) {
   tcp_event_group = xEventGroupCreate();
   char host_ip[] = HOST_IP_ADDR;
   int addr_family = 0;
   int ip_protocol = 0;

   while (1) {
      struct sockaddr_in dest_addr;
      inet_pton(AF_INET, host_ip, &dest_addr.sin_addr);
      dest_addr.sin_family = AF_INET;
      dest_addr.sin_port = htons(PORT);
      addr_family = AF_INET;
      ip_protocol = IPPROTO_IP;

      sock = socket(addr_family, SOCK_STREAM, ip_protocol);
      if (sock < 0) {
         ESP_LOGE(TAG_TCP, "Unable to create socket: errno %d", errno);
         xEventGroupClearBits(tcp_event_group, TCP_CONNECTED_BIT);
         break;
      }
      ESP_LOGI(TAG_TCP, "Socket created, connecting to %s:%d", host_ip, PORT);

      int err = connect(sock, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
      if (err != 0) {
         ESP_LOGE(TAG_TCP, "Socket unable to connect: errno %d", errno);
         xEventGroupClearBits(tcp_event_group, TCP_CONNECTED_BIT);
         break;
      }
      ESP_LOGI(TAG_TCP, "Successfully connected");
      xEventGroupSetBits(tcp_event_group, TCP_CONNECTED_BIT);

      while (1) {
         vTaskDelay(portMAX_DELAY);
      }

      if (sock != -1) {
         ESP_LOGE(TAG_TCP, "Shutting down socket and restarting...");
         xEventGroupClearBits(tcp_event_group, TCP_CONNECTED_BIT);
         shutdown(sock, 0);
         close(sock);
      }
   }
}

void tcp_server_task(void *pvParameters) {
   tcp_event_group = xEventGroupCreate();
   char addr_str[128];
   int ip_protocol = 0;
   int keepAlive = 1;
   int keepIdle = KEEPALIVE_IDLE;
   int keepInterval = KEEPALIVE_INTERVAL;
   int keepCount = KEEPALIVE_COUNT;
   struct sockaddr_storage dest_addr;

   struct sockaddr_in *dest_addr_ip4 = (struct sockaddr_in *)&dest_addr;
   dest_addr_ip4->sin_addr.s_addr = htonl(INADDR_ANY);
   dest_addr_ip4->sin_family = AF_INET;
   dest_addr_ip4->sin_port = htons(PORT);
   ip_protocol = IPPROTO_IP;

   int listen_sock = socket(AF_INET, SOCK_STREAM, ip_protocol);
   if (listen_sock < 0) {
      ESP_LOGE(TAG_TCP, "Unable to create socket: errno %d", errno);
      vTaskDelete(NULL);
      return;
   }
   int opt = 1;
   setsockopt(listen_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

   ESP_LOGI(TAG_TCP, "Socket created");

   int err = bind(listen_sock, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
   if (err != 0) {
      ESP_LOGE(TAG_TCP, "Socket unable to bind: errno %d", errno);
      ESP_LOGE(TAG_TCP, "IPPROTO: %d", AF_INET);
      goto CLEAN_UP;
   }
   ESP_LOGI(TAG_TCP, "Socket bound, TCP port %d", PORT);

   err = listen(listen_sock, 1);
   if (err != 0) {
      ESP_LOGE(TAG_TCP, "Error occurred during listen: errno %d", errno);
      goto CLEAN_UP;
   }

   while (1) {
      ESP_LOGI(TAG_TCP, "Socket listening");

      struct sockaddr_storage source_addr;
      socklen_t addr_len = sizeof(source_addr);
      sock = accept(listen_sock, (struct sockaddr *)&source_addr, &addr_len);
      if (sock < 0) {
         ESP_LOGE(TAG_TCP, "Unable to accept connection: errno %d", errno);
         break;
      }

      ESP_LOGW(TAG_TCP, "Socket %d", sock);

      // Set tcp keepalive option
      setsockopt(sock, SOL_SOCKET, SO_KEEPALIVE, &keepAlive, sizeof(int));
      setsockopt(sock, IPPROTO_TCP, TCP_KEEPIDLE, &keepIdle, sizeof(int));
      setsockopt(sock, IPPROTO_TCP, TCP_KEEPINTVL, &keepInterval, sizeof(int));
      setsockopt(sock, IPPROTO_TCP, TCP_KEEPCNT, &keepCount, sizeof(int));
      // Convert ip address to string
      if (source_addr.ss_family == PF_INET) {
         inet_ntoa_r(((struct sockaddr_in *)&source_addr)->sin_addr, addr_str, sizeof(addr_str) - 1);
      }
      ESP_LOGI(TAG_TCP, "Socket accepted ip address: %s", addr_str);
      xEventGroupSetBits(tcp_event_group, TCP_CONNECTED_BIT);
      while (1) {
         vTaskDelay(portMAX_DELAY);
      }

      shutdown(sock, 0);
      close(sock);
   }

CLEAN_UP:
   close(listen_sock);
   vTaskDelete(NULL);
}
