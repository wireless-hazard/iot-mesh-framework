#include "unity.h"
#include "esp_system.h"

void app_main(void){
	
	UNITY_BEGIN();
	unity_run_all_tests();
	UNITY_END();
	esp_system_abort("END OF THE UNITY TEST");
	// unity_run_menu();
	// esp_restart();
}
