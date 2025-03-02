#include <stdio.h>
#include "oled_display.h"
#include "onewire_dht11.h"

#include <string.h>
#include <stdio.h>
#include <inttypes.h>
#include <sys/param.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "esp_err.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_panel_ops.h"
#include "esp_http_client.h"
#include "esp_tls.h"

#include "nvs_flash.h"

#include "lwip/err.h"
#include "lwip/sys.h"

#include "driver/i2c.h"

#include "hal/lcd_types.h"

#include "lvgl.h"
#include "esp_lvgl_port.h"

#include "cJSON.h"
#include "ping/ping_sock.h"
#include "driver/ledc.h"
#include "esp_system.h" 


///////ping.c+ping.h
#include <stdio.h>
#include <string.h>
#include "sdkconfig.h"
#include "lwip/inet.h"
#include "lwip/netdb.h"
#include "lwip/sockets.h"
#include "esp_console.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "argtable3/argtable3.h"
#include "ping/ping_sock.h"
#include "esp_http_client.h"
#include "esp_http_server.h"

/* The examples use WiFiidf.py -p /dev/ttyUSB0 flash configuration that you can xxxxxxxxxxxxxset via project configuration menu

   If you'd rather not, just change the below entries to strings with
   the config you want - ie #define EXAMPLE_WIFI_SSID "mywifissid"
*/
// #define EXAMPLE_ESP_WIFI_SSID      "DukeVisitor"
// #define EXAMPLE_ESP_WIFI_PASS      ""
// #define EXAMPLE_ESP_WIFI_SSID      "OneEleven"
 #define EXAMPLE_ESP_WIFI_SSID      "SpectrumSetup-B2"
 #define EXAMPLE_ESP_WIFI_PASS      "quickflower066"
#define EXAMPLE_ESP_MAXIMUM_RETRY  5

#if CONFIG_ESP_WPA3_SAE_PWE_HUNT_AND_PECK
#define ESP_WIFI_SAE_MODE WPA3_SAE_PWE_HUNT_AND_PECK
#define EXAMPLE_H2E_IDENTIFIER ""
#elif CONFIG_ESP_WPA3_SAE_PWE_HASH_TO_ELEMENT
#define ESP_WIFI_SAE_MODE WPA3_SAE_PWE_HASH_TO_ELEMENT
#define EXAMPLE_H2E_IDENTIFIER CONFIG_ESP_WIFI_PW_ID
#elif CONFIG_ESP_WPA3_SAE_PWE_BOTH
#define ESP_WIFI_SAE_MODE WPA3_SAE_PWE_BOTH
#define EXAMPLE_H2E_IDENTIFIER CONFIG_ESP_WIFI_PW_ID
#endif
#if CONFIG_ESP_WIFI_AUTH_OPEN
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_OPEN
#elif CONFIG_ESP_WIFI_AUTH_WEP
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WEP
#elif CONFIG_ESP_WIFI_AUTH_WPA_PSK
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA_PSK
#elif CONFIG_ESP_WIFI_AUTH_WPA2_PSK
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA2_PSK
#elif CONFIG_ESP_WIFI_AUTH_WPA_WPA2_PSK
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA_WPA2_PSK
#elif CONFIG_ESP_WIFI_AUTH_WPA3_PSK
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA3_PSK
#elif CONFIG_ESP_WIFI_AUTH_WPA2_WPA3_PSK
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA2_WPA3_PSK
#elif CONFIG_ESP_WIFI_AUTH_WAPI_PSK 
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WAPI_PSK
#endif


static EventGroupHandle_t s_wifi_event_group;

/* The event group allows multiple bits for each event, but we only care about two events:
 * - we are connected to the AP with an IP
 * - we failed to connect after the maximum amount of retries */
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1


