#ifndef LVGL_TOUCH_H
#define LVGL_TOUCH_H

// Includes
#include <lvgl.h>
#include <XPT2046_Bitbang.h>
#include <string.h>
#include "constants.h"
#include "config.hpp"

// Function declarations
void init_touch(Configuration config);

#endif // LVGL_TOUCH_H