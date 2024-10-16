#include <ctype.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/unistd.h>

#include "driver/gpio.h"
#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"
#define EXAMPLE_MAX_CHAR_SIZE 64
#define MOUNT_POINT "/sdcard"

#define PIN_NUM_MISO 19
#define PIN_NUM_MOSI 23
#define PIN_NUM_CLK 18
#define PIN_NUM_CS 5

static const char *TAG = "sd_card";

static esp_err_t s_example_write_file(const char *path, char *data) {
   ESP_LOGI(TAG, "Abriendo archivo %s", path);
   FILE *f = fopen(path, "w");
   if (f == NULL) {
      ESP_LOGE(TAG, "Fallo abrir el archivo para escritura");
      return ESP_FAIL;
   }
   fprintf(f, "%s", data);  // Use "%s" to avoid formatting issues
   fclose(f);
   ESP_LOGI(TAG, "Archivo escrito");
   return ESP_OK;
}

void print_recursion_string(char **str) {
   uint8_t iterations = 0;
   uint8_t auxIndex = 0;
   if (**str == '[') {
      (*str)++;
      print_recursion_string(&str);
   } else {
      // get the number of iterations
      while (**str >= '0' && **str <= '9') {
         ESP_LOGI(TAG, "iterations before char %c %d\n", **str, iterations);
         iterations = (iterations * 10) + (**str - '0');
         (*str)++;
      }

      if (iterations == 0) {
         iterations = 1;
      }

      while ((*(*str + auxIndex)) != ']' && *(*str + auxIndex) != '\0') {
         if (*(*str + auxIndex) == '[') {
            while ((*(*str + auxIndex)) != ']' && *(*str + auxIndex) != '\0') auxIndex++;
         } else
            auxIndex++;
      }

      ESP_LOGI(TAG, "iterations: %d, char %c, auxIndex: %d\n", iterations, **str, auxIndex);
      for (int i = 0; i < iterations; i++) {
         for (int j = 0; j < auxIndex; j++) {
            if (*(*str + j) == '[') {
               print_recursion_string(*str + j + 1);
            } else {
               putchar(*(*str + j));
            }
         }
      }
      ESP_LOGI(TAG, "char: %c, char %x, auxIndex: %d\n", **str, **str, auxIndex);
      *str += auxIndex + 1;  // +1 to skip the ']'
      ESP_LOGI(TAG, "skipped to char: %c, char %x\n", **str, **str);
   }
}

void print_expanded_string(char *str) {
   while (*str) {
      if (*str == '[') {
         str++;
         print_recursion_string(&str);
         // ESP_LOGI(TAG, "skipped to char: %c, char %x\n", *str, *str);
      } else {
         // ESP_LOGI(TAG, "char: %c, char %x\n", *str, *str);
         putchar(*str++);
      }
   }
   putchar('\n');
}

static esp_err_t s_example_read_file(const char *path) {
   ESP_LOGI(TAG, "Abriendo archivo %s", path);
   FILE *f = fopen(path, "r");
   if (f == NULL) {
      ESP_LOGE(TAG, "Fallo abrir el archivo para lectura");
      return ESP_FAIL;
   }
   char line[EXAMPLE_MAX_CHAR_SIZE];
   fgets(line, sizeof(line), f);
   fclose(f);

   // Strip newline
   char *pos = strchr(line, '\n');
   if (pos) *pos = '\0';
   ESP_LOGI(TAG, "RAW values: '%s'", line);
   print_expanded_string(line);
   return ESP_OK;
}

void app_main(void) {
   esp_err_t ret;

   // Give some time for the SD card to stabilize
   vTaskDelay(pdMS_TO_TICKS(1000));

   esp_vfs_fat_sdmmc_mount_config_t mount_config = {
       .format_if_mount_failed = true, .max_files = 5, .allocation_unit_size = 16 * 1024};

   sdmmc_card_t *card;
   sdmmc_host_t host = SDSPI_HOST_DEFAULT();
   host.max_freq_khz = 5000;

   spi_bus_config_t bus_cfg = {
       .mosi_io_num = PIN_NUM_MOSI,
       .miso_io_num = PIN_NUM_MISO,
       .sclk_io_num = PIN_NUM_CLK,
       .quadwp_io_num = -1,
       .quadhd_io_num = -1,
       .max_transfer_sz = 4000,
   };

   ret = spi_bus_initialize(host.slot, &bus_cfg, SDSPI_DEFAULT_DMA);
   if (ret != ESP_OK) {
      ESP_LOGE(TAG, "Fallo inicializacion del bus.");
      return;
   }

   sdspi_device_config_t slot_config = SDSPI_DEVICE_CONFIG_DEFAULT();
   slot_config.gpio_cs = PIN_NUM_CS;
   slot_config.host_id = host.slot;

   ESP_LOGI(TAG, "Montando el filesystem");
   ret = esp_vfs_fat_sdspi_mount(MOUNT_POINT, &host, &slot_config, &mount_config, &card);
   if (ret != ESP_OK) {
      ESP_LOGE(TAG, "Fallo montar el filesystem (%s)", esp_err_to_name(ret));
      spi_bus_free(host.slot);
      return;
   }

   ESP_LOGI(TAG, "Filesystem montado");
   sdmmc_card_print_info(stdout, card);

   const char *file_hello = MOUNT_POINT "/ejemplo.txt";
   const char *file_hello2 = MOUNT_POINT "/ejemplo2.txt";
   const char *file_hello3 = MOUNT_POINT "/ejemplo3.txt";

   // ret = s_example_read_file(file_hello);
   // if (ret != ESP_OK) return;

   ret = s_example_read_file(file_hello2);
   if (ret != ESP_OK) return;

   // ret = s_example_read_file(file_hello3);
   // if (ret != ESP_OK) return;

   esp_vfs_fat_sdcard_unmount(MOUNT_POINT, card);
   ESP_LOGI(TAG, "Tarjeta desmontada");
   spi_bus_free(host.slot);
}
