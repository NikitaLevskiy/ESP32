#ifndef MAIN_H
#define MAIN_H

#include <array>
#include <iostream>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "soc/soc.h"
#include "string.h"
#include "stdint.h"
#include "driver/i2s.h"

// Gpio pin value
constexpr bool GPIO_PIN_SET = 1;
constexpr bool GPIO_PIN_RESET = 0;

namespace HD44780
{
	// Display data type
	constexpr bool COMMAND = 0;
	constexpr bool DATA = 1;
	
	// Display pins
	constexpr uint8_t RS_Pin = 23;
	constexpr uint8_t EN_Pin = 22;
	constexpr uint8_t D7_Pin = 5;
	constexpr uint8_t D6_Pin = 18;
	constexpr uint8_t D5_Pin = 19;
	constexpr uint8_t D4_Pin = 21;
	constexpr uint8_t D3_Pin = 0;
	constexpr uint8_t D2_Pin = 0;
	constexpr uint8_t D1_Pin = 0;
	constexpr uint8_t D0_Pin = 0;

	// Display settings
	// Mode: true - 8 bit, false - 4 bit
	// ROW_LENGTH - display row length (16 for current display)
	constexpr bool MODE_8_BIT = 1;
	constexpr bool MODE_4_BIT = 0;
	constexpr uint8_t ROW_LENGTH = 16;
	
	// ClearDisplay
	constexpr int8_t CLEAR = 0x01;
	
	//CursorHome
	constexpr int8_t HOME = 0x02;
	
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
			uint8_t RS, EN, D7, D6, D5, D4,
						    D3, D2, D1, D0;
			gpio_config_t conf_gpio;

			void Enable(void) const
			{
				gpio_set_level((gpio_num_t)EN, GPIO_PIN_SET);
				vTaskDelay(1 / portTICK_PERIOD_MS);

				gpio_set_level((gpio_num_t)EN, GPIO_PIN_RESET);
				vTaskDelay(1 / portTICK_PERIOD_MS);
			}
			
			void SendData(const int8_t& data, const bool& index) const
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
					int8_t temph = data >> 4;
					int8_t templ = data & 0x0F;
					
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
		
			HD44780_t(bool mode_v, uint8_t RS_o, uint8_t EN_o, uint8_t D7_o, uint8_t D6_o, uint8_t D5_o, uint8_t D4_o,
		                                                       uint8_t D3_o, uint8_t D2_o, uint8_t D1_o, uint8_t D0_o)
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
					
				conf_gpio.pin_bit_mask = (uint64_t)((1 << EN)|(1 << RS)|
													(1 << D7)|(1 << D6)|(1 << D5)|(1 << D4)|
													(1 << D3)|(1 << D2)|(1 << D1)|(1 << D0));
				conf_gpio.mode = GPIO_MODE_OUTPUT;
				conf_gpio.pull_up_en = (gpio_pullup_t)GPIO_PULLUP_DISABLE;
				conf_gpio.pull_down_en = (gpio_pulldown_t)GPIO_PULLDOWN_DISABLE;
				conf_gpio.intr_type = (gpio_int_type_t)GPIO_PIN_INTR_DISABLE;
				
