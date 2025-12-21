#include "HttpController.h"
#include <SPIFFS.h>
#include "pwm.h"

HttpController::HttpController(PwmControler *pwm, VmStateProvider *vmStateProvider) : 
    server(80), ws("/ws"), pwm(pwm), vmStateProvider(vmStateProvider)
{
}

void HttpController::end()
{
    vmStateProvider->unsubscribe(this);
    server.end();
}

bool HttpController::begin()
{
    if (!vmStateProvider->subscribe(this)) {
        Serial.println("Failed to subscribe to VM state updates");
        return false;
    }
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) { 
                    request->send(SPIFFS, "/index.html", "text/html"); 
            }
        );
    server.serveStatic("/html/", SPIFFS, "/");
    // curl "http://esppower.local/pwmon?channel=0&freq=4000&res=8&pin=0&duty=128"
    server.on("/pwmon", HTTP_GET, std::bind(&HttpController::handlePwmOn, this, std::placeholders::_1));
    server.on("/pwmset", HTTP_GET, std::bind(&HttpController::handlePwmSet, this, std::placeholders::_1));
    server.on("/pwmoff", HTTP_GET, std::bind(&HttpController::handlePwmOff, this, std::placeholders::_1));
    server.on("/calibration", HTTP_GET, std::bind(&HttpController::handleCalibration, this, std::placeholders::_1));
    // curl "http://esppower.local/pwmget"
    server.on("/pwmget", HTTP_GET, std::bind(&HttpController::handlePwmGet, this, std::placeholders::_1));
    server.on("/api/read_partitions", HTTP_GET, std::bind(&HttpController::handleReadPartitions, this, std::placeholders::_1));
    server.on("/api/state", HTTP_GET, std::bind(&HttpController::handleGetState, this, std::placeholders::_1));
    server.onNotFound([](AsyncWebServerRequest *request) { request->send(404, "text/plain", "Not found 1"); });
    ws.onEvent([this](AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len)
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
        } });
    server.addHandler(&ws);
    server.begin();
    return true;
}

void HttpController::handlePwmOff(AsyncWebServerRequest *request)
{
    pwm->onPwmEnd();
    request->send(200, "text/plain", "OK");
}

void HttpController::handlePwmGet(AsyncWebServerRequest *request)
{
    const PwmConfig &config = pwm->getPwm();
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

void HttpController::handlePwmSet(AsyncWebServerRequest *request)
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
        pwm->onPwmUpdate(duty);
        request->send(200, "text/plain", "OK");
    }
    else
    {
        request->send(400, "text/plain", "Missed argument: channel,duty");
    }
}

void HttpController::handleCalibration(AsyncWebServerRequest *request)
{
    // viewModel->vmOnCalibration();
    request->send(200, "text/plain", "OK");
}

void HttpController::handlePwmOn(AsyncWebServerRequest *request)
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
        Serial.print("Enable PWM on channel ");
        Serial.print(channel);
        Serial.print(" pin=");
        Serial.print(pin);
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

        pwm->onPwmStart(espPwmConfig);
        request->send(200, "text/plain", "OK");
    }
    else
    {
        request->send(400, "text/plain", "Missed argument: channel,freq,res,pin");
    }
}

void HttpController::onWebSocketMessage(void *arg, uint8_t *data, size_t len)
{
    AwsFrameInfo *info = (AwsFrameInfo *)arg;
    if (info->final && info->opcode == WS_TEXT)
    {
        data[len] = 0; // Null-terminate message
        Serial.printf("Received: %s\n", (char *)data);
        // ws.textAll("Echo: " + String((char*)data));  // Echo message back to all clients
    }
}

void HttpController::handleReadPartitions(AsyncWebServerRequest *request)
{
    String response;
    const esp_partition_t *partition = nullptr;
    esp_partition_iterator_t it = esp_partition_find(
        ESP_PARTITION_TYPE_ANY, 
        ESP_PARTITION_SUBTYPE_ANY, 
        NULL);
    while (it != NULL) {
        partition = esp_partition_get(it);
        response += "Partition: ";
        response += partition->label;
        response += ", Address: 0x";
        response += String(partition->address, HEX);
        response += ", Size: ";
        response += String(partition->size);
        response += " bytes\n";
        it = esp_partition_next(it);
    }
    esp_partition_iterator_release(it);
    request->send(200, "text/plain", response);
}

void HttpController::handleGetState(AsyncWebServerRequest *request) {
    const auto& state = vmStateProvider->getState();
    String response = "{\"dmmConnected\":\"" + String(state.dmmConnected ? "true" : "false") + "\"}";
    request->send(200, "application/json", response);
}

void HttpController::onStateChanged(const State& state) {
    String wsData = "{\"dmmConnected\":" + String(state.dmmConnected ? "true" : "false");
    if (state.dmmResult.isValid()) {
        // Serial.printf("DMM Reading: V=%.6f, I=%.6f\n", state.dmmResult.voltage, state.dmmResult.current);
        wsData += ",\"dmmValid\":true,\"voltage\":" + 
            String(state.dmmResult.voltage, 3) + 
            ",\"current\":" + String(state.dmmResult.current, 3);
    } else {
        wsData += ",\"dmmValid\":false";
    }
    wsData += "}";
    ws.textAll(wsData);
}