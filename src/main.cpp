#include <Arduino.h>
// DO NOT MODIFY
///////////////////////////////////////////////////////////////////////////////
// CONFIG (lines 4-33)

#define WRONG_PIN 31
const uint8_t CORRECT_PINS[5] = {26, 27, 28, 29, 30};

#define RESET_BTN_PIN 53
const uint8_t SWITCHES[4] = {50, 51, 48, 49};                                                                          // in order 8421 (see the board)
const uint16_t GAMELENGTH_POSSIBILITIES[16] = {5, 10, 15, 20, 30, 40, 60, 80, 100, 120, 150, 180, 210, 240, 300, 600}; // in seconds

#define PIEZO_PIN 46
#define LED_PIN LED_BUILTIN
// led - 52

#define PIEZO_SINGLEBEEP_LENGTH 100 // in ms

/*
SEQUENCE HELP:
in ms
[0] = length of first beep
[1] = length of delay after first beep
[2] = length of second beep
[3] = length of delay after second beep
[4] = length of third beep
sequence ends instantly after third beep
*/
#define CORRECT_SEQUENCE 200, 200, 50, 50, 50
#define WRONG_SEQUENCE 2000, 0, 0, 0, 0
#define WIRE_SELECT_SEQUENCE 50, 50, 50, 50, 50
#define TIMER_SELECT_SEQUENCE 50, 100, 150, 200, 250 

// CONFIG (lines 4-34)
///////////////////////////////////////////////////////////////////////////////
// DO NOT MODIFY

enum Gameover_reason
{
  REASON_NONE,
  REASON_CORRECT_WIRE,
  REASON_WRONG_WIRE,
  REASON_TIMEOUT,
  REASON_RESET
};

uint8_t correct_pin;
uint32_t gamelenght;
uint32_t gamestart_time;

bool ledvalue = false;
bool piezovalue = false;

uint32_t ledstart_time;
uint32_t piezostart_time;

uint32_t led_length;
uint32_t piezo_length;

uint32_t sequence[5];
int sequence_index = -1; // if >= 0 sequence is playing else = -1

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
  for (uint8_t i = 0; i < sizeof(SWITCHES) / sizeof(SWITCHES[0]); i++)
  {
    pinMode(SWITCHES[i], INPUT_PULLUP);
    arr[i] = digitalRead(SWITCHES[i]);
  }
  return binaryToDecimal(arr);
}

void led(bool value, size_t length)
{
  ledvalue = value;
  ledstart_time = millis();
  led_length = length;
}

void led(size_t length)
{
  led(!ledvalue, length);
}

void piezo(bool value, size_t length)
{
  if (sequence_index >= 0)
    return;
  piezovalue = value;
  piezostart_time = millis();
  piezo_length = length;
}

void makesequence(uint32_t one, uint32_t two, uint32_t three, uint32_t four, uint32_t five)
{
  sequence[0] = one;
  sequence[1] = two;
  sequence[2] = three;
  sequence[3] = four;
  sequence[4] = five;
  sequence_index = 0;
  piezovalue = 0;
  piezo_length = 0;
}

void restart()
{
  Serial.print("restart(); correct pin: ");
  Serial.print(correct_pin);
  Serial.print("; gamelength: ");
  Serial.println(gamelenght);
  led_length = 0;
  ledvalue = 0;
  piezovalue = 0;
  makesequence(CORRECT_SEQUENCE);
  gamestart_time = millis();
}

