#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/timer.h"
#include "driver/gpio.h"
#include "esp_log.h"

#define TIMER_DIVIDER  80
#define TIMER_INTERVAL 500000
#define LED 2

typedef struct {
    int timer_group;
    int timer_idx;
    int alarm_interval;
    bool auto_reload;
} timer_info_t;

typedef struct {
    timer_info_t info;
    uint64_t timer_counter_value;
} timer_event_t;

static xQueueHandle s_timer_queue;

static const char *TAG = "Timer";

static bool IRAM_ATTR timer_group_isr_callback(void *args)
{
    BaseType_t high_task_awoken = pdFALSE;
    timer_info_t *info = (timer_info_t *) args;

    uint64_t timer_counter_value = timer_group_get_counter_value_in_isr(info->timer_group, info->timer_idx);

    timer_event_t evt = {
        .info.timer_group = info->timer_group,
        .info.timer_idx = info->timer_idx,
        .info.auto_reload = info->auto_reload,
        .info.alarm_interval = info->alarm_interval,
        .timer_counter_value = timer_counter_value
    };

    xQueueSendFromISR(s_timer_queue, &evt, &high_task_awoken);

    return high_task_awoken == pdTRUE;
}

static void tg_timer_init(int group, int timer, int timer_interval)
{
    timer_config_t config = {
        .divider = TIMER_DIVIDER,
        .counter_dir = TIMER_COUNT_UP,
        .counter_en = TIMER_PAUSE,
        .alarm_en = TIMER_ALARM_EN,
        .auto_reload = true,
    };
	
    timer_init(group, timer, &config);
    timer_set_counter_value(group, timer, 0);
    timer_set_alarm_value(group, timer, timer_interval);
    timer_enable_intr(group, timer);

    timer_info_t *timer_info = calloc(1, sizeof(timer_info_t));
    timer_info->timer_group = group;
    timer_info->timer_idx = timer;
    timer_info->auto_reload = true;
    timer_info->alarm_interval = timer_interval;
    timer_isr_callback_add(group, timer, timer_group_isr_callback, timer_info, 0);

    timer_start(group, timer);
}

void vTask(void *pvParameters) {
	
	static int togglePin = 0;
	
	while(1) {
		
		timer_event_t evt;
		xQueueReceive(s_timer_queue, &evt, portMAX_DELAY);
		
		togglePin ^= 1;
		gpio_set_level(LED, togglePin);
		
		ESP_LOGI(TAG, "GPIO_TOGGLE_PIN");
		
	}
	
}

void app_main(void)
{
	s_timer_queue = xQueueCreate(10, sizeof(timer_event_t));
	tg_timer_init(TIMER_GROUP_0, TIMER_0, TIMER_INTERVAL);
	
	gpio_reset_pin(LED);
	gpio_set_direction(LED, GPIO_MODE_OUTPUT);
	
	xTaskCreate(vTask, "Timer", 2048, NULL, 1, NULL);
}
