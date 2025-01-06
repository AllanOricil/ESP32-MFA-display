#include "../ui.h"

void ui_key_creation_screen_init(void)
{
	ui_key_creation_screen = lv_obj_create(NULL);

	ui_key_creation_screen_password_textarea = lv_textarea_create(ui_key_creation_screen);
	lv_obj_set_size(ui_key_creation_screen_password_textarea, 200, 50);
	lv_obj_align(ui_key_creation_screen_password_textarea, LV_ALIGN_TOP_MID, 0, 20);
	lv_textarea_set_password_mode(ui_key_creation_screen_password_textarea, true);
	lv_textarea_set_placeholder_text(ui_key_creation_screen_password_textarea, "Enter password");

	lv_obj_t *keyboard = lv_keyboard_create(ui_key_creation_screen);
	lv_keyboard_set_textarea(keyboard, ui_key_creation_screen_password_textarea);
	lv_obj_align(keyboard, LV_ALIGN_BOTTOM_MID, 0, 0);

	lv_obj_add_event_cb(keyboard, ui_event_key_creation_screen_keybard, LV_EVENT_CLICKED, NULL);
}