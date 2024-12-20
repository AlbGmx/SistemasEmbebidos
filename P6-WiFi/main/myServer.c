#include "myServer.h"

static const char* TAG = "MyServer";

esp_err_t hello_get_handler(httpd_req_t* req) {
   char* buf;
   size_t buf_len;

   /* Get header value string length and allocate memory for length + 1,
    * extra byte for null termination */
   buf_len = httpd_req_get_hdr_value_len(req, "Host") + 1;
   if (buf_len > 1) {
      buf = malloc(buf_len);
      /* Copy null terminated value string into buffer */
      if (httpd_req_get_hdr_value_str(req, "Host", buf, buf_len) == ESP_OK) {
         ESP_LOGI(TAG, "Found header => Host: %s", buf);
      }
      free(buf);
   }

   buf_len = httpd_req_get_hdr_value_len(req, "Test-Header-2") + 1;
   if (buf_len > 1) {
      buf = malloc(buf_len);
      if (httpd_req_get_hdr_value_str(req, "Test-Header-2", buf, buf_len) == ESP_OK) {
         ESP_LOGI(TAG, "Found header => Test-Header-2: %s", buf);
      }
      free(buf);
   }

   buf_len = httpd_req_get_hdr_value_len(req, "Test-Header-1") + 1;
   if (buf_len > 1) {
      buf = malloc(buf_len);
      if (httpd_req_get_hdr_value_str(req, "Test-Header-1", buf, buf_len) == ESP_OK) {
         ESP_LOGI(TAG, "Found header => Test-Header-1: %s", buf);
      }
      free(buf);
   }

   /* Read URL query string length and allocate memory for length + 1,
    * extra byte for null termination */
   buf_len = httpd_req_get_url_query_len(req) + 1;
   if (buf_len > 1) {
      buf = malloc(buf_len);
      if (httpd_req_get_url_query_str(req, buf, buf_len) == ESP_OK) {
         ESP_LOGI(TAG, "Found URL query => %s", buf);
         char param[EXAMPLE_HTTP_QUERY_KEY_MAX_LEN], dec_param[EXAMPLE_HTTP_QUERY_KEY_MAX_LEN] = {0};
         /* Get value of expected key from query string */
         if (httpd_query_key_value(buf, "query1", param, sizeof(param)) == ESP_OK) {
            ESP_LOGI(TAG, "Found URL query parameter => query1=%s", param);
            example_uri_decode(dec_param, param, strnlen(param, EXAMPLE_HTTP_QUERY_KEY_MAX_LEN));
            ESP_LOGI(TAG, "Decoded query parameter => %s", dec_param);
         }
         if (httpd_query_key_value(buf, "query3", param, sizeof(param)) == ESP_OK) {
            ESP_LOGI(TAG, "Found URL query parameter => query3=%s", param);
            example_uri_decode(dec_param, param, strnlen(param, EXAMPLE_HTTP_QUERY_KEY_MAX_LEN));
            ESP_LOGI(TAG, "Decoded query parameter => %s", dec_param);
         }
         if (httpd_query_key_value(buf, "query2", param, sizeof(param)) == ESP_OK) {
            ESP_LOGI(TAG, "Found URL query parameter => query2=%s", param);
            example_uri_decode(dec_param, param, strnlen(param, EXAMPLE_HTTP_QUERY_KEY_MAX_LEN));
            ESP_LOGI(TAG, "Decoded query parameter => %s", dec_param);
         }
      }
      free(buf);
   }

   /* Set some custom headers */
   httpd_resp_set_hdr(req, "Custom-Header-1", "Custom-Value-1");
   httpd_resp_set_hdr(req, "Custom-Header-2", "Custom-Value-2");

   /* Send response with custom headers and body set as the
    * string passed in user context*/
   const char* resp_str = (const char*)req->user_ctx;
   httpd_resp_send(req, resp_str, HTTPD_RESP_USE_STRLEN);

   /* After sending the HTTP response the old HTTP request
    * headers are lost. Check if HTTP request headers can be read now. */
   if (httpd_req_get_hdr_value_len(req, "Host") == 0) {
      ESP_LOGI(TAG, "Request headers lost");
   }
   return ESP_OK;
}

