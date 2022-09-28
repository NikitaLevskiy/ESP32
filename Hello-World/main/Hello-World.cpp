#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "driver/uart.h"
#include "esp_system.h"
#include "soc/soc.h"
#include "string.h"

#define COMMAND  0
#define DATA     1

#define GPIO_PIN_SET   1
#define GPIO_PIN_RESET 0

#define RS_Pin 13
#define E_Pin  12
#define D7_Pin 23
#define D6_Pin 22
#define D5_Pin 21
#define D4_Pin 19
#define D3_Pin 18
#define D2_Pin  5
#define D1_Pin  4
#define D0_Pin  2

#define Mode8bit 1
#define Mode4bit 0

#define Clear          0x01
#define Home           0x02
#define DisplayOn         1
#define DisplayOff        0
#define CursorOn       0x02
#define CursorOff      0x00
#define BlinkingCursor 0x01

#define DisplayStringLength 16

#define SymbolAddr 0x00

#define TXD_PIN (GPIO_NUM_17)
#define RXD_PIN (GPIO_NUM_16)
#define RX_BUF_SIZE 1024
#define UART UART_NUM_2

extern "C" {
	void app_main(void);
}

class HD44780
{
	private:
	
		int mode;
		int RS, E, D7, D6, D5, D4,
		           D3, D2, D1, D0;
		
		void Enable(void)
		{
			gpio_set_level((gpio_num_t)E, GPIO_PIN_SET);
			vTaskDelay(1 / portTICK_PERIOD_MS);
			
			gpio_set_level((gpio_num_t)E, GPIO_PIN_RESET);
			vTaskDelay(1 / portTICK_PERIOD_MS);
		}
		
		void SendData(const unsigned char& data, const int& index)
		{	
			if (mode)
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
			
			vTaskDelay(2 / portTICK_PERIOD_MS);
		}
	
	
	public:
	
		HD44780(int mode_t, int RS_t, int E_t, int D7_t, int D6_t, int D5_t, int D4_t,
		                                       int D3_t, int D2_t, int D1_t, int D0_t)
		{
			mode = mode_t;
			
			RS = RS_t;
			E = E_t;
			D7 = D7_t;
			D6 = D6_t;
			D5 = D5_t;
			D4 = D4_t;
			D3 = D3_t;
			D2 = D2_t;
			D1 = D1_t;
			D0 = D0_t;
				
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
				.pin_bit_mask = (long long unsigned int)((1 << E)|(1 << RS)|
								                         (1 << D7)|(1 << D6)|(1 << D5)|(1 << D4)|
								                         (1 << D3)|(1 << D2)|(1 << D1)|(1 << D0)),
				.mode = GPIO_MODE_OUTPUT,
				.pull_up_en = (gpio_pullup_t)GPIO_PULLUP_DISABLE,
				.pull_down_en = (gpio_pulldown_t)GPIO_PULLDOWN_DISABLE,
				.intr_type = (gpio_int_type_t)GPIO_PIN_INTR_DISABLE
			};
		
			gpio_config(&conf_gpio);
				
			const int delay[] = {50, 5, 1, 1, 1, 1, 1, 1};
			const unsigned char array[] = {0x30, 0x30, 0x30, 0x38, 0x08, 0x01, 0x06, 0x0C};
				
			for (int i = 0; i < 8; i++)
			{
				vTaskDelay(delay[i] / portTICK_PERIOD_MS);
				SendData(array[i], COMMAND);
			}	
				
