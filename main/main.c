#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"

#define LED_1 GPIO_Pin_16
#define LED_2 GPIO_Pin_2

#define BUTTON GPIO_Pin_5

void vLED_2_Task(void *pvParameters);
void vButton_Task(void *pvParameters);

void GPIO_Init(void);

void app_main(void)
{
    GPIO_Init();

    xTaskCreate(
    	vLED_2_Task,
    	"Led 2",
    	configMINIMAL_STACK_SIZE+10,
    	NULL,
    	1,
    	NULL
    	);

    xTaskCreate(
    	vButton_Task,
    	"Button",
    	configMINIMAL_STACK_SIZE+10,
    	NULL,
    	1,
    	NULL
    	);
}

void vLED_1_Task(void *pvParameters)
{
	int level = 0;
	while(1)
	{
		gpio_set_level(GPIO_NUM_16, level);
		level = !level;
		vTaskDelay(2000 / portTICK_PERIOD_MS);
	}
}


void vLED_2_Task(void *pvParameters)
{
	int level = 0;
	while(1)
	{
		gpio_set_level(GPIO_NUM_2, level);
		level = !level;
		vTaskDelay(20 / portTICK_PERIOD_MS);
	}
}

void vButton_Task(void *pvParameters)
{
	while(1)
	{
		gpio_set_level(GPIO_NUM_16, gpio_get_level(GPIO_NUM_5));
		vTaskDelay(100 / portTICK_PERIOD_MS);
	}
}

void GPIO_Init(void)
{
	gpio_config_t io_config;

    io_config.intr_type = GPIO_INTR_DISABLE;
    io_config.mode = GPIO_MODE_OUTPUT;
    io_config.pin_bit_mask = ((1ULL << 16)|(1ULL << 2));
    io_config.pull_down_en = 0;
    io_config.pull_up_en = 0;
    gpio_config(&io_config);


    io_config.intr_type = GPIO_INTR_DISABLE;
    io_config.mode = GPIO_MODE_INPUT;
    io_config.pin_bit_mask = ((1ULL << 5));
    io_config.pull_up_en = 1;
    io_config.pull_down_en = 0;
    gpio_config(&io_config);

}