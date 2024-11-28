#ifndef MY_FUNCTIONS_H
#define MY_FUNCTIONS_H
#include <esp_http_server.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bme280.h"
#include "bme280_defs.h"
#include "driver/gpio.h"
#include "driver/i2c.h"
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
#include "nvs_flash.h"

// Constants
// #define SSID "ESP_NET"
// #define PASS "ESP_NET_IOT"
// #define SSID "IoT_AP"
// #define PASS "12345678"
// Default AP NAME
#define AP_SSID "ESP_AP_WEB"
#define MAX_CHAR 32
#define HISTORY_SIZE 10
#define DATA_SEND_RATE 10

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
#define MINIMUM_DELAY_MS 10
#define RELEASED 1
#define PUSHED 0

#define WIFI_AP_STARTED_BIT BIT0
#define WIFI_STA_STARTED_BIT BIT1
#define WIFI_CONNECTED_TO_AP_BIT BIT2

#define I2C_MASTER_SCL_IO GPIO_NUM_22
#define I2C_MASTER_SDA_IO GPIO_NUM_21
#define I2C_MASTER_NUM I2C_NUM_0
#define I2C_MASTER_TIMEOUT_MS 10000
#define I2C_MASTER_CLK 1000000
#define SECOND_IN_MILLIS 1000

#define MPU6050_ADDR 0x68             // Dirección I2C del MPU6050
#define MPU6050_REG_PWR_MGMT_1 0x6B   // Registro de gestión de energía
#define MPU6050_REG_GYRO_XOUT_H 0x43  // Registro de giroscopio (X alto)
#define MPU6050_HEADER 0x27
#define MPU6050_CMD_REQUEST_DATA 0x9F
#define MPU6050_GYRO_SENSITIVITY_250DPS 131.0

typedef struct {
   uint8_t device_id;
   struct bme280_dev dev;
   struct bme280_data sensor_data;
   struct bme280_settings settings;
   float gyro;
} sensors_data_struct_t;

#pragma pack(push, 1)  // exact fit - no padding
typedef struct {
   uint8_t device_id;  // 1 byte
   float temperature;  // 4 bytes
   float pressure;     // 4 bytes
   float humidity;     // 4 bytes
   float gyro;         // 2 bytes
} sensors_packet_t;
#pragma pack(pop)

typedef struct {
   sensors_packet_t data[HISTORY_SIZE];
   int head;
   int count;
} sensors_packet_history_t;

void init_history(sensors_packet_history_t *history);

void add_to_history(sensors_packet_history_t *history, sensors_packet_t *packet, uint8_t device_id);

void print_history(sensors_packet_history_t *history);

extern int sock;
extern httpd_handle_t server;
extern EventGroupHandle_t tcp_event_group;
extern EventGroupHandle_t tcp_event_group;
extern char deviceNumber[MAX_CHAR];
extern char ssid[MAX_CHAR];
extern char pass[MAX_CHAR];

void gpio_init(void);

int read_led(void);

float read_temperature(uint8_t);

float read_humidity(uint8_t);

float read_pressure(uint8_t);

float read_gyroscope(uint8_t);

esp_err_t info_get_handler(httpd_req_t *);

extern const httpd_uri_t infoSite;

httpd_handle_t start_webserver(void);

const char *format_mac_address(const uint8_t[6]);

void wifi_event_handler(void *, esp_event_base_t, int32_t, void *);

void wifi_init(void);

void wifi_init_sta(void);

esp_err_t device_register_read(uint8_t, uint8_t *, size_t);

esp_err_t device_register_write_byte(uint8_t, const uint8_t *, uint32_t, void *);

esp_err_t i2c_master_init(void);

void i2c_sensor_init(void);

void mpu6050_init(void);

float mpu6050_read_gyro_x();

void bme280_delay_us(uint32_t, void *);
void bme280_config_init(struct bme280_dev *, struct bme280_settings *);

uint8_t read_BME280_registers(struct bme280_data *, struct bme280_dev *);
void send_sensor_data_binary(sensors_data_struct_t *);

void delaySeconds(uint32_t);
void delayMilliseconds(uint32_t);

void sensors_task(void *);

void init_history(sensors_packet_history_t *);

void add_to_history(sensors_packet_history_t *, sensors_packet_t *, uint8_t);

void print_history(sensors_packet_history_t *);
#endif  // MY_FUNCTIONS_H