void setup()
{
  Serial.begin(115200);
  Serial.println("Starting...");
  pinMode(WRONG_PIN, INPUT_PULLUP);
  for (uint8_t i = 0; i < sizeof(CORRECT_PINS) / sizeof(CORRECT_PINS[0]); i++)
  {
    pinMode(CORRECT_PINS[i], INPUT_PULLUP);
  }
  pinMode(RESET_BTN_PIN, INPUT_PULLUP);
  for (uint8_t i = 0; i < sizeof(SWITCHES) / sizeof(SWITCHES[0]); i++)
  {
    pinMode(SWITCHES[i], INPUT_PULLUP);
  }
  pinMode(PIEZO_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);

  Serial.println("Welcome to the game!");

  makesequence(WIRE_SELECT_SEQUENCE);

  // select wire
  ////wait for button -> read the switches
  bool correct_pin_selected = false;
  bool previous = digitalRead(RESET_BTN_PIN);
  makesequence(WIRE_SELECT_SEQUENCE);
  while (!correct_pin_selected)
  {
    bool current = digitalRead(RESET_BTN_PIN);
    if (!current && previous)
    {
      uint8_t switches_state = readswitches();
      if (switches_state < sizeof(CORRECT_PINS) / sizeof(CORRECT_PINS[0]))
      {
        correct_pin = CORRECT_PINS[switches_state];
        correct_pin_selected = true;
      }
      else
      {
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
  makesequence(TIMER_SELECT_SEQUENCE);
  while (!timer_selected)
  {
    bool current = digitalRead(RESET_BTN_PIN);
    if (!current && previous)
    {
      uint8_t switches_state = readswitches();
      if (switches_state < sizeof(GAMELENGTH_POSSIBILITIES) / sizeof(GAMELENGTH_POSSIBILITIES[0]))
      {
        gamelenght = GAMELENGTH_POSSIBILITIES[switches_state] * 1000;
        timer_selected = true;
      }
      else
      {
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

  restart();
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
  double x = (float)(millis() - gamestart_time) / gamelenght;
  double n = BEEP_A1 * x + BEEP_A2 * x * x;
  double bps = BEEP_A0 * exp(n);

  // Convert bps (beeps per second) to a wait time in milliseconds
  // Serial.print(millis() - gamestart_time);
  // Serial.print(" ");
  // Serial.print(gamelenght);
  // Serial.print("wait: ");
  // Serial.println(1000.0 / bps);
  return 1000.0 / bps;
}

void gameover(Gameover_reason reason)
{
  if (reason == REASON_TIMEOUT || reason == REASON_WRONG_WIRE)
  {
    Serial.print("BOOM: ");
    Serial.println((millis() - gamestart_time) / 1000.f);
  }
}

void loop()
{
  // if reset reset
  if (!digitalRead(RESET_BTN_PIN))
  {
    restart();
  }

  // check wires
  ////check correct wire
  if (digitalRead(correct_pin) == 1)
  {
    // Serial.println("Correct wire!");
    // gameover(REASON_CORRECT_WIRE);
  }

  ////check wrong wires
  for (uint8_t i = 0; i < sizeof(CORRECT_PINS) / sizeof(CORRECT_PINS[0]); i++)
  {
    if (CORRECT_PINS[i] != correct_pin && digitalRead(CORRECT_PINS[i]) == 1)
    {
      // Serial.println("Wrong wire!");
      // gameover(REASON_WRONG_WIRE);
      break;
    }
  }
  if (digitalRead(WRONG_PIN) == 1)
  {
    // Serial.println("Wrong wire!");
    // gameover(REASON_WRONG_WIRE);
  }

  // check timer
  if (millis() - gamestart_time > gamelenght)
  {
    // Serial.println("Time's up!");
    gameover(REASON_TIMEOUT);
  }

  // set the led and piezo variables
  ////normal ticking
  if (millis() - ledstart_time > led_length)
  {
    piezo(1, PIEZO_SINGLEBEEP_LENGTH);
    led(calc_delay());
  }

  if (millis() - piezostart_time > piezo_length)
  {
    piezovalue = 0;
  }

  ////beep sequences
  if (sequence_index >= 0)
  {
    if (millis() - piezostart_time > piezo_length)
    {
      piezo_length = sequence[sequence_index];
      piezostart_time = millis();
      sequence_index++;
    }

    piezovalue = (sequence_index % 2);

    if (sequence_index > sizeof(sequence) / sizeof(sequence[0]))
    {
      sequence_index = -1;
    }
  }

  // set the led and piezo
  digitalWrite(LED_PIN, ledvalue);
  digitalWrite(PIEZO_PIN, piezovalue);
}