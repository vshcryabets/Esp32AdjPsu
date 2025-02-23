#ifdef ESP32
#include <Arduino.h>
#include "vm.h"
#include "ui.h"
#ifdef ESP32
  #include "esp32.h"  
#endif

struct VM viewModel;

void setup()
{
  Serial.begin(9600);
  vmInit(&viewModel);
  uiInit(&viewModel);
#ifdef STM8
  stm8Init());
#elif defined(ESP32)
  esp32Init(&viewModel);
#endif
  vmOnHwReady(&viewModel);
}


void loop()
{
#ifdef ESP32
  esp32Loop(&viewModel);
#endif
  vmUpdateState(&viewModel, millis());
}
#endif