#define STEP_PIN_1         (2) // Use GPIO 2 as LED output
#define STEP_PIN_2         (3) // Use GPIO 3 as LED output
#define STEP_PIN_3         (4) // Use GPIO 4 as LED output
#define STEP_PIN_4         (5) // Use GPIO 5 as LED output
#define STEP_PIN_5         (47) // Use GPIO 2 as LED output
#define STEP_PIN_6         (48) // Use GPIO 3 as LED output
#define STEP_PIN_7         (26) // Use GPIO 4 as LED output
#define STEP_PIN_8         (21) // Use GPIO 5 as LED output






//changable parameters 
#define hum_min 54.0f // Minimum humidity percentage
#define hum_max 70.0f // Maximum humidity percentage
#define STEPS_MAX 12000      // Number of steps from fully closed to fully open
#define ALL_THE_WAY 8000

//password led
//#define Pass_led                (35)

//ledc

#define LEDC_HS_TIMER          LEDC_TIMER_0
#define LEDC_HS_MODE           LEDC_LOW_SPEED_MODE
#define LEDC_HS_CH0_GPIO       (40) // Change this to your GPIO pin connected to the LED
#define LEDC_HS_CH0_CHANNEL    LEDC_CHANNEL_0
#define LEDC_TEST_DUTY         (4000) // Starting duty cycle
#define LEDC_TEST_FADE_TIME    (3000) // Duration for fade

static int s_retry_num = 0;

static const char *TAG = "Main";

static char str[50];
static bool sample;
static data_t data;
static lv_obj_t * screen = NULL;
volatile int step_number = 0; // Define step_number outside to persist its value
volatile bool flag_motor = false;
static volatile int led = 0;
volatile bool unlock = false;
volatile bool lock = false;
volatile uint16_t mypassword;
volatile int height = 0;

 volatile float HUMIDITY_MAX = 70.0f; // Minimum humidity percentage
 volatile float HUMIDITY_MIN = 54.0f; // Maximum humidity percentage

volatile int currentPosition = 0; // Current position of the window (0 = fully open)


void ledc_init() {
    ledc_timer_config_t ledc_timer = {
        .duty_resolution = LEDC_TIMER_13_BIT, // Set duty resolution
        .freq_hz = 5000,                     // Frequency in Hz
        .speed_mode = LEDC_HS_MODE,          // Timer mode
        .timer_num = LEDC_HS_TIMER           // Timer index
    };
    // Set configuration of timer0 for high speed channels
    ledc_timer_config(&ledc_timer);

    ledc_channel_config_t ledc_channel = {
        .channel    = LEDC_HS_CH0_CHANNEL,
        .duty       = 0,
        .gpio_num   = LEDC_HS_CH0_GPIO,
        .speed_mode = LEDC_HS_MODE,
        .hpoint     = 0,
        .timer_sel  = LEDC_HS_TIMER
    };
    // Set LED Controller with previously prepared configuration
    ledc_channel_config(&ledc_channel);

    // Initialize fade service.
    ledc_fade_func_install(0);
}


void set_led_brightness_based_on_humidity(float humidity) {
 if (humidity < 26) humidity = 26;
    if (humidity > 59) humidity = 59;

    // Map 30-70% humidity to 0-8191 PWM duty cycle
    uint32_t duty = (uint32_t)(((59 - humidity) / 40.0) * 8191);

    // Set the duty cycle for the LEDC channel
    ledc_set_duty(LEDC_HS_MODE, LEDC_HS_CH0_CHANNEL, duty);
    ledc_update_duty(LEDC_HS_MODE, LEDC_HS_CH0_CHANNEL);
}




static void diode_toggle(void* arg) { 
    sample = true;

}




/** copy form gpt **/

