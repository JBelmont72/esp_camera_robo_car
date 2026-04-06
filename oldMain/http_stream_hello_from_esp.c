// 13 dec 2025 duck.ai helped me with the http_stream.c and i was 
// able to see hello from esp on browser and with curl command. 192.168.1.58/
// need the forward slash


//#include "http_stream.h"
//#include "esp_log.h"

//static const char *TAG = "HTTP_STREAM";

//esp_err_t http_stream_start(void) {
//    ESP_LOGI(TAG, "HTTP stream server starting...");
    // your HTTP server init code here
//    return ESP_OK;
//}

// http_stream.c
//#include "esp_http_server.h"


#include "http_stream.h"
#include "esp_log.h"
#include "esp_http_server.h"  // Ensure this path is valid

static const char *TAG = "HTTP_STREAM";

static esp_err_t example_get_handler(httpd_req_t *req) {
    // Send a simple response back
    const char *resp = "Hello from ESP32!";
    httpd_resp_send(req, resp, strlen(resp));
    return ESP_OK;
}

esp_err_t http_stream_start(void) {
    httpd_handle_t server = NULL;

    // Configuration for the HTTP server
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.server_port = 80; // Change as necessary

    ESP_LOGI(TAG, "HTTP stream server starting...");

    // Start the HTTP server
    if (httpd_start(&server, &config) == ESP_OK) {
        httpd_uri_t example_uri = {
            .uri      = "/", // Endpoint for the server
            .method   = HTTP_GET,
            .handler  = example_get_handler,
            .user_ctx = NULL
        };

        // Register URI handler and check for success
        esp_err_t reg_result = httpd_register_uri_handler(server, &example_uri);
        if (reg_result != ESP_OK) {
            ESP_LOGE(TAG, "Failed to register URI handler");
            httpd_stop(server);  // Stop server if register failed
            return reg_result;
        }

        return ESP_OK;
    }

    ESP_LOGE(TAG, "Failed to start the HTTP server");
    return ESP_FAIL; // Indicate failure to start the server
}
