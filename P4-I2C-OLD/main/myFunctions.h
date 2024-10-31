#include "bme280.h"
#include "bme280_defs.h"
#include "driver/i2c.h"
#include "esp_log.h"

#define I2C_MASTER_SCL_IO GPIO_NUM_22
#define I2C_MASTER_SDA_IO GPIO_NUM_21
#define I2C_MASTER_NUM I2C_NUM_0
#define I2C_MASTER_TIMEOUT_MS 10000
#define I2C_MASTER_CLK 1000000

esp_err_t device_register_read(uint8_t, uint8_t *, size_t);
esp_err_t device_register_write_byte(uint8_t reg_ddr, const uint8_t *, uint32_t, void *);

esp_err_t i2c_master_init(void);

void bme280_delay_us(uint32_t, void *);
void bme280_config_init(struct bme280_dev *, struct bme280_settings *);

uint8_t readRegisters(struct bme280_data *, struct bme280_dev *);
void printRegisters(struct bme280_data *, uint8_t, int8_t);