esp_err_t _http_event_handler(esp_http_client_event_t *evt) {
    switch (evt->event_id) {
        case HTTP_EVENT_ERROR:
            ESP_LOGD(TAG, "HTTP_EVENT_ERROR");
            break;
        case HTTP_EVENT_ON_CONNECTED:
            ESP_LOGD(TAG, "HTTP_EVENT_ON_CONNECTED");
            break;
        case HTTP_EVENT_HEADER_SENT:
            ESP_LOGD(TAG, "HTTP_EVENT_HEADER_SENT");
            break;
        case HTTP_EVENT_ON_HEADER:
            ESP_LOGD(TAG, "HTTP_EVENT_ON_HEADER, key=%s, value=%s", evt->header_key, evt->header_value);
            break;
        case HTTP_EVENT_ON_DATA:
            ESP_LOGD(TAG, "HTTP_EVENT_ON_DATA, len=%d", evt->data_len);
            if (!esp_http_client_is_chunked_response(evt->client)) {
                // 打印响应数据的一部分
                ESP_LOGD(TAG, "%.*s", evt->data_len, (char*)evt->data);
            }
            break;
        case HTTP_EVENT_ON_FINISH:
            ESP_LOGD(TAG, "HTTP_EVENT_ON_FINISH");
            break;
        case HTTP_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "HTTP_EVENT_DISCONNECTED");
            int mbedtls_err = 0;
            esp_err_t err = esp_tls_get_and_clear_last_error((esp_tls_error_handle_t)evt->data, &mbedtls_err, NULL);
            if (err != 0) {
                ESP_LOGI(TAG, "Last esp error code: 0x%x", err);
                ESP_LOGI(TAG, "Last mbedtls failure: 0x%x", mbedtls_err);
            }
            break;
        default:
            break;
    }
    return ESP_OK;
}


void http_post_with_json(float temp, float hum) {
    // 初始化HTTP客户端配置
    esp_http_client_config_t config = {
        //.url = "http://127.0.0.1:5000", // 请替换<your_flask_server_ip>为您的Flask服务器IP
        //.url = "http://3.137.174.199:5000/sensor_data",
        .url = "http://3.137.174.199/sensor_data",
        .event_handler = _http_event_handler,
    };
    esp_http_client_handle_t client = esp_http_client_init(&config);

    // 创建JSON对象
    cJSON *root = cJSON_CreateObject();
    cJSON_AddNumberToObject(root, "temp", temp); // 示例温度值
    cJSON_AddNumberToObject(root, "humd", hum); // 示例湿度值
    char *post_data = cJSON_Print(root);

     ESP_LOGE(TAG, "1111");

    // 设置HTTP头
    esp_http_client_set_method(client, HTTP_METHOD_POST);
    ESP_LOGE(TAG, "22222");
    esp_http_client_set_header(client, "Content-Type", "application/json");
    esp_http_client_set_post_field(client, post_data, strlen(post_data));

    ESP_LOGE(TAG, "333");
    // 执行HTTP请求
    esp_err_t err = esp_http_client_perform(client);
    if (err == ESP_OK) {
        ESP_LOGI(TAG, "HTTP POST Request Status = %d, content_length = %"PRId64,
                 esp_http_client_get_status_code(client),
                 esp_http_client_get_content_length(client));
    } else {
        ESP_LOGE(TAG, "HTTP POST Request failed: %s", esp_err_to_name(err));
    }

    // 清理
    esp_http_client_cleanup(client);
    free(post_data);
    cJSON_Delete(root);
}

static void event_handler(void* arg, esp_event_base_t event_base,
                                int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        if (s_retry_num < EXAMPLE_ESP_MAXIMUM_RETRY) {
            esp_wifi_connect();
            s_retry_num++;
            ESP_LOGI(TAG, "retry to connect to the AP");
        } else {
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
        }
        ESP_LOGI(TAG,"connect to the AP fail");
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_num = 0;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
}



