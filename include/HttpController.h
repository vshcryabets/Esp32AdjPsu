#pragma once
#include <ESPAsyncWebServer.h>
#include "vm.h"

class HttpController: public VmStateReceiver {
private:
    mutable AsyncWebServer server; //(80);
    mutable AsyncWebSocket ws; //("/ws");
    PwmControler *pwm;
    VmStateProvider *vmStateProvider;
public:
    HttpController(PwmControler *pwm, VmStateProvider *vmStateProvider);
    void begin();
    void end();
private:
    void handlePwmOn(AsyncWebServerRequest *request);
    void handlePwmSet(AsyncWebServerRequest *request);
    void handlePwmOff(AsyncWebServerRequest *request);
    void handlePwmGet(AsyncWebServerRequest *request);
    void handleCalibration(AsyncWebServerRequest *request);
    void handleReadPartitions(AsyncWebServerRequest *request);
    void handleGetState(AsyncWebServerRequest *request);
    void onWebSocketEvent(AsyncWebSocket *server, AsyncWebSocketClient *client,
                          AwsEventType type, void *arg, uint8_t *data, size_t len);
    void onWebSocketMessage(void *arg, uint8_t *data, size_t len);
    void onStateChanged(const State& state) const override;
};