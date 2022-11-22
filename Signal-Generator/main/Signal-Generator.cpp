#include "main.h"
#include "freertos/queue.h"
#include "esp_log.h"

static const char *TAG = "Signal-Generator";

const std::array<uint16_t, 256> sin {
	32768, 33572, 34375, 35178, 35979, 36779, 37575, 38369, 39160, 39947, 40729, 41507, 42279, 43046, 43806, 44560, 
	45307, 46046, 46777, 47500, 48214, 48918, 49613, 50298, 50972, 51635, 52287, 52927, 53555, 54170, 54772, 55362, 
	55937, 56499, 57046, 57579, 58097, 58599, 59086, 59557, 60012, 60451, 60873, 61278, 61665, 62036, 62389, 62723, 
	63040, 63339, 63619, 63881, 64124, 64348, 64553, 64738, 64905, 65052, 65180, 65288, 65377, 65446, 65495, 65525, 
	65535, 65525, 65495, 65446, 65377, 65288, 65180, 65052, 64905, 64738, 64553, 64348, 64124, 63881, 63619, 63339, 
	63040, 62723, 62389, 62036, 61665, 61278, 60873, 60451, 60012, 59557, 59086, 58599, 58097, 57579, 57046, 56499, 
	55937, 55362, 54772, 54170, 53555, 52927, 52287, 51635, 50972, 50298, 49613, 48918, 48214, 47500, 46777, 46046, 
	45307, 44560, 43806, 43046, 42279, 41507, 40729, 39947, 39160, 38369, 37575, 36779, 35979, 35178, 34375, 33572, 
	32768, 31963, 31160, 30357, 29556, 28756, 27960, 27166, 26375, 25588, 24806, 24028, 23256, 22489, 21729, 20975, 
	20228, 19489, 18758, 18035, 17321, 16617, 15922, 15237, 14563, 13900, 13248, 12608, 11980, 11365, 10763, 10173, 
	9598, 9036, 8489, 7956, 7438, 6936, 6449, 5978, 5523, 5084, 4662, 4257, 3870, 3499, 3146, 2812, 
	2495, 2196, 1916, 1654, 1411, 1187, 982, 797, 630, 483, 355, 247, 158, 89, 40, 10, 
	1, 10, 40, 89, 158, 247, 355, 483, 630, 797, 982, 1187, 1411, 1654, 1916, 2196, 
	2495, 2812, 3146, 3499, 3870, 4257, 4662, 5084, 5523, 5978, 6449, 6936, 7438, 7956, 8489, 9036, 
	9598, 10173, 10763, 11365, 11980, 12608, 13248, 13900, 14563, 15237, 15922, 16617, 17321, 18035, 18758, 19489, 
	20228, 20975, 21729, 22489, 23256, 24028, 24806, 25588, 26375, 27166, 27960, 28756, 29556, 30357, 31160, 31963
};

constexpr  uint8_t SIN = 0;
constexpr  uint8_t SAW = 1;

QueueHandle_t Generator, Display;

const HD44780::HD44780_t HD44780_obj {HD44780::MODE_4_BIT,
								      HD44780::RS_Pin, 
								      HD44780::EN_Pin,
								      HD44780::D7_Pin,
								      HD44780::D6_Pin,
								      HD44780::D5_Pin,
								      HD44780::D4_Pin};

struct Parameters
{
	uint8_t Freq;
	uint8_t Mode;
};

extern "C" {
	void app_main(void);
}