void wifi_init_sta(void)
{
    s_wifi_event_group = xEventGroupCreate();

    ESP_ERROR_CHECK(esp_netif_init());

    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &event_handler,
                                                        NULL,
                                                        &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &event_handler,
                                                        NULL,
                                                        &instance_got_ip));

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = EXAMPLE_ESP_WIFI_SSID,
            .password = EXAMPLE_ESP_WIFI_PASS,
            /* Authmode threshold resets to WPA2 as default if password matches WPA2 standards (pasword len => 8).
             * If you want to connect the device to deprecated WEP/WPA networks, Please set the threshold value
             * to WIFI_AUTH_WEP/WIFI_AUTH_WPA_PSK and set the password with length and format matching to
             * WIFI_AUTH_WEP/WIFI_AUTH_WPA_PSK standards.
             */
            .threshold.authmode = WIFI_AUTH_WEP/WIFI_AUTH_WPA_PSK,

        },
    };
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config) );
    ESP_ERROR_CHECK(esp_wifi_start() );

    ESP_LOGI(TAG, "wifi_init_sta finished.");

    /* Waiting until either the connection is established (WIFI_CONNECTED_BIT) or connection failed for the maximum
     * number of re-tries (WIFI_FAIL_BIT). The bits are set by event_handler() (see above) */
    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
            WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
            pdFALSE,
            pdFALSE,
            portMAX_DELAY);

    /* xEventGroupWaitBits() returns the bits before the call returned, hence we can test which event actually
     * happened. */
    if (bits & WIFI_CONNECTED_BIT) {
        ESP_LOGI(TAG, "connected to ap SSID:%s password:%s",
                 EXAMPLE_ESP_WIFI_SSID, EXAMPLE_ESP_WIFI_PASS);
    } else if (bits & WIFI_FAIL_BIT) {
        ESP_LOGI(TAG, "Failed to connect to SSID:%s, password:%s",
                 EXAMPLE_ESP_WIFI_SSID, EXAMPLE_ESP_WIFI_PASS);
    } else {
        ESP_LOGE(TAG, "UNEXPECTED EVENT");
    }
}

//setup pin_number
void motor(void) {
    gpio_config_t io_conf; // Declare once

    // Zero out the io_conf struct
    memset(&io_conf, 0, sizeof(gpio_config_t));

    // Configure STEP_PIN_1
    io_conf.pin_bit_mask = (1ULL << STEP_PIN_1);
    io_conf.mode = GPIO_MODE_OUTPUT;
    gpio_config(&io_conf);

    // Configure STEP_PIN_2
    io_conf.pin_bit_mask = (1ULL << STEP_PIN_2);
    // No need to set mode again if it's the same
    gpio_config(&io_conf);

    // Configure STEP_PIN_3
    io_conf.pin_bit_mask = (1ULL << STEP_PIN_3);
    gpio_config(&io_conf);

    // Configure STEP_PIN_4
    io_conf.pin_bit_mask = (1ULL << STEP_PIN_4);
    gpio_config(&io_conf);


    // Configure STEP_PIN_5
    io_conf.pin_bit_mask = (1ULL << STEP_PIN_5);
    // No need to set mode again if it's the same
    gpio_config(&io_conf);

    // Configure STEP_PIN_6
    io_conf.pin_bit_mask = (1ULL << STEP_PIN_6);
    gpio_config(&io_conf);

    // Configure STEP_PIN_7
    io_conf.pin_bit_mask = (1ULL << STEP_PIN_7);
    gpio_config(&io_conf);
    
    // Configure STEP_PIN_8
    io_conf.pin_bit_mask = (1ULL << STEP_PIN_8);
    gpio_config(&io_conf);

}


