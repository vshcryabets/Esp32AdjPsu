#include "esp32.h"
#include <Arduino.h>
#include "vm.h"

#ifdef ESP32
#include <WiFiManager.h> // https://github.com/tzapu/WiFiManager
#include <WebServer.h>
#include <ESPmDNS.h>
#include <SPIFFS.h>
#include "Wire.h"

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

WebServer server(80);
VM *espState = nullptr;

void handle_OnConnect()
{
    if (SPIFFS.exists("/index.html"))
    {
        File file = SPIFFS.open("/index.html", "r");
        server.streamFile(file, "text/html");
        file.close();
    }
    else
    {
        server.send(404, "text/plain", "File Not Found");
    }
}

void handle_NotFound()
{
    server.send(404, "text/plain", "Not found");
}

void handle_PwmOff()
{
    for (uint8_t i = 0; i < server.args(); i++)
    {
        if (server.argName(i).equals("channel"))
        {
            PwmConfig espPwmConfig;
            espPwmConfig.channel = atoi(server.arg(i).c_str());
            vmOnPwmEnd(espState, &espPwmConfig);
            break;
        }
    }
    server.send(200, "text/plain", "OK");
}

void handle_PwmGet()
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
    server.send(200, "application/json", response.c_str());
}

void handle_PwmSet()
{
    int8_t channel = -1;
    int32_t duty = -1;
    for (uint8_t i = 0; i < server.args(); i++)
    {
        if (server.argName(i).equals("channel"))
            channel = atoi(server.arg(i).c_str());
        else if (server.argName(i).equals("duty"))
            duty = atoi(server.arg(i).c_str());
    }
    if (channel > -1 && duty > -1)
    {
        PwmConfig espPwmConfig;
        espPwmConfig.channel = channel;
        espPwmConfig.duty = duty;
        vmOnPwmUpdate(espState, &espPwmConfig);
        server.send(200, "text/plain", "OK");
    }
    else
    {
        server.send(400, "text/plain", "Missed argument: channel,duty");
    }
}

void handle_PwmOn()
{
    int8_t channel = -1;
    int32_t freq = -1;
    int8_t resolution = -1;
    int8_t pin = -1;
    int16_t duty = 0;
    for (uint8_t i = 0; i < server.args(); i++)
    {
        if (server.argName(i).equals("channel"))
            channel = atoi(server.arg(i).c_str());
        else if (server.argName(i).equals("freq"))
            freq = atoi(server.arg(i).c_str());
        else if (server.argName(i).equals("res"))
            resolution = atoi(server.arg(i).c_str());
        else if (server.argName(i).equals("pin"))
            pin = atoi(server.arg(i).c_str());
        else if (server.argName(i).equals("duty"))
            duty = atoi(server.arg(i).c_str());
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
        server.send(200, "text/plain", "OK");
    }
    else
    {
        server.send(400, "text/plain", "Missed argument: channel,freq,res,pin");
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

    server.on("/", handle_OnConnect);
    server.on("/pwmon", handle_PwmOn);
    server.on("/pwmset", handle_PwmSet);
    server.on("/pwmoff", handle_PwmOff);
    // curl "http://esppower.local/pwmget"
    server.on("/pwmget", handle_PwmGet);
    server.onNotFound(handle_NotFound);
    server.begin();
}

void esp32Loop(struct VM *state)
{
    server.handleClient();
}
#endif