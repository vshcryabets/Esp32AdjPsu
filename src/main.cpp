#include <atomic>
#include <new>

#include <SPIFFS.h>
#include <Arduino.h>
#include "vm.h"
#include <WiFiManager.h> // https://github.com/tzapu/WiFiManager
#include <WebServer.h>
#include <ESPmDNS.h>
#include "Wire.h"
#include <AsyncTCP.h>
#include "impl/DmmIna226.h"
#include "HttpController.h"

alignas(DmmIna226) uint8_t dmmBuffer[sizeof(DmmIna226)];
DmmIna226 *dmm;

alignas(PwmEsp32) uint8_t pwmBuffer[sizeof(PwmEsp32)];
PwmEsp32 *pwm;

alignas(HttpController) uint8_t httpControllerBuffer[sizeof(HttpController)];
HttpController *httpController;

VM viewModel;


#define LED_PIN 8
std::atomic<uint16_t> blinkPeriod(500);
void blinkTask(void *parameter) {
  pinMode(LED_PIN, OUTPUT);
  for (;;) {
    digitalWrite(LED_PIN, HIGH);
    vTaskDelay(blinkPeriod.load() / portTICK_PERIOD_MS);
    digitalWrite(LED_PIN, LOW);
    vTaskDelay(blinkPeriod.load() / portTICK_PERIOD_MS);
  }
}

WiFiManager wm;

const char *ssid = "MyESP32-AP";
const char *password = "password123";
int sdaPin = 4;
int sclPin = 5;

void setup()
{
    Serial.begin(9600);
    delay(2000);
    Serial.println("Hello, ESP32!"); // Print a message to the Serial Monitor

    xTaskCreate(
        blinkTask,
        "Blinker",
        240,  // Stack size
        NULL,
        1,  // Priority
        NULL
        );

    if (!SPIFFS.begin(true))
    {
        Serial.println("SPIFFS failed");
        blinkPeriod = 300;
        return;
    }

    Wire.begin(sdaPin, sclPin);
    Wire.setClock(50000);
    wm.setConfigPortalBlocking(true);
    bool res = wm.autoConnect("ESPPower", "E$PP0w4r"); // password protected ap
    if (!res)
    {
        Serial.println("Failed to connect");
        blinkPeriod = 300;
        // ESP.restart();
    }
    else
    {
        // if you get here you have connected to the WiFi
        blinkPeriod = 2000;
        Serial.println("connected...yeey :)");
    }

    delay(1000);
    dmm = new (dmmBuffer) DmmIna226(0x40);
    pwm = new (pwmBuffer) PwmEsp32();
    viewModel.init(dmm, pwm);

    if (!MDNS.begin("ESPPower"))
    {
        Serial.println("Error starting mDNS");
    }
    MDNS.addService("esppower", "tcp", 80);

    viewModel.onHwReady();
    httpController = new (httpControllerBuffer) HttpController(&viewModel);
    httpController->begin();
    Serial.println("Setup complete, starting loop...");
}

void loop()
{
}