#pragma once
#include <ESPAsyncWebServer.h>
#include "vm.h"

class HttpController: public VmStateReceiver {
private:
    AsyncWebServer server; //(80);
    AsyncWebSocket ws; //("/ws");
    PwmControler *pwm;
    VmStateProvider *vmStateProvider;
public:
    HttpController(PwmControler *pwm, VmStateProvider *vmStateProvider);
    /**
     * @brief Start the HTTP server and subscribe to VM state updates
     */
    bool begin();
    /**
     * @brief Stop the HTTP server and unsubscribe from VM state updates
     */
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
    void onStateChanged(const State& state) override;
};