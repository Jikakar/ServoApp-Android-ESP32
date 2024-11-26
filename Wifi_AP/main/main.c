#include "wifi.h"
#include "servo.h"
#include "web_server.h"
#include "nvs_flash.h"

void app_main() {
    nvs_flash_init();
    init_wifi();
    init_servo();
    start_web_server();
}
