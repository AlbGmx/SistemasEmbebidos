#include "my_functions.h"

#include "my_tcp.h"

esp_err_t device_register_read(uint8_t reg_addr, uint8_t *data, size_t len) {
   return i2c_master_write_read_device(I2C_MASTER_NUM, BME280_I2C_ADDR_SEC, &reg_addr, 1, data, len,
                                       I2C_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS);
}

esp_err_t device_register_write_byte(uint8_t reg_addr, const uint8_t *data, uint32_t len, void *intf_ptr) {
   i2c_cmd_handle_t cmd = i2c_cmd_link_create();
   i2c_master_start(cmd);
   i2c_master_write_byte(cmd, BME280_I2C_ADDR_SEC << 1 | I2C_MASTER_WRITE, true);
   i2c_master_write_byte(cmd, reg_addr, true);
   i2c_master_write(cmd, data, len, true);

   i2c_master_stop(cmd);
   esp_err_t ret = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, 1000 / portTICK_PERIOD_MS);
   i2c_cmd_link_delete(cmd);
   return ret;
}

esp_err_t i2c_master_init(void) {
   int i2c_master_port = I2C_MASTER_NUM;

   i2c_config_t conf = {
       .mode = I2C_MODE_MASTER,
       .sda_io_num = I2C_MASTER_SDA_IO,
       .scl_io_num = I2C_MASTER_SCL_IO,
       .sda_pullup_en = GPIO_PULLUP_ENABLE,
       .scl_pullup_en = GPIO_PULLUP_ENABLE,
       .master.clk_speed = I2C_MASTER_CLK,
       .clk_flags = 0,
   };

   esp_err_t err = i2c_param_config(i2c_master_port, &conf);
   if (err != ESP_OK) {
      return err;
   }
   return i2c_driver_install(i2c_master_port, conf.mode, 0, 0, 0);
}

void bme280_delay_us(uint32_t period, void *intf_ptr) { vTaskDelay(period / portTICK_PERIOD_MS); }

void bme280_config_init(struct bme280_dev *dev, struct bme280_settings *settings) {
   int8_t rslt = BME280_OK;

   dev->chip_id = BME280_I2C_ADDR_SEC;
   dev->intf = BME280_I2C_INTF;
   dev->read = (bme280_read_fptr_t)device_register_read;
   dev->write = (bme280_write_fptr_t)device_register_write_byte;
   dev->delay_us = (bme280_delay_us_fptr_t)bme280_delay_us;
   rslt = bme280_init(dev);

   settings->osr_h = BME280_OVERSAMPLING_1X;
   settings->osr_p = BME280_OVERSAMPLING_16X;
   settings->osr_t = BME280_OVERSAMPLING_2X;
   settings->filter = BME280_FILTER_COEFF_16;
   settings->standby_time = BME280_STANDBY_TIME_0_5_MS;
   rslt = bme280_set_sensor_settings(BME280_SEL_ALL_SETTINGS, settings, dev);
}

uint8_t readRegisters(struct bme280_data *sensor_data, struct bme280_dev *dev) {
   uint8_t rslt = BME280_OK;
   uint8_t reg_data[BME280_LEN_P_T_H_DATA] = {0};
   rslt = bme280_get_sensor_data(BME280_ALL, sensor_data, dev);
   return rslt;
}

void send_sensor_data_binary(struct bme280_data *sensor_data, uint8_t who_am_i) {
   struct sensor_packet packet = {
       .who_am_i = who_am_i,  // sanity check
       .temperature = sensor_data->temperature,
       .pressure = sensor_data->pressure,
       .humidity = sensor_data->humidity,
   };
   ESP_LOGI(TAG_FUNCTIONS, "Sending sensor data: who_am_i=0x%x, temp=%.1f, press=%.1f, humid=%.2f", packet.who_am_i,
            packet.temperature, packet.pressure, packet.humidity);
   tcp_send(sock, &packet, sizeof(packet));
}

void delaySeconds(uint32_t seconds) { vTaskDelay(seconds * SECOND_IN_MILLIS / portTICK_PERIOD_MS); }

void delayMillis(uint32_t millis) { vTaskDelay(millis / portTICK_PERIOD_MS); }

void bme280_task(void *pvParameters) {
   bme280_data_struct_t *bme280_data_struct = (bme280_data_struct_t *)pvParameters;
   int8_t rslt = BME280_OK;

   while (1) {
      rslt = bme280_set_sensor_mode(BME280_POWERMODE_FORCED, &bme280_data_struct->dev);
      rslt = device_register_read(BME280_REG_CHIP_ID, &bme280_data_struct->who_am_i, 1);
      rslt = readRegisters(&bme280_data_struct->sensor_data, &bme280_data_struct->dev);
      send_sensor_data_binary(&bme280_data_struct->sensor_data, bme280_data_struct->who_am_i);
      delaySeconds(1);
   }
}