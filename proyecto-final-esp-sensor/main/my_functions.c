#include "my_functions.h"

#include "my_tcp.h"

EventGroupHandle_t wifi_event_group = NULL;
static const char *TAG_FUNCTIONS = "Custom Functions";
static const char *TAG_WIFI_AP = "WiFI AP";
static const char *TAG_SERVER = "Server";
httpd_handle_t server;
char deviceNumber[MAX_CHAR] = {0};
char ssid[MAX_CHAR] = {0};
char pass[MAX_CHAR] = {0};
bool trigger_alert = false;

const httpd_uri_t infoSite = {
    .uri = "/info",
    .method = HTTP_GET,
    .handler = info_get_handler,
};

void gpio_init() {
   gpio_config_t io_conf;
   io_conf.intr_type = GPIO_INTR_DISABLE;
   io_conf.mode = GPIO_MODE_INPUT_OUTPUT;
   io_conf.pin_bit_mask = (1ULL << LED);
   io_conf.pull_down_en = GPIO_PULLDOWN_ENABLE;
   io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
   gpio_config(&io_conf);

   io_conf.intr_type = GPIO_INTR_DISABLE;
   io_conf.mode = GPIO_MODE_INPUT;
   io_conf.pin_bit_mask = (1ULL << ADC_SELECTED);
   io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
   io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
   gpio_config(&io_conf);

   io_conf.intr_type = GPIO_INTR_NEGEDGE;
   io_conf.mode = GPIO_MODE_INPUT;
   io_conf.pin_bit_mask = (1 << BUTTON_SEND_MESSAGE | 1 << RESTART_PIN);
   io_conf.pull_down_en = 0;
   io_conf.pull_up_en = 1;
   gpio_config(&io_conf);
}

int read_led() {
   int led_state = gpio_get_level(LED);
   ESP_LOGI(TAG_FUNCTIONS, "LED state is: %d", led_state);
   return led_state;
}

float read_temperature(uint8_t device_id) { return sensors_data_struct[device_id].sensor_data.temperature; }

float read_humidity(uint8_t device_id) { return sensors_data_struct[device_id].sensor_data.humidity; }

float read_pressure(uint8_t device_id) { return sensors_data_struct[device_id].sensor_data.pressure; }

float read_gyroscope(uint8_t device_id) { return sensors_data_struct[device_id].gyro; }

