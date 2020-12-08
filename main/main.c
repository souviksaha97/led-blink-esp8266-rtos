#include <stdio.h>


#include <driver/gpio.h>

#include <ssd1306/ssd1306.h>

#include <fonts/fonts.h>
#include <driver/i2c.h>
#include <esp_err.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_event.h"
#include "protocol_examples_common.h"
#include "nvs.h"
#include "nvs_flash.h"

#include <netdb.h>
#include <sys/socket.h>

#include <cJSON.h>

/* Constants that aren't configurable in menuconfig */
#define WEB_SERVER "worldtimeapi.org"
#define WEB_PORT 80
#define WEB_URL "http://worldtimeapi.org/api/ip"

static const char *TAG = "example";

static const char *REQUEST = "GET " WEB_URL " HTTP/1.1\r\n"
    "Host: "WEB_SERVER"\r\n"
    "User-Agent: esp-idf/1.0 esp32\r\n"
    "\r\n";


#define SCL_PIN 5
#define SDA_PIN 4
#define DISPLAY_WIDTH 128
#define DISPLAY_HEIGHT 64

uint8_t buff[DISPLAY_HEIGHT*DISPLAY_WIDTH/8];




static void http_get_task(void *pvParameters)
{
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
        .screen = SSD1306_SCREEN, // or SH1106_SCREEN
        .width = DISPLAY_WIDTH,
        .height = DISPLAY_HEIGHT};



    ssd1306_init(&dev);
    ssd1306_display_on(&dev, true);

    

    printf("Done\n");
    fflush(stdout);



    const struct addrinfo hints = {
        .ai_family = AF_INET,
        .ai_socktype = SOCK_STREAM,
    };
    struct addrinfo *res;
    struct in_addr *addr;
    int s, r;
    char recv_buf[2048];

    while(1) {
        int err = getaddrinfo(WEB_SERVER, "80", &hints, &res);

        if(err != 0 || res == NULL) {
            ESP_LOGE(TAG, "DNS lookup failed err=%d res=%p", err, res);
            vTaskDelay(1000 / portTICK_PERIOD_MS);
            continue;
        }

        /* Code to print the resolved IP.

           Note: inet_ntoa is non-reentrant, look at ipaddr_ntoa_r for "real" code */
        addr = &((struct sockaddr_in *)res->ai_addr)->sin_addr;
        ESP_LOGI(TAG, "DNS lookup succeeded. IP=%s", inet_ntoa(*addr));

        s = socket(res->ai_family, res->ai_socktype, 0);
        if(s < 0) {
            ESP_LOGE(TAG, "... Failed to allocate socket.");
            freeaddrinfo(res);
            vTaskDelay(1000 / portTICK_PERIOD_MS);
            continue;
        }
        ESP_LOGI(TAG, "... allocated socket");

        if(connect(s, res->ai_addr, res->ai_addrlen) != 0) {
            ESP_LOGE(TAG, "... socket connect failed errno=%d", errno);
            close(s);
            freeaddrinfo(res);
            vTaskDelay(4000 / portTICK_PERIOD_MS);
            continue;
        }

        ESP_LOGI(TAG, "... connected");
        freeaddrinfo(res);

        if (write(s, REQUEST, strlen(REQUEST)) < 0) {
            ESP_LOGE(TAG, "... socket send failed");
            close(s);
            vTaskDelay(4000 / portTICK_PERIOD_MS);
            continue;
        }
        ESP_LOGI(TAG, "... socket send success");

        struct timeval receiving_timeout;
        receiving_timeout.tv_sec = 5;
        receiving_timeout.tv_usec = 0;
        if (setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, &receiving_timeout,
                sizeof(receiving_timeout)) < 0) {
            ESP_LOGE(TAG, "... failed to set socket receiving timeout");
            close(s);
            vTaskDelay(4000 / portTICK_PERIOD_MS);
            continue;
        }
        ESP_LOGI(TAG, "... set socket receiving timeout success");

        /* Read HTTP response */
        do {
            bzero(recv_buf, sizeof(recv_buf));
            r = read(s, recv_buf, sizeof(recv_buf)-1);
            // printf("%s\n", recv_buf);
            int length = sizeof(recv_buf)/sizeof(recv_buf[0])-603;
            char token[length];
            // char datetime[30];

            // int total_length = sizeof(recv_buf)/sizeof(recv_buf[0])-603;

            for (int i = 0; i < length; i++)
            {
                token[i] = recv_buf[603+i];
            }
            // token = strtok(recv_buf, "\r\n\r\n");
            // token = strtok(0, "\r\n\r\n");
            printf("%s\n", token);
            cJSON *root = cJSON_Parse(token);
            const cJSON *json_Time = NULL;
            json_Time = cJSON_GetObjectItemCaseSensitive(root, "datetime");
            if (cJSON_IsString(json_Time) && (json_Time->valuestring != NULL))
            {                                
            
            char *datetime = cJSON_GetObjectItemCaseSensitive(root, "datetime")->valuestring;

            char *date = strtok(datetime, "T");

            char *time = strtok(strtok(NULL, "T"), ".");
            // char *time = strtok(NULL, "T");

            printf("%s\n%s", date, time);

            ssd1306_draw_string(&dev, buff, font_builtin_fonts[FONT_FACE_GLCD5x7], 5, 0, time,
            OLED_COLOR_WHITE, OLED_COLOR_BLACK);
            ssd1306_draw_string(&dev, buff, font_builtin_fonts[FONT_FACE_GLCD5x7], 5, 30, date,
            OLED_COLOR_WHITE, OLED_COLOR_BLACK);
            ssd1306_load_frame_buffer(&dev, buff);
            ssd1306_display_on(&dev, true);
            ssd1306_start_scroll_hori(&dev, true, 0, 1, FRAME_5);
            // ssd1306_start_scroll_hori(&dev, true, 1, 2, FRAME_5);
            }
            cJSON_Delete(root);
            

        } while(r > 0);

        ESP_LOGI(TAG, "... done reading from socket. Last read return=%d errno=%d\r\n", r, errno);
        close(s);
        for(int countdown = 10; countdown >= 0; countdown--) {
            ESP_LOGI(TAG, "%d... ", countdown);
            vTaskDelay(1000 / portTICK_PERIOD_MS);
        }
        ESP_LOGI(TAG, "Starting again!");
        ssd1306_display_on(&dev, false);
    }
}

