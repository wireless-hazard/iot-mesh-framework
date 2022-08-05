#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_wifi.h"
#include <cJSON.h>
#include "esp_log.h"

#include <string.h>
#include <esp_system.h>
#include <time.h>
#include <sys/time.h>

//Define the following macro to 1 if the sensor is going to be used
#define ULTRASONIC 0
#define GPS 0
#define TEMP 0
#define SCREEN 1

#if TEMP
#include "MLX90614_SMBus_Driver.h"
#include "MLX90614_API.h"
#endif

#if ULTRASONIC
#include <ultrasonic.h>
#endif //ULTRASONIC

#if GPS
#include "aos_gps_c.h"
#endif //GPS

#if SCREEN
#include "ssd1306.h"

#include "font8x8_basic.h"
#endif

#include "mesh_framework.h"

#include <driver/adc.h>
#include "driver/gpio.h"
#include "esp_sleep.h"

#define MAX_DISTANCE_CM 300 // 5m max
#define TRIGGER_GPIO GPIO_NUM_23
#define ECHO_GPIO GPIO_NUM_22

#if TEMP
#define SDA GPIO_NUM_25
#define SCL GPIO_NUM_33
#endif

RTC_DATA_ATTR float num_of_wakes[10];

#if ULTRASONIC
static ultrasonic_sensor_t sensor;
#endif
#if SCREEN
static SSD1306_t dev;
#endif
#if GPS
static uBloxGPS_t *gps = NULL;
#endif

void custom_callback(char *parameter, size_t lenght){

	if (strcmp(parameter, "ON") == 0){
		gpio_set_level(2, 1);
	}else if (strcmp(parameter, "OFF") == 0){
		gpio_set_level(2, 0);
	}
	#if SCREEN
	ssd1306_clear_screen(&dev, false);
	ssd1306_contrast(&dev, 0xff);
  	ssd1306_display_text(&dev, 0, parameter, lenght, false);
  	vTaskDelay(3000 / portTICK_PERIOD_MS);
	#endif
	return;
}

void time_message_generator(char final_message[], int value){ //Formata a data atual do ESP para dentro do array especificado
	char strftime_buff[64];
	time_t now = 0;
    struct tm timeinfo = { 0 }; 
    uint8_t self_mac[6] = {0};
    char mac_str[18] = {0};
    char parent_mac_str[18] = {0};

    wifi_ap_record_t mesh_parent;

    time(&now); //Pega o tempo armazenado no RTC
	setenv("TZ", "UTC+3", 1); //Configura variaveis de ambiente com esse time zona
	tzset(); //Define a time zone
	localtime_r(&now, &timeinfo); //Reformata o horario pego do RTC

	esp_read_mac(self_mac,ESP_MAC_WIFI_SOFTAP); //Pega o MAC da interface Access Point
	strftime(strftime_buff, sizeof(strftime_buff), "%c", &timeinfo); //Passa a struct anterior para uma string
	sprintf(final_message, "%s",strftime_buff);
	sprintf(mac_str,MACSTR,MAC2STR(self_mac));

	esp_wifi_sta_get_ap_info(&mesh_parent);
	sprintf(parent_mac_str,MACSTR,MAC2STR(mesh_parent.bssid));

	cJSON *json_mqtt = cJSON_CreateObject();
	cJSON_AddStringToObject(json_mqtt,"mac", mac_str);
	cJSON_AddStringToObject(json_mqtt,"parent_mac", parent_mac_str);
	cJSON_AddNumberToObject(json_mqtt,"rssi", mesh_parent.rssi);
	cJSON_AddNumberToObject(json_mqtt,"distance", value);
	cJSON_AddStringToObject(json_mqtt,"time", final_message);

	#if GPS
	cJSON_AddNumberToObject(json_mqtt,"lat", (-1)*gps_GetLat(gps));
	cJSON_AddNumberToObject(json_mqtt,"lon", (-1)*gps_GetLng(gps));

	#endif //GPS

	#if TEMP
	float temp = 0;
	MLX90614_GetTa(I2C_NUM_0, 0x3, &temp);
	cJSON_AddNumberToObject(json_mqtt,"temp", temp);
	#endif

	char *string2 = cJSON_Print(json_mqtt);
	strcpy(final_message,string2);
	free(string2);
	free(json_mqtt);
}

int next_sleep_time(const struct tm timeinfo, int fixed_gap){ //Recebe o valor em minutos e calcula em quantos segundos o ESP devera acordar, considerando o inicio em uma hora exata.
	if (fixed_gap <= 0){ //Se o valor do delay for de 0 minutos, retorna 0 minutos sem fazer nenhum calculo
		return 0;
	}

	int minutes = timeinfo.tm_min; //Pega o valor dos minutos atuais do RTC
	int rouded_minutes = (minutes / fixed_gap) * fixed_gap; //Transforma o valor dos minutos atuais do RTC o multiplo anterior de fixed_gap 
	int next_minutes = rouded_minutes + fixed_gap - 1; //Calcula qual o prox valor de minutos que seja multiplo de fixed_gap 
	int next_seconds = 60 - timeinfo.tm_sec; //Calcula o valor de segundos até que seja o minuto exato: XX:XX:00

	return ((next_minutes - minutes)*60 + next_seconds); //Retorna o tempo em segundos até que o ESP tenha seu RTC a XX:AA:00 sendo AA o prox valor em minutos multiplo de fixed_gap
}

