#include "Http.h"
#include <SPIFFS.h>
#include "pwm.h"

HttpHandler::HttpHandler(VmPwm *vmPwm): server(80), ws("/ws"), vmPwm(vmPwm)
{
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
                { 
                    Serial.println("Serving /index.html");
                    request->send(SPIFFS, "/index.html", "text/html"); 
                });
    server.on("/pwmon", HTTP_GET, std::bind(&HttpHandler::handlePwmOn, this, std::placeholders::_1));
    server.on("/pwmset", HTTP_GET, std::bind(&HttpHandler::handlePwmSet, this, std::placeholders::_1));
    server.on("/pwmoff", HTTP_GET, std::bind(&HttpHandler::handlePwmOff, this, std::placeholders::_1));
    server.on("/calibration", HTTP_GET, std::bind(&HttpHandler::handleCalibration, this, std::placeholders::_1));
    // curl "http://esppower.local/pwmget"
    server.on("/pwmget", HTTP_GET, std::bind(&HttpHandler::handlePwmGet, this, std::placeholders::_1));
    server.onNotFound([](AsyncWebServerRequest *request){
        request->send(404, "text/plain", "Not found");
    });
    ws.onEvent([this](AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len){
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
    });
    server.addHandler(&ws);
    server.begin();
}

void HttpHandler::handlePwmOff(AsyncWebServerRequest *request)
{
    if (request->hasParam("channel"))
    {
        const AsyncWebParameter *p = request->getParam("channel");
        PwmConfig espPwmConfig;
        espPwmConfig.channel = atoi(p->value().c_str());
        vmPwm->onPwmEnd();
    }
    request->send(200, "text/plain", "OK");
}

void HttpHandler::handlePwmGet(AsyncWebServerRequest *request)
{
    const PwmConfig &config = vmPwm->getPwm();
    String response;
    response += "{\"isActive\":";
    response += config.isActive;
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

void HttpHandler::handlePwmSet(AsyncWebServerRequest *request)
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
        vmPwm->onPwmUpdate(duty);
        request->send(200, "text/plain", "OK");
    }
    else
    {
        request->send(400, "text/plain", "Missed argument: channel,duty");
    }
}

void HttpHandler::handleCalibration(AsyncWebServerRequest *request)
{
    // viewModel->vmOnCalibration();
    request->send(200, "text/plain", "OK");
}

void HttpHandler::handlePwmOn(AsyncWebServerRequest *request)
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

        vmPwm->onPwmStart(espPwmConfig);
        request->send(200, "text/plain", "OK");
    }
    else
    {
        request->send(400, "text/plain", "Missed argument: channel,freq,res,pin");
    }
}

void HttpHandler::onWebSocketMessage(void *arg, uint8_t *data, size_t len)
{
    AwsFrameInfo *info = (AwsFrameInfo *)arg;
    if (info->final && info->opcode == WS_TEXT)
    {
        data[len] = 0; // Null-terminate message
        Serial.printf("Received: %s\n", (char *)data);
        // ws.textAll("Echo: " + String((char*)data));  // Echo message back to all clients
    }
}
