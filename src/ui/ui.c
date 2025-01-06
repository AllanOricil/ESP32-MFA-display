#include "ui.h"

static const char *TAG = "ui";

lv_obj_t *ui_init_screen;
lv_obj_t *ui_totp_screen;
lv_obj_t *ui_pin_screen;
lv_obj_t *ui_key_creation_screen;
lv_obj_t *ui_key_creation_screen_password_textarea;
lv_obj_t *ui_touch_calibration_screen;
lv_obj_t *ui_pin_screen_textarea;
lv_obj_t *ui_touch_calibration_screen_label;
lv_obj_t *ui_touch_calibration_screen_dot_top_left;
lv_obj_t *ui_touch_calibration_screen_dot_bottom_right;
Config config;
void ui_totp_screen_init(void);
void ui_pin_screen_init(void);
void ui_event_totp_screen(lv_event_t *e);
void ui_event_pin_screen_keyboard_button(lv_event_t *e);
void ui_event_pin_screen_textarea(lv_event_t *e);
void ui_event_key_creation_screen_keybard(lv_event_t *e);
void ui_totp_screen_update_totp_labels();
void ui_totp_screen_update_totp_countdowns();
void ui_totp_screen_render_totp_components();
void ui_totp_screen_render_active_group_index();
void ui_touch_calibration_screen_init(void);
void ui_touch_calibration_screen_step_2();
void ui_touch_calibration_screen_step_3();
void ui_touch_calibration_screen_destroy();

#if LV_COLOR_DEPTH != 16
#error "LV_COLOR_DEPTH should be 16bit to match SquareLine Studio's settings"
#endif

void ui_event_totp_screen(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    if (event_code == LV_EVENT_GESTURE)
    {
        on_totp_screen_gesture(e);
    }
}

void ui_event_pin_screen_keyboard_button(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    if (event_code == LV_EVENT_VALUE_CHANGED)
    {
        on_pin_screen_keyboard_button_clicked(e);
    }
}

void ui_event_pin_screen_textarea(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    if (event_code == LV_EVENT_READY)
    {
        on_pin_screen_form_submit(e);
    }
}

void ui_event_key_creation_screen_keybard(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    if (event_code == LV_EVENT_READY)
    {
        on_key_creation_form_submit(e);
    }
}

void init_ui(
    bool display_pin_screen,
    int max_unlock_attempts)
{
    ESP_LOGI(TAG, "initializing ui");
    config.display_pin_screen = display_pin_screen;
    config.unlock_attempts = max_unlock_attempts;
    config.max_unlock_attempts = max_unlock_attempts;

    lv_disp_t *disp = lv_disp_get_default();
    lv_theme_t *theme = lv_theme_default_init(
        disp,
        lv_palette_main(LV_PALETTE_BLUE),
        lv_palette_main(LV_PALETTE_RED),
        true,
        LV_FONT_DEFAULT);
    lv_disp_set_theme(disp, theme);
    // NOTE: initialize screens
    ui_touch_calibration_screen_init();
    ui_key_creation_screen_init();
    ui_totp_screen_init();
    ui_pin_screen_init();
    ESP_LOGI(TAG, "ui initialized");
}

void load_first_screen()
{
    lv_scr_load(ui_key_creation_screen);
}

void ui_task_handler()
{
    lv_task_handler();
}