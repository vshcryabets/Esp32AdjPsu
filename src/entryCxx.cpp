// #ifdef ESP32
#include <Arduino.h>
#include "vm.h"
#include "ui.h"
#ifdef ESP32
  #include "esp32.h"  
#endif

// struct VM viewModel;
#define LED_BUILTIN 8

#define LED_BUILTIN 8

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(115200);  // Start the Serial communication to send messages to the computer
  Serial.println("Hello, ESP32!");  // Print a message to the Serial Monitor
  delay(1000);  // Wait for a second to ensure the Serial Monitor is ready
  Serial.println("Setup complete, starting loop...");  // Indicate setup is complete
}

void loop() {
  // put your main code here, to run repeatedly:
  digitalWrite(LED_BUILTIN, LOW);  // turn the LED on (LOW is the voltage level)
  delay(100);                      // wait for a second
  digitalWrite(LED_BUILTIN, HIGH);   // turn the LED off by making the voltage HIGH
  delay(900);
  Serial.println("LED toggled");  // Print a message to the Serial Monitor
}

// void setup()
// {
//   pinMode(LED_BUILTIN, OUTPUT);

//   Serial.begin(115200); // Set baud rate to 115200
//   // Serial.setDebugOutput(true);
//   pinMode(LED_BUILTIN, OUTPUT);
//   sleep(1000); // Allow time for serial monitor to connect
//   for (int i = 0; i < 2000; i++) {
//     Serial.print(".");
//     digitalWrite(LED_BUILTIN, LOW); // Turn off the LED initially
//     sleep(500); // Allow time for serial monitor to connect
//     digitalWrite(LED_BUILTIN, HIGH); // Turn on the LED to indicate setup start
//     sleep(500); // Allow time for serial monitor to connect
//   }
// //   Serial.println("Starting ESP32 AdjPsu...");
// //   vmInit(&viewModel);
// //   uiInit(&viewModel);
// // #ifdef STM8
// //   stm8Init());
// // #elif defined(ESP32)
// //   esp32Init(&viewModel);
// // #endif
// //   vmOnHwReady(&viewModel);
// }


// void loop()
// {
// #ifdef ESP32
//   esp32Loop(&viewModel);
// #endif
//   vmUpdateState(&viewModel, millis());
// }
// #endif