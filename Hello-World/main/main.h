#ifndef MAIN_H
#define MAIN_H

#include <array>
#include <iostream>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/uart.h"
#include "soc/soc.h"
#include "string.h"

namespace HD44780
{
	// Gpio pin value
	constexpr bool GPIO_PIN_SET = 1;
	constexpr bool GPIO_PIN_RESET = 0;
	
	// Display data type
	constexpr bool COMMAND = 0;
	constexpr bool DATA = 1;
	
	// Display pins
	constexpr int RS_Pin = 13;
	constexpr int EN_Pin = 12;
	constexpr int D7_Pin = 23;
	constexpr int D6_Pin = 22;
	constexpr int D5_Pin = 21;
	constexpr int D4_Pin = 19;
	constexpr int D3_Pin = 18;
	constexpr int D2_Pin = 5;
	constexpr int D1_Pin = 4;
	constexpr int D0_Pin = 2;

	// Display settings
	// Mode: true - 8 bit, false - 4 bit
	// ROW_LENGTH - display row length (16 for current display)
	constexpr bool MODE_8_BIT = 1;
	constexpr bool MODE_4_BIT = 0;
	constexpr int ROW_LENGTH = 16;
	
	// ClearDisplay
	constexpr char CLEAR = 0x01;
	
	//CursorHome
	constexpr char HOME = 0x02;
	
	// EntryModeSet
	constexpr bool INC = 1;
	constexpr bool DEC = 0;
	constexpr bool DISPLAY_SHIFT_ON = 1;
	constexpr bool DISPLAY_SHIFT_OFF = 0;
	
	// PowerMode
	constexpr bool DISPLAY_ON = 1;
	constexpr bool DISPLAY_OFF = 0;
	constexpr bool CURSOR_ON = 1;
	constexpr bool CURSOR_OFF = 0;
	constexpr bool BLINKING_CURSOR_ON = 1;
	constexpr bool BLINKING_CURSOR_OFF = 0;
	
	// ShiftMode
	constexpr bool LEFT_SHIFT = 0;
	constexpr bool RIGHT_SHIFT = 1;
	constexpr bool CURSOR_SHIFT = 0;
	constexpr bool DISPLAY_SHIFT = 1;
	
	// FunctionSet
	constexpr bool DATA_LENGTH_8 = 1;
	constexpr bool DATA_LENGTH_4 = 0;
	constexpr bool NUMBER_OF_LINES_1 = 0;
	constexpr bool NUMBER_OF_LINES_2 = 1;
	constexpr bool FONT_5x8 = 0;
	constexpr bool FONT_5x10 = 1;
	
	class HD44780_t
	{
		private:
	
			bool mode;
			int RS, EN, D7, D6, D5, D4,
						D3, D2, D1, D0;
			gpio_config_t conf_gpio;
			
			void Enable(void) const
			{
				gpio_set_level((gpio_num_t)EN, GPIO_PIN_SET);
				vTaskDelay(1 / portTICK_PERIOD_MS);
				
				gpio_set_level((gpio_num_t)EN, GPIO_PIN_RESET);
				vTaskDelay(1 / portTICK_PERIOD_MS);
			}
			
			void SendData(const char& data, const bool& index) const
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
					char temph = data >> 4;
					char templ = data & 0x0F;
					
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
		
			HD44780_t(bool mode_v, int RS_o, int EN_o, int D7_o, int D6_o, int D5_o, int D4_o,
		                                               int D3_o, int D2_o, int D1_o, int D0_o)
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
				
				const std::array<int, 8> delay {50, 5, 1, 1, 1, 1, 1, 1};
				const std::array<char, 8> initCodes {0x30, 0x30, 0x30, 0x38, 0x08, 0x01, 0x06, 0x0C};
				const int* delayPtr = delay.data();
				const char* initCodesPtr = initCodes.data();
				
						
				for (int i = 0; i < initCodes.size(); i++)
				{
					vTaskDelay(delayPtr[i] / portTICK_PERIOD_MS);
					SendData(initCodesPtr[i], COMMAND);
				}	
						
				vTaskDelay(10 / portTICK_PERIOD_MS);
			}
			
			HD44780_t(bool mode_v, int RS_o, int EN_o, int D7_o, int D6_o, int D5_o, int D4_o)
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
				
