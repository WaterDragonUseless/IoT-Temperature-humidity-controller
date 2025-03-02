#include "onewire_dht11.h"

static const char *TAG = "OneWire";

// Check the duration is under tolerance
inline bool checkRange(const uint32_t time, const uint32_t time_dur) {
    return (time > time_dur - TOLERANCE && time < time_dur + TOLERANCE) ? true : false;
}

// The time duration for level 0 and 1 to determine the bit is 1 or 0
uint16_t transfer_bit(frame_t * frame) {
    if (!checkRange(frame->level0_time, BIT_LOW)) return 2; 
    else if (checkRange(frame->level1_time, BIT0_HIGH)) return 0; 
    else if (checkRange(frame->level1_time, BIT1_HIGH)) return 1;
    else return 2;
}

// Get one frame of value - 0 time duration and 1 time duration
void get_frame(frame_t * frame) {
    static uint32_t startTime0, endTime0;
    static uint32_t startTime1, endTime1;
    int loop1 = 0, loop2 = 0;

    // Get 0 time duration
    startTime0 = esp_timer_get_time();
    while (gpio_get_level(PIN_NUM) == LOW && loop1 < TIME_OUT)  {loop1++;}
    endTime0 = esp_timer_get_time();

    // Get 1 time duration
    startTime1 = esp_timer_get_time();
    while (gpio_get_level(PIN_NUM) == HIGH && loop2 < TIME_OUT) {loop2++;}
    endTime1 = esp_timer_get_time();

    // Put the duration into frame
    frame->level0_time = endTime0 - startTime0;
    frame->level1_time = endTime1 - startTime1;
}

// Initializa a gpio pin
void OneWireInit(void) {
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << PIN_NUM), 
        .mode = GPIO_MODE_INPUT_OUTPUT_OD,
        .pull_up_en = GPIO_PULLUP_ENABLE,
    };
    gpio_config(&io_conf);
    ESP_LOGI(TAG, "Onewire set-up complete");
}

// Sent the signal first for 18000us low and then 20us high
esp_err_t sendStartSignal(void) {
    gpio_set_level(PIN_NUM, LOW);
    esp_rom_delay_us(START_TRANS_LOW_DUR);
    gpio_set_level(PIN_NUM, HIGH);
    esp_rom_delay_us(START_TRANS_HIGH_DUR);
    return ESP_OK;
}

// Receive the start frame and check if it is correct
esp_err_t checkStartResponse(void) {
    static frame_t frame;

    //while (gpio_get_level(PIN_NUM) == HIGH) {}

    // Get start frame response from the sensor
    get_frame(&frame);

    // Check pull down 80us
    if (!checkRange(frame.level0_time, START_REC_LOW_DUR) || 
        !checkRange(frame.level1_time, START_REC_HIGH_DUR)) {
        return ESP_FAIL;
    }

    return ESP_OK;
}

// update sensor data
esp_err_t readSensorData(data_t * data) {
    static frame_t frame[40];
    uint16_t temp_int = 0, humi_int = 0, sum_int = 0;

    // Start signal
    if (sendStartSignal() == ESP_FAIL) return ESP_FAIL;
    if (checkStartResponse() == ESP_FAIL) return ESP_FAIL;

    // Read 40 bit from the sensor
    for (int i = 0; i < 40; i++) {
        get_frame(&frame[i]);
    }

    for (int i = 0; i < 40; i++) {
        // Using rise and fall time to determine bit number
        uint16_t num = transfer_bit(&frame[i]);
        if (num == 2) return ESP_FAIL;

        // Transfer humidity
        if (i < 16) {
            humi_int <<= 1;
            humi_int += num;
        }
        // Transfer temperature
        else if (i < 32) {
            temp_int <<= 1;
            temp_int += num;
        }
        // Transfer checkSum
        else {
            sum_int <<= 1;
            sum_int += num;
        }
    }

    // Data validation using checksum
    if (((temp_int >> 8) + (temp_int & 0xff) + (humi_int >> 8) + (humi_int & 0xff)) != sum_int) {
        return ESP_FAIL;
    }

    // Transfer data into float
    data->temp = (float)temp_int / 256.0f;
    data->humi = (float)humi_int / 256.0f;

    return ESP_OK;
}