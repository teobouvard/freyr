#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/task.h"

#include "esp_event.h"
#include "esp_log.h"
#include "esp_mac.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "nvs_flash.h"

/* WiFi configuration can be set using project configuration menu. */
#define EXAMPLE_ESP_WIFI_SSID CONFIG_ESP_WIFI_SSID
#define EXAMPLE_ESP_WIFI_PASS CONFIG_ESP_WIFI_PASSWORD
#define EXAMPLE_ESP_MAXIMUM_RETRY CONFIG_ESP_WIFI_MAXIMUM_RETRY

#if CONFIG_ESP_WIFI_AUTH_OPEN
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_OPEN
#elif CONFIG_ESP_WIFI_AUTH_WEP
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WEP
#elif CONFIG_ESP_WIFI_AUTH_WPA_PSK
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA_PSK
#elif CONFIG_ESP_WIFI_AUTH_WPA2_PSK
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA2_PSK
#elif CONFIG_ESP_WIFI_AUTH_WPA_WPA2_PSK
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA_WPA2_PSK
#elif CONFIG_ESP_WIFI_AUTH_WPA3_PSK
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA3_PSK
#elif CONFIG_ESP_WIFI_AUTH_WPA2_WPA3_PSK
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA2_WPA3_PSK
#elif CONFIG_ESP_WIFI_AUTH_WAPI_PSK
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WAPI_PSK
#endif

static const char *TAG = "app_wifi";
static char IPV4_ADDR[] = "000.000.000.000";
static char MAC_ADDR[] = "XX:XX:XX:XX:XX:XX";
static int n_retries = 0;

/* FreeRTOS event group to signal when we are connected */
static EventGroupHandle_t wifi_event_group;
#define WIFI_CONNECTED_BIT BIT0

static void on_wifi_start(void *arg, esp_event_base_t event_base,
                          int32_t event_id, void *event_data) {
  ESP_LOGI(TAG, "wifi started");
  ESP_ERROR_CHECK(esp_wifi_connect());
}

static void on_wifi_connect(void *arg, esp_event_base_t event_base,
                            int32_t event_id, void *event_data) {
  n_retries = 0;
  ESP_LOGI(TAG, "wifi connected");
}

static void on_wifi_got_ip(void *arg, esp_event_base_t event_base,
                           int32_t event_id, void *event_data) {
  ESP_LOGI(TAG, "got IP address");
  ip_event_got_ip_t *event = event_data;
  snprintf(IPV4_ADDR, sizeof(IPV4_ADDR), IPSTR, IP2STR(&event->ip_info.ip));
  xEventGroupSetBits(wifi_event_group, WIFI_CONNECTED_BIT);
}

static void on_wifi_disconnect(void *arg, esp_event_base_t event_base,
                               int32_t event_id, void *event_data) {
  ESP_LOGI(TAG, "wifi disconnected");
  if (n_retries++ < EXAMPLE_ESP_MAXIMUM_RETRY) {
    ESP_LOGI(TAG, "retrying connection %d/%d", n_retries,
             EXAMPLE_ESP_MAXIMUM_RETRY);
    ESP_ERROR_CHECK(esp_wifi_connect());
  } else {
    ESP_LOGI(TAG, "rebooting");
    esp_restart();
  }
}

void wifi_connect(void) {
  wifi_event_group = xEventGroupCreate();

  // Initialize non-volatile storage used for WiFi data
  ESP_ERROR_CHECK(nvs_flash_init());

  // Initialize TCP/IP stack
  ESP_ERROR_CHECK(esp_netif_init());

  uint8_t mac_addr[6];
  ESP_ERROR_CHECK(esp_read_mac(mac_addr, ESP_MAC_WIFI_STA));
  snprintf(MAC_ADDR, sizeof(MAC_ADDR), MACSTR, MAC2STR(mac_addr));

  // Create event loop
  ESP_ERROR_CHECK(esp_event_loop_create_default());

  // Create default WiFi station
  esp_netif_create_default_wifi_sta();

  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  ESP_ERROR_CHECK(esp_wifi_init(&cfg));

  ESP_ERROR_CHECK(esp_event_handler_register(
      WIFI_EVENT, WIFI_EVENT_STA_CONNECTED, &on_wifi_connect, NULL));
  ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP,
                                             &on_wifi_got_ip, NULL));
  ESP_ERROR_CHECK(esp_event_handler_register(
      WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, &on_wifi_disconnect, NULL));
  ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_STA_START,
                                             &on_wifi_start, NULL));

  wifi_config_t wifi_config = {
      .sta =
          {
              .ssid = EXAMPLE_ESP_WIFI_SSID,
              .password = EXAMPLE_ESP_WIFI_PASS,
              .threshold.authmode = ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD,
          },
  };
  ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
  ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
  ESP_ERROR_CHECK(esp_wifi_start());

  EventBits_t status = xEventGroupWaitBits(wifi_event_group, WIFI_CONNECTED_BIT,
                                           pdFALSE, pdFALSE, portMAX_DELAY);

  if (status & WIFI_CONNECTED_BIT) {
    ESP_LOGI(TAG, "connected to wifi");
  } else {
    ESP_LOGI(TAG, "failed to connect to wifi");
    esp_restart();
  }
}

const char *wifi_get_ip() { return IPV4_ADDR; }

const char *wifi_get_mac() { return MAC_ADDR; }
