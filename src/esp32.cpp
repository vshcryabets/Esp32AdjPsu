#include <Arduino.h>
#include "vm.h"
#include "esp32.h"
#include <WiFiManager.h> // https://github.com/tzapu/WiFiManager
#include <WebServer.h>
#include <ESPmDNS.h>
#include <SPIFFS.h>
#include "Wire.h"
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "DmmIna226.h"

VM *viewModel;
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");
EspState mcuState;

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

void handleNotFound(AsyncWebServerRequest *request)
{
    request->send(404, "text/plain", "Not found");
}

void handlePwmOff(AsyncWebServerRequest *request)
{
    if (request->hasParam("channel"))
    {
        const AsyncWebParameter *p = request->getParam("channel");
        PwmConfig espPwmConfig;
        espPwmConfig.channel = atoi(p->value().c_str());
        viewModel->vmOnPwmEnd(&espPwmConfig);
    }
    request->send(200, "text/plain", "OK");
}

void handlePwmGet(AsyncWebServerRequest *request)
{
    PwmConfig config;
    viewModel->vmOnPwmGet(&config);
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
        viewModel->vmOnPwmUpdate(&espPwmConfig);
        request->send(200, "text/plain", "OK");
    }
    else
    {
        request->send(400, "text/plain", "Missed argument: channel,duty");
    }
}

void handleCalibration(AsyncWebServerRequest *request)
{
    viewModel->vmOnCalibration();
    request->send(200, "text/plain", "OK");
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

        viewModel->vmOnPwmStart(&espPwmConfig);
        request->send(200, "text/plain", "OK");
    }
    else
    {
        request->send(400, "text/plain", "Missed argument: channel,freq,res,pin");
    }
}

void onWebSocketMessage(void *arg, uint8_t *data, size_t len)
{
    AwsFrameInfo *info = (AwsFrameInfo *)arg;
    if (info->final && info->opcode == WS_TEXT)
    {
        data[len] = 0; // Null-terminate message
        Serial.printf("Received: %s\n", (char *)data);
        // ws.textAll("Echo: " + String((char*)data));  // Echo message back to all clients
    }
}

void onWebSocketEvent(AsyncWebSocket *server, AsyncWebSocketClient *client,
                      AwsEventType type, void *arg, uint8_t *data, size_t len)
{
    switch (type)
    {
    case WS_EVT_CONNECT:
        Serial.printf("Client #%u connected\n", client->id());
        break;
    case WS_EVT_DISCONNECT:
        Serial.printf("Client #%u disconnected\n", client->id());
        break;
    case WS_EVT_DATA:
        onWebSocketMessage(arg, data, len);
        break;
    }
}

void setup()
{
    Serial.begin(9600);
    viewModel = new VM(new DmmIna226(0x40)); // Initialize VM with DmmIna226 instance

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
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
              { request->send(SPIFFS, "/index.html", "text/html"); });
    server.on("/pwmon", HTTP_GET, handlePwmOn);
    server.on("/pwmset", HTTP_GET, handlePwmSet);
    server.on("/pwmoff", HTTP_GET, handlePwmOff);
    server.on("/calibration", HTTP_GET, handleCalibration);
    // curl "http://esppower.local/pwmget"
    server.on("/pwmget", HTTP_GET, handlePwmGet);
    server.onNotFound(handleNotFound);
    ws.onEvent(onWebSocketEvent);
    server.addHandler(&ws);
    server.begin();

    viewModel->vmOnHwReady();
}

void loop()
{
    // float input = state->pwm.duty / 255.f;
    // float target = input * 10.0f;

    // mcuState.nn.train_step(input, target, 0.1f); // learning rate = 0.1
    // mcuState.nn.debugOutput(input, target);

    // static int count = 0;
    // if (++count % 100 == 0) {
    //   save_weights(&mcuState.nn);
    // }

    if (viewModel->isDirty)
    {
        if (viewModel->dmmResult.timestamp != mcuState.dmmSendTime)
        {
            const DmmResult &result = viewModel->dmmResult;
            ws.cleanupClients();
            mcuState.dmmSendTime = result.timestamp;
            String response;
            response += "{\"v\":";
            response += result.voltage;
            response += ",\"c\":";
            response += result.current;
            response += ",\"ts\":";
            response += result.timestamp;
            response += "}";
            ws.textAll(response);
        }
    }
}