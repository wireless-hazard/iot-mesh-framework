#include "unity.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "mesh_framework.h"

void setUp(void) {

}

void tearDown(void) {
}

TEST_CASE("networking init without config","[init]")
{
	esp_err_t err = meshf_start(portMAX_DELAY);
	TEST_ASSERT_EQUAL_INT(ESP_ERR_INVALID_STATE, err);
}

TEST_CASE("proper mesh initialization","[init]")
{
	printf("\nFREE HEAP: %d\n",esp_get_minimum_free_heap_size());
	meshf_init();
	esp_err_t err = meshf_start(35000/portTICK_PERIOD_MS);
	TEST_ASSERT_EQUAL_INT(ESP_OK, err);
	err = meshf_stop();
	TEST_ASSERT_EQUAL_INT(ESP_OK, err);
	printf("\nFREE HEAP: %d\n",esp_get_minimum_free_heap_size());
}

TEST_CASE("sequencially initialization/deinitialization", "[init]")
{
	TEST_FAIL();
}