void BtnTask(void *arg)
{
	uint8_t counterFreqBtn = 0;
	uint8_t counterModeBtn = 0;
	struct Parameters GenParams {1, SIN};
	
	while(1)
	{
		if(gpio_get_level((gpio_num_t)FREQBTN) == GPIO_PIN_RESET)
		{
			counterFreqBtn++;
		}
		else
		{
			counterFreqBtn = 0;
		}
		
		if(counterFreqBtn == 4)
		{
			if(GenParams.Freq == 10)
			{
				GenParams.Freq = 1;
			}
			else
			{
				GenParams.Freq += 1;
			}
			
			xQueueSend(Generator, &GenParams, ( TickType_t )0);
			xQueueSend(Display, &GenParams, ( TickType_t )0);
			
			ESP_LOGI(TAG, "%d kHz output frequency", GenParams.Freq);
			
			while(gpio_get_level((gpio_num_t)FREQBTN) == GPIO_PIN_RESET);
			
			counterFreqBtn = 0;
		}
		
		if(gpio_get_level((gpio_num_t)MODEBTN) == GPIO_PIN_RESET)
		{
			counterModeBtn++;
		}
		else
		{
			counterModeBtn = 0;
		}
		
		if(counterModeBtn == 4)
		{	
			if(GenParams.Mode == SAW)
			{
				GenParams.Mode = SIN;
				ESP_LOGI(TAG, "Output signal mode SIN");
			}
			else
			{
				GenParams.Mode = SAW;
				ESP_LOGI(TAG, "Ouput signal mode SAW");
			}
			
			xQueueSend(Generator, &GenParams, ( TickType_t )0);
			xQueueSend(Display, &GenParams, ( TickType_t )0);
			
			while(gpio_get_level((gpio_num_t)MODEBTN) == GPIO_PIN_RESET);
			
			counterModeBtn = 0;
		}
		
		vTaskDelay(10 / portTICK_PERIOD_MS);
	}
}

void DisplayUpdateTask(void *arg)
{
	uint8_t FreqTens, FreqOnes;
	struct Parameters GenParams {1, SIN};
	
	while(1)
	{	
		xQueueReceive(Display, &GenParams, portMAX_DELAY);
		
		HD44780_obj.Goto(6, 1);
		
		if(GenParams.Mode == SIN)
		{
			HD44780_obj.PrintString("Sin");
		}
		else
		{
			HD44780_obj.PrintString("Saw");
		}
		
		FreqTens = GenParams.Freq / 10;
		FreqOnes = GenParams.Freq % 10;
			
		const std::array<int8_t, 5> Buffer {toString(FreqTens), toString(FreqOnes), 'k', 'H', 'z'};
			
		HD44780_obj.Goto(11, 2);
			
		for(const int8_t& item : Buffer)
		{
			HD44780_obj.PrintSymbol(item);
		}
	}
}

void WaveGenTask(void *arg)
{
	size_t bytes_read;
	uint32_t PhaseAcc = 0;
	uint16_t Index = 0;
	uint32_t PhaseStep = 0;
	uint16_t buffer[DMA_BUF_LEN*2];
    struct Parameters GenParams {1, SIN};
	
	while(1)
	{
		xQueueReceive(Generator, &GenParams, ( TickType_t )0);
		
		PhaseStep = (uint32_t)GenParams.Freq * 1000 * 4294967296 / SAMPLE_RATE;
		
		for(uint16_t i = 0; i < DMA_BUF_LEN; i++)
		{
			if(GenParams.Mode == SIN)
			{
				buffer[i*2] = buffer[i*2+1] = sin[Index];
			}
			else if(GenParams.Mode == SAW)
			{
				buffer[i*2] = buffer[i*2+1] = Index << 8;
			}
			
			PhaseAcc += PhaseStep;
			Index = (uint8_t)(PhaseAcc >> 24);
		}
		
		i2s_write(I2S_NUM_0, buffer, sizeof(buffer), &bytes_read, portMAX_DELAY);
	}
}

void app_main(void)
{
	HD44780_obj.Init();
	SystemInit();

	HD44780_obj.CursorHome();
	HD44780_obj.PrintString("Mode:Sin");
	HD44780_obj.Goto(1, 2);
	HD44780_obj.PrintString("Frequency:01kHz");
	
	Generator = xQueueCreate(1, sizeof(Parameters));
	Display = xQueueCreate(1, sizeof(Parameters));
	
	ESP_LOGI(TAG, "Initialization is finished!");
	ESP_LOGI(TAG, "1 kHz output frequency");
	
	xTaskCreate(WaveGenTask, "GeneratorCore", 8192, NULL, configMAX_PRIORITIES - 1, NULL);
	xTaskCreate(BtnTask, "Buttons", 4096, NULL, 1, NULL);
	xTaskCreate(DisplayUpdateTask, "Display", 1024, NULL, 1, NULL);
}