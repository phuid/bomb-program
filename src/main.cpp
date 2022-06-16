#include <Arduino.h>

#define WRONG_PIN 31
const uint8_t correct_pins[5] = {26, 27, 28, 29, 30};

#define RESET_BTN_PIN 53
const uint8_t switches[4] = {50, 51, 48, 49};                                                                        // in order 8421 (see the board)
const uint16_t gamelenght_possibilities[16] = {0, 5, 10, 15, 20, 30, 40, 60, 80, 100, 120, 150, 180, 210, 240, 300}; // in seconds

#define PIEZO_PIN 46
#define LED_PIN LED_BUILTIN
// led - 52

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

uint8_t correct_pin;
uint16_t gamelenght;

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
        gamelenght = gamelenght_possibilities[switches_state];
        timer_selected = true;
      }
    }
    previous = current;
    if (current)
      delay(10);
  }

  Serial.print("Timer: ");
  Serial.println(gamelenght);

  // wait for button -> start the game
}

void loop()
{
  // if reset reset

  // check wires

  // check timer

  // set the led and piezo
}