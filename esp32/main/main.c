#include <stdio.h>
#include <unistd.h>

#include "esp_err.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "nvs_flash.h"
#include "wifi.h"

static const char *TAG = "main";

void app_main(void) {
  esp_log_level_set("*", ESP_LOG_VERBOSE);
  esp_log_level_set(TAG, ESP_LOG_VERBOSE);

  ESP_LOGI(TAG, "IDF version: %s", esp_get_idf_version());
  ESP_LOGI(TAG, "free memory: %d bytes", esp_get_free_heap_size());

  wifi_connect();
}