esp_err_t measure_distance(float *distance){
	assert(distance != NULL);
	
    #if ULTRASONIC 

    esp_err_t err = ultrasonic_measure(&sensor, MAX_DISTANCE_CM, distance);
    *distance = (*distance)*100.0;

	#else

    esp_err_t err = ESP_OK;
    //Generates a number between 0 MAX_DISTANCE_CM
    *distance = (float)(esp_random()/(UINT32_MAX/MAX_DISTANCE_CM + 1)); 

    #endif
    
    return err;
    
    
}

void app_main(void) {
	
	gpio_reset_pin(2);
    gpio_set_direction(2, GPIO_MODE_OUTPUT);
    gpio_set_level(2, 1); //Acende o LED interno do ESP para mostrar que o ESP esta ligado
    
    char mqtt_data[150];

	uint8_t rx_mensagem[180] = {0,};
	time_t now = 0;
    struct tm timeinfo = { 0 };
    meshf_init(); //Inicializa as configuracoes da rede MESH
    
	#if ULTRASONIC
	sensor.trigger_pin = TRIGGER_GPIO;
    sensor.echo_pin = ECHO_GPIO;
    ESP_ERROR_CHECK(ultrasonic_init(&sensor));
    #endif

	num_of_wakes[0] = 0;
	ESP_LOGW("MESH_TAG","SENSOR STATUS: %s\nDISTANCE: %f\n",esp_err_to_name(measure_distance(&num_of_wakes[0])),num_of_wakes[0]);
				
    num_of_wakes[0] = 0;
    		
    esp_err_t mesh_on = meshf_start(45000/portTICK_PERIOD_MS); //Inicializa a rede MESH
    if (mesh_on != ESP_OK){
		esp_restart();
	}

	meshf_start_sntp(); //Se conecta ao server SNTP e atualiza o seu relogio RTC (caso seja root)
	ESP_ERROR_CHECK(meshf_rx(rx_mensagem)); //Seta o buffer para recepcao das mensagens
	meshf_start_mqtt(); //Conecta-se ao servidor MQTT
	ESP_ERROR_CHECK(meshf_asktime(45000/portTICK_PERIOD_MS)); //Pede ao noh root pelo horario atual que foi recebido pelo SNTP (caso nao seja root)
	ESP_ERROR_CHECK(meshf_mqtt_subscribe("/data/esp32/downstream", 0, &custom_callback));

	#if TEMP
	MLX90614_SMBusInit(I2C_NUM_0, SDA, SCL, 10000);
	#endif

	#if GPS
	gps = init_Gps(UART_NUM_2, GPIO_NUM_17, GPIO_NUM_16);
	#endif

	#if SCREEN
	i2c_master_init(&dev, CONFIG_SDA_GPIO, CONFIG_SCL_GPIO, CONFIG_RESET_GPIO);
	ESP_LOGI("tag", "Panel is 128x64");
	ssd1306_init(&dev, 128, 64);

	#endif
	while(true)
	{
		time(&now); //Pega o tempo armazenado no RTC
		setenv("TZ", "UTC+3", 1); //Configura variaveis de ambiente com esse time zona
		tzset(); //Define a time zone
		localtime_r(&now, &timeinfo); //Reformata o horario pego do RTC

		measure_distance(&num_of_wakes[0]);
		time_message_generator(mqtt_data,num_of_wakes[0]); //Formata a data atual do ESP para dentro do array especificado
		printf("%s\n",mqtt_data);
		esp_err_t resp = meshf_mqtt_publish("/data/esp32",strlen("/data/esp32"),mqtt_data,strlen(mqtt_data)); //Publica a data atual no topico /data/esp32
		printf("PUBLICACAO MQTT = %s\n",esp_err_to_name(resp));

    	int awake_until = next_sleep_time(timeinfo,1); //Calcula até quanto tempo para XX:AA:00. Sendo AA os minutos multiplos de 5 mais prox.
		ESP_LOGW("MESH_TAG","Vai continuar acordado por %d segundos\n",awake_until);
		ESP_LOGI("TAG","Free Heap memory: %d",esp_get_free_heap_size());
		ESP_LOGI("TAG","Free Heap memory: %d",heap_caps_get_largest_free_block(MALLOC_CAP_8BIT));
		meshf_sleep_time(awake_until*1000); //Bloqueia o fluxo do codigo até que o horario estipulado anteriormente seja atingido
	}
}