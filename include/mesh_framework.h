#pragma once

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*custom_callback_function)(char *parameter, size_t param_lenght);

void STR2MAC(uint8_t *address,char rec_string[17]);
void tx_p2p(void *pvParameters);
void meshf_tx_p2p(char mac_destination[],uint8_t transmitted_data[],uint16_t data_size);
void meshf_tx_TODS(char ip_destination[],int port,uint8_t transmitted_data[],uint16_t data_size);
esp_err_t meshf_rx(uint8_t *array_data);
esp_err_t meshf_asktime(TickType_t xTicksToWait);
esp_err_t meshf_mqtt_publish(const char *topic, uint16_t topic_size, const char *data, uint16_t data_size);
esp_err_t meshf_mqtt_subscribe(const char *topic, int qos, custom_callback_function func_addr);
void meshf_ping(char mac_destination[]);
void meshf_rssi_info(int8_t *rssi,char interested_mac[]);
void meshf_task_debugger(void);
void data_ready();
void free_rx_buffer();
void meshf_sleep_time(float delay);
void meshf_init();
esp_err_t meshf_start(TickType_t xTicksToWait);
esp_err_t meshf_stop(void);
esp_err_t meshf_start_mqtt();
esp_err_t meshf_start_sntp();

#ifdef __cplusplus
}
#endif
