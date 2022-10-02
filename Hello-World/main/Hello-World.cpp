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
#define EN_Pin 12
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

#define Speaker 0x00

#define TXD_PIN (GPIO_NUM_17)
#define RXD_PIN (GPIO_NUM_16)
#define RX_BUF_SIZE 1024
#define UART UART_NUM_2

extern "C" {
	void app_main(void);
}

class HD44780_t
{
	private:
	
		int mode;
		int RS, EN, D7, D6, D5, D4,
		            D3, D2, D1, D0;
		gpio_config_t conf_gpio;
		
		void Enable(void)
		{
			gpio_set_level((gpio_num_t)EN, GPIO_PIN_SET);
			vTaskDelay(1 / portTICK_PERIOD_MS);
			
			gpio_set_level((gpio_num_t)EN, GPIO_PIN_RESET);
			vTaskDelay(1 / portTICK_PERIOD_MS);
		}
		
		void SendData(const char& data, const int& index)
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
	
		HD44780_t(int mode_v, int RS_o, int EN_o, int D7_o, int D6_o, int D5_o, int D4_o,
		                                          int D3_o, int D2_o, int D1_o, int D0_o)
		{
			if (mode_v)
			{
				mode = mode_v;
			
				RS = RS_o;
				EN = EN_o;
				D7 = D7_o;
				D6 = D6_o;
				D5 = D5_o;
				D4 = D4_o;
				D3 = D3_o;
				D2 = D2_o;
				D1 = D1_o;
				D0 = D0_o;
					
				gpio_pad_select_gpio(RS);
				gpio_pad_select_gpio(EN);
				gpio_pad_select_gpio(D7);
				gpio_pad_select_gpio(D6);
				gpio_pad_select_gpio(D5);
				gpio_pad_select_gpio(D4);
				gpio_pad_select_gpio(D3);
				gpio_pad_select_gpio(D2);
				gpio_pad_select_gpio(D1);
				gpio_pad_select_gpio(D0);
				
				conf_gpio.pin_bit_mask = (long long unsigned int)((1 << EN)|(1 << RS)|
																  (1 << D7)|(1 << D6)|(1 << D5)|(1 << D4)|
																  (1 << D3)|(1 << D2)|(1 << D1)|(1 << D0));
				conf_gpio.mode = GPIO_MODE_OUTPUT;
				conf_gpio.pull_up_en = (gpio_pullup_t)GPIO_PULLUP_DISABLE;
				conf_gpio.pull_down_en = (gpio_pulldown_t)GPIO_PULLDOWN_DISABLE;
				conf_gpio.intr_type = (gpio_int_type_t)GPIO_PIN_INTR_DISABLE;
			
				gpio_config(&conf_gpio);
					
				const int delay[] = {50, 5, 1, 1, 1, 1, 1, 1};
				const unsigned char array[] = {0x30, 0x30, 0x30, 0x38, 0x08, 0x01, 0x06, 0x0C};
					
				for (int i = 0; i < sizeof(array); i++)
				{
					vTaskDelay(delay[i] / portTICK_PERIOD_MS);
					SendData(array[i], COMMAND);
				}	
					
				vTaskDelay(10 / portTICK_PERIOD_MS);
			}
		}
		
