
// SLAVE
#include <stdio.h>

#include "driver/i2c.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define I2C_SENSOR_SCL_IO 22       // Pin SCL para el sensor
#define I2C_SENSOR_SDA_IO 21       // Pin SDA para el sensor
#define I2C_SENSOR_NUM I2C_NUM_0   // Bus I2C para el sensor
#define I2C_SENSOR_FREQ_HZ 100000  // Frecuencia I2C

#define I2C_SLAVE_SCL_IO 26      // Pin SCL para el esclavo
#define I2C_SLAVE_SDA_IO 25      // Pin SDA para el esclavo
#define I2C_SLAVE_NUM I2C_NUM_1  // Bus I2C para el esclavo
#define I2C_SLAVE_ADDR 0x28      // Dirección del esclavo

#define MPU6050_ADDR 0x68             // Dirección I2C del MPU6050
#define MPU6050_REG_PWR_MGMT_1 0x6B   // Registro de gestión de energía
#define MPU6050_REG_GYRO_XOUT_H 0x43  // Registro de giroscopio (X alto)

#define HEADER 0x27
#define CMD_REQUEST_DATA 0x9F
#define GYRO_SENSITIVITY_250DPS 131.0

// Función para inicializar el I2C en modo máster para el sensor
void i2c_sensor_init() {
   i2c_config_t conf = {
       .mode = I2C_MODE_MASTER,
       .sda_io_num = I2C_SENSOR_SDA_IO,
       .scl_io_num = I2C_SENSOR_SCL_IO,
       .sda_pullup_en = GPIO_PULLUP_ENABLE,
       .scl_pullup_en = GPIO_PULLUP_ENABLE,
       .master.clk_speed = I2C_SENSOR_FREQ_HZ,
   };
   i2c_param_config(I2C_SENSOR_NUM, &conf);
   i2c_driver_install(I2C_SENSOR_NUM, conf.mode, 0, 0, 0);
}

void i2c_slave_init() {
   i2c_config_t conf = {
       .mode = I2C_MODE_SLAVE,
       .sda_io_num = I2C_SLAVE_SDA_IO,
       .scl_io_num = I2C_SLAVE_SCL_IO,
       .sda_pullup_en = GPIO_PULLUP_ENABLE,
       .scl_pullup_en = GPIO_PULLUP_ENABLE,
       .slave.addr_10bit_en = 0,
       .slave.slave_addr = I2C_SLAVE_ADDR,
   };
   i2c_param_config(I2C_SLAVE_NUM, &conf);
   // Instalar el driver del I2C
   esp_err_t err = i2c_driver_install(I2C_SLAVE_NUM, conf.mode, 128, 128, 0);
   if (err != ESP_OK) {
      printf("Error al instalar el driver I2C: %s\n", esp_err_to_name(err));
   }
}

// Función para inicializar el MPU6050 (sacarlo del modo sleep)
void mpu6050_init() {
   uint8_t data = 0;
   i2c_cmd_handle_t cmd = i2c_cmd_link_create();
   i2c_master_start(cmd);
   i2c_master_write_byte(cmd, (MPU6050_ADDR << 1) | I2C_MASTER_WRITE, true);
   i2c_master_write_byte(cmd, MPU6050_REG_PWR_MGMT_1, true);
   i2c_master_write_byte(cmd, data, true);
   i2c_master_stop(cmd);
   i2c_master_cmd_begin(I2C_SENSOR_NUM, cmd, 1000 / portTICK_PERIOD_MS);
   i2c_cmd_link_delete(cmd);
}

int16_t mpu6050_read_gyro_x() {
   uint8_t data_h, data_l;
   i2c_cmd_handle_t cmd = i2c_cmd_link_create();

   // Leer el byte alto
   i2c_master_start(cmd);
   i2c_master_write_byte(cmd, (MPU6050_ADDR << 1) | I2C_MASTER_WRITE, true);
   i2c_master_write_byte(cmd, MPU6050_REG_GYRO_XOUT_H, true);
   i2c_master_stop(cmd);
   i2c_master_cmd_begin(I2C_SENSOR_NUM, cmd, 1000 / portTICK_PERIOD_MS);
   i2c_cmd_link_delete(cmd);

   // Leer el byte bajo
   cmd = i2c_cmd_link_create();
   i2c_master_start(cmd);
   i2c_master_write_byte(cmd, (MPU6050_ADDR << 1) | I2C_MASTER_READ, true);
   i2c_master_read_byte(cmd, &data_h, I2C_MASTER_ACK);
   i2c_master_read_byte(cmd, &data_l, I2C_MASTER_NACK);
   i2c_master_stop(cmd);
   i2c_master_cmd_begin(I2C_SENSOR_NUM, cmd, 1000 / portTICK_PERIOD_MS);
   i2c_cmd_link_delete(cmd);

   int16_t gyro_value = (data_h << 8) | data_l;

   // Agrega un mensaje de depuración aquí
   printf("Giroscopio leído: %d (%02X %02X)\n", gyro_value, data_h, data_l);

   return gyro_value;
}

void send_gyro_data_to_master() {
   int16_t gyro_x = mpu6050_read_gyro_x();  // Lee el valor del giroscopio X
   uint8_t response[4] = {
       HEADER,                // Encabezado
       CMD_REQUEST_DATA,      // Comando
       (gyro_x >> 8) & 0xFF,  // Byte alto del giroscopio
       gyro_x & 0xFF          // Byte bajo del giroscopio
   };

   // Imprimir los datos que se enviarán al maestro
   printf("Enviando respuesta al maestro: %02X %02X %02X %02X\n", response[0], response[1], response[2], response[3]);

   // Inicia el comando para enviar la respuesta

   i2c_slave_write_buffer(I2C_SLAVE_NUM, response, 4, -1);
}

void i2c_slave_task() {
   uint8_t data[2];
   while (1) {
      int ret = i2c_slave_read_buffer(I2C_SLAVE_NUM, data, sizeof(data), portMAX_DELAY);
      if (ret > 0) {
         printf("Solicitud recibida del máster: %02X %02X\n", data[0], data[1]);
         if (data[0] == HEADER && data[1] == CMD_REQUEST_DATA) {
            printf("Comando de solicitud de datos recibido\n");
            send_gyro_data_to_master();
         }
      }
   }
}

void app_main() {
   i2c_sensor_init();  // Initialize the I2C master for the sensor
   i2c_slave_init();   // Initialize the I2C slave
   mpu6050_init();     // Initialize the MPU6050 sensor
   xTaskCreate(i2c_slave_task, "i2c_slave_task", 2048, NULL, 10, NULL);
}