			vTaskDelay(10 / portTICK_PERIOD_MS);
		}
		
		HD44780(int mode_t, int RS_t, int E_t, int D7_t, int D6_t, int D5_t, int D4_t)
		{
			if (!mode_t)
			{
				mode = mode_t;
			
				RS = RS_t;
				E = E_t;
				D7 = D7_t;
				D6 = D6_t;
				D5 = D5_t;
				D4 = D4_t;
				
				gpio_pad_select_gpio(RS);
				gpio_pad_select_gpio(E);
				gpio_pad_select_gpio(D7);
				gpio_pad_select_gpio(D6);
				gpio_pad_select_gpio(D5);
				gpio_pad_select_gpio(D4);
				
				const gpio_config_t conf_gpio = {	
					.pin_bit_mask = (long long unsigned int)((1 << E)|(1 << RS)|
									                         (1 << D7)|(1 << D6)|(1 << D5)|(1 << D4)),
					.mode = GPIO_MODE_OUTPUT,
					.pull_up_en = (gpio_pullup_t)GPIO_PULLUP_DISABLE,
					.pull_down_en = (gpio_pulldown_t)GPIO_PULLDOWN_DISABLE,
					.intr_type = (gpio_int_type_t)GPIO_PIN_INTR_DISABLE
				};
		
				gpio_config(&conf_gpio);
				
				const int delay[] = {50, 5, 1, 1, 1, 1, 1, 1, 1};
				const unsigned char array[] = {0x03, 0x03, 0x03, 0x02, 0x28, 0x08, 0x01, 0x06, 0x0C};
				
				for (int i = 0; i < 9; i++)
				{
					vTaskDelay(delay[i] / portTICK_PERIOD_MS);
						
					if (i < 4)
					{
						REG_WRITE(GPIO_OUT_REG, (COMMAND << RS)|
												(((array[i] & 0x8) >> 3) << D7)|(((array[i] & 0x4) >> 2) << D6)|
												(((array[i] & 0x2) >> 1) << D5)|(((array[i] & 0x1) >> 0) << D4));
						Enable();
					}
					else
					{
						SendData(array[i], COMMAND);
					}
				}
				
				vTaskDelay(10 / portTICK_PERIOD_MS);
			}
		}	

		HD44780()
		{
			mode = Mode8bit;
			RS = 0;
			E = 0;
			D7 = 0;
			D6 = 0;
			D5 = 0;
			D4 = 0;
			D3 = 0;
			D2 = 0;
			D1 = 0;
			D0 = 0;
		}

		void ClearDisplay(void)
		{
			SendData(Clear, COMMAND);
		}

		void CursorHome(void)
		{
			SendData(Home, COMMAND);
		}
		
		void ViewMode(const int& display, const int& cursor)
		{
			SendData((1 << 3)|(display << 2)|(cursor << 0), COMMAND);
		}
		
		void Goto(const int& x, const int& y)
		{	
			SendData((1 << 7)|((y-1) << 6)|((x-1) << 0), COMMAND);
		}
		
		void CGRAMSymbol(const int& addr, const unsigned char *array)
		{
			SendData((1 << 6)|(addr << 0), COMMAND);
			
			for (int i = 0; i < 8; i++)
			{
				SendData(*array++, DATA);
			}
			
			SendData(0x80, COMMAND);
		}
		
		void PrintSymbol(const int& addr)
		{
			SendData(addr, DATA);
		}
		
		void PrintString(const unsigned char* array)
		{
			while(*array)
			{
				SendData(*array++, DATA);
			}
		}
		
		void ClearString(const int& string)
		{	
			Goto(1, string);
		
			for (int i = 0; i < DisplayStringLength; i++)
			{
				PrintSymbol(' ');
			}
					
			Goto(1, string);
		}
		
		void SpeakerSymbol(void)
		{
			const unsigned char symbol[] = {0x01, 0x03, 0x07, 0x0F, 0x0F, 0x07, 0x03, 0x01};
			CGRAMSymbol(SymbolAddr, symbol);
		}

		void HelloWorld(void)
		{
			const unsigned char array[] = {'H','e','l','l','o',' ','w','o','r','l','d','!'};
			
			for (int i = 0; i < 12; i++)
			{
				SendData(array[i], DATA);
			}
		}	
};

void UARTInit(const int uart)
{
	uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_APB
    };
	
	uart_driver_install(uart, RX_BUF_SIZE * 2, 0, 0, NULL, 0);
    uart_param_config(uart, &uart_config);
    uart_set_pin(uart, TXD_PIN, RXD_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
}

int UartPrintString(const char* data)
{
	const int len = strlen(data);
    const int txBytes = uart_write_bytes(UART, data, len);
	return txBytes;
}

QueueHandle_t ClassParam;

void UartRx(void *arg)
{
	unsigned char* data = (unsigned char*) malloc(RX_BUF_SIZE+1);
	
	HD44780 display;
	xQueueReceive(ClassParam, &display, portMAX_DELAY);
	
	while(1)
	{
		const int rxBytes = uart_read_bytes(UART, data, RX_BUF_SIZE, 1000 / portTICK_RATE_MS);
		
		if (rxBytes > 0 && rxBytes <= DisplayStringLength)
		{
            data[rxBytes] = 0;
			
			display.ClearString(2);
			display.PrintString(data);
			display.Goto(1, 2);
        }
		
		vTaskDelay(1000 / portTICK_PERIOD_MS);
	}
	
	free(data);
}

void app_main(void)
{
	UARTInit(UART);
	
	HD44780 HD44780_t(Mode4bit, RS_Pin, E_Pin, D7_Pin, D6_Pin, D5_Pin, D4_Pin);
	UartPrintString("HD44780 Initialization is done!\n");
	
	HD44780_t.SpeakerSymbol();
	UartPrintString("Creating own symbol!\n");
	
	HD44780_t.HelloWorld();
	UartPrintString("Print Hello world string!\n");
	
	HD44780_t.PrintSymbol(SymbolAddr);
	UartPrintString("Print speaker symbol!\n");
	
    ClassParam = xQueueCreate(1, sizeof(HD44780_t));
    xQueueSend(ClassParam, &HD44780_t, 1);
	
	UartPrintString("Now you can print your own string!\n");
	
	xTaskCreate(UartRx, "UartRx", RX_BUF_SIZE*2, NULL, configMAX_PRIORITIES, NULL);
}