const httpd_uri_t hello = {.uri = "/hello",
                           .method = HTTP_GET,
                           .handler = hello_get_handler,
                           /* Let's pass response string in user
                            * context to demonstrate it's usage */
                           .user_ctx = "Hello World!"};

esp_err_t echo_post_handler(httpd_req_t* req) {
   char buf[100];
   int ret, remaining = req->content_len;

   while (remaining > 0) {
      /* Read the data for the request */
      if ((ret = httpd_req_recv(req, buf, MIN(remaining, sizeof(buf)))) <= 0) {
         if (ret == HTTPD_SOCK_ERR_TIMEOUT) {
            /* Retry receiving if timeout occurred */
            continue;
         }
         return ESP_FAIL;
      }

      /* Send back the same data */
      httpd_resp_send_chunk(req, buf, ret);
      remaining -= ret;

      /* Log data received */
      ESP_LOGI(TAG, "=========== RECEIVED DATA ==========");
      ESP_LOGI(TAG, "%.*s", ret, buf);
      ESP_LOGI(TAG, "====================================");
   }

   // End response
   httpd_resp_send_chunk(req, NULL, 0);
   return ESP_OK;
}

const httpd_uri_t echo = {.uri = "/echo", .method = HTTP_POST, .handler = echo_post_handler, .user_ctx = NULL};

static const httpd_uri_t indice = {
    .uri = "/indice",
    .method = HTTP_GET,
    .handler = index_get_handler,
};

esp_err_t index_get_handler(httpd_req_t* req) {
   ESP_LOGI(TAG, "Request received for URI: %s", req->uri);
   char* buf;
   size_t buf_len;
   extern unsigned char index_html_start[] asm("_binary_index_html_start");
   extern unsigned char index_html_end[] asm("_binary_index_html_end");
   size_t index_html_size = index_html_end - index_html_start;
   char* index_html = malloc(index_html_size);
   if (!index_html) {
      ESP_LOGE(TAG, "Failed to allocate memory for index_html");
      return ESP_ERR_NO_MEM;
   }
   memcpy(index_html, index_html_start, index_html_size);

   buf_len = httpd_req_get_hdr_value_len(req, "Host") + 1;
   if (buf_len > 1) {
      buf = malloc(buf_len);
      if (httpd_req_get_hdr_value_str(req, "Host", buf, buf_len) == ESP_OK) {
         ESP_LOGI(TAG, "Found header => Host: %s", buf);
      }
      free(buf);
   }

   buf_len = httpd_req_get_url_query_len(req) + 1;
   if (buf_len > 1) {
      buf = malloc(buf_len);

      if (httpd_req_get_url_query_str(req, buf, buf_len) == ESP_OK) {
         ESP_LOGI(TAG, "Found URL query => %s", buf);
         char param[EXAMPLE_HTTP_QUERY_KEY_MAX_LEN], dec_param[EXAMPLE_HTTP_QUERY_KEY_MAX_LEN] = {0};
         if (httpd_query_key_value(buf, "xpos", param, sizeof(param)) == ESP_OK) {
            ESP_LOGI(TAG, "Found URL query parameter => query1=%s", param);
            example_uri_decode(dec_param, param, strnlen(param, EXAMPLE_HTTP_QUERY_KEY_MAX_LEN));
            ESP_LOGI(TAG, "Decoded query parameter => %s", dec_param);
         }
         if (httpd_query_key_value(buf, "ypos", param, sizeof(param)) == ESP_OK) {
            ESP_LOGI(TAG, "Found URL query parameter => query3=%s", param);
            example_uri_decode(dec_param, param, strnlen(param, EXAMPLE_HTTP_QUERY_KEY_MAX_LEN));
            ESP_LOGI(TAG, "Decoded query parameter => %s", dec_param);
         }
      }
      free(buf);
   }

   httpd_resp_set_type(req, "text/html");
   httpd_resp_send(req, (const char*)index_html, index_html_size);
   if (httpd_req_get_hdr_value_len(req, "Host") == 0) {
      ESP_LOGI(TAG, "Request headers lost");
   }
   free(index_html);
   return ESP_OK;
}