void OneStep(bool dir) {
    if (dir) {
        switch (step_number) {
            case 0:
                gpio_set_level(STEP_PIN_1, 1);
                gpio_set_level(STEP_PIN_2, 0);
                gpio_set_level(STEP_PIN_3, 0);
                gpio_set_level(STEP_PIN_4, 0);

                //left            
                gpio_set_level(STEP_PIN_5, 1);
                gpio_set_level(STEP_PIN_6, 0);
                gpio_set_level(STEP_PIN_7, 0);
                gpio_set_level(STEP_PIN_8, 0);
                //ESP_LOGI(TAG, "111");
                break;
            case 1:
                gpio_set_level(STEP_PIN_1, 0);
                gpio_set_level(STEP_PIN_2, 1);
                gpio_set_level(STEP_PIN_3, 0);
                gpio_set_level(STEP_PIN_4, 0);

                gpio_set_level(STEP_PIN_5, 0);
                gpio_set_level(STEP_PIN_6, 1);
                gpio_set_level(STEP_PIN_7, 0);
                gpio_set_level(STEP_PIN_8, 0);                
                //ESP_LOGI(TAG, "222");
                break;
            case 2:
                gpio_set_level(STEP_PIN_1, 0);
                gpio_set_level(STEP_PIN_2, 0);
                gpio_set_level(STEP_PIN_3, 1);
                gpio_set_level(STEP_PIN_4, 0);

                gpio_set_level(STEP_PIN_5, 0);
                gpio_set_level(STEP_PIN_6, 0);
                gpio_set_level(STEP_PIN_7, 1);
                gpio_set_level(STEP_PIN_8, 0);  
                //ESP_LOGI(TAG, "333");
                break;
            case 3:
                gpio_set_level(STEP_PIN_1, 0);
                gpio_set_level(STEP_PIN_2, 0);
                gpio_set_level(STEP_PIN_3, 0);
                gpio_set_level(STEP_PIN_4, 1);

                gpio_set_level(STEP_PIN_5, 0);
                gpio_set_level(STEP_PIN_6, 0);
                gpio_set_level(STEP_PIN_7, 0);
                gpio_set_level(STEP_PIN_8, 1);  
                //ESP_LOGI(TAG, "444");
                break;
        }
    } else {
        switch (step_number) {
            case 0:
                gpio_set_level(STEP_PIN_1, 0);
                gpio_set_level(STEP_PIN_2, 0);
                gpio_set_level(STEP_PIN_3, 0);
                gpio_set_level(STEP_PIN_4, 1);

                gpio_set_level(STEP_PIN_5, 0);
                gpio_set_level(STEP_PIN_6, 0);
                gpio_set_level(STEP_PIN_7, 0);
                gpio_set_level(STEP_PIN_8, 1);  

                break;
            case 1:
                gpio_set_level(STEP_PIN_1, 0);
                gpio_set_level(STEP_PIN_2, 0);
                gpio_set_level(STEP_PIN_3, 1);
                gpio_set_level(STEP_PIN_4, 0);

                gpio_set_level(STEP_PIN_5, 0);
                gpio_set_level(STEP_PIN_6, 0);
                gpio_set_level(STEP_PIN_7, 1);
                gpio_set_level(STEP_PIN_8, 0);  
                break;
            case 2:
                gpio_set_level(STEP_PIN_1, 0);
                gpio_set_level(STEP_PIN_2, 1);
                gpio_set_level(STEP_PIN_3, 0);
                gpio_set_level(STEP_PIN_4, 0);

                gpio_set_level(STEP_PIN_5, 0);
                gpio_set_level(STEP_PIN_6, 1);
                gpio_set_level(STEP_PIN_7, 0);
                gpio_set_level(STEP_PIN_8, 0);   
                break;
            case 3:
                gpio_set_level(STEP_PIN_1, 1);
                gpio_set_level(STEP_PIN_2, 0);
                gpio_set_level(STEP_PIN_3, 0);
                gpio_set_level(STEP_PIN_4, 0);

                gpio_set_level(STEP_PIN_5, 1);
                gpio_set_level(STEP_PIN_6, 0);
                gpio_set_level(STEP_PIN_7, 0);
                gpio_set_level(STEP_PIN_8, 0);   
                break;
        }
    }
    step_number++;
    if (step_number > 3) {
        step_number = 0;
    }
}

