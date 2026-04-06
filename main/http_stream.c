

#include "http_stream.h"
#include "esp_log.h"
#include "esp_http_server.h"
#include "esp_camera.h"

static const char *TAG = "HTTP_STREAM";

/* Boundary string used by MJPEG multipart stream */
#define PART_BOUNDARY "frame"

/* HTTP content type for MJPEG streaming */
static const char *STREAM_CONTENT_TYPE =
    "multipart/x-mixed-replace;boundary=" PART_BOUNDARY;

/* Boundary marker sent before each JPEG frame */
static const char *STREAM_BOUNDARY =
    "\r\n--" PART_BOUNDARY "\r\n";

/* Per-frame HTTP header:
   - Content-Type: image/jpeg
   - Content-Length: size of JPEG buffer */
static const char *STREAM_PART =
    "Content-Type: image/jpeg\r\nContent-Length: %u\r\n\r\n";

/* Main MJPEG stream handler */
static esp_err_t stream_handler(httpd_req_t *req)
{
    camera_fb_t *fb = NULL;
    char buf[64];

    /* Tell browser this is an MJPEG stream */
    httpd_resp_set_type(req, STREAM_CONTENT_TYPE);

    /* Infinite loop: capture → send → repeat */
    while (true) {

        /* Grab a frame from the camera driver */
        fb = esp_camera_fb_get();
        if (!fb) {
            ESP_LOGE(TAG, "Camera capture failed");
            return ESP_FAIL;
        }

        ESP_LOGI("HTTP_STREAM", "Frame %dx%d (%u bytes)",
            fb->width, fb->height, fb->len);

/* added below 17 dec  adds stream verification in stream handler*/
        if (fb->format != PIXFORMAT_JPEG) {
            esp_camera_fb_return(fb);
            continue;
        }



        /* Build per-frame HTTP header */
        size_t hlen = snprintf(buf, sizeof(buf), STREAM_PART, fb->len);

        /* Send:
           1) boundary
           2) headers
           3) JPEG image data */
        if (httpd_resp_send_chunk(req, STREAM_BOUNDARY, strlen(STREAM_BOUNDARY)) != ESP_OK ||
            httpd_resp_send_chunk(req, buf, hlen) != ESP_OK ||
            httpd_resp_send_chunk(req, (const char *)fb->buf, fb->len) != ESP_OK) {

            /* On error, release frame buffer and stop streaming */
            esp_camera_fb_return(fb);
            break;
        }

        /* IMPORTANT: Return frame buffer to driver */
        esp_camera_fb_return(fb);
    }

    return ESP_OK;
}

/* Start HTTP server and register /stream endpoint */
esp_err_t http_stream_start(void)
{
    httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    /* Run server on port 80 */
    config.server_port = 80;





    ESP_LOGI(TAG, "Starting MJPEG stream server");

    if (httpd_start(&server, &config) == ESP_OK) {

        httpd_uri_t stream_uri = {
            .uri      = "/stream",
            .method   = HTTP_GET,
            .handler  = stream_handler,
            .user_ctx = NULL
        };

        httpd_register_uri_handler(server, &stream_uri);
        return ESP_OK;
    }

    return ESP_FAIL;
}




/**
#include "http_stream.h"
#include "esp_log.h"
#include "esp_http_server.h"
#include "esp_camera.h"

static const char *TAG = "HTTP_STREAM";

#define PART_BOUNDARY "frame"
static const char *STREAM_CONTENT_TYPE =
    "multipart/x-mixed-replace;boundary=" PART_BOUNDARY;
static const char *STREAM_BOUNDARY =
    "\r\n--" PART_BOUNDARY "\r\n";
static const char *STREAM_PART =
    "Content-Type: image/jpeg\r\nContent-Length: %u\r\n\r\n";

static esp_err_t stream_handler(httpd_req_t *req)
{
    camera_fb_t *fb = NULL;
    char buf[64];

    httpd_resp_set_type(req, STREAM_CONTENT_TYPE);

    while (true) {
        fb = esp_camera_fb_get();
        if (!fb) {
            ESP_LOGE(TAG, "Camera capture failed");
            return ESP_FAIL;
        }

        size_t hlen = snprintf(buf, sizeof(buf), STREAM_PART, fb->len);
        if (httpd_resp_send_chunk(req, STREAM_BOUNDARY, strlen(STREAM_BOUNDARY)) != ESP_OK ||
            httpd_resp_send_chunk(req, buf, hlen) != ESP_OK ||
            httpd_resp_send_chunk(req, (const char *)fb->buf, fb->len) != ESP_OK) {

            esp_camera_fb_return(fb);
            break;
        }

        esp_camera_fb_return(fb);
    }

    return ESP_OK;
}

esp_err_t http_stream_start(void)
{
    httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.server_port = 80;

    ESP_LOGI(TAG, "Starting MJPEG stream server");

    if (httpd_start(&server, &config) == ESP_OK) {
        httpd_uri_t stream_uri = {
            .uri      = "/stream",
            .method   = HTTP_GET,
            .handler  = stream_handler,
            .user_ctx = NULL
        };

        httpd_register_uri_handler(server, &stream_uri);
        return ESP_OK;
    }

    return ESP_FAIL;
}
*/

