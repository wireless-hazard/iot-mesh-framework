#include <string.h>
#include "esp_system.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_mesh.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/event_groups.h"

#include <esp_system.h>
#include <time.h>
#include <sys/time.h>
#include <cJSON.h>

#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"

#include "mqtt_client.h"
#include "esp_sntp.h"

#ifdef __cplusplus
extern "C" {
#endif

void STR2MAC(uint8_t *address,char rec_string[17]);
uint16_t meshf_uint8_t_json_creator(uint8_t json_value[], uint8_t key[], uint16_t size_key, uint8_t value[], uint16_t size_value);
void tx_p2p(void *pvParameters);
void meshf_tx_p2p(char mac_destination[],uint8_t transmitted_data[],uint16_t data_size);
void meshf_tx_TODS(char ip_destination[],int port,uint8_t transmitted_data[],uint16_t data_size);
esp_err_t meshf_rx(uint8_t *array_data);
esp_err_t meshf_asktime(TickType_t xTicksToWait);
esp_err_t meshf_mqtt_publish(const char *topic, uint16_t topic_size, const char *data, uint16_t data_size);
esp_err_t meshf_mqtt_subscribe(const char *topic, int qos, void (*custom_callback_function)(char *parameter, size_t param_lenght));
void meshf_ping(char mac_destination[]);
void meshf_rssi_info(int8_t *rssi,char interested_mac[]);
void meshf_task_debugger(void);
void data_ready();
void free_rx_buffer();
void meshf_sleep_time(float delay);
void meshf_init();
esp_err_t meshf_start(TickType_t xTicksToWait);
esp_err_t meshf_stop(void);
void meshf_start_mqtt();
void meshf_start_sntp();

extern esp_mqtt_client_handle_t mqtt_handler;
extern mesh_data_t rx_data;

#ifdef __cplusplus
}
#endif
