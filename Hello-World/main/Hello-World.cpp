#include "main.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include <array>
#include <iostream>

constexpr char SYMBOL_ADDR = 0x00;
QueueHandle_t Object;

extern "C" {
	void app_main(void);
}

void UartRx(void *arg)
{
	char* data = new char [RX_BUF_SIZE+1];
	
	HD44780::HD44780_t Display_obj;
	xQueueReceive(Object, &Display_obj, portMAX_DELAY);
	
	while(1)
	{
		const int rxBytes = uart_read_bytes(UART, data, RX_BUF_SIZE, 100);
		
		if (rxBytes > 0 && rxBytes <= HD44780::ROW_LENGTH)
		{
            data[rxBytes] = 0;
			
			Display_obj.ClearString(2);
			Display_obj.PrintString(data);
			
			std::cout << "Hello world: " << rxBytes << " bytes received" << std::endl;
			std::cout << "Hello world: ";
			
			for(int i = 0; i < rxBytes; i++)
			{
				std::cout << data[i];
			}
			
			std::cout << " string printed" << std::endl;
        }
		
		vTaskDelay(1000 / portTICK_PERIOD_MS);
	}
	
	delete []data;
}

void app_main(void)
{
	UartInit();
	HD44780::HD44780_t HD44780_obj {HD44780::MODE_4_BIT,
								    HD44780::RS_Pin, 
								    HD44780::EN_Pin,
								    HD44780::D7_Pin,
								    HD44780::D6_Pin,
								    HD44780::D5_Pin,
								    HD44780::D4_Pin};
								   
	std::cout << "Hello world: Display initialization finished" << std::endl;
	
	// Print a string  Hello world!
	HD44780_obj.PrintString("Hello world!");
	std::cout << "Hello world: Print string" << std::endl;
	
	// Create your own symbol and print it
	const std::array<char, 8> symbol {0x01, 0x03, 0x07, 0x0F, 0x0F, 0x07, 0x03, 0x01};
	HD44780_obj.CGRAMSymbol(SYMBOL_ADDR, symbol.data());
	HD44780_obj.Goto(13, 1);
	HD44780_obj.PrintSymbol(SYMBOL_ADDR);
	std::cout << "Hello world: Create and print speaker symbol" << std::endl;
	
	Object = xQueueCreate(1, sizeof(HD44780_obj));
    xQueueSend(Object, &HD44780_obj, 1);
	
	xTaskCreate(UartRx, "UartRx", RX_BUF_SIZE*2, NULL, configMAX_PRIORITIES, NULL);
}