		HD44780_t(int mode_v, int RS_o, int EN_o, int D7_o, int D6_o, int D5_o, int D4_o)
		{
			if (!mode_v)
			{
				mode = mode_v;
			
				RS = RS_o;
				EN = EN_o;
				D7 = D7_o;
				D6 = D6_o;
				D5 = D5_o;
				D4 = D4_o;
				
				gpio_pad_select_gpio(RS);
				gpio_pad_select_gpio(EN);
				gpio_pad_select_gpio(D7);
				gpio_pad_select_gpio(D6);
				gpio_pad_select_gpio(D5);
				gpio_pad_select_gpio(D4);
				
				conf_gpio.pin_bit_mask = (long long unsigned int)((1 << EN)|(1 << RS)|
								                                  (1 << D7)|(1 << D6)|(1 << D5)|(1 << D4));
				conf_gpio.mode = GPIO_MODE_OUTPUT;
				conf_gpio.pull_up_en = (gpio_pullup_t)GPIO_PULLUP_DISABLE;
				conf_gpio.pull_down_en = (gpio_pulldown_t)GPIO_PULLDOWN_DISABLE;
				conf_gpio.intr_type = (gpio_int_type_t)GPIO_PIN_INTR_DISABLE;
		
				gpio_config(&conf_gpio);
				
				const int delay[] = {50, 5, 1, 1, 1, 1, 1, 1, 1};
				const unsigned char array[] = {0x03, 0x03, 0x03, 0x02, 0x28, 0x08, 0x01, 0x06, 0x0C};
				
				for (int i = 0; i < sizeof(array); i++)
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

		HD44780_t()
		{
			mode = Mode8bit;
			RS = 0;
			EN = 0;
			D7 = 0;
			D6 = 0;
			D5 = 0;
			D4 = 0;
			D3 = 0;
			D2 = 0;
			D1 = 0;
			D0 = 0;
			
			conf_gpio.pin_bit_mask = (long long unsigned int)((1 << EN)|(1 << RS)|
								                              (1 << D7)|(1 << D6)|(1 << D5)|(1 << D4)|
								                              (1 << D3)|(1 << D2)|(1 << D1)|(1 << D0));
			conf_gpio.mode = GPIO_MODE_OUTPUT;
			conf_gpio.pull_up_en = (gpio_pullup_t)GPIO_PULLUP_DISABLE;
			conf_gpio.pull_down_en = (gpio_pulldown_t)GPIO_PULLDOWN_DISABLE;
			conf_gpio.intr_type = (gpio_int_type_t)GPIO_PIN_INTR_DISABLE;
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
		
		void CGRAMSymbol(const int& addr, const char* symbol)
		{
			SendData((1 << 6)|(addr << 0), COMMAND);
			
			for (int i = 0; i < 8; i++)
			{
				SendData(symbol[i], DATA);
			}
			
			SendData(0x80, COMMAND);
		}
		
		void PrintSymbol(const int& addr)
		{
			SendData(addr, DATA);
		}
		
		void PrintString(const char* string)
		{
			const int len = strlen(string);
			
			for (int i = 0; i < len; i++)
			{
				PrintSymbol(string[i]);
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
			const char symbol[] = {0x01, 0x03, 0x07, 0x0F, 0x0F, 0x07, 0x03, 0x01};
			CGRAMSymbol(Speaker, symbol);
		}

		void HelloWorld(void)
		{
			PrintString("Hello world!");
		}	
};

class Uart_t
{
	private:
		
		int uart;
		int TX, RX, BufSize;
		uart_config_t uart_config;
		
		
	public:
	
		Uart_t(int uart_v, int TX_io, int RX_io, int BufSize_v)
		{
			uart = uart_v;
			TX = TX_io;
			RX = RX_io;
			BufSize = BufSize_v;
			
			uart_config.baud_rate = 115200;
			uart_config.data_bits = UART_DATA_8_BITS;
			uart_config.parity = UART_PARITY_DISABLE;
			uart_config.stop_bits = UART_STOP_BITS_1;
			uart_config.flow_ctrl = UART_HW_FLOWCTRL_DISABLE;
			uart_config.source_clk = UART_SCLK_APB;
			
			uart_driver_install(uart, BufSize * 2, 0, 0, NULL, 0);
			uart_param_config(uart, &uart_config);
			uart_set_pin(uart, TX, RX, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
		}
		
		int PrintString(const char* string)
		{
			const int len = strlen(string);
			const int txBytes = uart_write_bytes(UART, string, len);
			return txBytes;
		}
};


QueueHandle_t ClassParam;

void UartRx(void *arg)
{
	char* data = (char*) malloc(RX_BUF_SIZE+1);
	
	HD44780_t display_obj;
	xQueueReceive(ClassParam, &display_obj, portMAX_DELAY);
	
	while(1)
	{
		const int rxBytes = uart_read_bytes(UART, data, RX_BUF_SIZE, 1000 / portTICK_RATE_MS);
		
		if (rxBytes > 0 && rxBytes <= DisplayStringLength)
		{
            data[rxBytes] = 0;
			
			display_obj.ClearString(2);
			display_obj.PrintString(data);
			display_obj.Goto(1, 2);
        }
		
		vTaskDelay(1000 / portTICK_PERIOD_MS);
	}
	
	free(data);
}

void app_main(void)
{
	Uart_t Uart_obj(UART, TXD_PIN, RXD_PIN, RX_BUF_SIZE);
	
	HD44780_t HD44780_obj(Mode4bit, RS_Pin, EN_Pin, D7_Pin, D6_Pin, D5_Pin, D4_Pin);
	Uart_obj.PrintString("HD44780 Initialization is done!\n");
	
	HD44780_obj.SpeakerSymbol();
	Uart_obj.PrintString("Creating own symbol!\n");
	
	HD44780_obj.HelloWorld();
	Uart_obj.PrintString("Print Hello world string!\n");
	
	HD44780_obj.PrintSymbol(Speaker);
	Uart_obj.PrintString("Print speaker symbol!\n");
	
    ClassParam = xQueueCreate(1, sizeof(HD44780_obj));
    xQueueSend(ClassParam, &HD44780_obj, 1);
	
	Uart_obj.PrintString("Now you can print your own string!\n");
	
	xTaskCreate(UartRx, "UartRx", RX_BUF_SIZE*2, NULL, configMAX_PRIORITIES, NULL);
}