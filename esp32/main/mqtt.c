#include "esp_err.h"
#include "esp_log.h"
#include "mqtt_client.h"

#include "mqtt.h"

#define EXAMPLE_MQTT_BROKER_URL CONFIG_ESP_MQTT_BROKER_URL

static const char *TAG = "app_mqtt";
static const char *MQTT_PUBLISH_TOPIC = "/sensors/0/data";
static const char *MQTT_SUBSCRIBE_TOPIC = "/sensors/requests";
static const int MQTT_QOS = 0;
static const int MQTT_USE_DATA_LENGTH = 0;

static void mqtt_handle_message(esp_mqtt_event_handle_t event) {
  if (!event) {
    return;
  }
  esp_mqtt_client_handle_t client = event->client;
  printf("TOPIC: %.*s\n", event->topic_len, event->topic);
  printf("DATA: %.*s\n", event->data_len, event->data);
  esp_mqtt_client_publish(client, MQTT_PUBLISH_TOPIC, "sample_data",
                          MQTT_USE_DATA_LENGTH, MQTT_QOS, 0);
};

static void mqtt_event_handler(void *handler_args, esp_event_base_t base,
                               int32_t event_id, void *event_data) {
  esp_mqtt_event_handle_t event = event_data;
  esp_mqtt_client_handle_t client = event->client;

  switch (event_id) {
  case MQTT_EVENT_CONNECTED:
    ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
    esp_mqtt_client_subscribe(client, MQTT_SUBSCRIBE_TOPIC, MQTT_QOS);
    break;
  case MQTT_EVENT_DISCONNECTED:
    ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
    break;
  case MQTT_EVENT_SUBSCRIBED:
    ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED (%d)", event->msg_id);
    break;
  case MQTT_EVENT_UNSUBSCRIBED:
    ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED (%d)", event->msg_id);
    break;
  case MQTT_EVENT_PUBLISHED:
    ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED (%d)", event->msg_id);
    break;
  case MQTT_EVENT_DATA:
    ESP_LOGI(TAG, "MQTT_EVENT_DATA (%.*s)", event->topic_len, event->topic);
    mqtt_handle_message(event);
    break;
  case MQTT_EVENT_ERROR:
    ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
    break;
  default:
    ESP_LOGI(TAG, "MQTT_EVENT_OTHER: %d", event->event_id);
    break;
  }
}

void mqtt_connect() {
  // Create and initialize MQTT client
  esp_mqtt_client_config_t mqtt_cfg = {
      .uri = EXAMPLE_MQTT_BROKER_URL,
  };
  esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);
  if (client == NULL) {
    ESP_LOGE(TAG, "Could not initialize MQTT client");
    abort();
  }

  // Set event handler
  ESP_ERROR_CHECK(esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID,
                                                 mqtt_event_handler, NULL));

  // Start client
  ESP_ERROR_CHECK(esp_mqtt_client_start(client));
}
