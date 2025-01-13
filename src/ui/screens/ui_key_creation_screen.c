#include "../ui.h"
#include "constants.h"

void ui_key_creation_screen_init(void)
{
	ui_key_creation_screen = lv_obj_create(NULL);
	lv_obj_set_flex_flow(ui_key_creation_screen, LV_FLEX_FLOW_COLUMN);
	lv_obj_set_flex_align(ui_key_creation_screen, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
	lv_obj_set_layout(ui_key_creation_screen, LV_LAYOUT_FLEX);
	lv_obj_set_style_pad_row(ui_key_creation_screen, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_pad_all(ui_key_creation_screen, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_scrollbar_mode(ui_key_creation_screen, LV_SCROLLBAR_MODE_OFF);
	lv_obj_set_style_bg_color(ui_key_creation_screen, lv_color_make(12, 18, 30), LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_bg_opa(ui_key_creation_screen, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

	lv_obj_t *password_textareas_container = lv_obj_create(ui_key_creation_screen);
	lv_obj_set_width(password_textareas_container, LV_HOR_RES);
	lv_obj_set_height(password_textareas_container, LV_SIZE_CONTENT);
	lv_obj_set_flex_flow(password_textareas_container, LV_FLEX_FLOW_COLUMN);
	lv_obj_set_flex_align(password_textareas_container, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
	lv_obj_set_scrollbar_mode(password_textareas_container, LV_SCROLLBAR_MODE_OFF);
	lv_obj_set_style_pad_all(password_textareas_container, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_radius(password_textareas_container, 0, 0);
	lv_obj_set_style_border_opa(password_textareas_container, LV_OPA_TRANSP, LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_bg_color(password_textareas_container, lv_color_make(12, 18, 30), LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_bg_opa(password_textareas_container, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

	ui_key_creation_screen_password_textarea = lv_textarea_create(password_textareas_container);
	lv_obj_set_width(ui_key_creation_screen_password_textarea, LV_HOR_RES - 15);
	lv_obj_set_height(ui_key_creation_screen_password_textarea, 40);
	lv_obj_set_style_text_align(ui_key_creation_screen_password_textarea, LV_TEXT_ALIGN_CENTER, 0);
	lv_textarea_set_password_mode(ui_key_creation_screen_password_textarea, true);
	lv_textarea_set_placeholder_text(ui_key_creation_screen_password_textarea, "Enter password");
	lv_textarea_set_max_length(ui_key_creation_screen_password_textarea, MAX_PASSWORD_LENGTH);
	lv_textarea_set_one_line(ui_key_creation_screen_password_textarea, true);
	lv_obj_set_style_text_font(ui_key_creation_screen_password_textarea, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_text_color(ui_key_creation_screen_password_textarea, lv_color_make(254, 254, 254), LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_text_color(ui_key_creation_screen_password_textarea, lv_color_make(254, 254, 254), LV_PART_TEXTAREA_PLACEHOLDER);
	lv_obj_set_style_bg_color(ui_key_creation_screen_password_textarea, lv_color_make(3, 6, 10), LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_border_opa(ui_key_creation_screen_password_textarea, LV_OPA_TRANSP, LV_PART_MAIN | LV_STATE_DEFAULT);

	ui_key_creation_screen_password_confirmation_textarea = lv_textarea_create(password_textareas_container);
	lv_obj_set_width(ui_key_creation_screen_password_confirmation_textarea, LV_HOR_RES - 15);
	lv_obj_set_height(ui_key_creation_screen_password_confirmation_textarea, 40);
	lv_obj_set_style_text_align(ui_key_creation_screen_password_confirmation_textarea, LV_TEXT_ALIGN_CENTER, 0);
	lv_textarea_set_password_mode(ui_key_creation_screen_password_confirmation_textarea, true);
	lv_textarea_set_placeholder_text(ui_key_creation_screen_password_confirmation_textarea, "Confirm password");
	lv_textarea_set_max_length(ui_key_creation_screen_password_confirmation_textarea, MAX_PASSWORD_LENGTH);
	lv_textarea_set_one_line(ui_key_creation_screen_password_confirmation_textarea, true);
	lv_obj_set_style_text_font(ui_key_creation_screen_password_confirmation_textarea, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_text_color(ui_key_creation_screen_password_confirmation_textarea, lv_color_make(254, 254, 254), LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_text_color(ui_key_creation_screen_password_confirmation_textarea, lv_color_make(254, 254, 254), LV_PART_TEXTAREA_PLACEHOLDER);
	lv_obj_set_style_bg_color(ui_key_creation_screen_password_confirmation_textarea, lv_color_make(3, 6, 10), LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_border_opa(ui_key_creation_screen_password_confirmation_textarea, LV_OPA_TRANSP, LV_PART_MAIN | LV_STATE_DEFAULT);

	lv_obj_t *keyboard = lv_keyboard_create(ui_key_creation_screen);
	lv_keyboard_set_textarea(keyboard, ui_key_creation_screen_password_textarea);
	lv_obj_align(keyboard, LV_ALIGN_BOTTOM_MID, 0, 0);
	lv_obj_set_style_pad_all(keyboard, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_flex_grow(keyboard, 2);
	lv_obj_set_style_bg_color(keyboard, lv_color_make(0, 0, 0), LV_PART_MAIN | LV_STATE_DEFAULT);

	static lv_style_t buttons_style;
	lv_style_init(&buttons_style);
	lv_style_set_text_color(&buttons_style, lv_color_make(254, 254, 254));
	lv_style_set_bg_color(&buttons_style, lv_color_make(9, 18, 30));
	lv_obj_add_style(keyboard, &buttons_style, LV_PART_ITEMS);

	static lv_style_t buttons_style_checked_state;
	lv_style_init(&buttons_style_checked_state);
	lv_style_set_text_color(&buttons_style_checked_state, lv_color_make(254, 254, 254));
	lv_style_set_bg_color(&buttons_style_checked_state, lv_color_make(3, 6, 10));
	lv_obj_add_style(keyboard, &buttons_style_checked_state, LV_PART_ITEMS | LV_STATE_CHECKED);

	lv_obj_add_event_cb(keyboard, ui_event_key_creation_screen_keybard, LV_EVENT_ALL, NULL);
	lv_obj_add_event_cb(ui_key_creation_screen_password_textarea, ui_event_key_creation_screen_textarea, LV_EVENT_ALL, keyboard);
	lv_obj_add_event_cb(ui_key_creation_screen_password_confirmation_textarea, ui_event_key_creation_screen_textarea, LV_EVENT_ALL, keyboard);
}