/*
 * mqtt.h
 *
 *  Created on: 15 sep 2022
 *      Author: klaslofstedt
 */
#ifndef _MQTT_H_
#define _MQTT_H_

#define MQTT_OVERHEAD_SIZE 6   // 2 (header), 2 (QoS identifier), 2 (topic length)
#define MQTT_TOPIC_MAX_SIZE 32 // Chosen because currently longest topic is 31 bytes
#define MQTT_DATA_MAX_LEN (CONFIG_MQTT_BUFFER_SIZE - MQTT_TOPIC_MAX_SIZE - MQTT_OVERHEAD_SIZE)

typedef void (*mqtt_received_callback_t)(const char *data, int data_len);

bool mqtt_init(void);
bool mqtt_stop(void);
bool mqtt_register_subscription(const char* topic, mqtt_received_callback_t received_callback);
bool mqtt_subscribe(void);
bool mqtt_publish(const char* topic, const char* data);

#endif /* _MQTT_H_ */