				gpio_config(&conf_gpio);
			}
			
			HD44780_t(bool mode_v, uint8_t RS_o, uint8_t EN_o, uint8_t D7_o, uint8_t D6_o, uint8_t D5_o, uint8_t D4_o)
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
					
				conf_gpio.pin_bit_mask = (uint64_t)((1 << EN)|(1 << RS)|
													(1 << D7)|(1 << D6)|(1 << D5)|(1 << D4));
				conf_gpio.mode = GPIO_MODE_OUTPUT;
				conf_gpio.pull_up_en = (gpio_pullup_t)GPIO_PULLUP_DISABLE;
				conf_gpio.pull_down_en = (gpio_pulldown_t)GPIO_PULLDOWN_DISABLE;
				conf_gpio.intr_type = (gpio_int_type_t)GPIO_PIN_INTR_DISABLE;
			
				gpio_config(&conf_gpio);
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
					
				conf_gpio.pin_bit_mask = (uint64_t)((1 << EN)|(1 << RS)|
													(1 << D7)|(1 << D6)|(1 << D5)|(1 << D4)|
													(1 << D3)|(1 << D2)|(1 << D1)|(1 << D0));
				conf_gpio.mode = GPIO_MODE_OUTPUT;
				conf_gpio.pull_up_en = (gpio_pullup_t)GPIO_PULLUP_DISABLE;
				conf_gpio.pull_down_en = (gpio_pulldown_t)GPIO_PULLDOWN_DISABLE;
				conf_gpio.intr_type = (gpio_int_type_t)GPIO_PIN_INTR_DISABLE;
			
				gpio_config(&conf_gpio);
			}
			
			~HD44780_t()
			{
				
			}
			
			void Init(void) const
			{
				if(mode == MODE_8_BIT)
				{
					const std::array<uint8_t, 8> delay {50, 5, 1, 1, 1, 1, 1, 1};
					const std::array<int8_t, 8> initCodes {0x30, 0x30, 0x30, 0x38, 0x08, 0x01, 0x06, 0x0C};
					const uint8_t* delayPtr = delay.data();
					const int8_t* initCodesPtr = initCodes.data();
					
							
					for (uint8_t i = 0; i < initCodes.size(); i++)
					{
						vTaskDelay(delayPtr[i] / portTICK_PERIOD_MS);
						SendData(initCodesPtr[i], COMMAND);
					}	
				}
				else if(mode == MODE_4_BIT)
				{
					const std::array<uint8_t, 9> delay {50, 5, 1, 1, 1, 1, 1, 1, 1};
					const std::array<int8_t, 9> initCodes {0x03, 0x03, 0x03, 0x02, 0x28, 0x08, 0x01, 0x06, 0x0C};
					const uint8_t* delayPtr = delay.data();
					const int8_t* initCodesPtr = initCodes.data();
					
						
					for (uint8_t i = 0; i < initCodes.size(); i++)
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
				}
				
				vTaskDelay(10 / portTICK_PERIOD_MS);
			}
			
			void Clear(void) const
			{
				SendData(CLEAR, COMMAND);
			}
			
			void CursorHome(void) const
			{
				SendData(HOME, COMMAND);
			}
			
			void PrintSymbol(const int8_t& addr) const
			{
				SendData(addr, DATA);
			}
			
			void PrintString(const char* string) const
			{
				const uint8_t len = strlen(string);
				
				for (uint8_t i = 0; i < len; i++)
				{
					PrintSymbol(string[i]);
				}
			}
			
			void ClearString(const uint8_t& string) const
			{
				Goto(1, string);
				
				for (uint8_t i = 0; i < ROW_LENGTH; i++)
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
			
			void Goto(const uint8_t& x, const uint8_t& y) const
			{	
				SendData((1 << 7)|((y-1) << 6)|((x-1) << 0), COMMAND);
			}
			
			void CGRAMSymbol(const int8_t& addr, const int8_t* symbol) const
			{
				SendData((1 << 6)|(addr << 0), COMMAND);
				
				for (uint8_t i = 0; i < 8; i++)
				{
					SendData(symbol[i], DATA);
				}
				
				SendData(0x80, COMMAND);
			}
	};
}

constexpr  uint8_t FREQBTN = 4;
constexpr  uint8_t MODEBTN = 15;
constexpr uint32_t SAMPLE_RATE = 100000;
constexpr uint16_t DMA_BUF_LEN = 1024;
constexpr  uint8_t DMA_NUM_BUF = 2;

void SystemInit(void)
{
	// Gpio init
	gpio_set_direction((gpio_num_t)FREQBTN, GPIO_MODE_INPUT);
	gpio_set_pull_mode((gpio_num_t)FREQBTN, GPIO_PULLUP_ONLY);
	
	gpio_set_direction((gpio_num_t)MODEBTN, GPIO_MODE_INPUT);
	gpio_set_pull_mode((gpio_num_t)MODEBTN, GPIO_PULLUP_ONLY);
	
	// DAC init
	i2s_config_t i2s_config = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX | I2S_MODE_DAC_BUILT_IN),
        .sample_rate = SAMPLE_RATE,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
        .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
        .communication_format = I2S_COMM_FORMAT_STAND_MSB,
		.intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
        .dma_buf_count = DMA_NUM_BUF,
        .dma_buf_len = DMA_BUF_LEN,
        .use_apll = false
        
    };

    i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
    i2s_set_pin(I2S_NUM_0, NULL);
}

int8_t toString(const int8_t& number)
{
	int8_t temp = 0;
	
	switch(number)
	{
		case 0: temp = '0';
		        break;
		
		case 1: temp = '1';
		        break;
				
		case 2: temp = '2';
		        break;
				
		case 3: temp = '3';
		        break;
				
		case 4: temp = '4';
		        break;
				
		case 5: temp = '5';
		        break;
				
		case 6: temp = '6';
		        break;
				
		case 7: temp = '7';
		        break;
				
		case 8: temp = '8';
		        break;
				
		case 9: temp = '9';
		        break;
	}
	
	return temp;
}

#endif