esp_err_t info_get_handler(httpd_req_t *req) {
   char *auxStr =
       "<div class=\"data\">\n"
       "<p><span>Gyroscope:</span> %.2f°/s</p>\n"
       "<p><span>Temperature:</span> %2.1f°C</p>\n"
       "<p><span>Pressure:</span> %2.2fkPa</p>\n"
       "<p><span>Humidity:</span> %2.1f%%</p>\n"
       "</div>\n"
       "</div>\n";
   extern unsigned char info_start[] asm("_binary_info_top_html_start");
   extern unsigned char info_end[] asm("_binary_info_top_html_end");
   size_t info_len = info_end - info_start;
   char *infoHtml = malloc(BUFFER_MAX);
   if (infoHtml == NULL) {
      ESP_LOGE(TAG_SERVER, "Failed to allocate memory");
      return ESP_FAIL;
   }

   httpd_resp_set_type(req, "text/html");
   httpd_resp_send_chunk(req, (const char *)info_start, info_len);

   if (history[0].data[0].temperature > 30.0 || history[1].data[0].temperature > 30.0) trigger_alert = true;
   snprintf(infoHtml, info_len, "<div class=\"segment\" id=\"segment1\">\n<h2>Inside Package</h2>\n");
   httpd_resp_send_chunk(req, infoHtml, HTTPD_RESP_USE_STRLEN);
   snprintf(infoHtml, info_len, auxStr, history[0].data->gyro, history[0].data->temperature, history[0].data->pressure,
            history[0].data->humidity);
   httpd_resp_send_chunk(req, infoHtml, HTTPD_RESP_USE_STRLEN);
   snprintf(infoHtml, info_len, "<div class=\"segment\" id=\"segment2\">\n<h2>Ouside Package</h2>\n");
   httpd_resp_send_chunk(req, infoHtml, HTTPD_RESP_USE_STRLEN);
   snprintf(infoHtml, info_len, auxStr, history[1].data->gyro, history[1].data->temperature, history[1].data->pressure,
            history[1].data->humidity);
   httpd_resp_send_chunk(req, infoHtml, HTTPD_RESP_USE_STRLEN);
   if (trigger_alert) {
      snprintf(infoHtml, info_len,
               "<div class=\"alert\" id=\"alert\">\n<h2>Temperature Alert!</h2>\n"
               "<div class=\"data\">\n"
               "<p><span>🚨 Temperature Alert ! 🚨</span></p>"
               "</div>\n"
               "</div>\n");
      httpd_resp_send_chunk(req, infoHtml, HTTPD_RESP_USE_STRLEN);
   }

   for (int i = 0; i < 2; i++) {
      snprintf(infoHtml, info_len,
               "<div class='table-container'>\n"  // Open table container
               "<h1>%s</h1>\n"
               "<table border='0'>\n"
               "<tr>\n"
               "<th>#</th>\n"
               "<th>Temperature<br>(°C)</th>\n"
               "<th>Pressure<br>(KPa)</th>\n"
               "<th>Humidity<br>(%%)</th>\n"
               "<th>Gyro<br>(°/s)</th>\n"
               "</tr>\n",
               (i == 0) ? "Inside Package" : "Outside Package");

      httpd_resp_send_chunk(req, infoHtml, HTTPD_RESP_USE_STRLEN);

      for (int j = 0; j < history[i].count; j++) {
         char row[BUFFER_MAX];
         int length = 0;

         length += snprintf(row + length, sizeof(row) - length, "<tr>\n");
         length += snprintf(row + length, sizeof(row) - length, "<td>%d</td>\n", j + 1);
         length += snprintf(row + length, sizeof(row) - length, "<td>%.2f</td>\n", history[i].data[j].temperature);
         length += snprintf(row + length, sizeof(row) - length, "<td>%.2f</td>\n", history[i].data[j].pressure);
         length += snprintf(row + length, sizeof(row) - length, "<td>%.2f</td>\n", history[i].data[j].humidity);
         length += snprintf(row + length, sizeof(row) - length, "<td>%.2f</td>\n", history[i].data[j].gyro);
         length += snprintf(row + length, sizeof(row) - length, "</tr>\n");
         httpd_resp_send_chunk(req, row, HTTPD_RESP_USE_STRLEN);
      }

      httpd_resp_send_chunk(req, "</table>\n</div>\n<br>\n", HTTPD_RESP_USE_STRLEN);
   }

   // Send the closing part of the HTML structure
   httpd_resp_send_chunk(req,
                         "<footer>\n"
                         "<p>&copy; 2024 Sensor Dashboard | Sistemas Embebidos </p>\n"
                         "</footer>\n"
                         "</html>\n",
                         HTTPD_RESP_USE_STRLEN);

   httpd_resp_send_chunk(req, NULL, 0);

   if (httpd_req_get_hdr_value_len(req, "Host") == 0) {
      ESP_LOGI(TAG_SERVER, "Request headers lost");
   }
   free(infoHtml);

   return ESP_OK;
}

httpd_handle_t start_webserver(void) {
   httpd_handle_t server = NULL;
   httpd_config_t config = HTTPD_DEFAULT_CONFIG();
   config.stack_size = 4096 * 8;

   ESP_LOGI(TAG_SERVER, "Iniciando el servidor en el puerto: '%d'", config.server_port);
   if (httpd_start(&server, &config) == ESP_OK) {
      ESP_LOGI(TAG_SERVER, "Registrando manejadores de URI");
      httpd_register_uri_handler(server, &infoSite);
      return server;
   }

   ESP_LOGE(TAG_SERVER, "Error starting server!");
   return NULL;
}

