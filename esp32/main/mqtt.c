#include "esp_err.h"
#include "esp_log.h"
#include "mqtt_client.h"

#include "mqtt.h"
#include "wifi.h"

#define EXAMPLE_MQTT_BROKER_URL CONFIG_ESP_MQTT_BROKER_URL

static const char *TAG = "app_mqtt";
static const char *MQTT_PUBLISH_TOPIC_FMT = "sensors/%s/%s";
static const char *MQTT_SUBSCRIBE_TOPIC = "sensors/requests";
static char MQTT_PUBLISH_TOPIC[128];
static const int MQTT_QOS = 1;
static const int MQTT_USE_DATA_LENGTH = 0;

typedef struct {
  int msg_id;
  callback_fn_t cb;
} msg_callback_t;
static msg_callback_t current_callback = {
    .msg_id = -1,
    .cb = NULL,
};

static void mqtt_set_topic(const char *topic) {
  snprintf(MQTT_PUBLISH_TOPIC, sizeof(MQTT_PUBLISH_TOPIC),
           MQTT_PUBLISH_TOPIC_FMT, wifi_get_mac(), topic);
}

static void on_msg_published(void *handler_args, esp_event_base_t base,
                             int32_t event_id, void *event_data) {
  esp_mqtt_event_handle_t event = event_data;
  ESP_LOGI(TAG, "published message %d", event->msg_id);
  if (current_callback.msg_id == event->msg_id && current_callback.cb != NULL) {
    ESP_LOGI(TAG, "running registered callback");
    current_callback.cb();
  }
}

int mqtt_send_message(esp_mqtt_client_handle_t client, const char *topic,
                      const char *data, callback_fn_t on_sent) {
  if (!client) {
    return -1;
  }
  mqtt_set_topic(topic);
  int msg_id = esp_mqtt_client_publish(client, MQTT_PUBLISH_TOPIC, data,
                                       MQTT_USE_DATA_LENGTH, MQTT_QOS, 0);
  if (msg_id == -1) {
    return -1;
  }

  if (on_sent != NULL) {
    ESP_LOGI(TAG, "sent message %d, registering callback", msg_id);
    current_callback.msg_id = msg_id;
    current_callback.cb = on_sent;
    ESP_ERROR_CHECK(esp_mqtt_client_register_event(client, MQTT_EVENT_PUBLISHED,
                                                   on_msg_published, NULL));
  }

  return msg_id;
}

static void mqtt_handle_message(esp_mqtt_event_handle_t event) {
  if (!event) {
    return;
  }
  ESP_LOGI(TAG, "TOPIC: %.*s", event->topic_len, event->topic);
  ESP_LOGI(TAG, "DATA: %.*s", event->data_len, event->data);
};

static void mqtt_event_handler(void *handler_args, esp_event_base_t base,
                               int32_t event_id, void *event_data) {
  esp_mqtt_event_handle_t event = event_data;

  switch (event_id) {
  case MQTT_EVENT_CONNECTED:
    ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
    esp_mqtt_client_subscribe(event->client, MQTT_SUBSCRIBE_TOPIC, MQTT_QOS);
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
  case MQTT_EVENT_BEFORE_CONNECT:
    ESP_LOGI(TAG, "MQTT_EVENT_BEFORE_CONNECT");
    break;
  default:
    ESP_LOGI(TAG, "MQTT_EVENT_OTHER: %d", event->event_id);
    break;
  }
}

esp_mqtt_client_handle_t mqtt_connect() {
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

  return client;
}
