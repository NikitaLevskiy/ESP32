#include "main.h"
#include "esp_log.h"

constexpr int8_t SYMBOL_ADDR = 0x00;
static const char *TAG = "Hello-world";

const HD44780::HD44780_t HD44780_obj {HD44780::MODE_4_BIT,
								      HD44780::RS_Pin, 
								      HD44780::EN_Pin,
								      HD44780::D7_Pin,
								      HD44780::D6_Pin,
								      HD44780::D5_Pin,
								      HD44780::D4_Pin};

extern "C" {
	void app_main(void);
}

void UartRx(void *arg)
{
	char* data = new char [RX_BUF_SIZE+1];
	
	while(1)
	{
		const uint8_t rxBytes = uart_read_bytes(UART, data, RX_BUF_SIZE, 100);
		
		if (rxBytes > 0 && rxBytes <= HD44780::ROW_LENGTH)
		{
            data[rxBytes] = 0;
			
			HD44780_obj.ClearString(2);
			HD44780_obj.PrintString(data);
			
			ESP_LOGI(TAG, "Read %d bytes: '%s'", rxBytes, data);
        }
		
		vTaskDelay(1000 / portTICK_PERIOD_MS);
	}
	
	delete []data;
}

void app_main(void)
{
	UartInit();
	HD44780_obj.Init();
	ESP_LOGI(TAG, "Display initialization finished!");
	
	// Print a string  Hello world!
	HD44780_obj.PrintString("Hello world!");
	ESP_LOGI(TAG, "Print Hello world string!");
	
	// Create your own symbol and print it
	const std::array<int8_t, 8> symbol {0x01, 0x03, 0x07, 0x0F, 0x0F, 0x07, 0x03, 0x01};
	HD44780_obj.CGRAMSymbol(SYMBOL_ADDR, symbol.data());
	HD44780_obj.Goto(13, 1);
	HD44780_obj.PrintSymbol(SYMBOL_ADDR);
	ESP_LOGI(TAG, "Create and print speaker symbol!");
	
	xTaskCreate(UartRx, "UartRx", RX_BUF_SIZE*2, NULL, configMAX_PRIORITIES, NULL);
}