const char *format_mac_address(const uint8_t mac[6]) {
   static char mac_str[18];
   snprintf(mac_str, sizeof(mac_str), "%02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
   return mac_str;
}

void wifi_event_handler(void *event_handler_arg, esp_event_base_t event_base, int32_t event_id, void *event_data) {
   switch (event_id) {
      case WIFI_EVENT_AP_START:
         xEventGroupSetBits(wifi_event_group, WIFI_AP_STARTED_BIT);
         ESP_LOGI(TAG_WIFI_AP, "Access Point started");
         break;
      case WIFI_EVENT_AP_STOP:
         ESP_LOGI(TAG_WIFI_AP, "Access Point stopped");
         break;
      case WIFI_EVENT_AP_STACONNECTED:
         wifi_event_ap_staconnected_t *event_ap_staconnected = (wifi_event_ap_staconnected_t *)event_data;
         ESP_LOGI(TAG_WIFI_AP, "%s connected to AP, ID: %d", format_mac_address(event_ap_staconnected->mac),
                  event_ap_staconnected->aid);
         break;
      case WIFI_EVENT_AP_STADISCONNECTED:
         wifi_event_ap_stadisconnected_t *event_ap_stadisconnected = (wifi_event_ap_stadisconnected_t *)event_data;
         ESP_LOGI(TAG_WIFI_AP, "%s disconnected from AP, ID: %d", format_mac_address(event_ap_stadisconnected->mac),
                  event_ap_stadisconnected->aid);

         break;
      default:
         ESP_LOGW(TAG_WIFI_AP, "Unhandled event ID: %ld", event_id);
         break;
   }
}

void wifi_init() {
   wifi_event_group = xEventGroupCreate();
   esp_netif_init();
   esp_event_loop_create_default();

   wifi_init_config_t wifi_initiation = WIFI_INIT_CONFIG_DEFAULT();
   esp_wifi_init(&wifi_initiation);

   esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, wifi_event_handler, NULL);
   esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, wifi_event_handler, NULL);

   esp_netif_create_default_wifi_ap();
   wifi_config_t wifi_config = {.ap = {.ssid = AP_SSID, .max_connection = 4, .authmode = WIFI_AUTH_OPEN}};
   esp_wifi_set_mode(WIFI_MODE_AP);
   esp_wifi_set_config(ESP_IF_WIFI_AP, &wifi_config);
   esp_wifi_start();
   ESP_LOGW(TAG_WIFI_AP, "Wi-Fi AP started as SSID: %s", AP_SSID);
}

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
   dev->chip_id = BME280_I2C_ADDR_SEC;
   dev->intf = BME280_I2C_INTF;
   dev->read = (bme280_read_fptr_t)device_register_read;
   dev->write = (bme280_write_fptr_t)device_register_write_byte;
   dev->delay_us = (bme280_delay_us_fptr_t)bme280_delay_us;
   bme280_init(dev);

   settings->osr_h = BME280_OVERSAMPLING_1X;
   settings->osr_p = BME280_OVERSAMPLING_16X;
   settings->osr_t = BME280_OVERSAMPLING_2X;
   settings->filter = BME280_FILTER_COEFF_16;
   settings->standby_time = BME280_STANDBY_TIME_0_5_MS;
   bme280_set_sensor_settings(BME280_SEL_ALL_SETTINGS, settings, dev);
}

uint8_t read_BME280_registers(struct bme280_data *sensor_data, struct bme280_dev *dev) {
   uint8_t rslt = BME280_OK;
   rslt = bme280_get_sensor_data(BME280_ALL, sensor_data, dev);
   return rslt;
}

void send_sensor_data_binary(sensors_data_struct_t *sensors_data_struct) {
   sensors_packet_t packet = {
       .device_id = sensors_data_struct->device_id,
       .temperature = sensors_data_struct->sensor_data.temperature,
       .pressure = sensors_data_struct->sensor_data.pressure / 1000,  // Convert to kPa
       .humidity = sensors_data_struct->sensor_data.humidity,
       .gyro = sensors_data_struct->gyro,
   };
   ESP_LOGI(TAG_FUNCTIONS, "Sending sensor data: device_id=0x%x, temp=%.1f, press=%.1f, humid=%.2f, gyro=%.2f",
            packet.device_id, packet.temperature, packet.pressure, packet.humidity, packet.gyro);
   tcp_send(sock, &packet, sizeof(packet));
}