esp_err_t any_handler(httpd_req_t* req) {
   const char* resp_str = (const char*)req->user_ctx;
   httpd_resp_send(req, resp_str, HTTPD_RESP_USE_STRLEN);

   // End response
   httpd_resp_send_chunk(req, NULL, 0);
   return ESP_OK;
}

const httpd_uri_t any = {.uri = "/any", .method = HTTP_ANY, .handler = any_handler, .user_ctx = "Any!"};

esp_err_t http_404_error_handler(httpd_req_t* req, httpd_err_code_t err) {
   if (strcmp("/hello", req->uri) == 0) {
      httpd_resp_send_err(req, HTTPD_404_NOT_FOUND, "/hello URI is not available");

      return ESP_OK;
   } else if (strcmp("/echo", req->uri) == 0) {
      httpd_resp_send_err(req, HTTPD_404_NOT_FOUND, "/echo URI is not available");
      return ESP_FAIL;
   }
   httpd_resp_send_err(req, HTTPD_404_NOT_FOUND, "Some 404 error message");
   return ESP_FAIL;
}

esp_err_t ctrl_put_handler(httpd_req_t* req) {
   char buf;
   int ret;

   if ((ret = httpd_req_recv(req, &buf, 1)) <= 0) {
      if (ret == HTTPD_SOCK_ERR_TIMEOUT) {
         httpd_resp_send_408(req);
      }
      return ESP_FAIL;
   }

   if (buf == '0') {
      ESP_LOGI(TAG, "Unregistering /hello and /echo URIs");
      httpd_unregister_uri(req->handle, "/hello");
      httpd_unregister_uri(req->handle, "/echo");
      httpd_unregister_uri(req->handle, "/indice");
      httpd_register_err_handler(req->handle, HTTPD_404_NOT_FOUND, http_404_error_handler);
   } else {
      ESP_LOGI(TAG, "Registering /hello and /echo URIs");
      httpd_register_uri_handler(req->handle, &hello);
      httpd_register_uri_handler(req->handle, &echo);
      httpd_register_uri_handler(req->handle, &indice);
      httpd_register_err_handler(req->handle, HTTPD_404_NOT_FOUND, NULL);
   }

   httpd_resp_send(req, NULL, 0);
   return ESP_OK;
}

const httpd_uri_t ctrl = {.uri = "/ctrl", .method = HTTP_PUT, .handler = ctrl_put_handler, .user_ctx = NULL};

httpd_handle_t start_webserver(void) {
   httpd_handle_t server = NULL;
   httpd_config_t config = HTTPD_DEFAULT_CONFIG();
   config.lru_purge_enable = true;

   ESP_LOGI(TAG, "Iniciando servidor en puerto: '%d'", config.server_port);
   if (httpd_start(&server, &config) == ESP_OK) {
      ESP_LOGI(TAG, "Registrando URI Handlers");
      httpd_register_uri_handler(server, &hello);
      httpd_register_uri_handler(server, &echo);
      httpd_register_uri_handler(server, &ctrl);
      httpd_register_uri_handler(server, &any);
      httpd_register_uri_handler(server, &indice);
      return server;
   }

   ESP_LOGE(TAG, "Error iniciando el servidor!");
   return NULL;
}

esp_err_t stop_webserver(httpd_handle_t server) { return httpd_stop(server); }

void disconnect_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
   httpd_handle_t* server = (httpd_handle_t*)arg;
   if (*server) {
      ESP_LOGI(TAG, "Deteniendo servidor");
      if (stop_webserver(*server) == ESP_OK) {
         *server = NULL;
      } else {
         ESP_LOGE(TAG, "Failed to stop http server");
      }
   }
}

void connect_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
   httpd_handle_t* server = (httpd_handle_t*)arg;
   if (*server == NULL) {
      ESP_LOGI(TAG, "Starting webserver ?");
      *server = start_webserver();
   }
}
