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

#define COMMAND  0
#define DATA     1

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

void Enable(void)
{
	gpio_set_level(E, GPIO_PIN_SET);
	vTaskDelay(1 / portTICK_PERIOD_MS);
	
	gpio_set_level(E, GPIO_PIN_RESET);
	vTaskDelay(1 / portTICK_PERIOD_MS);
}

void SendData(const unsigned char data, int index, int mode)
{
	if (mode == Mode8bit)
	{
		REG_WRITE(GPIO_OUT_REG, (index << RS)|
								(((data & 0x80) >> 7) << D7)|(((data & 0x40) >> 6) << D6)|
		                        (((data & 0x20) >> 5) << D5)|(((data & 0x10) >> 4) << D4)|
		                        (((data & 0x08) >> 3) << D3)|(((data & 0x04) >> 2) << D2)|
								(((data & 0x02) >> 1) << D1)|(((data & 0x01) >> 0) << D0));
		Enable();
	}
	else
	{
		unsigned char temph = data >> 4;
		unsigned char templ = data & 0x0F;
		
		REG_WRITE(GPIO_OUT_REG, (index << RS)|
								(((temph & 0x8) >> 3) << D7)|(((temph & 0x4) >> 2) << D6)|
		                        (((temph & 0x2) >> 1) << D5)|(((temph & 0x1) >> 0) << D4));
		Enable();
		
		REG_WRITE(GPIO_OUT_REG, (index << RS)|
								(((templ & 0x8) >> 3) << D7)|(((templ & 0x4) >> 2) << D6)|
		                        (((templ & 0x2) >> 1) << D5)|(((templ & 0x1) >> 0) << D4));
		Enable();
	}
	
	vTaskDelay(10 / portTICK_PERIOD_MS);
}

void DisplayMode(int mode)
{
	int delay[] = {50, 5, 1, 1, 1, 1, 1, 1, 1};
	unsigned char array8bit[] = {0x30, 0x30, 0x30, 0x38, 0x08, 0x01, 0x06, 0x0C};
	unsigned char array4bit[] = {0x03, 0x03, 0x03, 0x02, 0x28, 0x08, 0x01, 0x06, 0x0C};
	
	if (mode == Mode8bit)
	{
		for (int i = 0; i < 8; i++)
		{
			vTaskDelay(delay[i] / portTICK_PERIOD_MS);
			SendData(array8bit[i], COMMAND, mode);
		}
	}
	else
	{
		for (int i = 0; i < 9; i++)
		{
			vTaskDelay(delay[i] / portTICK_PERIOD_MS);
			
			if (i < 4)
			{
				REG_WRITE(GPIO_OUT_REG, (COMMAND << RS)|
										(((array4bit[i] & 0x8) >> 3) << D7)|(((array4bit[i] & 0x4) >> 2) << D6)|
										(((array4bit[i] & 0x2) >> 1) << D5)|(((array4bit[i] & 0x1) >> 0) << D4));
				Enable();
			}
			else
			{
				SendData(array4bit[i], COMMAND, mode);
			}
		}
	}
	
	vTaskDelay(10 / portTICK_PERIOD_MS);
}

void app_main(void)
{	
	
}