#ifndef __FREYR_WIFI_H__
#define __FREYR_WIFI_H__

/**
 * Connect to a WiFi Access Point.
 *
 * This uses the network parameters defined during project configuration.
 */
void wifi_connect();

/**
 * Get current IP address.
 *
 * Returns a pointer to a statically-allocated string containing the current
 * IPv4 address, with a default value when no address has been assigned.
 */
const char *wifi_get_ip();

#endif /* __FREYR_WIFI_H__ */
