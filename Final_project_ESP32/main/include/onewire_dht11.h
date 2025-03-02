#ifndef ONEWIRE_DHT11_H
#define ONEWIRE_DHT11_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_err.h"
#include "driver/gpio.h"
#include "esp_timer.h"

#define PIN_NUM                     GPIO_NUM_38
#define HIGH                        1
#define LOW                         0

#define START_TRANS_LOW_DUR         18000
#define START_TRANS_HIGH_DUR        20
#define START_REC_LOW_DUR           75
#define START_REC_HIGH_DUR          80
#define BIT_LOW                     50
#define BIT0_HIGH                   26
#define BIT1_HIGH                   70
#define TOLERANCE                   20
#define TIME_OUT                    10000

// Frame structure: record the 0 time and 1 time
struct _frame_t {
    uint16_t level0_time;
    uint16_t level1_time;
};
typedef struct _frame_t frame_t;

// Data structure: recode the humidity and temperature
struct _data_t {
    float temp;
    float humi;
};
typedef struct _data_t data_t;

void OneWireInit(void);
esp_err_t sendStartSignal(void);
esp_err_t checkStartResponse(void);
esp_err_t readSensorData(data_t * data);

#endif