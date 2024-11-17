/*
 * SPDX-FileCopyrightText: 2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Unlicense OR CC0-1.0
 */
#include "bme280.h"
#include "bme280_defs.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "my_functions.h"
#include "my_tcp.h"
#include "nvs_flash.h"
#include "protocol_examples_common.h"

EventGroupHandle_t tcp_event_group;

bme280_data_struct_t bme280_data_struct = {
    .who_am_i = 0,
    .dev = {0},
    .sensor_data = {0},
    .settings = {0},
};
int sock;

void app_main(void) {
   ESP_ERROR_CHECK(nvs_flash_init());
   ESP_ERROR_CHECK(esp_netif_init());
   ESP_ERROR_CHECK(i2c_master_init());
   ESP_ERROR_CHECK(esp_event_loop_create_default());
   ESP_ERROR_CHECK(example_connect());

   bme280_config_init(&bme280_data_struct.dev, &bme280_data_struct.settings);
   xTaskCreate(tcp_client_task, "tcp_client_task", 4096, NULL, 5, NULL);
   xTaskCreate(bme280_task, "bme280_task", 4096, (void *)&bme280_data_struct, 5, NULL);
}
