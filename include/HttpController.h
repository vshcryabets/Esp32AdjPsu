#pragma once
#include <ESPAsyncWebServer.h>
#include "vm.h"

class HttpController {
private:
    AsyncWebServer server; //(80);
    AsyncWebSocket ws; //("/ws");
    PwmControler *pwm;
public:
    HttpController(PwmControler *pwm);
    void begin();
private:
    void handlePwmOn(AsyncWebServerRequest *request);
    void handlePwmSet(AsyncWebServerRequest *request);
    void handlePwmOff(AsyncWebServerRequest *request);
    void handlePwmGet(AsyncWebServerRequest *request);
    void handleCalibration(AsyncWebServerRequest *request);
    void onWebSocketEvent(AsyncWebSocket *server, AsyncWebSocketClient *client,
                          AwsEventType type, void *arg, uint8_t *data, size_t len);
    void onWebSocketMessage(void *arg, uint8_t *data, size_t len);
};