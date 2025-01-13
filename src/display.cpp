#include "display.hpp"

static const char *TAG = "display";

TFT_eSPI tft = TFT_eSPI(DISPLAY_WIDTH, DISPLAY_HEIGHT);
static lv_disp_drv_t disp_drv;
unsigned long sleep_timeout = 0;
unsigned long time_display_turned_off = 0;
bool show_pin_screen = false;
bool display_is_active = false;

void turn_off_display()
{
    ledcWrite(PWM_CHANNEL_BCKL, 0);
    display_is_active = false;

    // NOTE: The PIN screen is loaded immediately after the display turns off. This prevents users from briefly seeing the TOTP screen when the display is turned back on.
    if (show_pin_screen)
    {
        lv_textarea_set_text(ui_pin_screen_textarea, "");
        lv_disp_load_scr(ui_pin_screen);
    }

    ESP_LOGI(TAG, "display turned off");
}

void turn_on_display()
{
    // NOTE: number could be accidentally be pressed when awaking the display with a touch in the keys area. So, this will ensure the text area is reset
    if (show_pin_screen)
    {
        lv_textarea_set_text(ui_pin_screen_textarea, "");
    }
    ledcWrite(PWM_CHANNEL_BCKL, 0.5 * PWM_MAX_BCKL);
    display_is_active = true;
    ESP_LOGI(TAG, "display turned on");
}

void reset_display_off_timer()
{
    time_display_turned_off = millis();
    if (!display_is_active)
    {
        turn_on_display();
    }
    ESP_LOGI(TAG, "display timeout timer was reset");
}

void check_display_timeout()
{
    lv_obj_t *active_screen = lv_scr_act();
    if (sleep_timeout && (active_screen == ui_totp_screen || active_screen == ui_pin_screen))
    {
        unsigned long elapsed_time = millis() - time_display_turned_off;
        if (elapsed_time >= sleep_timeout && display_is_active)
        {
            ESP_LOGI(TAG, "display timeout reached");
            turn_off_display();
            time_display_turned_off = millis();
        }
    }
}

void display_handle_single_touch()
{
    reset_display_off_timer();
}

void display_handle_double_touch()
{
    lv_obj_t *active_screen = lv_scr_act();
    if (active_screen == ui_totp_screen)
    {

        if (display_is_active)
        {
            turn_off_display();
        }
        else
        {
            turn_on_display();
            reset_display_off_timer();
        }
    }
}

void on_display_change(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p)
{
    uint32_t w = (area->x2 - area->x1 + 1);
    uint32_t h = (area->y2 - area->y1 + 1);

    tft.startWrite();
    tft.setAddrWindow(area->x1, area->y1, w, h);

    for (int y = area->y1; y <= area->y2; y++)
    {
        for (int x = area->x1; x <= area->x2; x++)
        {
            uint32_t buffer_pos = (y - area->y1) * w + (x - area->x1);
            uint16_t color = color_p[buffer_pos].full;
            tft.writeColor(color, 1);
        }
    }

    tft.endWrite();
    lv_disp_flush_ready(disp);
}

void init_display(Configuration config)
{
    ESP_LOGI(TAG, "initializing display");

    sleep_timeout = config.display.sleep_timeout * 1000;
    show_pin_screen = config.is_authentication_configured();

    ESP_LOGD(TAG, "initiliazing backlight");
    pinMode(TFT_BCKL, OUTPUT);
    digitalWrite(TFT_BCKL, HIGH);
    ESP_LOGD(TAG, "backlight initialized");

    ESP_LOGD(TAG, "initializing tft");
    tft.begin();
    tft.setSwapBytes(true);
    tft.fillScreen(ILI9341_PINK);
    // NOTE: Landscape orientation
    tft.setRotation(2);
    ESP_LOGD(TAG, "tft initialized");

    display_is_active = true;
    ESP_LOGD(TAG, "initializing lvgl");
    lv_init();
    ESP_LOGD(TAG, "initializing lvgl");
    display_register();

    ESP_LOGI(TAG, "display initialized");
}

void display_register()
{
    ESP_LOGD(TAG, "registering display in lvgl");
    lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res = DISPLAY_WIDTH;
    disp_drv.ver_res = DISPLAY_HEIGHT;
    disp_drv.flush_cb = on_display_change;
    disp_drv.draw_buf = (lv_disp_draw_buf_t *)malloc(sizeof(lv_disp_draw_buf_t));
    disp_drv.antialiasing = 1;
    void *draw_buffer = heap_caps_malloc(sizeof(lv_color_t) * LVGL_BUFFER_PIXELS, LVGL_BUFFER_MALLOC_FLAGS);
    lv_disp_draw_buf_init(disp_drv.draw_buf, draw_buffer, NULL, LVGL_BUFFER_PIXELS);
    lv_disp_t *disp = lv_disp_drv_register(&disp_drv);
    if (!disp)
    {
        ESP_LOGE(TAG, "failed to register display in lvgl");
        return;
    }
    lv_obj_clean(lv_scr_act());
    ledcSetup(PWM_CHANNEL_BCKL, PWM_FREQ_BCKL, PWM_BITS_BCKL);
    ledcAttachPin(21, PWM_CHANNEL_BCKL);
    ledcWrite(PWM_CHANNEL_BCKL, 0.5 * PWM_MAX_BCKL);
    reset_display_off_timer();
    ESP_LOGD(TAG, "display registered in lvgl");
}
