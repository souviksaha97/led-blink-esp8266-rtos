#include <stdio.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <driver/gpio.h>

#include <ssd1306/ssd1306.h>
#include <driver/i2c.h>
#include <esp_err.h>

#define SCL_PIN 5
#define SDA_PIN 4
#define DISPLAY_WIDTH 128
#define DISPLAY_HEIGHT 64

uint8_t buff[DISPLAY_HEIGHT*DISPLAY_WIDTH/8];

void app_main()
{
    // init i2s
    int i2c_master_port = I2C_NUM_0;
    i2c_config_t conf;
    conf.mode = I2C_MODE_MASTER;
    conf.sda_io_num = SDA_PIN;
    conf.sda_pullup_en = 1;
    conf.scl_io_num = SCL_PIN;
    conf.scl_pullup_en = 1;
    conf.clk_stretch_tick = 300;
    ESP_ERROR_CHECK(i2c_driver_install(i2c_master_port, conf.mode));
    ESP_ERROR_CHECK(i2c_param_config(i2c_master_port, &conf));

    // init ssd1306
    ssd1306_t dev = {
        .i2c_port = i2c_master_port,
        .i2c_addr = SSD1306_I2C_ADDR_0,
        .screen = SH1106_SCREEN, // or SH1106_SCREEN
        .width = DISPLAY_WIDTH,
        .height = DISPLAY_HEIGHT};



    printf("%d init\n",ssd1306_init(&dev));
    printf("%d on\n",ssd1306_display_on(&dev,1));
    printf("%d Circle", ssd1306_draw_circle(&dev, buff, 10, 20, 10, OLED_COLOR_WHITE));
    ssd1306_load_frame_buffer(&dev, buff);
    printf("Done\n");
    fflush(stdout);
    while(1);

}