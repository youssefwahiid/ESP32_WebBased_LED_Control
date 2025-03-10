#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_http_server.h"
#include "driver/gpio.h"

#define LED1_GPIO GPIO_NUM_2
#define LED2_GPIO GPIO_NUM_4

static const char *TAG = "LED_WEB_CONTROL";

// HTML content for the web page
static const char *HTML_PAGE = "<!DOCTYPE html>\n"
"<html lang=\"en\">\n"
"<head>\n"
"    <meta charset=\"UTF-8\">\n"
"    <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n"
"    <title>ESP32 LED Control</title>\n"
"    <style>\n"
"        body {\n"
"            font-family: Arial, sans-serif;\n"
"            background-color: #f4f4f9;\n"
"            text-align: center;\n"
"            margin: 0;\n"
"            padding: 0;\n"
"        }\n"
"        h1 {\n"
"            color: #333;\n"
"            margin-top: 50px;\n"
"        }\n"
"        .button-container {\n"
"            margin-top: 20px;\n"
"        }\n"
"        button {\n"
"            background-color: #4CAF50;\n"
"            color: white;\n"
"            padding: 10px 20px;\n"
"            margin: 10px;\n"
"            border: none;\n"
"            border-radius: 5px;\n"
"            font-size: 16px;\n"
"            cursor: pointer;\n"
"            transition: background-color 0.3s ease;\n"
"        }\n"
"        button:hover {\n"
"            background-color: #45a049;\n"
"        }\n"
"        button:active {\n"
"            background-color: #3e8e41;\n"
"        }\n"
"    </style>\n"
"    <script>\n"
"        function sendRequest(uri) {\n"
"            fetch(uri)\n"
"                .then(response => console.log('Request successful: ' + uri))\n"
"                .catch(err => console.error('Error: ' + err));\n"
"        }\n"
"    </script>\n"
"</head>\n"
"<body>\n"
"    <h1>ESP32 LED Control</h1>\n"
"    <div class=\"button-container\">\n"
"        <button onclick=\"sendRequest('/led1/on')\">Turn LED1 ON</button>\n"
"        <button onclick=\"sendRequest('/led1/off')\">Turn LED1 OFF</button>\n"
"        <button onclick=\"sendRequest('/led2/on')\">Turn LED2 ON</button>\n"
"        <button onclick=\"sendRequest('/led2/off')\">Turn LED2 OFF</button>\n"
"    </div>\n"
"</body>\n"
"</html>";



// Handler for the root URL
static esp_err_t root_handler(httpd_req_t *req) {
    httpd_resp_send(req, HTML_PAGE, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

// Handler for LED1 ON
static esp_err_t led1_on_handler(httpd_req_t *req) {
    gpio_set_level(LED1_GPIO, 1);
    httpd_resp_send(req, "LED1 is ON", HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

// Handler for LED1 OFF
static esp_err_t led1_off_handler(httpd_req_t *req) {
    gpio_set_level(LED1_GPIO, 0);
    httpd_resp_send(req, "LED1 is OFF", HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

// Handler for LED2 ON
static esp_err_t led2_on_handler(httpd_req_t *req) {
    gpio_set_level(LED2_GPIO, 1);
    httpd_resp_send(req, "LED2 is ON", HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

// Handler for LED2 OFF
static esp_err_t led2_off_handler(httpd_req_t *req) {
    gpio_set_level(LED2_GPIO, 0);
    httpd_resp_send(req, "LED2 is OFF", HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

// Function to start the web server
static httpd_handle_t start_webserver(void) {
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    httpd_handle_t server = NULL;

    if (httpd_start(&server, &config) == ESP_OK) {
        // Register root URI handler
        httpd_uri_t root = {
            .uri = "/",
            .method = HTTP_GET,
            .handler = root_handler
        };
        httpd_register_uri_handler(server, &root);

        // Register URI handlers for LED control
        httpd_uri_t led1_on = { .uri = "/led1/on", .method = HTTP_GET, .handler = led1_on_handler };
        httpd_uri_t led1_off = { .uri = "/led1/off", .method = HTTP_GET, .handler = led1_off_handler };
        httpd_uri_t led2_on = { .uri = "/led2/on", .method = HTTP_GET, .handler = led2_on_handler };
        httpd_uri_t led2_off = { .uri = "/led2/off", .method = HTTP_GET, .handler = led2_off_handler };

        httpd_register_uri_handler(server, &led1_on);
        httpd_register_uri_handler(server, &led1_off);
        httpd_register_uri_handler(server, &led2_on);
        httpd_register_uri_handler(server, &led2_off);
    }

    return server;
}

// Main function
void app_main(void) {
    // Initialize NVS
    ESP_ERROR_CHECK(nvs_flash_init());

    // Initialize the network stack
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    // Configure Wi-Fi
    esp_netif_create_default_wifi_sta();
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = "Airbox-86C2",
            .password = "WGaWvxCa",
        },
    };
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());
    ESP_ERROR_CHECK(esp_wifi_connect());

    // Configure GPIOs for LEDs
    gpio_set_direction(LED1_GPIO, GPIO_MODE_OUTPUT);
    gpio_set_direction(LED2_GPIO, GPIO_MODE_OUTPUT);

    // Start the web server
    start_webserver();
}
