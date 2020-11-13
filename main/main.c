#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"

#define LED_1 GPIO_Pin_16
#define LED_2 GPIO_Pin_2

void vLED_1_Task(void *pvParameters);
void vLED_2_Task(void *pvParameters);
void GPIO_Init(void);

void app_main(void)
{
    GPIO_Init();

    xTaskCreate(
    	vLED_1_Task,
    	"Led 1",
    	configMINIMAL_STACK_SIZE+10,
    	NULL,
    	1,
    	NULL);

    xTaskCreate(
    	vLED_2_Task,
    	"Led 2",
    	configMINIMAL_STACK_SIZE+10,
    	NULL,
    	1,
    	NULL
    	);
}

void vLED_1_Task(void *pvParameters)
{
	int level = 0;
	while(1){
		gpio_set_level(GPIO_NUM_16, level);
		level = !level;
	vTaskDelay(500 / portTICK_PERIOD_MS);
	}
}


void vLED_2_Task(void *pvParameters)
{
	int level = 0;
	while(1){
		gpio_set_level(GPIO_NUM_2, level);
		level = !level;
	vTaskDelay(700 / portTICK_PERIOD_MS);
	}
}

void GPIO_Init(void)
{
	gpio_config_t led_config;

    led_config.intr_type = GPIO_INTR_DISABLE;
    led_config.mode = GPIO_MODE_OUTPUT;
    led_config.pin_bit_mask = LED_1|LED_2;
    led_config.pull_down_en = 0;
    led_config.pull_up_en = 0;
    gpio_config(&led_config);
}