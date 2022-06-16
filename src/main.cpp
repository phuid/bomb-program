#include <Arduino.h>

#define WRONG_PIN 31
const uint8_t correct_pins[5] = {26, 27, 28, 29, 30};

#define PIEZO_PIN 46
#define LED_PIN 52
#define RESET_BTN_PIN 53
const uint8_t switches[4] = {50, 51, 48, 49}; //in order 8421 (see the board)
const uint16_t gamelenght_possibilities[16] = {0, 5, 10, 15, 20, 30, 40, 60, 80, 100, 120, 150, 180, 210, 240, 300}; //in seconds

void setup() {
  
}

void loop() {
  
}