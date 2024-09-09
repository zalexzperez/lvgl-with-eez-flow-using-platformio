#include <Arduino.h>
#include "TFT_eSPI.h"
#include "ui/ui.h"
#include "ui/vars.h" // To get and set native variables if desired

const uint8_t display_BKL_pin = 48;

TFT_eSPI tft = TFT_eSPI(); // load TFT service
#define TFT_HOR_RES 320
#define TFT_VER_RES 172

/* LVGL draw into this buffer, 1/10 screen size usually works well. The size is in bytes */
#define DRAW_BUF_SIZE (TFT_HOR_RES * TFT_VER_RES / 10 * (LV_COLOR_DEPTH / 8)) // Note: Could increase it up to divided by 4
uint32_t draw_buf[DRAW_BUF_SIZE / 2];
uint32_t draw_buf2[DRAW_BUF_SIZE / 2];

/* Function prototypes here*/

void IRAM_ATTR my_disp_flush(lv_display_t *display, const lv_area_t *area, uint8_t *px_map); // Function to flush display
void encoder_read(lv_indev_t *drv, lv_indev_data_t *data);

#if LV_USE_LOG != 0
void my_print(lv_log_level_t level, const char *buf)
{
    LV_UNUSED(level);
    Serial.println(buf);
    Serial.flush();
}
#endif

void setup()
{
    // put your setup code here, to run once:
    Serial.begin(250000);
    delay(1500);

    Serial.println("Starting up LVGL with flow platformIO example!");

    pinMode(display_BKL_pin, OUTPUT);
    digitalWrite(display_BKL_pin, LOW);
    

    // Initialize LVGL library
    lv_init();

    /* Set a tick source so that LVGL will know how much time elapsed. */
    lv_tick_set_cb(xTaskGetTickCount);

    /* Register print function for debugging */
#if LV_USE_LOG != 0
    lv_log_register_print_cb(my_print);
#endif

    lv_display_t *disp;

#if LV_USE_TFT_ESPI
    /*TFT_eSPI can be enabled lv_conf.h to initialize the display in a simple way*/
    disp = lv_tft_espi_create(LV_TFT_HOR_RES, LV_TFT_VER_RES, draw_buf, sizeof(draw_buf));
    lv_display_set_rotation(disp, LV_DISPLAY_ROTATION_270);
#else
    /* Manually created display */
    tft.begin();        /* TFT init */
    tft.setRotation(3); /* Landscape orientation */
    tft.initDMA();

    disp = lv_display_create(TFT_HOR_RES, TFT_VER_RES);
    lv_display_set_flush_cb(disp, my_disp_flush);
    lv_display_set_buffers(disp, draw_buf, draw_buf2, sizeof(draw_buf), LV_DISPLAY_RENDER_MODE_PARTIAL);
#endif

    /*Initialize the input device driver*/
    lv_indev_t *indev = lv_indev_create();
    lv_indev_set_type(indev, LV_INDEV_TYPE_ENCODER);
    lv_indev_set_read_cb(indev, encoder_read);

    ui_init(); // EEZ GUI init

    digitalWrite(display_BKL_pin, HIGH); // Turn backlight on

    Serial.printf("performAnimation value: %d\n", get_var_perform_animation());
}

void loop()
{
    // Put your main code here, to run repeatedly:

    uint32_t time_till_next = 5; // Some value

    // Timer handler needs to be called periodically to handle the tasks of LVGL
    time_till_next = lv_timer_handler(); // lv_task_handler() is LVGL v8 only
    ui_tick();

    delay(10);
    //vTaskDelay(time_till_next / portTICK_PERIOD_MS); // Delay to avoid unnecessary polling
}

void encoder_read(lv_indev_t *drv, lv_indev_data_t *data)
{
    // Implementation required
}

void IRAM_ATTR my_disp_flush(lv_display_t *display, const lv_area_t *area, uint8_t *px_map)
{
    uint32_t w = (area->x2 - area->x1 + 1);
    uint32_t h = (area->y2 - area->y1 + 1);

    uint16_t *buf16 = (uint16_t *)px_map; // It's a 16 bit (RGB565) display

    tft.setAddrWindow(area->x1, area->y1, w, h);
    tft.startWrite();

    tft.setSwapBytes(true);
    tft.pushPixelsDMA(buf16, w * h); // Push line to screen
    tft.endWrite();

    lv_disp_flush_ready(display);
}