void adjustWindowToHumidity(float humidity) {
    int targetPosition;
    int stepsToMove;

    // Clamp the humidity value to our operational range
    if (humidity < HUMIDITY_MIN) humidity = HUMIDITY_MIN;
    if (humidity > HUMIDITY_MAX) humidity = HUMIDITY_MAX;
    ESP_LOGI(TAG, "max hum = %f, min_hum = %f", HUMIDITY_MAX, HUMIDITY_MIN );
    // Map the humidity value to the target position in steps
    targetPosition = (((humidity - HUMIDITY_MIN) / (HUMIDITY_MAX - HUMIDITY_MIN)) * STEPS_MAX);
    ESP_LOGI(TAG, "humidity - HUMIDITY_MIN = %f, HUMIDITY_MAX - HUMIDITY_MIN = %f, (humidity - HUMIDITY_MIN) / (HUMIDITY_MAX - HUMIDITY_MIN) = %f", humidity - HUMIDITY_MIN, HUMIDITY_MAX - HUMIDITY_MIN, (humidity - HUMIDITY_MIN) / (HUMIDITY_MAX - HUMIDITY_MIN));
    // Determine how many steps to move and in which direction
    stepsToMove = targetPosition - currentPosition;
    ESP_LOGI(TAG, "humidity = %.1f targetPosition = %d, stepsToMove = %d", humidity, targetPosition, stepsToMove);
    // Move the motor the necessary number of steps

    if (stepsToMove > 0) {
        // Move up
        int stepsMoved = 0; // Initialize steps moved counter
        while (stepsMoved < stepsToMove) {
            OneStep(false); // True moves the window down
            currentPosition++;
            stepsMoved++;
            ESP_LOGI(TAG, "currentPosition = %d, stepsMoved = %d", currentPosition, stepsMoved);
        }
    } else if (stepsToMove < 0) {
        // Move down
        stepsToMove = -stepsToMove; // Make positive
        int stepsMoved = 0; // Initialize steps moved counter
        while (stepsMoved < stepsToMove) {
            OneStep(true); // Assuming false moves the window down
            currentPosition--;
            stepsMoved++;
            ESP_LOGI(TAG, "currentPosition = %d, stepsMoved = %d", currentPosition, stepsMoved);
  
        }
    }

    // No else case needed - if stepsToMove is 0, the window is already in the correct position
}



//Copy from CP1

esp_err_t on_get_handler(httpd_req_t *req) {
    led = 1;
    //gpio_set_level(LED_PIN, 1); // Turn on LED
    httpd_resp_send(req, "The system is unlocked", HTTPD_RESP_USE_STRLEN);
    ESP_LOGI(TAG, "LED should be on");
    return ESP_OK;
}

esp_err_t off_get_handler(httpd_req_t *req) {
    led = 0;
    //gpio_set_level(LED_PIN, 0); // Turn off LED
    httpd_resp_send(req, "LED is OFF2222", HTTPD_RESP_USE_STRLEN);
    ESP_LOGI(TAG, "LED should be off");
    return ESP_OK;
}

esp_err_t led_status_get_handler(httpd_req_t *req) {
    char * resp;
    if (led == 0) {
        httpd_resp_send(req, "OFF", HTTPD_RESP_USE_STRLEN);
        resp = "LED is OFF";
        ESP_LOGI(TAG, "LED should be on3333");
    } else {
        httpd_resp_send(req, "ON_ON", HTTPD_RESP_USE_STRLEN);
        resp = "LED is ON";
        ESP_LOGI(TAG, "LED should be off4444");
    }
    return ESP_OK;
}

esp_err_t temp_status_get_handler(httpd_req_t *req) {
    ESP_LOGI(TAG, "Sending temperature data");

    // Simulating a temperature value
    float temp = data.temp; // Example temperature
    // Allocate a buffer for the response
    char tempStr[20];
    sprintf(tempStr, " %.1f °C", temp);

    // Send the temperature string as response
    httpd_resp_send(req, tempStr, HTTPD_RESP_USE_STRLEN);

    return ESP_OK;
}

/* Handler for humidity status */
esp_err_t humid_status_get_handler(httpd_req_t *req) {

    // Simulating a temperature value
    float hum = data.humi; // Example temperature
    // Allocate a buffer for the response
    char humStr[20];
    sprintf(humStr, " %.1f ", hum);

    // Send the temperature string as response
    httpd_resp_send(req, humStr, HTTPD_RESP_USE_STRLEN);

    return ESP_OK;
}


