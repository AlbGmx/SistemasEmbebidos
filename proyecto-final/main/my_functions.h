#ifndef MY_FUNCTIONS_H
#define MY_FUNCTIONS_H
#include "bme280.h"
#include "bme280_defs.h"
#include "driver/i2c.h"
#include "esp_log.h"

#define I2C_MASTER_SCL_IO GPIO_NUM_22
#define I2C_MASTER_SDA_IO GPIO_NUM_21
#define I2C_MASTER_NUM I2C_NUM_0
#define I2C_MASTER_TIMEOUT_MS 10000
#define I2C_MASTER_CLK 1000000
#define SECOND_IN_MILLIS 1000

esp_err_t device_register_read(uint8_t, uint8_t *, size_t);
esp_err_t device_register_write_byte(uint8_t reg_ddr, const uint8_t *, uint32_t, void *);

esp_err_t i2c_master_init(void);

typedef struct {
   uint8_t who_am_i;
   struct bme280_dev dev;
   struct bme280_data sensor_data;
   struct bme280_settings settings;
} bme280_data_struct_t;

#pragma pack(push, 1)  // Ensure no padding between structure members
struct sensor_packet {
   uint8_t who_am_i;   // 1 byte
   float temperature;  // 4 bytes
   float pressure;     // 4 bytes
   float humidity;     // 4 bytes
};
#pragma pack(pop)

static const char *TAG_FUNCTIONS = "Custom Functions";
extern int sock;

void bme280_delay_us(uint32_t, void *);
void bme280_config_init(struct bme280_dev *, struct bme280_settings *);

uint8_t readRegisters(struct bme280_data *, struct bme280_dev *);
void send_sensor_data_binary(struct bme280_data *sensor_data, uint8_t who_am_i);

void delaySeconds(uint32_t);
void delayMilliseconds(uint32_t);

void bme280_task(void *pvParams);

#endif  // MY_FUNCTIONS_H