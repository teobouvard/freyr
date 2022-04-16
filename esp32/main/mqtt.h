#ifndef __FREYR_MQTT_H__
#define __FREYR_MQTT_H__

#include "mqtt_client.h"

typedef void (*callback_fn_t)(void);

/**
 * Connect a MQTT broker.
 *
 * The broker parameters used are defined during project configuration.
 *
 */
esp_mqtt_client_handle_t mqtt_connect();

/**
 * Send data to a given topic, running callback once message is published.
 *
 * Returns the message ID.
 */
int mqtt_send_message(esp_mqtt_client_handle_t client, const char *topic,
                      const char *data, callback_fn_t on_sent);

#endif /* __FREYR_MQTT_H__ */