/* Handler for humidity status */
esp_err_t password_get_handler(httpd_req_t *req) {

    int password_int = mypassword;
    char password_str[12];  // Increased size to handle up to INT_MAX and null terminator
    snprintf(password_str, sizeof(password_str), "%d", password_int);
    // Send the generated password as response
    httpd_resp_send(req, password_str, HTTPD_RESP_USE_STRLEN);

    ESP_LOGI(TAG, "Sending the password: %s", password_str);
    return ESP_OK;

}

esp_err_t unlock_get_handler(httpd_req_t *req) {

    unlock = true;
    ESP_LOGI(TAG, "The door is unlocked__");
    return ESP_OK;

}

esp_err_t lock_get_handler(httpd_req_t *req) {

    lock = true;
    ESP_LOGI(TAG, "The door is locked__");
    return ESP_OK;

}


esp_err_t max_get_handler(httpd_req_t *req) {
    char*  buf;
    size_t buf_len;
    float value;

    // Get the length of the incoming request URL query string
    buf_len = httpd_req_get_url_query_len(req) + 1;
    if (buf_len > 1) {
        buf = malloc(buf_len);
        if (httpd_req_get_url_query_str(req, buf, buf_len) == ESP_OK) {
            char param[32];  // Buffer for the parameter value

            // Get the value of the parameter "max" from the query string
            if (httpd_query_key_value(buf, "max", param, sizeof(param)) == ESP_OK) {
                value = atof(param);
                HUMIDITY_MAX = value;  // Set the maximum humidity
                ESP_LOGI(TAG, "THe Maximum hum = %f", value);
                httpd_resp_send(req, "Maximum humidity updated", HTTPD_RESP_USE_STRLEN);
            } else {
                httpd_resp_send(req, "Parameter 'max' not found", HTTPD_RESP_USE_STRLEN);
            }
        }
        free(buf);
    } else {
        httpd_resp_send(req, "Query not found", HTTPD_RESP_USE_STRLEN);
    }

    return ESP_OK;
}



esp_err_t min_get_handler(httpd_req_t *req) {
    char*  buf;
    size_t buf_len;
    float value;
    ESP_LOGI(TAG, "I go into min handler");
    
    // Get the length of the incoming request URL query string
    buf_len = httpd_req_get_url_query_len(req) + 1;
    if (buf_len > 1) {
        buf = malloc(buf_len);
        if (httpd_req_get_url_query_str(req, buf, buf_len) == ESP_OK) {
            char param[32];  // Buffer for the parameter value

            // Get the value of the parameter "min" from the query string
            if (httpd_query_key_value(buf, "min", param, sizeof(param)) == ESP_OK) {
                value = atof(param);
                HUMIDITY_MIN = value;  // Set the minimum humidity
                ESP_LOGI(TAG, "THe Min_hum = %f", value);
                httpd_resp_send(req, "Minimum humidity updated", HTTPD_RESP_USE_STRLEN);
            } else {
                httpd_resp_send(req, "Parameter 'min' not found", HTTPD_RESP_USE_STRLEN);
            }
        }
        free(buf);
    } else {
        httpd_resp_send(req, "Query not found", HTTPD_RESP_USE_STRLEN);
    }

    

    return ESP_OK;
}


//handlers integrater

