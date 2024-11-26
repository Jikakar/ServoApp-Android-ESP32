#include "web_server.h"
#include "servo.h"
#include "esp_log.h"
#include <stdio.h>

static const char *TAG = "WEB_SERVER";

// HTML content
static const char *HTML_PAGE =
    "<!DOCTYPE html>"
    "<html>"
    "<head>"
    "<title>Servo Control</title>"
    "<style>"
    "body { font-family: Arial, sans-serif; text-align: center; }"
    "button { margin: 10px; padding: 10px 20px; font-size: 16px; }"
    "input { font-size: 16px; padding: 5px; margin: 10px; }"
    "</style>"
    "</head>"
    "<body>"
    "<h1>ESP32 Servo Control</h1>"
    "<p>Current Angle: <span id='current_angle'>0</span>°</p>"
    "<p>"
    "<button onclick='changeAngle(-2)'>Decrease by 2</button>"
    "<button onclick='changeAngle(2)'>Increase by 2</button>"
    "</p>"
    "<p>"
    "<label for='angle_input'>Set Angle: </label>"
    "<input id='angle_input' type='number'>"
    "<button onclick='setAngle()'>Set</button>"
    "</p>"
    "<p>Status: <span id='status'>Waiting...</span></p>"
    "<script>"
    "function changeAngle(delta) {"
    "  fetch(`/change_angle?delta=${delta}`).then(response => response.text()).then(data => {"
    "    document.getElementById('status').innerText = data;"
    "    updateCurrentAngle();"
    "  });"
    "}"
    "function setAngle() {"
    "  const angle = document.getElementById('angle_input').value;"
    "  fetch(`/set_angle?angle=${angle}`).then(response => response.text()).then(data => {"
    "    document.getElementById('status').innerText = data;"
    "    updateCurrentAngle();"
    "  });"
    "}"
    "function updateCurrentAngle() {"
    "  fetch('/get_angle').then(response => response.text()).then(angle => {"
    "    document.getElementById('current_angle').innerText = angle;"
    "  });"
    "}"
    "updateCurrentAngle();"
    "</script>"
    "</body>"
    "</html>";

// Main page handler
static esp_err_t root_handler(httpd_req_t *req) {
    httpd_resp_send(req, HTML_PAGE, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

// Handler for changing the angle relatively
static esp_err_t change_angle_handler(httpd_req_t *req) {
    char query[32];
    if (httpd_req_get_url_query_str(req, query, sizeof(query)) == ESP_OK) {
        char delta_str[8];
        if (httpd_query_key_value(query, "delta", delta_str, sizeof(delta_str)) == ESP_OK) {
            int delta = atoi(delta_str);
            set_servo_angle(current_angle + delta); // Update angle relatively
            httpd_resp_send(req, "Angle updated relatively", HTTPD_RESP_USE_STRLEN);
            return ESP_OK;
        }
    }
    httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Bad Request");
    return ESP_FAIL;
}

// Handler for setting the absolute angle
static esp_err_t set_angle_handler(httpd_req_t *req) {
    char query[32];
    if (httpd_req_get_url_query_str(req, query, sizeof(query)) == ESP_OK) {
        char angle_str[8];
        if (httpd_query_key_value(query, "angle", angle_str, sizeof(angle_str)) == ESP_OK) {
            int angle = atoi(angle_str);
            set_servo_angle(angle); // Update angle absolutely
            httpd_resp_send(req, "Angle updated absolutely", HTTPD_RESP_USE_STRLEN);
            return ESP_OK;
        }
    }
    httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Bad Request");
    return ESP_FAIL;
}

// Handler for getting the current angle
static esp_err_t get_angle_handler(httpd_req_t *req) {
    char angle_str[8];
    snprintf(angle_str, sizeof(angle_str), "%d", current_angle);
    httpd_resp_send(req, angle_str, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

// Start the web server
httpd_handle_t start_web_server() {
    httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    if (httpd_start(&server, &config) == ESP_OK) {
        httpd_uri_t root_uri = {
            .uri = "/",
            .method = HTTP_GET,
            .handler = root_handler,
        };
        httpd_register_uri_handler(server, &root_uri);

        httpd_uri_t set_angle_uri = {
            .uri = "/set_angle",
            .method = HTTP_GET,
            .handler = set_angle_handler,
        };
        httpd_register_uri_handler(server, &set_angle_uri);

        httpd_uri_t change_angle_uri = {
            .uri = "/change_angle",
            .method = HTTP_GET,
            .handler = change_angle_handler,
        };
        httpd_register_uri_handler(server, &change_angle_uri);

        httpd_uri_t get_angle_uri = {
            .uri = "/get_angle",
            .method = HTTP_GET,
            .handler = get_angle_handler,
        };
        httpd_register_uri_handler(server, &get_angle_uri);

        ESP_LOGI(TAG, "Web server started successfully.");
    } else {
        ESP_LOGE(TAG, "Failed to start web server.");
    }
    return server;
}
