#include <time.h>
#include <string.h>
#include <esp_log.h>
#include "ui.h"
#include "services.h"
#include "constants.h"
#include "auth.h"
#include "mfa.h"
#include "encryption.hpp"

static const char *TAG = "ui_events";

void on_totp_screen_gesture(lv_event_t *e)
{
    lv_dir_t dir = lv_indev_get_gesture_dir(lv_indev_get_act());
    ESP_LOGD(TAG, "dir %d", dir);
    // NOTE: it is inverted because the gesture event is inverted
    bool group_changed = false;
    if (dir == LV_DIR_LEFT)
    {
        group_changed = change_active_group_right();
    }
    else if (dir == LV_DIR_RIGHT)
    {
        group_changed = change_active_group_left();
    }

    if (group_changed)
    {
        update_totps();
        ui_totp_screen_render_totp_components();
        ui_totp_screen_update_totp_labels();
        ui_totp_screen_update_totp_countdowns();
    }
}

void on_pin_screen_keyboard_button_clicked(lv_event_t *e)
{
    lv_obj_t *keyboard = lv_event_get_target(e);
    uint32_t btn_id = lv_btnmatrix_get_selected_btn(keyboard);
    ESP_LOGD(TAG, "button id %u", btn_id);
    if (btn_id != LV_BTNMATRIX_BTN_NONE)
    {
        const char *btn_text = lv_btnmatrix_get_btn_text(keyboard, btn_id);

        if (strcmp(btn_text, "DEL") == 0)
            lv_textarea_del_char(ui_pin_screen_textarea);
        else if (strcmp(btn_text, "CLEAR") == 0)
            lv_textarea_set_text(ui_pin_screen_textarea, "");
        else if (strcmp(btn_text, "OK") == 0)
            lv_event_send(ui_pin_screen_textarea, LV_EVENT_READY, NULL);
        else
            lv_textarea_add_text(ui_pin_screen_textarea, btn_text);
    }
}

void on_pin_screen_form_submit(lv_event_t *e)
{
    lv_obj_t *textarea = lv_event_get_target(e);
    const char *pin = lv_textarea_get_text(textarea);

    if (!validate_pin(pin))
    {
        config.unlock_attempts--;

        if (!config.unlock_attempts)
        {
            lv_obj_center(lv_msgbox_create(NULL, "ERROR", "BOARD LOCKED", NULL, false));
            return;
        }

        char message[128];
        sprintf(message, "Wrong PIN\nAttempts left: %d", config.unlock_attempts);
        lv_obj_center(lv_msgbox_create(NULL, "ERROR", message, NULL, true));
        return;
    }

    config.unlock_attempts = config.max_unlock_attempts;

    lv_scr_load(ui_totp_screen);
}

void on_key_creation_form_submit(lv_event_t *e)
{
    const char *password = lv_textarea_get_text(ui_key_creation_screen_password_textarea);

    ESP_LOGD(TAG, "Password: %s\n", password);
    generate_key(password);
}