void app_main()
{
    // init i2s
    // int i2c_master_port = I2C_NUM_0;
    // i2c_config_t conf;
    // conf.mode = I2C_MODE_MASTER;
    // conf.sda_io_num = SDA_PIN;
    // conf.sda_pullup_en = 1;
    // conf.scl_io_num = SCL_PIN;
    // conf.scl_pullup_en = 1;
    // conf.clk_stretch_tick = 300;
    // ESP_ERROR_CHECK(i2c_driver_install(i2c_master_port, conf.mode));
    // ESP_ERROR_CHECK(i2c_param_config(i2c_master_port, &conf));

    // // init ssd1306
    // ssd1306_t dev = {
    //     .i2c_port = i2c_master_port,
    //     .i2c_addr = SSD1306_I2C_ADDR_0,
    //     .screen = SSD1306_SCREEN, // or SH1106_SCREEN
    //     .width = DISPLAY_WIDTH,
    //     .height = DISPLAY_HEIGHT};



    // printf("%d init\n",ssd1306_init(&dev));
    // printf("%d on\n",ssd1306_display_on(&dev,1));
    // ssd1306_draw_string(&dev, buff, font_builtin_fonts[FONT_FACE_GLCD5x7], 5, 0, "Hello World!",
    // OLED_COLOR_WHITE, OLED_COLOR_BLACK);
    // ssd1306_load_frame_buffer(&dev, buff);
    // // ssd1306_start_scroll_hori(&dev, false, 0, 1, FRAME_5);

    // printf("Done\n");
    // fflush(stdout);
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());


    ESP_ERROR_CHECK(example_connect());
    xTaskCreate(&http_get_task, "http_get_task", 16384, NULL, 5, NULL);

}