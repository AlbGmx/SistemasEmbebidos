#include <ctype.h>
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/unistd.h>

#include "driver/gpio.h"
#include "esp_vfs_fat.h"
#include "myUart.h"
#include "sdmmc_cmd.h"

#define EXAMPLE_MAX_CHAR_SIZE 64
#define MOUNT_POINT "/sdcard"

#define PIN_NUM_MISO 19
#define PIN_NUM_MOSI 23
#define PIN_NUM_CLK 18
#define PIN_NUM_CS 5

#define START_LOOP_CHAR '['
#define END_LOOP_CHAR ']'
#define END_LOOP_CHAR_TO_IGNORE 1

static const char *TAG = "sd_card";

void list_files_in_directory(const char *path) {
   struct dirent *de;
   DIR *dr = opendir(path);

   if (dr == NULL) {
      ESP_LOGE(TAG, "Could not open directory: %s", path);
      return;
   }

   ESP_LOGI(TAG, "\tImprimiendo FileSystem: %s", path);
   ESP_LOGI(TAG, "\t=============================================");
   while ((de = readdir(dr)) != NULL) {
      ESP_LOGI(TAG, "\t\tFilename --> %s", de->d_name);
   }
   ESP_LOGI(TAG, "\t=============================================\n\n");
   closedir(dr);
}

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

int getLoopEndingChar(const char *str) {
   int auxIndex = 0;
   int openBrackets = 1;

   while (str[auxIndex] && openBrackets > 0) {
      char currentChar = str[auxIndex++];

      if (currentChar == START_LOOP_CHAR) {
         openBrackets++;
      } else if (currentChar == END_LOOP_CHAR) {
         openBrackets--;
      }
   }

   return openBrackets == 0 ? auxIndex : -1;
}

int8_t print_recursion_string(char *str) {
   int8_t auxIndex = getLoopEndingChar(str);
   if (auxIndex < 0) {
      ESP_LOGE(TAG, "Mismatched brackets.\n");
      return -1;
   }

   int counter = 0;
   int iterations = 0;

   while (isdigit((unsigned char)str[counter]) && counter < auxIndex) {
      iterations = iterations * 10 + (str[counter] - '0');
      counter++;
   }

   if (iterations == 0) iterations = 1;

   char *content_start = &str[counter];
   int content_length = auxIndex - counter - END_LOOP_CHAR_TO_IGNORE;

   for (int i = 0; i < iterations; ++i) {
      int j = 0;
      while (j < content_length) {
         if (content_start[j] == START_LOOP_CHAR) {
            j++;
            int8_t nestedIndex = print_recursion_string(&content_start[j]);
            if (nestedIndex < 0) return -1;
            j += nestedIndex;
         } else {
            putchar(content_start[j]);
            j++;
         }
      }
   }
   return auxIndex;
}

void print_expanded_string(char *str) {
   while (*str) {
      if (*str == START_LOOP_CHAR) {
         int auxIndex = print_recursion_string(str + 1);
         if (auxIndex < 0) {
            ESP_LOGE(TAG, "Error\n");
            return;
         }
         str += auxIndex + 1;
      } else {
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
   ESP_LOGI(TAG, "RAW values: '%s'\n", line);
   print_expanded_string(line);
   printf("\n");
   return ESP_OK;
}

void app_main(void) {
   init_UARTs();

   // Give some time for the SD card to stabilize
   vTaskDelay(pdMS_TO_TICKS(1000));

   esp_vfs_fat_sdmmc_mount_config_t mount_config = {
       .format_if_mount_failed = true,
       .max_files = 5,
       .allocation_unit_size = 16 * 1024,
   };

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

   ESP_ERROR_CHECK(spi_bus_initialize(host.slot, &bus_cfg, SDSPI_DEFAULT_DMA));

   sdspi_device_config_t slot_config = SDSPI_DEVICE_CONFIG_DEFAULT();
   slot_config.gpio_cs = PIN_NUM_CS;
   slot_config.host_id = host.slot;

   ESP_ERROR_CHECK(esp_vfs_fat_sdspi_mount(MOUNT_POINT, &host, &slot_config, &mount_config, &card));

   ESP_LOGI(TAG, "Filesystem montado");
   sdmmc_card_print_info(stdout, card);
   char *aux = (char *)malloc(EXAMPLE_MAX_CHAR_SIZE);
   char *filename_path = (char *)malloc(EXAMPLE_MAX_CHAR_SIZE);
   if (!aux || !filename_path) {
      ESP_LOGE(TAG, "Memory allocation failed!");
   } else {
      while (true) {
         list_files_in_directory(MOUNT_POINT);

         put_str(UART_CONSOLE, "Leer o escribir archivo? (r/w): ");
         char action = get_char(UART_CONSOLE);

         if (action != 'r' && action != 'w') {
            ESP_LOGE(TAG, "Accion invalida");
            continue;
         }

         snprintf(aux, EXAMPLE_MAX_CHAR_SIZE, "\nIntroduce el nombre de archivo a %s: ", (action == 'r') ? "leer" : "ecribir");
         put_str(UART_CONSOLE, aux);

         char *data = get_line(UART_CONSOLE);
         if (data == NULL) {
            ESP_LOGE(TAG, "Error leyendo nombre de archivo!");
            continue;
         }

         snprintf(filename_path, EXAMPLE_MAX_CHAR_SIZE, "%s/%s", MOUNT_POINT, data);
         esp_err_t ret;

         if (action == 'r') {
            ret = s_example_read_file(filename_path);
            if (ret == ESP_FAIL) {
               ESP_LOGE(TAG, "Error leyendo archivo!");
            }
         } else {
            put_str(UART_CONSOLE, "\nIntroduce datos a escribir: ");
            data = get_line(UART_CONSOLE);
            if (data == NULL) {
               ESP_LOGE(TAG, "Error obteniendo data a escribir!");
               continue;
            }
            ESP_ERROR_CHECK(s_example_write_file(filename_path, data));
         }

         ESP_LOGI(TAG, "Hacer alguna otra accion? (y/n): ");
         if (get_char(UART_CONSOLE) != 'y') {
            break;
         }
      }

      // Free allocated memory
      free(filename_path);
      free(aux);

      esp_vfs_fat_sdcard_unmount(MOUNT_POINT, card);
      ESP_LOGW(TAG, "Tarjeta desmontada");
      spi_bus_free(host.slot);
   }
}
