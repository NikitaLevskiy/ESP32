#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/uart.h"
#include "esp_system.h"
#include "soc/soc.h"

#define GPIO_PIN_SET   1
#define GPIO_PIN_RESET 0

#define Mode8bit 1
#define Mode4bit 0

#define RS 13
#define E  12
#define D7 23
#define D6 22
#define D5 21
#define D4 19
#define D3 18
#define D2  5
#define D1  4
#define D0  2

void GPIOInit(int mode)
{
	if (mode == Mode8bit)
	{
		gpio_pad_select_gpio(RS);
		gpio_pad_select_gpio(E);
		gpio_pad_select_gpio(D7);
		gpio_pad_select_gpio(D6);
		gpio_pad_select_gpio(D5);
		gpio_pad_select_gpio(D4);
		gpio_pad_select_gpio(D3);
		gpio_pad_select_gpio(D2);
		gpio_pad_select_gpio(D1);
		gpio_pad_select_gpio(D0);
		
		
		const gpio_config_t conf_gpio = {	
			.pin_bit_mask = (1 << E)|(1 << RS)|
							(1 << D7)|(1 << D6)|(1 << D5)|(1 << D4)|
							(1 << D3)|(1 << D2)|(1 << D1)|(1 << D0),
			.mode = GPIO_MODE_OUTPUT,
			.pull_up_en = GPIO_PULLUP_DISABLE,
			.pull_down_en = GPIO_PULLDOWN_DISABLE,
			.intr_type = GPIO_PIN_INTR_DISABLE
		};
	
		gpio_config(&conf_gpio);
	}
	else
	{
		gpio_pad_select_gpio(RS);
		gpio_pad_select_gpio(E);
		gpio_pad_select_gpio(D7);
		gpio_pad_select_gpio(D6);
		gpio_pad_select_gpio(D5);
		gpio_pad_select_gpio(D4);
		
		const gpio_config_t conf_gpio = {
			.pin_bit_mask = (1 << E)|(1 << RS)|
							(1 << D7)|(1 << D6)|(1 << D5)|(1 << D4),
			.mode = GPIO_MODE_OUTPUT,
			.pull_up_en = GPIO_PULLUP_DISABLE,
			.pull_down_en = GPIO_PULLDOWN_DISABLE,
			.intr_type = GPIO_PIN_INTR_DISABLE
		};
	
		gpio_config(&conf_gpio);
	}
	
	gpio_set_level(RS, GPIO_PIN_RESET);
	gpio_set_level(E, GPIO_PIN_RESET);
}

void app_main(void)
{	
	
}