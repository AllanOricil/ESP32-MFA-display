#include "touch-screen.hpp"

static const char *TAG = "touch-screen";

void single_touch_handler()
{
	ESP_LOGV(TAG, "calling single touch handler");
	display_handle_single_touch();
	ESP_LOGV(TAG, "single touch handler executed");
}

void double_touch_handler()
{
	ESP_LOGV(TAG, "calling double touch handler");
	display_handle_double_touch();
	ESP_LOGV(TAG, "double touch handler executed");
}

void init_touch_screen(Configuration config)
{
	ESP_LOGI(TAG, "initializing touch screen");

	init_display(config);
	init_touch(
		config.touch.calibrate,
		single_touch_handler,
		double_touch_handler);

	ESP_LOGI(TAG, "touch screen initialized");
}

void display_timeout_handler()
{
	check_display_timeout();
}