#include "esp32.h"
#include <Arduino.h>
#include "vm.h"

#ifdef ESP32
#include <WiFiManager.h> // https://github.com/tzapu/WiFiManager
#include <WebServer.h>
#include <ESPmDNS.h>
#include <SPIFFS.h>
#include "Wire.h"
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

#pragma region PWM
extern "C" void pwmStart(struct PwmConfig *config)
{
    auto err = ledcSetup(config->channel, config->freq, config->resolution);
    Serial.print("ledcSetup=");
    Serial.println(err);
    ledcAttachPin(config->pin, config->channel);
    ledcWrite(config->channel, config->duty);
}

extern "C" void pwmSetup(struct PwmConfig *config)
{
    ledcWrite(config->channel, config->duty);
}

extern "C" void pwmStop(struct PwmConfig *config)
{
    ledcWrite(config->channel, 0);
}
#pragma endregion

AsyncWebServer server(80);
VM *espState = nullptr;

void handleNotFound(AsyncWebServerRequest *request)
{
    request->send(404, "text/plain", "Not found");
}

void handlePwmOff(AsyncWebServerRequest *request) {
    if (request->hasParam("channel")) {
        const AsyncWebParameter* p = request->getParam("channel");
        PwmConfig espPwmConfig;
        espPwmConfig.channel = atoi(p->value().c_str());
        vmOnPwmEnd(espState, &espPwmConfig);
    }
    request->send(200, "text/plain", "OK");
}

void handlePwmGet(AsyncWebServerRequest *request)
{
    PwmConfig config;
    vmOnPwmGet(espState, &config);
    String response;
    response += "{\"work\":";
    response += config.work;
    response += ",\"duty\":";
    response += config.duty;
    response += ",\"channel\":";
    response += config.channel;
    response += ",\"pin\":";
    response += config.pin;
    response += ",\"freq\":";
    response += config.freq;
    response += "}";
    request->send(200, "application/json", response.c_str());
}

void handlePwmSet(AsyncWebServerRequest *request)
{
    int8_t channel = -1;
    int32_t duty = -1;
    for (uint8_t i = 0; i < request->args(); i++)
    {
        if (request->argName(i).equals("channel"))
            channel = atoi(request->arg(i).c_str());
        else if (request->argName(i).equals("duty"))
            duty = atoi(request->arg(i).c_str());
    }
    if (channel > -1 && duty > -1)
    {
        PwmConfig espPwmConfig;
        espPwmConfig.channel = channel;
        espPwmConfig.duty = duty;
        vmOnPwmUpdate(espState, &espPwmConfig);
        request->send(200, "text/plain", "OK");
    }
    else
    {
        request->send(400, "text/plain", "Missed argument: channel,duty");
    }
}

void handlePwmOn(AsyncWebServerRequest *request)
{
    int8_t channel = -1;
    int32_t freq = -1;
    int8_t resolution = -1;
    int8_t pin = -1;
    int16_t duty = 0;
    for (uint8_t i = 0; i < request->args(); i++)
    {
        if (request->argName(i).equals("channel"))
            channel = atoi(request->arg(i).c_str());
        else if (request->argName(i).equals("freq"))
            freq = atoi(request->arg(i).c_str());
        else if (request->argName(i).equals("res"))
            resolution = atoi(request->arg(i).c_str());
        else if (request->argName(i).equals("pin"))
            pin = atoi(request->arg(i).c_str());
        else if (request->argName(i).equals("duty"))
            duty = atoi(request->arg(i).c_str());
    }
    if (channel > -1 && freq > -1 && resolution > -1 && pin > -1)
    {
        Serial.print("Enable PWM on ");
        Serial.print(channel);
        Serial.print(" freq=");
        Serial.print(freq);
        Serial.print(" duty=");
        Serial.print(duty);
        Serial.print(" res=");
        Serial.println(resolution);
        PwmConfig espPwmConfig;
        espPwmConfig.channel = channel;
        espPwmConfig.freq = freq;
        espPwmConfig.resolution = resolution;
        espPwmConfig.pin = pin;
        espPwmConfig.duty = duty;

        vmOnPwmStart(espState, &espPwmConfig);
        request->send(200, "text/plain", "OK");
    }
    else
    {
        request->send(400, "text/plain", "Missed argument: channel,freq,res,pin");
    }
}

void esp32Init(struct VM *state)
{
    espState = state;
    WiFiManager wm;
    Wire.begin();
    Wire.setClock(50000);
    bool res = wm.autoConnect("ESPPower", "E$PP0w4r"); // password protected ap
    if (!res)
    {
        Serial.println("Failed to connect");
        // ESP.restart();
    }
    else
    {
        // if you get here you have connected to the WiFi
        Serial.println("connected...yeey :)");
    }

    if (!MDNS.begin("ESPPower"))
    {
        Serial.println("Error starting mDNS");
    }
    MDNS.addService("esppower", "tcp", 80);

    if (!SPIFFS.begin(true))
    {
        Serial.println("SPIFFS failed");
        return;
    }

    // Обробка запиту до "/"
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(SPIFFS, "/index.html", "text/html");
    });
    server.on("/pwmon", HTTP_GET, handlePwmOn);
    server.on("/pwmset", HTTP_GET, handlePwmSet);
    server.on("/pwmoff", HTTP_GET, handlePwmOff);
    // curl "http://esppower.local/pwmget"
    server.on("/pwmget", HTTP_GET, handlePwmGet);
    server.onNotFound(handleNotFound);
    server.begin();
}

void esp32Loop(struct VM *state)
{
    // server.handleClient();
}
#endif