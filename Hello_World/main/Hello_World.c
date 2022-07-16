#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_system.h"
#include "esp_log.h"

#define LED 2
#define GPIO_PIN_SET   1
#define GPIO_PIN_RESET 0
#define Delay 1000

static const char *TAG = "Blinking LED";

void vTask(void *pvParameters) {
	
	while(1) {
		
		ESP_LOGI(TAG, "GPIO_PIN_SET");
		gpio_set_level(LED, GPIO_PIN_SET);
		vTaskDelay(Delay / portTICK_PERIOD_MS);
		
		ESP_LOGI(TAG, "GPIO_PIN_RESET");
		gpio_set_level(LED, GPIO_PIN_RESET);
		vTaskDelay(Delay / portTICK_PERIOD_MS);
		
	}
	
}

void app_main(void) {

	gpio_reset_pin(LED);
	gpio_set_direction(LED, GPIO_MODE_OUTPUT);
	
	xTaskCreate(vTask, "LED", 2048, NULL, 1, NULL);

}