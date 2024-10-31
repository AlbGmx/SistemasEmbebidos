#ifndef myServer_h
#define myServer_h
#include <esp_http_server.h>
#include <esp_system.h>
#include <esp_wifi.h>
#include <stdlib.h>
#include <sys/param.h>
#include <unistd.h>

#include "esp_check.h"
#include "esp_tls.h"
#include "esp_tls_crypto.h"
#include "myWifi.h"
#include "protocol_examples_common.h"
#include "protocol_examples_utils.h"
#define EXAMPLE_HTTP_QUERY_KEY_MAX_LEN (64)

esp_err_t hello_get_handler(httpd_req_t*);
esp_err_t echo_post_handler(httpd_req_t*);
esp_err_t any_handler(httpd_req_t*);
esp_err_t http_404_error_handler(httpd_req_t*, httpd_err_code_t);
esp_err_t ctrl_put_handler(httpd_req_t*);
esp_err_t index_get_handler(httpd_req_t*);
httpd_handle_t start_webserver(void);
esp_err_t stop_webserver(httpd_handle_t);
void disconnect_handler(void*, esp_event_base_t, int32_t, void*);
void connect_handler(void*, esp_event_base_t, int32_t, void*);

#endif