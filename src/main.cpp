#include <Arduino.h>
#include <esp_log.h>
#include "ui/ui.h"
#include "auth.h"
#include "constants.h"
#include "services.h"
#include "mfa.h"
#include "config.hpp"
#include "utils.hpp"
#include "clock.hpp"
#include "storage.hpp"
#include "wifi.hpp"
#include "touch-screen.hpp"
#include "manager.hpp"
#include "encryption.hpp"

static const char *TAG = "main";

enum ApplicationState
{
  START,
  TOUCH_CALIBRATION_START,
  TOUCH_CALIBRATION_MIN,
  TOUCH_CALIBRATION_MAX,
  TOUCH_CALIBRATION_UPDATE,
  TOUCH_CALIBRATION_COMPLETE,
  TOTPS_UPDATE
};

void setup()
{
#if ENABLE_SERIAL
  Serial.begin(115200);
#endif
  ESP_LOGI(TAG, "----------- begin setup ------------");
  init_storage();
  Configuration config = Configuration::load();
  init_encryption(config.encryption.salt.c_str());
  load_services();
  init_auth(
      config.authentication.pin.hash.c_str(),
      config.authentication.pin.key.c_str(),
      config.manager.authentication.username.c_str(),
      config.manager.authentication.password.c_str(),
      config.manager.authentication.key.c_str(),
      config.manager.authentication.session_length);
  init_touch_screen(config);
  const char *local_network_ip = init_wifi(config).c_str();
  init_clock();
  if (config.is_manager_configured())
  {
    init_manager(config, local_network_ip);
  }
  init_ui(
      config.is_authentication_configured(),
      config.authentication.unlock_attempts);
  ESP_LOGI(TAG, "----------- end setup ------------");
}

void loop()
{
  // NOTE: display available free memory
  print_free_memory();
  static ApplicationState application_state = START;
  switch (application_state)
  {
  case START:
    if (touch_is_calibrated())
    {
      application_state = TOUCH_CALIBRATION_COMPLETE;
    }
    else
    {
      application_state = TOUCH_CALIBRATION_START;
    }
    break;

  case TOUCH_CALIBRATION_START:
    static unsigned long state_change_time = 0;
    lv_scr_load(ui_touch_calibration_screen);
    application_state = TOUCH_CALIBRATION_MIN;
    state_change_time = millis();
    break;

  case TOUCH_CALIBRATION_MIN:
    if (millis() - state_change_time > TOUCH_TIME_TO_CALIBRATE_EACH_POINT)
    {
      touch_calibrate_min();
      ui_touch_calibration_screen_step_2();
      application_state = TOUCH_CALIBRATION_MAX;
      state_change_time = millis();
    }
    break;

  case TOUCH_CALIBRATION_MAX:
    if (millis() - state_change_time > TOUCH_TIME_TO_CALIBRATE_EACH_POINT)
    {
      touch_calibrate_max();
      application_state = TOUCH_CALIBRATION_UPDATE;
    }
    break;

  case TOUCH_CALIBRATION_UPDATE:
    touch_save_calibration();
    touch_register();
    touch_set_calibrated();
    ui_touch_calibration_screen_step_3();
    application_state = TOUCH_CALIBRATION_COMPLETE;
    state_change_time = millis();
    break;

  case TOUCH_CALIBRATION_COMPLETE:
    if (millis() - state_change_time > TOUCH_TIME_DISPLAYING_SUCCESS_CALIBRATION_MESSAGE)
    {
      lv_obj_clean(lv_scr_act());
      load_first_screen();
      reset_display_off_timer();
      ui_touch_calibration_screen_destroy();
      application_state = TOTPS_UPDATE;
    }
    break;

  case TOTPS_UPDATE:
    display_timeout_handler();
    unsigned long elapsed_number_of_time_steps = get_elapsed_number_of_time_steps();
    static unsigned long last_step = 0;
    if (elapsed_number_of_time_steps != last_step)
    {
      update_totps();
      ui_totp_screen_update_totp_labels();
      last_step = elapsed_number_of_time_steps;
    }

    int current_second = get_second();
    static unsigned long previous_second = 0;
    if (current_second != previous_second)
    {
      ui_totp_screen_update_totp_countdowns();
      previous_second = current_second;
    }
    break;
  }
  ui_task_handler();
}
