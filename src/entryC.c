#ifdef STM8
#include <Arduino.h>
#include "config.h"
#include "vm.h"
#include "ui.h"

struct VM viewModel;

void setup()
{
  Serial_begin(9600);
  vmInit(&viewModel);
  uiInit(&viewModel);
#ifdef STM8
  stm8Init());
#else defined(ESP32)
  esp32Init();
#endif
}

void loop()
{
  // uint16_t res = !digitalRead(PIN_UP) << 3 | 
  //   !digitalRead(PIN_DOWN) << 2 | 
  //   !digitalRead(PIN_LEFT) << 1 | 
  //   !digitalRead(PIN_RIGHT) << 0;
  // vmOnButtons(&viewModel, res);

  // vmUpdateState(&viewModel);
  // uiDraw(&viewModel);
 
  // delay(20);
#ifdef ESP32
  esp32Loop();
#endif
  vmUpdateState(&viewModel);
}
#endif