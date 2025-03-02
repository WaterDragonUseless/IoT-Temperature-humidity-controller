#ifndef OLED_DISPLAY_H
#define OLED_DISPLAY_H

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_timer.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_panel_ops.h"
#include "driver/i2c.h"
#include "esp_err.h"
#include "esp_log.h"
#include "hal/lcd_types.h"
#include "lvgl.h"
#include "esp_lvgl_port.h"

#define I2C_HOST                    0
#define LCD_PIXEL_CLOCK_HZ          (400 * 1000)
#define PIN_NUM_SDA                 17
#define PIN_NUM_SCL                 18
#define PIN_NUM_RST                 21
#define I2C_HW_ADDR                 0x3C
#define LCD_CMD_BITS                8
#define LCD_PARAM_BITS              8

#define LCD_H_RES                   128
#define LCD_V_RES                   64

lv_obj_t * lcd_init(void);
lv_obj_t * create_label(lv_obj_t * screen);

void display_clear(void);
void display_text(lv_obj_t * label, const char * text);

#endif