void delaySeconds(uint32_t seconds) { vTaskDelay(seconds * SECOND_IN_MILLIS / portTICK_PERIOD_MS); }

void delayMillis(uint32_t millis) { vTaskDelay(millis / portTICK_PERIOD_MS); }

void sensors_task(void *pvParameters) {
   sensors_data_struct_t *sensors_data_struct = (sensors_data_struct_t *)pvParameters;

   while (1) {
      bme280_set_sensor_mode(BME280_POWERMODE_FORCED, &sensors_data_struct->dev);
      read_BME280_registers(&sensors_data_struct->sensor_data, &sensors_data_struct->dev);
      sensors_data_struct->gyro = mpu6050_read_gyro_x();
      send_sensor_data_binary(sensors_data_struct);
      delaySeconds(DATA_SEND_RATE);
   }
}

void i2c_sensor_init() {
   i2c_config_t conf = {
       .mode = I2C_MODE_MASTER,
       .sda_io_num = I2C_MASTER_SDA_IO,
       .scl_io_num = I2C_MASTER_SCL_IO,
       .sda_pullup_en = GPIO_PULLUP_ENABLE,
       .scl_pullup_en = GPIO_PULLUP_ENABLE,
       .master.clk_speed = I2C_MASTER_CLK,
   };
   i2c_param_config(I2C_MASTER_NUM, &conf);
   i2c_driver_install(I2C_MASTER_NUM, conf.mode, 0, 0, 0);
}

void mpu6050_init() {
   uint8_t data = 0;
   i2c_cmd_handle_t cmd = i2c_cmd_link_create();
   i2c_master_start(cmd);
   i2c_master_write_byte(cmd, (MPU6050_ADDR << 1) | I2C_MASTER_WRITE, true);
   i2c_master_write_byte(cmd, MPU6050_REG_PWR_MGMT_1, true);
   i2c_master_write_byte(cmd, data, true);
   i2c_master_stop(cmd);
   i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, 1000 / portTICK_PERIOD_MS);
   i2c_cmd_link_delete(cmd);
}

float mpu6050_read_gyro_x() {
   uint8_t data_h, data_l;
   i2c_cmd_handle_t cmd = i2c_cmd_link_create();

   // Leer el byte alto
   i2c_master_start(cmd);
   i2c_master_write_byte(cmd, (MPU6050_ADDR << 1) | I2C_MASTER_WRITE, true);
   i2c_master_write_byte(cmd, MPU6050_REG_GYRO_XOUT_H, true);
   i2c_master_stop(cmd);
   i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, 1000 / portTICK_PERIOD_MS);
   i2c_cmd_link_delete(cmd);

   // Leer el byte bajo
   cmd = i2c_cmd_link_create();
   i2c_master_start(cmd);
   i2c_master_write_byte(cmd, (MPU6050_ADDR << 1) | I2C_MASTER_READ, true);
   i2c_master_read_byte(cmd, &data_h, I2C_MASTER_ACK);
   i2c_master_read_byte(cmd, &data_l, I2C_MASTER_NACK);
   i2c_master_stop(cmd);
   i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, 1000 / portTICK_PERIOD_MS);
   i2c_cmd_link_delete(cmd);

   int16_t gyro_value = (data_h << 8) | data_l;

   float gyro_value_deg = gyro_value / MPU6050_GYRO_SENSITIVITY_250DPS;

   return gyro_value_deg;
}

void init_history(sensors_packet_history_t *history) {
   memset(history, 0, sizeof(sensors_packet_history_t));
   history->head = -1;  // Ningún dato almacenado al inicio
   history->count = 0;
}

void add_to_history(sensors_packet_history_t *history, sensors_packet_t *new_data, uint8_t device_id) {
   history[device_id].head = (history[device_id].head + 1) % HISTORY_SIZE;
   history[device_id].data[history[device_id].head] = *new_data;
   if (history[device_id].count < HISTORY_SIZE) {
      history[device_id].count++;
   }
}
