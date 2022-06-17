#include <Arduino.h>

#define WRONG_PIN 31
const uint8_t correct_pins[5] = {26, 27, 28, 29, 30};

#define RESET_BTN_PIN 53
const uint8_t switches[4] = {50, 51, 48, 49};                                                                        // in order 8421 (see the board)
const uint16_t gamelenght_possibilities[16] = {5, 10, 15, 20, 30, 40, 60, 80, 100, 120, 150, 180, 210, 240, 300, 600}; // in seconds

#define PIEZO_PIN 46
#define LED_PIN LED_BUILTIN
// led - 52

#define PIEZO_SINGLEBEEP_LENGTH 100 // in ms

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

uint8_t correct_pin;
size_t gamelenght;
size_t gamestart_time;

bool ledvalue = false;
bool piezovalue = false;

size_t ledstart_time;
size_t piezostart_time;

size_t led_length;
size_t piezo_length;

int power(int base, int exponent)
{
  int result = 1;
  for (int i = 0; i < exponent; i++)
  {
    result *= base;
  }
  return result;
}

// function that converts an array of bools in binary to decimal
uint8_t binaryToDecimal(bool *binary)
{
  uint8_t decimal = 0;
  for (uint8_t i = 0; i < 4; i++)
  {
    decimal += binary[i] * power(2, i);
  }
  return decimal;
}

uint8_t readswitches()
{
  bool arr[4];
  for (uint8_t i = 0; i < sizeof(switches) / sizeof(switches[0]); i++)
  {
    pinMode(switches[i], INPUT_PULLUP);
    arr[i] = digitalRead(switches[i]);
  }
  return binaryToDecimal(arr);
}

void led(bool value, size_t length)
{
  ledvalue = 1;
  ledstart_time = millis();
  led_length = length;
}

void led(size_t length)
{
  ledvalue = !ledvalue;
  ledstart_time = millis();
  led_length = length;
}

void piezo(bool value, size_t length)
{
  piezovalue = 1;
  piezostart_time = millis();
  piezo_length = length;
}

void setup()
{
  Serial.begin(115200);
  Serial.println("Starting...");
  pinMode(WRONG_PIN, INPUT_PULLUP);
  for (uint8_t i = 0; i < sizeof(correct_pins) / sizeof(correct_pins[0]); i++)
  {
    pinMode(correct_pins[i], INPUT_PULLUP);
  }
  pinMode(RESET_BTN_PIN, INPUT_PULLUP);
  for (uint8_t i = 0; i < sizeof(switches) / sizeof(switches[0]); i++)
  {
    pinMode(switches[i], INPUT_PULLUP);
  }
  pinMode(PIEZO_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);

  Serial.println("Welcome to the game!");

  // select wire
  ////wait for button -> read the switches
  bool correct_pin_selected = false;
  bool previous = digitalRead(RESET_BTN_PIN);
  while (!correct_pin_selected)
  {
    bool current = digitalRead(RESET_BTN_PIN);
    if (!current && previous)
    {
      uint8_t switches_state = readswitches();
      if (switches_state < sizeof(correct_pins) / sizeof(correct_pins[0]))
      {
        correct_pin = correct_pins[switches_state];
        correct_pin_selected = true;
      }
      else {
        Serial.print("Invalid switch state: ");
        Serial.println(switches_state);
      }
    }
    previous = current;
    if (current)
      delay(10);
  }

  Serial.print("Correct pin: ");
  Serial.println(correct_pin);

  // select timer
  ////wait for button -> read the switches
  bool timer_selected = false;
  previous = digitalRead(RESET_BTN_PIN);
  while (!timer_selected)
  {
    bool current = digitalRead(RESET_BTN_PIN);
    if (!current && previous)
    {
      uint8_t switches_state = readswitches();
      if (switches_state < sizeof(gamelenght_possibilities) / sizeof(gamelenght_possibilities[0]))
      {
        gamelenght = gamelenght_possibilities[switches_state] * 1000;
        timer_selected = true;
      }
      else {
        Serial.print("Invalid switch state: ");
        Serial.println(switches_state);
      }
    }
    previous = current;
    if (current)
      delay(10);
  }

  Serial.print("Timer: ");
  Serial.println(gamelenght);

  // wait for button -> start the game

  gamestart_time = millis();
}

size_t calc_delay()
{
// from https://github.com/WouterGritter/CSGO-Bomb/blob/master/armed_state.ino
/**
 * Time between beeps is an exponential function
 *
 * A0 = 1.04865151217746
 * A1 = 0.244017811416199
 * A2 = 1.76379778668885
 *
 * Y(bps) = A0 * E^(A1*X + A2*X^2)
 * X = % OF TIME PASSED (0.0 - 1.0)
 * Y = BEEPS PER SECOND
 */
#define BEEP_A0 1.04865151217746
#define BEEP_A1 0.244017811416199
#define BEEP_A2 1.76379778668885
  // Calculate next beep time
  float x = (float)(millis() - gamestart_time) / gamelenght;
  float n = BEEP_A1 * x + BEEP_A2 * x * x;
  float bps = BEEP_A0 * exp(n);

  // Convert bps (beeps per second) to a wait time in milliseconds
  Serial.print(millis() - gamestart_time);
  Serial.print(" ");
  Serial.print(gamelenght);
  Serial.print("wait: ");
  Serial.println(1000.0 / bps);
  return 1000.0 / bps;
}

void loop()
{
  // if reset reset

  // check wires

  // check timer

  // set the led and piezo variables
  ////normal ticking
  if (millis() - ledstart_time > led_length)
  {
    piezo(1, PIEZO_SINGLEBEEP_LENGTH);
    led(calc_delay());
  }

  ////beep sequences

  // set the led and piezo
  digitalWrite(LED_PIN, ledvalue);
  digitalWrite(PIEZO_PIN, piezovalue);
}