#include <stdint.h>

#include "esp_err.h"
#include "esp_log.h"
#include "esp_sleep.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "mqtt.h"
#include "mqtt_client.h"
#include "wifi.h"

static const char *TAG = "app_main";
static const int64_t SLEEP_DURATION = 10000000;
static const int64_t WAIT_DELAY = 1000 / portTICK_PERIOD_MS;

void on_message_sent_sleep() {
  ESP_ERROR_CHECK(esp_sleep_enable_timer_wakeup(SLEEP_DURATION));
  esp_deep_sleep_start();
}

void on_message_sent_log() {
  ESP_LOGI(TAG, "message sent");
  vTaskDelay(5 * WAIT_DELAY);
}

void app_main(void) {
  esp_log_level_set("*", ESP_LOG_VERBOSE);
  esp_log_level_set(TAG, ESP_LOG_VERBOSE);

  ESP_LOGI(TAG, "IDF version: %s", esp_get_idf_version());
  ESP_LOGI(TAG, "free memory: %d bytes", esp_get_free_heap_size());

  wifi_connect();
  esp_mqtt_client_handle_t client = mqtt_connect();

  while (true) {
    // TODO read data from sensors
    // mqtt_send_message(client, "sample_topic", "sample_data",
    // on_message_sent_sleep);
    mqtt_send_message(client, "sample_topic", "sample_data",
                      on_message_sent_log);
    vTaskDelay(WAIT_DELAY);
  }
}
