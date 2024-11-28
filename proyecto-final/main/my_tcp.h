#ifndef MY_TCP_H
#define MY_TCP_H
#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "esp_log.h"
#include "esp_netif.h"
#include "freertos/event_groups.h"
#include "my_functions.h"
#include "sdkconfig.h"

#define HOST_IP_ADDR "192.168.4.1"
#define PORT 8266
#define KEEPALIVE_IDLE 15
#define KEEPALIVE_INTERVAL 10
#define KEEPALIVE_COUNT 1
#define BUFFER_MAX 4096
#define TCP_CONNECTED_BIT BIT0
#define TCP_CONNECTED_TWO_DEVICES_BIT BIT1
#define SENSOR_DATA_READ_BIT BIT2
#define SENSOR_DATA_RECEIVED_BIT BIT3
#define WIFI_CONNECTED_BIT BIT4

extern EventGroupHandle_t tcp_event_group;
extern sensors_data_struct_t sensors_data_struct[2];
extern sensors_packet_history_t history[2];

void tcp_send(int sock, const void *data, size_t len);
void tcp_data_received(sensors_packet_t *received_packet);
void tcp_client_task();
void tcp_server_task(void *pvParameters);

#endif  // MY_TCP_H