// MASTER
#include <stdio.h>

#include "driver/i2c.h"
#include "driver/uart.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define I2C_MASTER_SCL_IO 26       // Pin SCL del máster
#define I2C_MASTER_SDA_IO 25       // Pin SDA del máster
#define I2C_MASTER_NUM I2C_NUM_0   // Número del bus I2C
#define I2C_MASTER_FREQ_HZ 100000  // Frecuencia I2C
#define I2C_SLAVE_ADDR 0x28        // Dirección del esclavo
#define I2C_TIMEOUT_MS 500         // Tiempo de espera para la comunicación

#define HEADER 0x27
#define CMD_REQUEST_DATA 0x9F

void i2c_master_init() {
   i2c_config_t conf = {
       .mode = I2C_MODE_MASTER,
       .sda_io_num = I2C_MASTER_SDA_IO,
       .scl_io_num = I2C_MASTER_SCL_IO,
       .sda_pullup_en = GPIO_PULLUP_ENABLE,
       .scl_pullup_en = GPIO_PULLUP_ENABLE,
       .master.clk_speed = I2C_MASTER_FREQ_HZ,
   };
   i2c_param_config(I2C_MASTER_NUM, &conf);
   // Instalar el driver del I2C
   esp_err_t err = i2c_driver_install(I2C_MASTER_NUM, conf.mode, 0, 0, 0);
   if (err != ESP_OK) {
      printf("Error al instalar el driver I2C: %s\n", esp_err_to_name(err));
   }
}

// Envía la solicitud de datos al esclavo
esp_err_t request_data_from_slave() {
   uint8_t request[2] = {HEADER, CMD_REQUEST_DATA};
   i2c_cmd_handle_t cmd = i2c_cmd_link_create();
   i2c_master_start(cmd);
   i2c_master_write_byte(cmd, (I2C_SLAVE_ADDR << 1) | I2C_MASTER_WRITE, true);
   i2c_master_write(cmd, request, sizeof(request), true);
   i2c_master_stop(cmd);
   esp_err_t ret = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, I2C_TIMEOUT_MS / portTICK_PERIOD_MS);
   i2c_cmd_link_delete(cmd);

   // Agregar mensaje de depuración
   if (ret == ESP_OK) {
      printf("Solicitud enviada al esclavo: %02X %02X\n", request[0], request[1]);

   } else {
      printf("Error al enviar solicitud: %s\n", esp_err_to_name(ret));
   }

   return ret;
}

esp_err_t read_data_from_slave() {
   uint8_t response[4] = {0};  // Inicializa el buffer de respuesta
   i2c_cmd_handle_t cmd = i2c_cmd_link_create();
   i2c_master_start(cmd);

   // Escribe la dirección del esclavo para leer
   i2c_master_write_byte(cmd, (I2C_SLAVE_ADDR << 1) | I2C_MASTER_READ, true);

   // Leer los primeros 3 bytes con ACK
   i2c_master_read(cmd, response, 3, I2C_MASTER_ACK);

   // Leer el último byte con NACK
   i2c_master_read_byte(cmd, &response[3], I2C_MASTER_NACK);

   i2c_master_stop(cmd);
   esp_err_t ret = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, I2C_TIMEOUT_MS / portTICK_PERIOD_MS);
   i2c_cmd_link_delete(cmd);

   if (ret == ESP_OK) {
      // Imprimir la respuesta para depuración
      printf("Respuesta del esclavo: %02X %02X %02X %02X\n", response[0], response[1], response[2], response[3]);

      if (response[0] == HEADER && response[1] == CMD_REQUEST_DATA) {
         int16_t gyro_x = (response[2] << 8) | response[3];
         float gyro_x_dps = gyro_x / 131.0;  // Conversión de crudo a DPS para ±250 °/s
         printf("Giroscopio X: %.2f °/s\n", gyro_x_dps);
      } else {
         printf("Header o comando incorrecto: %02X %02X\n", response[0], response[1]);
      }
   } else {
      printf("Error en la lectura: %s\n", esp_err_to_name(ret));
   }
   return ret;
}

// Tarea para solicitar y leer los datos del esclavo
void i2c_master_task() {
   while (1) {
      esp_err_t ret;
      int attempts = 0;
      do {
         ret = request_data_from_slave();
         if (ret == ESP_OK) {
            ret = read_data_from_slave();
            if (ret == ESP_OK) {
               break;
            }
         }
         attempts++;
         vTaskDelay(500 / portTICK_PERIOD_MS);  // Espera de 500 ms antes de reintentar
      } while (attempts < 3);

      if (ret != ESP_OK) {
         printf("Comunicacion terminada, el periferico no responde\n");
      }

      vTaskDelay(2000 / portTICK_PERIOD_MS);  // Espera de 2 segundos antes de la siguiente solicitud
   }
}

void uart_init() {
   const int UART_NUM = UART_NUM_0;  // UART0 por defecto
   uart_config_t uart_config = {
       .baud_rate = 115200,
       .data_bits = UART_DATA_8_BITS,
       .parity = UART_PARITY_DISABLE,
       .stop_bits = UART_STOP_BITS_1,
       .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
   };
   uart_driver_install(UART_NUM, 2048, 0, 0, NULL, 0);
   uart_param_config(UART_NUM, &uart_config);
}

void app_main() {
   uart_init();
   i2c_master_init();
   xTaskCreate(i2c_master_task, "i2c_master_task", 2048, NULL, 10, NULL);
}