				const std::array<int, 9> delay {50, 5, 1, 1, 1, 1, 1, 1, 1};
				const std::array<char, 9> initCodes {0x03, 0x03, 0x03, 0x02, 0x28, 0x08, 0x01, 0x06, 0x0C};
				const int* delayPtr = delay.data();
				const char* initCodesPtr = initCodes.data();
				
					
				for (int i = 0; i < initCodes.size(); i++)
				{
					vTaskDelay(delayPtr[i] / portTICK_PERIOD_MS);
				
					if (i < 4)
					{
						REG_WRITE(GPIO_OUT_REG, (COMMAND << RS)|
												(((initCodesPtr[i] & 0x8) >> 3) << D7)|(((initCodesPtr[i] & 0x4) >> 2) << D6)|
												(((initCodesPtr[i] & 0x2) >> 1) << D5)|(((initCodesPtr[i] & 0x1) >> 0) << D4));
						Enable();
					}
					else
					{
						SendData(initCodesPtr[i], COMMAND);
					}
				}
					
					vTaskDelay(10 / portTICK_PERIOD_MS);
			}

			HD44780_t()
			{
				mode = MODE_8_BIT;
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
																	  (1 << D7)|(1 << D6)|(1 << D5)|(1 << D4));
				conf_gpio.mode = GPIO_MODE_OUTPUT;
				conf_gpio.pull_up_en = (gpio_pullup_t)GPIO_PULLUP_DISABLE;
				conf_gpio.pull_down_en = (gpio_pulldown_t)GPIO_PULLDOWN_DISABLE;
				conf_gpio.intr_type = (gpio_int_type_t)GPIO_PIN_INTR_DISABLE;
			
				gpio_config(&conf_gpio);
			}
			
			~HD44780_t()
			{
				
			}
			
			void Clear(void) const
			{
				SendData(CLEAR, COMMAND);
			}
			
			void CursorHome(void) const
			{
				SendData(HOME, COMMAND);
			}
			
			void PrintSymbol(const char& addr) const
			{
				SendData(addr, DATA);
			}
			
			void PrintString(const char* string) const
			{
				const int len = strlen(string);
				
				for (int i = 0; i < len; i++)
				{
					PrintSymbol(string[i]);
				}
			}
			
			void ClearString(const int& string) const
			{
				Goto(1, string);
				
				for(int i = 0; i < ROW_LENGTH; i++)
				{
					PrintSymbol(' ');
				}
				
				Goto(1, string);
			}
			
			void EntryModeSet(const bool& addr, const bool& shift) const
			{
				SendData((1 << 2)|(addr << 1)|(shift << 0), COMMAND);
			}
			
			void PowerMode(const bool& display, const bool& cursor, const bool& blinkOfCursor) const
			{
				SendData((1 << 3)|(display << 2)|(cursor << 1)|(blinkOfCursor << 0), COMMAND);
			}
			
			void ShiftMode(const bool& item, const bool& direction) const
			{
				SendData((1 << 4)|(item << 3)|(direction << 2), COMMAND);
			}
			
			void FunctionSet(const bool& dataLength, const bool& numberOfLines, const bool& font) const
			{
				SendData((1 << 5)|(dataLength << 4)|(numberOfLines << 3)|(font << 2), COMMAND);
			}
			
			void Goto(const int& x, const int& y) const
			{	
				SendData((1 << 7)|((y-1) << 6)|((x-1) << 0), COMMAND);
			}
			
			void CGRAMSymbol(const char& addr, const char* symbol) const
			{
				SendData((1 << 6)|(addr << 0), COMMAND);
				
				for (int i = 0; i < 8; i++)
				{
					SendData(symbol[i], DATA);
				}
				
				SendData(0x80, COMMAND);
			}
	};
}

const int TX_Pin = 17;
const int RX_Pin = 16;
const int RX_BUF_SIZE = 1024;
const int UART = UART_NUM_2;

void UartInit(void)
{
	uart_config_t uart_config;
	
	uart_config.baud_rate = 115200;
	uart_config.data_bits = UART_DATA_8_BITS;
	uart_config.parity = UART_PARITY_DISABLE;
	uart_config.stop_bits = UART_STOP_BITS_1;
	uart_config.flow_ctrl = UART_HW_FLOWCTRL_DISABLE;
	uart_config.source_clk = UART_SCLK_APB;
	uart_config.rx_flow_ctrl_thresh = 100;
			
	uart_driver_install(UART, RX_BUF_SIZE * 2, 0, 0, NULL, 0);
	uart_param_config(UART, &uart_config);
	uart_set_pin(UART, TX_Pin, RX_Pin, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
}

#endif