void register_server_uri_handlers(httpd_handle_t server) {


    httpd_uri_t on_uri = {
        .uri      = "/on",
        .method   = HTTP_GET,
        .handler  = on_get_handler,
        .user_ctx = NULL
    };
    httpd_uri_t off_uri = {
        .uri      = "/off",
        .method   = HTTP_GET,
        .handler  = off_get_handler,
        .user_ctx = NULL
    };

    httpd_uri_t password_uri = {
        .uri      = "/Password",
        .method   = HTTP_GET,
        .handler  = password_get_handler,
        .user_ctx = NULL
    };

    httpd_uri_t min_uri = {
        .uri      = "/Min",
        .method   = HTTP_GET,
        .handler  = min_get_handler,
        .user_ctx = NULL
    };

    httpd_uri_t unlock_uri = {
        .uri      = "/Unlock",
        .method   = HTTP_GET,
        .handler  = unlock_get_handler,
        .user_ctx = NULL
    };

    httpd_uri_t lock_uri = {
        .uri      = "/Lock",
        .method   = HTTP_GET,
        .handler  = lock_get_handler,
        .user_ctx = NULL
    };


    httpd_uri_t max_uri = {
        .uri      = "/Max",
        .method   = HTTP_GET,
        .handler  = max_get_handler,
        .user_ctx = NULL
    };

    



    
    // Register URI handlers
    ESP_LOGI(TAG, "7777777");
    httpd_register_uri_handler(server, &on_uri);
    httpd_register_uri_handler(server, &off_uri);
    httpd_register_uri_handler(server, &password_uri);
    httpd_register_uri_handler(server, &unlock_uri);
    httpd_register_uri_handler(server, &lock_uri);
    httpd_register_uri_handler(server, &min_uri);
    httpd_register_uri_handler(server, &max_uri);

    
}



void start_webserver(void) {
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    ESP_LOGI(TAG, "1111");
    // Start the httpd server
    httpd_handle_t server = NULL;
    if (httpd_start(&server, &config) == ESP_OK) {
        // Register URI handlers
        
        register_server_uri_handlers(server);
    }
}



uint16_t Generate_password() {

    uint32_t random_number = esp_random();  // Generate a random 32-bit number
    uint16_t password = random_number % 10000;  // Reduce it to a 4-digit number

    char password_str[5];
    snprintf(password_str, sizeof(password_str), "%04u", password);  // Format as a 4-digit string, including leading zeros

    ESP_LOGI(TAG, "Generated 4-digit Password: %s", password_str);
    return password;
}

void push_to_top(void) {

    //To pull the door to the top for the first correct code
    while (currentPosition < ALL_THE_WAY) {
        OneStep(true); // True moves the window down
        currentPosition++;
        ESP_LOGI(TAG, "currentPosition = %d", currentPosition);
    }
}

void shut_down(void) {

    //To pull the door to the top for the first correct code
    while (currentPosition < ALL_THE_WAY) {
        OneStep(false); // True moves the window down
        currentPosition++;
        ESP_LOGI(TAG, "currentPosition = %d", currentPosition);
    }
}


void app_main(void)
{   
    //GEnerate code
    mypassword = Generate_password();
    
    bool first_up = true;
    bool first_lock = true;

    //motor Init
    motor();
    
    //init ledc
    ledc_init();

    //connect to wifi
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ESP_LOGI(TAG, "ESP_WIFI_MODE_STA");
    wifi_init_sta();

    //timer init
    esp_timer_create_args_t timer_args_diode = {
    .callback = &diode_toggle,
    .name = "diodetimer", 
    };
    esp_timer_handle_t diode_timer;
    esp_timer_create(&timer_args_diode, &diode_timer); 
    
    //start timer
    esp_timer_start_periodic(diode_timer, 100000);
    
    // DHT11 init
    OneWireInit();

    //copy from cp1
    start_webserver();

    while (1) {

        //ESP_LOGI(TAG, "my led status = %d", led);
        //gpio_set_level(Pass_led, led); 

        if(first_up && unlock == true) {
            first_up = false;
            //To pull the door to the top for the first correct code
            push_to_top();
            currentPosition = 0;
        }

        if(!first_up && unlock == true) {
            adjustWindowToHumidity(data.humi);
        }

        if(first_lock&& lock == true) {
            shut_down();
            first_lock = false;
            while (1) {

            }
            //first up = true; //continue to lift up
        }

        set_led_brightness_based_on_humidity(data.humi);
        
        if (sample) {
            sample = false;

            //wait
            while (readSensorData(&data) == ESP_FAIL) {
                vTaskDelay(50 / portTICK_PERIOD_MS);
            }

            //ESP_LOGI(TAG, "Temp: %.1f, Humi: %.1f", data.temp, data.humi);
        }
    }
}

