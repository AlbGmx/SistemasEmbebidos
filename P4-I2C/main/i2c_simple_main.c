#include "myFunctions.h"

static const char *TAG = "I2C BME280";

esp_err_t rslt;
uint8_t who_am_i;
struct bme280_dev dev;
struct bme280_settings settings;
struct bme280_data sensor_data;

static void bme280_task() {

   while (1) {
      rslt = bme280_set_sensor_mode(BME280_POWERMODE_FORCED, &dev);
      rslt = device_register_read(BME280_REG_CHIP_ID, &who_am_i, 1);
      rslt = readRegisters(&sensor_data, &dev);
      printRegisters(&sensor_data, who_am_i, rslt);
      vTaskDelay(3000 / portTICK_PERIOD_MS);
   }
}

void app_main(void) {
   ESP_ERROR_CHECK(i2c_master_init());
   ESP_LOGI(TAG, "I2C initialized successfully");
   bme280_config_init(&dev, &settings);

   xTaskCreate(bme280_task, "bme280_task", 4096, NULL, 5, NULL);
}
