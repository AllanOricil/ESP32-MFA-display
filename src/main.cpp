// System Includes
#include <Arduino.h>
#include <SD.h>
#include <WiFi.h>
#include <time.h>
#include <lvgl.h>

// Library Includes
#include <esp32_smartdisplay.h>
#include <ui/ui.h>
#include <mfa.hpp>

// Local Includes
#include "totp-map.h"
#include "configuration.h"
#include "Base32.h"
#include "ESP32Time.h"

#define TF_CS 5
#define TOTP_PERIOD 30000

ESP32Time rtc;

const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 0;  // Replace with your actual timezone offset (in seconds)
const int   daylightOffset_sec = 3600; // Replace with your daylight saving offset (in seconds)

const char* ssid = WIFI_SSID;
const char* pwd = WIFI_PASSWORD;

void init_wifi(){
  Serial.printf("Connecting to %s", ssid);
  WiFi.begin(ssid, pwd);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("WiFi connected.");
}

void sync_time(){
  // NOTE: retrieve time from the NTP server and setting the system time with it
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

  // NOTE: wait until time is retrieved
  struct tm timeinfo;
  time_t now;

  while ((now = time(nullptr)) < 24*3600) {
    Serial.println("Waiting for time to be set...");
    delay(500);
  }

  gmtime_r(&now, &timeinfo); // initialize timeinfo

  // NOTE: set RTC time with the time from the NTP server
  rtc.setTimeStruct(timeinfo); 
  Serial.println(rtc.getDateTime());  
}

DecodedBase32Secret decode_encoded_base32_secret(char *secret) {
  Serial.printf("decoding %s\n", secret);

  byte* tempDecoded = NULL;
  Base32 base32;

  int secretLength = strlen(secret);
  byte *secretBytes = new byte[secretLength];
  
  for (int i = 0; i < secretLength; i++) {
    secretBytes[i] = secret[i];
  }
  
  int keyLength = base32.fromBase32(secretBytes, secretLength, (byte*&)tempDecoded);

  Serial.println("decoded base 32 secret");
  for (int i = 0; i < keyLength; i++) {
      Serial.print(tempDecoded[i], HEX);
      Serial.print(" ");
  }
  Serial.printf("decoded base 32 secret length: %d\n", keyLength);

  DecodedBase32Secret decodedBase32Secret;
  decodedBase32Secret.length = keyLength;
  decodedBase32Secret.value = tempDecoded;

  delete[] secretBytes;

  return decodedBase32Secret; 
}

void init_secrets(){
  Serial.println("fetching /secrets.txt");

  if(!SD.begin(TF_CS)){
    Serial.println("sd card mount failed");
    return;
  }
  if (!SD.exists("/secrets.txt")) {
    Serial.println("secrets.txt file does not exist");
    return;
  } 

  File file = SD.open("/secrets.txt");
  if (!file) {
    Serial.println("failed to open /secrets.txt for reading");
    return;
  }

  Serial.println("decoding secrets");
  while (file.available()) {
    String line = file.readStringUntil('\n');
    Serial.printf("processing line: %s\n", line.c_str());
    int delimiterIndex = line.indexOf(',');
    if (delimiterIndex != -1) {
      // NOTE: converts const char* to char* because of upsert_decoded_base32_secret
      char* service = strdup(line.substring(0, delimiterIndex).c_str());
      char* encodedBase32Secret = strdup(line.substring(delimiterIndex + 1).c_str());
      Serial.printf("service: %s\n", service);
      Serial.printf("encoded base 32 secret: %s\n", encodedBase32Secret);
      upsert_decoded_base32_secret(service, decode_encoded_base32_secret(encodedBase32Secret));
      Serial.println("service and secret information stored\n");
    }
  }
  Serial.println("all secrets were decoded");

  file.close();
}

void setup() {
  Serial.begin(115200);

  // SETUP
  init_wifi();
  sync_time();
  init_secrets();

  // GENERATE FIRST TOTPS BASED ON THE CURRENT TIME
  generate_totps();

  // UI
  smartdisplay_init();
  auto disp = lv_disp_get_default();
  lv_disp_set_rotation(disp, LV_DISP_ROT_90);
  smartdisplay_lcd_set_backlight(1);
  ui_init();
}

void loop() {
  // NOTE: ensures totps are generated exactly every 30 seconds. For example: 00:00:00, 00:00:30, 00:01:00, 00:01:30...
  unsigned long now = ((rtc.getMinute() * 60) + rtc.getSecond()) * 1000;
  static unsigned long nextTrigger = 0;
  if (now >= nextTrigger) {
    generate_totps();
    refresh_totp_labels();
    nextTrigger = ((now / TOTP_PERIOD) + 1) * TOTP_PERIOD;
  }

  // NOTE: ensures the counter is updated on every second, instead of after 1 second
  static unsigned long previousSecond = 0;
  unsigned long currentSecond = rtc.getSecond();
  if (currentSecond != previousSecond) {
      refresh_counter_bars();
      previousSecond = currentSecond;
  }

  lv_timer_handler();
}

