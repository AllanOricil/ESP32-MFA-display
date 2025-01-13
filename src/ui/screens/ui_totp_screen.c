#include "../ui.h"
#include "services.h"
#include "constants.h"
#include "clock.hpp"

#define LV_LABEL_SCROLL_SPEED (15)

lv_obj_t *create_totp_component(
    lv_obj_t *parent,
    const char *service,
    const char *totp,
    const int index)
{
    lv_obj_t *container = lv_obj_create(parent);
    // NOTE: this is used to find which index of
    container->user_data = index;
    lv_obj_set_width(container, 150);
    lv_obj_set_height(container, LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(container, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(container, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_obj_set_style_bg_color(container, lv_color_make(3, 6, 10), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(container, LV_OPA_TRANSP, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(container, 5, 0);
    lv_obj_set_scrollbar_mode(container, LV_SCROLLBAR_MODE_OFF);

    lv_obj_t *totp_service = lv_label_create(container);
    lv_label_set_text(totp_service, service);
    lv_obj_set_width(totp_service, LV_PCT(100));
    lv_obj_set_style_text_opa(totp_service, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(totp_service, lv_color_make(254, 254, 254), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(totp_service, LV_TEXT_ALIGN_LEFT, 0);
    lv_obj_set_style_text_font(totp_service, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_label_set_long_mode(totp_service, LV_LABEL_LONG_SCROLL_CIRCULAR);

    lv_obj_t *totp_label = lv_label_create(container);
    lv_label_set_text(totp_label, totp);
    lv_obj_set_style_text_opa(totp_label, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(totp_label, lv_color_make(254, 254, 254), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(totp_label, LV_TEXT_ALIGN_LEFT, 0);
    lv_obj_set_style_text_font(totp_label, &lv_font_montserrat_22, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t *totp_countdown = lv_bar_create(container);
    lv_bar_set_range(totp_countdown, 0, 30);
    lv_bar_set_value(totp_countdown, 30, LV_ANIM_OFF);
    lv_bar_set_start_value(totp_countdown, 0, LV_ANIM_OFF);
    lv_obj_set_width(totp_countdown, LV_PCT(100));
    lv_obj_set_height(totp_countdown, 5);
    lv_obj_set_style_pad_top(totp_countdown, 2, 0);
    lv_obj_set_style_bg_color(totp_countdown, lv_color_make(37, 50, 76), LV_PART_MAIN);
    lv_obj_set_style_bg_color(totp_countdown, lv_color_make(37, 196, 244), LV_PART_INDICATOR);

    return container;
}

void ui_totp_screen_init(void)
{
    ui_totp_screen = lv_obj_create(NULL);
    lv_obj_add_flag(ui_totp_screen, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scrollbar_mode(ui_totp_screen, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_bg_color(ui_totp_screen, lv_color_make(12, 18, 30), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_totp_screen, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_all(ui_totp_screen, 5, LV_PART_MAIN | LV_STATE_DEFAULT);

    // NOTE: set up flex container properties for the screen
    lv_obj_set_flex_flow(ui_totp_screen, LV_FLEX_FLOW_ROW_WRAP);
    lv_obj_set_flex_align(ui_totp_screen, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_obj_set_layout(ui_totp_screen, LV_LAYOUT_FLEX);

    ui_totp_screen_render_totp_components();

    lv_obj_add_event_cb(ui_totp_screen, ui_event_totp_screen, LV_EVENT_ALL, NULL);
}

void ui_totp_screen_render_totp_components(void)
{
    lv_obj_clean(ui_totp_screen);
    Service *services = get_services();
    uint8_t active_group = get_active_group();
    for (int i = 0; i < MAX_NUMBER_OF_SERVICES; i++)
    {
        Service *service = &services[i];
        if (service->name[0] != '\0' && service->group == active_group)
        {
            create_totp_component(ui_totp_screen, service->name, service->totp, i);
        }
    }
}

// void fade_out_task(lv_timer_t *timer)
// {
//     lv_obj_t *label = (lv_obj_t *)timer->user_data;
//     lv_anim_t fade_out;
//     lv_anim_init(&fade_out);
//     lv_anim_set_var(&fade_out, label);
//     lv_anim_set_time(&fade_out, 1000);
//     lv_anim_set_values(&fade_out, LV_OPA_100, LV_OPA_0);
//     lv_anim_set_exec_cb(&fade_out, (lv_anim_exec_xcb_t)lv_obj_set_style_text_opa);
//     lv_anim_start(&fade_out);
//     lv_timer_del(timer);
// }

void ui_totp_screen_render_active_group_index()
{
    lv_obj_clean(ui_totp_screen);

    lv_obj_t *card = lv_obj_create(ui_totp_screen);
    lv_obj_set_style_bg_color(card, lv_color_make(3, 6, 10), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(card, LV_OPA_100, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(card, 5, 0);
    lv_obj_set_style_border_opa(card, LV_OPA_TRANSP, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_scrollbar_mode(card, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_size(card, lv_obj_get_width(ui_totp_screen) - 10, lv_obj_get_height(ui_totp_screen) - 10);
    lv_obj_align(card, LV_ALIGN_CENTER, 0, 0);

    int active_group = get_active_group();
    lv_obj_t *group_label = lv_label_create(card);
    lv_label_set_text_fmt(group_label, "Group %d", active_group);
    lv_obj_set_style_text_color(group_label, lv_color_white(), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(group_label, &lv_font_montserrat_22, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(group_label, LV_OPA_100, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_align(group_label, LV_ALIGN_CENTER, 0, 0);

    // lv_timer_create(fade_out_task, 1500, group_label);
}

void ui_totp_screen_update_totp_labels()
{
    int index = 0;
    lv_obj_t *totp_component = lv_obj_get_child(ui_totp_screen, index);
    Service *services = get_services();
    while (totp_component)
    {
        Service *service = &services[(int)totp_component->user_data];
        lv_obj_t *totp_label = lv_obj_get_child(totp_component, 1);
        lv_label_set_text(totp_label, service->totp);
        index++;
        totp_component = lv_obj_get_child(ui_totp_screen, index);
    }
}

void ui_totp_screen_update_totp_countdowns()
{
    int index = 0;
    lv_obj_t *totp_component = lv_obj_get_child(ui_totp_screen, index);
    int second = get_second();
    while (totp_component)
    {
        lv_obj_t *totp_countdown = lv_obj_get_child(totp_component, 2);
        lv_bar_set_value(totp_countdown, TOTP_PERIOD - second % TOTP_PERIOD, LV_ANIM_OFF);
        index++;
        totp_component = lv_obj_get_child(ui_totp_screen, index);
    }
}
