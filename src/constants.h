#define CONFIG_FILE_PATH "/config.yml"
#define SERVICES_FILE_PATH "/services.yml"

// NOTE: NTP server variables
#define NTP_SERVER_URL "pool.ntp.org"
#define GMT_OFFSET_SEC 0
#define DAYLIGHT_OFFSET_SEC 3600

// NOTE: all totps must have a period of 30 seconds, for now
#define TOTP_PERIOD 30
// NOTE: max number of groups of services.
#define MAX_NUMBER_OF_GROUPS 10
// NOTE: due to mem limits there has to exist a max number of services we can generate TOTPs for
// NOTE: the YAML parser is limiting this number. It could be 150, but the parser can handle up to 113.
#define MAX_NUMBER_OF_SERVICES 100
// NOTE: 60 digits + 1 for the null-terminating character
#define MAX_SERVICE_NAME_LENGTH 61

// NOTE: 6 digits + 1 for the null-terminating character
#define MAX_TOTP_LENGTH 7

// PIN
#define PIN_MIN_LENGTH 6
#define PIN_MAX_LENGTH 20

// AUTHENTICATION
#define MAX_UNLOCK_ATTEMPTS 3

// MANAGER AUTHENTICATION
// NOTE: how many minutes the session is valid. For security purposes, sessions are set to 5 minutes but can be overwritten by the User in config.yml
#define MAX_MANAGER_SESSION_LENGTH 5

// UI
#define PIN_SCREEN_NAME "pin"
#define TOTP_SCREEN_NAME "totp"
#define TOUCH_CALIBRATION_SCREEN_NAME "touch-calibration"

// TOUCH
#define TOUCH_DOUBLE_TOUCH_INTERVAL 300
#define TOUCH_CALIBRATION_SPIFFS_FILE_PATH "/calibration.txt"
#define TOUCH_TIME_TO_CALIBRATE_EACH_POINT 5000
#define TOUCH_TIME_DISPLAYING_SUCCESS_CALIBRATION_MESSAGE 2000

// DISPLAY
#define LVGL_TICK_PERIOD_MS 1
#define LVGL_BUFFER_PIXELS (DISPLAY_WIDTH * DISPLAY_HEIGHT / 4)
#define LVGL_BUFFER_MALLOC_FLAGS (MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT)
#define PWM_CHANNEL_BCKL (SOC_LEDC_CHANNEL_NUM - 1)
#define PWM_FREQ_BCKL 400
#define PWM_BITS_BCKL 8
#define PWM_MAX_BCKL ((1 << PWM_BITS_BCKL) - 1)
#define SLEEP_TIMEOUT 10

// ENCRYPTION
#define SALT_SIZE 16
#define KEY_SIZE 32
#define ITERATIONS 10000
#define KEY_FILE_PATH "/key.bin"